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

#include <mindalpha/io.h>
#include <mindalpha/string_utils.h>
#include <mindalpha/combine_schema.h>

namespace mindalpha
{

void CombineSchema::Clear()
{
    column_name_map_.clear();
    combine_columns_.clear();
    combine_columns_aliases_.clear();
    combine_columns_aliases_hashes_.clear();
    column_names_.clear();
    column_name_source_.clear();
    combine_schema_source_.clear();
}

void CombineSchema::LoadColumnNameFromStream(std::istream& stream)
{
    using namespace std::string_view_literals;
    std::string line;
    std::string source;
    int i = 0;
    while (std::getline(stream, line))
    {
        source.append(line);
        source.push_back('\n');
        if (line[0] == '#')
            continue;
        const auto svpair = SplitStringView(line, " "sv);
        int index = -1;
        std::string_view name;
        if (svpair.size() != 2)
        {
            index = i;
            name = svpair[0];
        }
        else
        {
            index = std::stoi(std::string(svpair[0]));
            name = svpair[1];
        }
        const auto name_alias_pair = SplitStringView(name, "@"sv);
        std::string_view col_name;
        if (name_alias_pair.size() == 2)
            col_name = name_alias_pair[1];
        else
            col_name = name_alias_pair[0];
        column_name_map_[std::string(col_name)] = index;
        column_names_.emplace_back(col_name);
        i++;
    }
    column_name_source_ = std::move(source);
}

void CombineSchema::LoadColumnNameFromSource(const std::string& source)
{
    std::istringstream stream(source);
    LoadColumnNameFromStream(stream);
}

void CombineSchema::LoadColumnNameFromFile(const std::string& uri)
{
    std::string source = StreamReadAll(uri);
    LoadColumnNameFromSource(source);
}

void CombineSchema::LoadCombineSchemaFromStream(std::istream& stream)
{
    using namespace std::string_view_literals;
    std::string line;
    std::string source;
    while (std::getline(stream, line))
    {
        source.append(line);
        source.push_back('\n');
        if (line[0] == '#')
            continue;
        const auto svs = SplitStringView(std::string_view(line), "#"sv);
        std::vector<std::string> combines;
        std::vector<std::string> aliases;
        std::vector<uint64_t> hashes;
        combines.reserve(svs.size());
        aliases.reserve(svs.size());
        hashes.reserve(svs.size());
        for (const auto sv : svs)
        {
            const auto name_alias_pair = SplitStringView(sv, "@"sv);
            const auto col = name_alias_pair[0];
            combines.emplace_back(col);
            uint64_t h = 0;
            if (name_alias_pair.size() > 1)
            {
                aliases.emplace_back(name_alias_pair[1]);
                h = BKDRHashWithEqualPostfix(name_alias_pair[1]);
            }
            else
            {
                aliases.emplace_back(col);
                h = BKDRHashWithEqualPostfix(col);
            }
            hashes.push_back(h);
        }
        combine_columns_.emplace_back(std::move(combines));
        combine_columns_aliases_.push_back(std::move(aliases));
        combine_columns_aliases_hashes_.push_back(std::move(hashes));
    }
    combine_schema_source_ = std::move(source);
}

void CombineSchema::LoadCombineSchemaFromSource(const std::string& source)
{
    std::istringstream stream(source);
    LoadCombineSchemaFromStream(stream);
}

void CombineSchema::LoadCombineSchemaFromFile(const std::string& uri)
{
    std::string source = StreamReadAll(uri);
    LoadCombineSchemaFromSource(source);
}

std::tuple<std::vector<uint64_t>, std::vector<uint64_t>>
CombineSchema::CombineToIndicesAndOffsets(const IndexBatch& batch, bool feature_offset) const
{
    const size_t minibatch_size = batch.GetRows();
    const size_t feature_count = GetFeatureCount();
    const size_t offsets_per_row = feature_offset ? feature_count : 1;
    std::vector<uint64_t> indices;
    std::vector<uint64_t> offsets;
    indices.reserve(minibatch_size * feature_count * 20);
    offsets.reserve(minibatch_size * offsets_per_row);
    for (size_t i = 0; i < minibatch_size; i++)
    {
        if (!feature_offset)
            offsets.push_back(indices.size());
        for (size_t j = 0; j < feature_count; j++)
        {
            if (feature_offset)
                offsets.push_back(indices.size());
            const std::vector<std::string>& combine = combine_columns_.at(j);
            const std::vector<std::string>& alias = combine_columns_aliases_.at(j);
            const std::vector<uint64_t>& name_hashes = combine_columns_aliases_hashes_.at(j);
            std::vector<const StringViewHashVector*> splits;
            splits.reserve(combine.size());
            bool has_none = false;
            size_t total_result = 1;
            for (const std::string& column_name : combine)
            {
                const StringViewHashVector* const cell = GetCell(batch, i, column_name);
                if (cell == nullptr)
                {
                    has_none = true;
                    break;
                }
                total_result *= cell->size();
                splits.push_back(cell);
            }
            if (!has_none)
            {
                indices.reserve(indices.size() + total_result);
                CombineOneFeature(splits, alias, name_hashes, indices, total_result);
            }
        }
    }
    return std::make_tuple(std::move(indices), std::move(offsets));
}

const StringViewHashVector*
CombineSchema::GetCell(const IndexBatch& batch, size_t i, const std::string& column_name) const
{
    const size_t padding = -1;
    const StringViewHashVector& vec = batch.GetCell(i, padding, column_name);
    return vec.empty() ? nullptr : &vec;
}

void CombineSchema::CombineOneFeature(const std::vector<const StringViewHashVector*>& splits,
                                      const std::vector<std::string>& names,
                                      const std::vector<uint64_t>& name_hashes,
                                      std::vector<uint64_t>& combine_hashes,
                                      size_t total_results)
{
    if (splits.size() != names.size())
    {
        std::ostringstream sout;
        sout << "number of splits and names mismatch; ";
        sout << splits.size() << " != " << names.size() << ". ";
        sout << "names = [";
        for (size_t i = 0; i < names.size(); i++)
            sout << (i ? ", " : "") << names.at(i);
        sout << "]";
        throw std::runtime_error(sout.str());
    }
    if (total_results == 1)
    {
        uint64_t h = CombineOneField(name_hashes.at(0), splits.at(0)->at(0).hash_);
        for (size_t i = 1; i < splits.size(); i++)
            h = ConcatOneField(h, name_hashes.at(i), splits.at(i)->at(0).hash_);
        combine_hashes.push_back(h);
    }
    else if (splits.size() == 1)
    {
        const StringViewHashVector& split = *splits.at(0);
        for (const StringViewHash& item : split)
        {
            const uint64_t h = CombineOneField(name_hashes.at(0), item.hash_);
            combine_hashes.push_back(h);
        }
    }
    else
    {
        static thread_local std::vector<size_t> prd_fwd(64);
        static thread_local std::vector<size_t> prd_bwd(64);
        prd_fwd.clear();
        prd_bwd.clear();
        prd_fwd.resize(splits.size());
        prd_bwd.resize(splits.size());
        prd_fwd.at(0) = 1;
        for (size_t i = 1; i < splits.size(); i++)
            prd_fwd.at(i) = prd_fwd.at(i - 1) * splits.at(i - 1)->size();
        prd_bwd.at(splits.size() - 1) = 1;
        for (size_t i = splits.size() - 1; i > 0; i--)
            prd_bwd.at(i - 1) = prd_bwd.at(i) * splits.at(i)->size();

        const size_t begin = combine_hashes.size();
        combine_hashes.resize(begin + total_results);
        uint64_t* const result = &combine_hashes.at(begin);

        const StringViewHashVector& split = *splits.at(0);
        const size_t loops = prd_fwd.at(0);
        const size_t each_repeat = prd_bwd.at(0);
        for(size_t l = 0; l < loops; l++)
        {
            size_t base = l * split.size() * each_repeat;
            for (const StringViewHash& item : split)
            {
                const uint64_t h = CombineOneField(name_hashes.at(0), item.hash_);
                for (size_t r = 0; r < each_repeat; r++)
                    result[base + r] = h;
                base += each_repeat;
            }
        }

        for (size_t i = 1; i < splits.size(); i++)
        {
            const StringViewHashVector& split = *splits.at(i);
            const size_t loops = prd_fwd.at(i);
            const size_t each_repeat = prd_bwd.at(i);
            for (size_t l = 0; l < loops; l++)
            {
                size_t base = l * split.size() * each_repeat;
                for (const StringViewHash& item : split)
                {
                    for (size_t r = 0; r < each_repeat; r++)
                    {
                        uint64_t& h = result[base + r];
                        h = ConcatOneField(h, name_hashes.at(i), item.hash_);
                    }
                    base += each_repeat;
                }
            }
        }
    }
}

uint64_t CombineSchema::ComputeFeatureHash(const std::vector<std::pair<std::string, std::string>>& feature)
{
    if (feature.empty())
        throw std::runtime_error("feature can not be empty");
    uint64_t h = 0;
    for (size_t i = 0; i < feature.size(); i++)
    {
        const std::pair<std::string, std::string>& p = feature.at(i);
        if (p.second == "none")
            throw std::runtime_error("none as value is invalid, because it should have been filtered");
        const uint64_t name = BKDRHashWithEqualPostfix(p.first);
        const uint64_t value = BKDRHash(p.second);
        if (i == 0)
            h = CombineOneField(name, value);
        else
            h = ConcatOneField(h, name, value);
    }
    return h;
}

}
