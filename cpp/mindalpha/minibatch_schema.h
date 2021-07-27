#pragma once

#include <sstream>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>
namespace mindalpha {
typedef int ColumnType;
struct Column {
  size_t idx;
  ColumnType type;
};
class MinibatchSchema {
  public:
  MinibatchSchema();
  void LoadColumnNameFromStream(std::istream &stream);
  void LoadColumnNameFromSource(const std::string &source);
  void LoadColumnNameFromFile(const std::string &uri);
  void Clear();
  Column GetColumn(const std::string &column_name) const;
  std::string GetSchemaString() const;
  std::string ToString() const;
  private:
  std::unordered_map<std::string, int> column_name_map_;
  std::vector<std::string> column_names_;
  std::string column_name_source_;
};
} // namespace mindalpha