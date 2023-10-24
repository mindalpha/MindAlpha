//
// Copyright 2023 Mobvista
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

#include <mindalpha/array_hash_map.h>
#include <mindalpha/array_hash_map_reader.h>
#include <mindalpha/array_hash_map_writer.h>
#include <mindalpha/io.h>
#include <mindalpha/sparse_tensor_meta.h>
#include <mindalpha/stack_trace_utils.h>
#include <mindalpha/string_utils.h>
#include <sstream>

namespace mindalpha {

class ArrayHashMapHelper {
  public:
  ArrayHashMapHelper() : stream_(nullptr) {
  }
  int Load(const std::string &path, const UserOption &option);
  int Each();
  template <typename Func>
  int Each(Func &&fn);
  ~ArrayHashMapHelper();
  const std::string ToString() const {
    return fmt::format("DataSize={} {}", data_.size(), meta_.ToString());
  }
  inline size_t Size() {
      return data_.size();
  }
  inline auto& GetData() {
      return data_;
  }
  private:
  std::unique_ptr<ArrayHashMapReader> reader_;
  ArrayHashMap<uint64_t, uint8_t> data_;
  SparseTensorMeta meta_;
  Stream *stream_;
  bool data_only_;
  bool transform_key_;
  std::string feature_name_;
  std::string path_;
};
template <typename Func>
int ArrayHashMapHelper::Each(Func &&fn) {
  data_.Each([&fn](int64_t i, uint64_t key, const uint8_t *values, uint64_t count) {
    fn(i, key, values, count);
  });
  return 0;
}
} // namespace mindalpha
