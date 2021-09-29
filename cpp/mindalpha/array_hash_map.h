//
// Copyright 2021 Mobvista
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <memory>
#include <utility>
#include <spdlog/spdlog.h>
#include <mindalpha/hashtable_helpers.h>
#include <mindalpha/stack_trace_utils.h>
#include <mindalpha/memory_buffer.h>
#include <mindalpha/map_file_header.h>

//
// ``array_hash_map.h`` defines class ``ArrayHashMap`` which avoids pointers when
// implementing hashtable and is more memory efficient and serialization friendly
// than ``std::unordered_map``. The hash algorithm is also improved by avoiding
// modulo of general primes.
//

namespace mindalpha
{

template<typename TKey, typename TValue>
class ArrayHashMap
{
public:
    ArrayHashMap()
    {
        static_assert(sizeof(TKey) <= sizeof(uint64_t), "invalid key type");
    }

    explicit ArrayHashMap(int64_t value_count_per_key)
        : ArrayHashMap()
    {
        if (value_count_per_key < 0)
        {
            std::string serr;
            serr.append("value_count_per_key must be non-negative.\n\n");
            serr.append(GetStackTrace());
            spdlog::error(serr);
            throw std::runtime_error(serr);
        }
        value_count_per_key_ = static_cast<uint64_t>(value_count_per_key);
    }

    ArrayHashMap(ArrayHashMap&& rhs)
        : keys_buffer_(std::move(rhs.keys_buffer_))
        , values_buffer_(std::move(rhs.values_buffer_))
        , next_buffer_(std::move(rhs.next_buffer_))
        , first_buffer_(std::move(rhs.first_buffer_))
        , key_count_(rhs.key_count_)
        , bucket_count_(rhs.bucket_count_)
        , value_count_(rhs.value_count_)
        , value_count_per_key_(rhs.value_count_per_key_)
        , keys_(rhs.keys_)
        , values_(rhs.values_)
        , next_(rhs.next_)
        , first_(rhs.first_)
    {
        rhs.key_count_ = 0;
        rhs.bucket_count_ = 0;
        rhs.value_count_ = 0;
        rhs.value_count_per_key_ = static_cast<uint64_t>(-1);
        rhs.keys_ = nullptr;
        rhs.values_ = nullptr;
        rhs.next_ = nullptr;
        rhs.first_ = nullptr;
    }

    ~ArrayHashMap()
    {
        key_count_ = 0;
        bucket_count_ = 0;
        value_count_ = 0;
        value_count_per_key_ = static_cast<uint64_t>(-1);
        keys_ = nullptr;
        values_ = nullptr;
        next_ = nullptr;
        first_ = nullptr;
    }

    void Swap(ArrayHashMap& other)
    {
        keys_buffer_.Swap(other.keys_buffer_);
        values_buffer_.Swap(other.values_buffer_);
        next_buffer_.Swap(other.next_buffer_);
        first_buffer_.Swap(other.first_buffer_);
        std::swap(key_count_, other.key_count_);
        std::swap(bucket_count_, other.bucket_count_);
        std::swap(value_count_, other.value_count_);
        std::swap(value_count_per_key_, other.value_count_per_key_);
        std::swap(keys_, other.keys_);
        std::swap(values_, other.values_);
        std::swap(next_, other.next_);
        std::swap(first_, other.first_);
    }

    const TKey* GetKeysArray() const { return keys_; }
    const TValue* GetValuesArray() const { return values_; }

    void Reserve(uint64_t size)
    {
        if (value_count_per_key_ == static_cast<uint64_t>(-1))
        {
            std::string serr;
            serr.append("value_count_per_key is not set.\n\n");
            serr.append(GetStackTrace());
            spdlog::error(serr);
            throw std::runtime_error(serr);
        }
        if (bucket_count_ >= size)
            return;
        Reallocate(size);
    }

