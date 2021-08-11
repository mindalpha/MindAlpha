#include <mindalpha/io.h>
#include <mindalpha/minibatch_schema.h>
#include <mindalpha/string_utils.h>
#include <fmt/format.h>
namespace mindalpha {
MinibatchSchema::MinibatchSchema() {
}
void MinibatchSchema::LoadColumnNameFromStream(std::istream &stream) {
  using namespace std::string_view_literals;
  std::string line;
  std::string source;
  int i = 0;
  while (std::getline(stream, line)) {
    source.append(line);
    source.push_back('\n');
    if (line[0] == '#')
      continue;
    const auto svpair = SplitStringView(line, " "sv);
    int index = -1;
    std::string_view name;
    if (svpair.size() != 2) {
      index = i;
      name = svpair[0];
    }
    else {
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

void MinibatchSchema::LoadColumnNameFromSource(const std::string &source) {
  std::istringstream stream(source);
  LoadColumnNameFromStream(stream);
}

void MinibatchSchema::LoadColumnNameFromFile(const std::string &uri) {
  std::string source = StreamReadAll(uri);
  LoadColumnNameFromSource(source);
}
Column MinibatchSchema::GetColumn(const std::string &column_name) const {
  const size_t column_index = column_name_map_.at(column_name);
  return Column{column_index, 0};
}
void MinibatchSchema::Clear() {
  column_name_source_.clear();
  column_names_.clear();
  column_name_map_.clear();
}
std::string MinibatchSchema::GetSchemaString() const {
    fmt::memory_buffer buff;
    buff.reserve(1024);
    // schema detail
    // project string ,article string ,requests integer ,bytes_served long
    for (auto& name : column_names_) {
         fmt::format_to(buff, "{} string,", name);
    }
    if (buff.size()) {
        buff.resize(buff.size() - 1);
    }
    return std::string(buff.data(), buff.size());
}
std::string MinibatchSchema::ToString() const {
  // TODO
  std::string json_str;
  return json_str;
}
} // namespace mindalpha
