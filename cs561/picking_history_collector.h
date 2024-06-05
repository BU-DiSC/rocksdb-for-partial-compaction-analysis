#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <atomic>

//#include "version_set.h"
#include "cs561/cs561_log.h"
#include "cs561/file_serializer.h"
#include "cs561/version_forest.h"
#include "db/version_edit.h"

namespace ROCKSDB_NAMESPACE {

class PickingHistoryCollector {
  private:
  std::string DUMP_FILEPATH_LEVEL0;
  std::string DUMP_FILEPATH_LEVEL1;

  // the version forests which stores the history of all
  // levels
  VersionForest forests;

  // current global minimum WA
  size_t global_min_WA;
  size_t global_min_WA_corresponding_left_bytes;

  // current WA
  std::atomic<size_t> WA;
  std::atomic<size_t> WA_corresponding_left_bytes;

  // current compaction time
  size_t compaction_time;

  // remaining bytes to be inserted
  std::atomic<size_t> left_bytes;

  // Information of each compaction for a run
  std::vector<CompactionInfo> compactions_info;

  private:
  /**
   * serialize the Fsize vector.
   * @brief firstly sort the hashed Fsize vector to avoid
   * the same version but different order scenario, then
   * return the serialized hashed Fsize vector
   * @note the result is non-deserializable.
   *       and the hash collision is ignored.
   * @param temp: the current file version
   * @return serialized hash string
   */
  static std::string serialize_Fsize_vec(
      const std::vector<Fsize>& temp);

  public:
  /**
   * Constructor
   * @param root_path: the root path of all the files
   */
  PickingHistoryCollector();

  ~PickingHistoryCollector();

  // TODO: Chen
  /**
   * Update the current WA
   * @param new_WA: the newly computed WA
   */
  void UpdateWA(size_t new_WA);

  /**
   * Update the current compaction time in microseconds
   * @param new_compaction_time: the newly computed
   * compaction time
   */
  void UpdateCompactionTime(size_t new_compaction_time);

  // TODO: Chen
  /**
   * Check whether the current WA has already exceeded the
   * minimum even all the left compactions are trivial move.
   */
  bool CheckContinue();

  // TODO: Chen
  /**
   * Update the left inserts write
   * @param dec: decrement of left bytes
   */
  void UpdateLeftBytes(size_t dec);

  /**
   * Get forests
   */
  VersionForest& GetVersionForest();

  /**
   * Dump all record to files
   */
  void DumpToFile();
  void DumpWAResult();
  void DumpWAMinimum();

  /**
   * Collect information for a compaction
   * @param level level of the compaction
   * @param files current version of all levels
   * @param file_overlapping_ratio the overlapping ratio of
   * each file in the level
   * @param num_level the number of levels
   * @param temp version of a level
   * @param hash_value hash value of the version
   * @param index index of the version to be chosen to
   * compact
   */
  void CollectCompactionInfo(
      int level, std::vector<FileMetaData*>* files,
      const std::vector<uint64_t>& file_overlapping_ratio,
      int num_level, size_t hash_value, size_t index);

  /**
   * Get the vector of compaction information
   */
  std::vector<CompactionInfo>& GetCompactionsInfo();

  /**
   * Get the current WA
   */
  size_t GetWA();
};

}  // namespace ROCKSDB_NAMESPACE