    void Reallocate(uint64_t size)
    {
        if (value_count_per_key_ == static_cast<uint64_t>(-1))
        {
            std::string serr;
            serr.append("value_count_per_key is not set.\n\n");
            serr.append(GetStackTrace());
            spdlog::error(serr);
            throw std::runtime_error(serr);
        }
        if (key_count_ > size)
            return;
        if (size == 0)
        {
            Deallocate();
            return;
        }
        const uint64_t bucket_count = HashtableHelpers::GetPowerBucketCount(size);
        const uint64_t limit = std::numeric_limits<uint32_t>::max();
        if (bucket_count > limit)
        {
            std::string serr;
            serr.append("store " + std::to_string(size) + " keys ");
            serr.append("requires " + std::to_string(bucket_count) + " buckets, ");
            serr.append("but at most " + std::to_string(limit) + " are allowed.\n\n");
            serr.append(GetStackTrace());
            spdlog::error(serr);
            throw std::runtime_error(serr);
        }
        keys_buffer_.Reallocate(bucket_count * sizeof(TKey));
        values_buffer_.Reallocate(bucket_count * value_count_per_key_ * sizeof(TValue));
        next_buffer_.Reallocate(bucket_count * sizeof(uint32_t));
        first_buffer_.Reallocate(bucket_count * sizeof(uint32_t));
        bucket_count_ = bucket_count;
        keys_ = static_cast<TKey*>(keys_buffer_.GetPointer());
        values_ = static_cast<TValue*>(values_buffer_.GetPointer());
        next_ = static_cast<uint32_t*>(next_buffer_.GetPointer());
        first_ = static_cast<uint32_t*>(first_buffer_.GetPointer());
        BuildHashIndex();
    }

    int64_t Find(TKey key) const
    {
        if (bucket_count_ == 0)
            return -1;
        const uint32_t nil = uint32_t(-1);
        const uint64_t bucket = GetBucket(key);
        uint32_t i = first_[bucket];
        while (i != nil)
        {
            if (keys_[i] == key)
                return static_cast<int64_t>(i);
            i = next_[i];
        }
        return -1;
    }

    int64_t FindOrInit(TKey key)
    {
        bool is_new;
        int64_t index;
        GetOrInit(key, is_new, index);
        return index;
    }

    const TValue* Get(TKey key) const
    {
        if (bucket_count_ == 0)
            return nullptr;
        if (value_count_per_key_ == static_cast<uint64_t>(-1))
        {
            std::string serr;
            serr.append("value_count_per_key is not set.\n\n");
            serr.append(GetStackTrace());
            spdlog::error(serr);
            throw std::runtime_error(serr);
        }
        const uint32_t nil = uint32_t(-1);
        const uint64_t bucket = get_bucket(key);
        uint32_t i = first_[bucket];
        while (i != nil)
        {
            if (keys_[i] == key)
                return &values_[i * value_count_per_key_];
            i = next_[i];
        }
        return nullptr;
    }

    TValue* GetOrInit(TKey key, bool& is_new, int64_t& index)
    {
        if (value_count_per_key_ == static_cast<uint64_t>(-1))
        {
            std::string serr;
            serr.append("value_count_per_key is not set.\n\n");
            serr.append(GetStackTrace());
            spdlog::error(serr);
            throw std::runtime_error(serr);
        }
        const uint32_t nil = uint32_t(-1);
        if (bucket_count_ > 0)
        {
            const uint64_t b = GetBucket(key);
            uint32_t i = first_[b];
            while (i != nil)
            {
                if (keys_[i] == key)
                {
                    is_new = false;
                    index = i;
                    return &values_[i * value_count_per_key_];
                }
                i = next_[i];
            }
        }
        if (key_count_ == bucket_count_)
            EnsureCapacity();
        const uint64_t bucket = GetBucket(key);
        index = static_cast<int64_t>(key_count_);
        keys_[index] = key;
        next_[index] = first_[bucket];
        first_[bucket] = static_cast<uint32_t>(index);
        is_new = true;
        key_count_++;
        value_count_ += value_count_per_key_;
        return &values_[index * value_count_per_key_];
    }

    TValue* GetOrInit(TKey key, bool& is_new)
    {
        int64_t index;
        return GetOrInit(key, is_new, index);
    }

    TValue* GetOrInit(TKey key)
    {
        bool is_new;
        return GetOrInit(key, is_new);
    }

    void Clear()
    {
        key_count_ = 0;
        value_count_ = 0;
        BuildHashIndex();
    }

    void Deallocate()
    {
        keys_buffer_.Deallocate();
        values_buffer_.Deallocate();
        next_buffer_.Deallocate();
        first_buffer_.Deallocate();
        key_count_ = 0;
        bucket_count_ = 0;
        value_count_ = 0;
        keys_ = nullptr;
        values_ = nullptr;
        next_ = nullptr;
        first_ = nullptr;
    }

