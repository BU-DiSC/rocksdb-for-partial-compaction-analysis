#pragma once
#include <cassert>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "cs561/cs561_log.h"
#include "cs561/file_serializer.h"

namespace ROCKSDB_NAMESPACE {

struct VersionNode {
  // id of the version
  size_t id{};

  // id of parent version
  size_t parent_id{};

  // indicate the version of the files
  size_t hash_value{};

  // total WA until this version
  // size_t total_WA;

  // indicate the files that have been chosen
  // the index of the vector is the index of the chosen file
  // in this version and value is the id of the new version
  // after compaction. The real node will be stored in an
  // array in LevelVersionForest. Child node will not be
  // created when have another compaction on after this
  // compaction, so we use long max as the temporary id to
  // indicated this situation. If the child is long max,
  // this means that there is no compaction after choosing
  // this file and the child is a leaf.
  std::vector<size_t> chosen_children;

  // the number of files in this version
  int file_num{};

  // tag for whether this version is a leaf, when
  // constructing, it's default to be true in case of there
  // is not compaction on it bool is_leaf{};

  // tag for whether we have enumerate all possibilities
  // after this version
  bool fully_enumerated{};

  VersionNode() = default;

  VersionNode(size_t id_, size_t parent_id_,
              size_t hash_value_, int file_num_)
      : id(id_),
        parent_id(parent_id_),
        hash_value(hash_value_),
        file_num(file_num_) {
    chosen_children.resize(file_num, std::numeric_limits<size_t>::max()-1);
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  const VersionNode& node);

  friend std::istream& operator>>(std::istream& is,
                                  VersionNode& node);
};

// version forest of a level
class LevelVersionTree {
public:
  // TODO: Peixu
  explicit LevelVersionTree(const std::string& fp);

  // TODO: Peixu
  ~LevelVersionTree() noexcept;

  // TODO: Ran
  /**
   * get the index of the file in the current version
   * @param hash_value hash_value of the version
   * @param file_num file number of the version
   * @return long_max if there is no new file to pick,
   * otherwise the index of the file to pick in the version
   */
  size_t GetCompactionFile(size_t hash_value, int file_num);

  // TODO: Peixu
  // dump forest to file
  void DumpToFile();

  /**
   * Set the current version to be fully enumerated
   */
  void SetCurrentVersionFullyEnumerated(size_t index);

  /**
   * Check whether the version exists
   */
  bool IsVersionExist(size_t hash_value);

private:
  // a list of version nodes
  std::vector<VersionNode> version_nodes;
  // map hash value of a version to the id of the
  // VersionNode in version_nodes
  std::unordered_map<size_t, size_t> hash_to_id;
  // version id of the last compaction
  size_t last_version_id = std::numeric_limits<size_t>::max();
  size_t last_chosen_file_index = std::numeric_limits<size_t>::max();
  // file path of this LevelVersionForest
  // Important: file_path cannot contain ',' '\n'
  const std::string file_path;

  // TODO: Peixu
  // load forest from file
  void LoadFromFile();

  // TODO: Ran
  // add when doesn't exist the version node of the hash
  // value this will only happen when
  void AddNode(size_t hash_value, int file_num);
};

// Contain tree for each level
class VersionForest {
  public:
  // Dump to file
  void DumpToFile();

  private:
  // file path of the version node in disk
  std::string file_path;
  // version forests for each level
  std::vector<LevelVersionTree> level_version_trees;

  public:
  // TODO: Peixu
  explicit VersionForest(const std::vector<std::string>& level_file_path);

  // TODO: Peixu
  ~VersionForest() = default;

  // TODO: Ran
  // get the index of the file in the current version of a
  // certain level
  size_t GetCompactionFile(int level, size_t hash_value,
                           int file_num);

  /**
   * Get LevelVersionForest
   */
  LevelVersionTree& GetLevelVersionTree(int level);

  /**
   * Check whether the version exists
   */
  bool IsVersionExist(int level, size_t hash_value);
};

}  // namespace ROCKSDB_NAMESPACE
