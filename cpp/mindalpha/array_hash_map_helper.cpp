#include <mindalpha/array_hash_map_helper.h>
#include <mindalpha/logging.h>
#include <mindalpha/sparse_tensor_meta.h>
namespace mindalpha {

ArrayHashMapHelper::~ArrayHashMapHelper() {
  std::unique_ptr<Stream> stream_guard(stream_);
}
int ArrayHashMapHelper::Load(const std::string &path, const UserOption &option) {
  transform_key_ = option.json["transform_key"].bool_value();
  data_only_ = option.json["data_only"].bool_value();
  feature_name_ = option.json["feature_name"].string_value();
  auto sparse_meta_path = option.json["sparse_meta_path"].string_value();
  std::string str = StreamReadAll(sparse_meta_path);
  std::string err;
  json11::Json json = json11::Json::parse(str, err);
  meta_ = SparseTensorMeta::FromJson(json);
  LOG(INFO) << fmt::format("{} {}", str, data_.size());
  stream_ = Stream::Create(path.c_str(), "r", false);
  reader_ = std::make_unique<ArrayHashMapReader>(
      meta_, data_, stream_, data_only_, transform_key_, feature_name_, path);
  MapFileHeader header;
  if (reader_->DetectBinaryMode(header)) {
    uint64_t offset = sizeof(header);
    data_.DeserializeWithHeader(
        path,
        [this, &offset](void *ptr, size_t size, const std::string &hint, const std::string &what) {
          const size_t nread = stream_->Read(ptr, size);
          if (nread != size) {
            std::string serr = fmt::format("{} incomplete {},"
                                           "{} bytes expected, but only "
                                           "{} are read successfully. offset = {}\n\n"
                                           "{}",
                                           hint,
                                           what,
                                           size,
                                           nread,
                                           offset,
                                           GetStackTrace());
            spdlog::error(serr);
            throw std::runtime_error(serr);
          }
        },
        header);
  } else {
    reader_->Read();
  }
  LOG(INFO) << fmt::format("ReadFrom {} {} {}", path, str, data_.size());
  return 0;
}
int ArrayHashMapHelper::Each() {
  data_.Each([this](uint64_t i, uint64_t key, const uint8_t *values, uint64_t count) {
    LOG(INFO) << fmt::format("{} {} {}", i, key, count);
    return true;
  });
  return 0;
}

} // namespace mindalpha