    template<typename Func>
    void Prune(Func pred)
    {
        if (value_count_per_key_ == static_cast<uint64_t>(-1))
        {
            std::string serr;
            serr.append("value_count_per_key is not set.\n\n");
            serr.append(GetStackTrace());
            spdlog::error(serr);
            throw std::runtime_error(serr);
        }
        uint64_t v = 0;
        for (uint64_t i = 0; i < key_count_; i++)
        {
            const TKey key = keys_[i];
            const TValue* values = &values_[i * value_count_per_key_];
            if (!pred(i, key, values, value_count_per_key_))
            {
                if (v != i)
                {
                    keys_[v] = key;
                    memcpy(&values_[v * value_count_per_key_], values, value_count_per_key_ * sizeof(TValue));
                }
                v++;
            }
        }
        if (v < key_count_)
        {
            key_count_ = v;
            value_count_ = v * value_count_per_key_;
            Reallocate(key_count_);
        }
    }

    void Dump(std::ostream& out = std::cerr, uint64_t count_limit = uint64_t(-1)) const
    {
        if (value_count_per_key_ == static_cast<uint64_t>(-1))
        {
            std::string serr;
            serr.append("value_count_per_key is not set.\n\n");
            serr.append(GetStackTrace());
            spdlog::error(serr);
            throw std::runtime_error(serr);
        }
        for (uint64_t i = 0; i < key_count_; i++)
        {
            if (i >= count_limit)
                break;
            TKey key = keys_[i];;
            const TValue* values = &values_[i * value_count_per_key_];
            out << key << ": [";
            for (uint64_t j = 0; j < value_count_per_key_; j++)
                out << (j ? ", " : "") << as_number(values[j]);
            out << "]\n";
        }
    }

    uint64_t GetHashCode() const
    {
        uint64_t hash = 0;
        for (uint64_t key: *this)
        {
            uint64_t c = key;
            const TValue* const values = Get(key);
            const uint8_t* const bytes = reinterpret_cast<const uint8_t*>(values);
            const uint64_t num_bytes = value_count_per_key_ * sizeof(TValue);
            for (uint64_t i = 0; i < num_bytes; i++)
                c = c * 31 + bytes[i];
            hash ^= c;
        }
        return hash;
    }

    class iterator
    {
    public:
        iterator(const ArrayHashMap<TKey, TValue>* map, uint64_t index)
            : map_(map), index_(index) { }

        iterator& operator++()
        {
            if (index_ < map_->key_count_)
                index_++;
            return *this;
        }

        TKey operator*() const
        {
            if (index_ < map_->key_count_)
                return map_->keys_[index_];
            else
                return TKey(-1);
        }

        bool operator==(const iterator& rhs) const
        {
            return index_ == rhs.index_ && map_ == rhs.map_;
        }

        bool operator!=(const iterator& rhs) const
        {
            return !(*this == rhs);
        }

    private:
        const ArrayHashMap<TKey, TValue>* map_;
        uint64_t index_;
    };

    iterator begin() const
    {
        return iterator(this, 0);
    }

    iterator end() const
    {
        return iterator(this, key_count_);
    }

    uint64_t size() const
    {
        return key_count_;
    }

    template<typename Func>
    void Each(Func action)
    {
        if (value_count_per_key_ == static_cast<uint64_t>(-1))
        {
            std::string serr;
            serr.append("value_count_per_key is not set.\n\n");
            serr.append(GetStackTrace());
            spdlog::error(serr);
            throw std::runtime_error(serr);
        }
        for (uint64_t i = 0; i < key_count_; i++)
        {
            const TKey key = keys_[i];
            const TValue* values = &values_[i * value_count_per_key_];
            action(i, key, values, value_count_per_key_);
        }
    }

