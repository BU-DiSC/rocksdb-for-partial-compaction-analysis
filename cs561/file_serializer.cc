#include "cs561/file_serializer.h"

namespace ROCKSDB_NAMESPACE {

FileSerializer::FileSerializer() {}

FileSerializer::~FileSerializer() {}

std::string FileSerializer::Serialize(
    const rocksdb::FileMetaData* file) {
  FileTag tag;
  tag.smallest = file->smallest;
  tag.largest = file->largest;
  tag.num_entries = file->num_entries;
  std::string result;
  result.append(tag.smallest.Encode().ToString());
  result.append("\0");
  result.append(tag.largest.Encode().ToString());
  result.append("\0");
  result.append(std::to_string(tag.num_entries));
  return result;
}

bool FileSerializer::IsSameVersion(
    std::vector<std::string>& version1,
    std::vector<std::string>& version2) {
  if (version1.size() != version2.size()) {
    return false;
  }
  // build a set for verison2
  std::unordered_set<std::string> version2_hashset;
  for (std::size_t i = 0; i < version2.size(); i++) {
    version2_hashset.insert(version2[i]);
  }
  // Check whether all the files in version1 are in version2
  for (std::size_t i = 0; i < version1.size(); i++) {
    if (version2_hashset.find(version1[i]) ==
        version2_hashset.end()) {
      return false;
    }
    // erase the corresponding file in version2 set
    version2_hashset.erase(version1[i]);
  }
  return true;
}

}  // namespace ROCKSDB_NAMESPACE