    template<typename Func>
    void Serialize(const std::string& path, Func write, uint64_t value_count_per_key = static_cast<uint64_t>(-1))
    {
        if (value_count_per_key_ == static_cast<uint64_t>(-1))
        {
            std::string serr;
            serr.append("value_count_per_key is not set.\n\n");
            serr.append(GetStackTrace());
            spdlog::error(serr);
            throw std::runtime_error(serr);
        }
        if (value_count_per_key == static_cast<uint64_t>(-1))
            value_count_per_key = value_count_per_key_;
        if (value_count_per_key > value_count_per_key_)
        {
            std::string serr;
            serr.append("value_count_per_key exceeds that in the map.\n\n");
            serr.append(GetStackTrace());
            spdlog::error(serr);
            throw std::runtime_error(serr);
        }
        std::string hint;
        hint.append("Fail to serialize ArrayHashMap to \"");
        hint.append(path);
        hint.append("\"; ");
        MapFileHeader header;
        header.FillBasicFields();
        header.key_type = static_cast<uint64_t>(DataTypeToCode<TKey>::value);
        header.value_type = static_cast<uint64_t>(DataTypeToCode<TValue>::value);
        header.key_count = key_count_;
        header.bucket_count = bucket_count_;
        header.value_count = value_count_per_key * key_count_;
        header.value_count_per_key = value_count_per_key;
        write(static_cast<const void*>(&header), sizeof(header));
        write(static_cast<const void*>(keys_), key_count_ * sizeof(TKey));
        if (value_count_per_key == value_count_per_key_)
            write(static_cast<const void*>(values_), value_count_ * sizeof(TValue));
        else
        {
            for (uint64_t i = 0; i < key_count_; i++)
            {
                const TValue* values = &values_[i * value_count_per_key_];
                write(static_cast<const void*>(values), value_count_per_key * sizeof(TValue));
            }
        }
        write(static_cast<const void*>(next_), key_count_ * sizeof(uint32_t));
        write(static_cast<const void*>(first_), bucket_count_ * sizeof(uint32_t));
    }

    template<typename Func>
    void Deserialize(const std::string& path, Func read)
    {
        std::string hint;
        hint.append("Fail to deserialize ArrayHashMap from \"");
        hint.append(path);
        hint.append("\"; ");
        MapFileHeader header;
        read(static_cast<void*>(&header), sizeof(header), hint, "map file header");
        DeserializeWithHeader(path, read, header);
    }

    template<typename Func>
    void DeserializeWithHeader(const std::string& path, Func read, const MapFileHeader& header)
    {
        std::string hint;
        hint.append("Fail to deserialize ArrayHashMap from \"");
        hint.append(path);
        hint.append("\"; ");
        header.Validate(hint);
        uint64_t value_count = header.value_count;
        uint64_t value_count_per_key = header.value_count_per_key;
        if (header.key_type != static_cast<uint64_t>(DataTypeToCode<TKey>::value))
        {
            const DataType key_type_1 = static_cast<DataType>(header.key_type);
            const DataType key_type_2 = DataTypeToCode<TKey>::value;
            const size_t key_size_1 = DataTypeToSize(key_type_1);
            const size_t key_size_2 = DataTypeToSize(key_type_2);
            if (key_size_1 != key_size_2)
            {
                std::string serr;
                serr.append("key types mismatch; ");
                serr.append("expect '" + DataTypeToString(DataTypeToCode<TKey>::value) + "', ");
                serr.append("found '" + DataTypeToString(static_cast<DataType>(header.key_type)) + "'.\n\n");
                serr.append(GetStackTrace());
                spdlog::error(serr);
                throw std::runtime_error(serr);
            }
        }
        if (header.value_type != static_cast<uint64_t>(DataTypeToCode<TValue>::value))
        {
            const DataType value_type_1 = static_cast<DataType>(header.value_type);
            const DataType value_type_2 = DataTypeToCode<TValue>::value;
            const size_t value_size_1 = DataTypeToSize(value_type_1);
            const size_t value_size_2 = DataTypeToSize(value_type_2);
            if (value_size_1 != value_size_2)
            {
                if (value_count_per_key * value_size_1 % value_size_2 == 0)
                {
                    value_count = value_count * value_size_1 / value_size_2;
                    value_count_per_key = value_count_per_key * value_size_1 / value_size_2;
                }
                else
                {
                    std::string serr;
                    serr.append("value types mismatch; ");
                    serr.append("expect '" + DataTypeToString(DataTypeToCode<TValue>::value) + "', ");
                    serr.append("found '" + DataTypeToString(static_cast<DataType>(header.value_type)) + "'. ");
                    serr.append("value_count_per_key = " + std::to_string(value_count_per_key) + "\n\n");
                    serr.append(GetStackTrace());
                    spdlog::error(serr);
                    throw std::runtime_error(serr);
                }
            }
        }
        value_count_per_key_ = value_count_per_key;
        Clear();
        Reserve(header.bucket_count);
        read(static_cast<void*>(keys_), header.key_count * sizeof(TKey), hint, "keys array");
        read(static_cast<void*>(values_), value_count * sizeof(TValue), hint, "values array");
        read(static_cast<void*>(next_), header.key_count * sizeof(uint32_t), hint, "next array");
        read(static_cast<void*>(first_), header.bucket_count * sizeof(uint32_t), hint, "first array");
        key_count_ = header.key_count;
        bucket_count_ = header.bucket_count;
        value_count_ = value_count;
    }

    void SerializeTo(const std::string& path, uint64_t value_count_per_key = static_cast<uint64_t>(-1))
    {
        FILE* fout = fopen(path.c_str(), "wb");
        if (fout == NULL)
        {
            std::string serr;
            serr.append("can not open file \"" + path + "\" for map serializing; ");
            serr.append(strerror(errno));
            serr.append("\n\n");
            serr.append(GetStackTrace());
            spdlog::error(serr);
            throw std::runtime_error(serr);
        }
        std::unique_ptr<FILE, decltype(&fclose)> fout_guard(fout, &fclose);
        Serialize(path, [fout](const void* ptr, size_t size) { fwrite(ptr, 1, size, fout); }, value_count_per_key);
    }

    void DeserializeFrom(const std::string& path)
    {
        std::string hint;
        hint.append("Fail to deserialize ArrayHashMap from \"");
        hint.append(path);
        hint.append("\"; ");
        FILE* fin = fopen(path.c_str(), "rb");
        if (fin == NULL)
        {
            std::string serr;
            serr.append(hint);
            serr.append("can not open file. ");
            serr.append(strerror(errno));
            serr.append("\n\n");
            serr.append(GetStackTrace());
            spdlog::error(serr);
            throw std::runtime_error(serr);
        }
        uint64_t offset = 0;
        std::unique_ptr<FILE, decltype(&fclose)> fin_guard(fin, &fclose);
        Deserialize(path, [fin, &offset](void* ptr, size_t size, const std::string& hint, const std::string& what) {
            const size_t nread = fread(ptr, 1, size, fin);
            if (nread != size)
            {
                std::string serr;
                serr.append(hint);
                serr.append("incomplete " + what + ", ");
                serr.append(std::to_string(size) + " bytes expected, ");
                serr.append("but only " + std::to_string(nread) + " are read successfully. ");
                serr.append("offset = " + std::to_string(offset) + "\n\n");
                serr.append(GetStackTrace());
                spdlog::error(serr);
                throw std::runtime_error(serr);
            }
            offset += nread;
        });
    }

private:
    uint64_t GetBucket(TKey key) const
    {
        return HashtableHelpers::FastModulo(static_cast<uint64_t>(key)) & (bucket_count_ - 1);
    }

    void BuildHashIndex()
    {
        memset(first_, -1, bucket_count_ * sizeof(uint32_t));
        for (uint64_t i = 0; i < key_count_; i++)
        {
            const TKey key = keys_[i];
            const uint64_t bucket = GetBucket(key);
            next_[i] = first_[bucket];
            first_[bucket] = static_cast<uint32_t>(i);
        }
    }

    void EnsureCapacity()
    {
        uint64_t min_capacity = key_count_ * 2;
        if (min_capacity == 0)
            min_capacity = 1000;
        uint64_t size = HashtableHelpers::GetPowerBucketCount(min_capacity);
        uint64_t capacity = size;
        if (capacity < min_capacity)
            capacity = min_capacity;
        Reserve(capacity);
    }

    MemoryBuffer keys_buffer_;
    MemoryBuffer values_buffer_;
    MemoryBuffer next_buffer_;
    MemoryBuffer first_buffer_;
    uint64_t key_count_ = 0;
    uint64_t bucket_count_ = 0;
    uint64_t value_count_ = 0;
    uint64_t value_count_per_key_ = static_cast<uint64_t>(-1);
    TKey* keys_ = nullptr;
    TValue* values_ = nullptr;
    uint32_t* next_ = nullptr;
    uint32_t* first_ = nullptr;
};

}
