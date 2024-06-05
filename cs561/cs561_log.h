#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "db/version_edit.h"

namespace ROCKSDB_NAMESPACE {

struct FileInfo {
  std::string smallest;
  std::string largest;
  uint64_t num_entries = 0;
  uint64_t num_deletions = 0;
  uint64_t raw_key_size = 0;
  uint64_t raw_value_size = 0;
  uint64_t overlapping_bytes = 0;
  uint64_t file_size = 0;

  FileInfo(const FileMetaData* file,
           uint64_t overlapping_bytes_ = 0)
      : smallest(file->smallest.user_key().ToString()),
        largest(file->largest.user_key().ToString()),
        num_entries(file->num_entries),
        num_deletions(file->num_deletions),
        raw_key_size(file->raw_key_size),
        raw_value_size(file->raw_value_size),
        overlapping_bytes(overlapping_bytes_),
        file_size(file->fd.file_size) {}
};

struct CompactionInfo {
  std::vector<std::vector<FileInfo>> level_files_info;
  std::vector<uint64_t> file_overlapping_ratio;
  int level;
  size_t hash_value;
  size_t index;
  size_t choice_rank;  // the overlapping ratio rank of the
                       // chosen file

  CompactionInfo(
      int level_, std::vector<FileMetaData*>* level_file,
      const std::vector<uint64_t>& file_overlapping_ratio_,
      int num_level, size_t hash_value_, size_t index_)
      : file_overlapping_ratio(file_overlapping_ratio_),
        level(level_),
        hash_value(hash_value_),
        index(index_) {
    level_files_info =
        std::vector<std::vector<FileInfo>>(num_level);
    for (int i = 0; i < num_level; ++i) {
      level_files_info[i].reserve(level_file[i].size());
      for (size_t j = 0; j < level_file[i].size(); ++j) {
        if (level == i) {
          level_files_info[i].emplace_back(
              FileInfo(level_file[i][j],
                       file_overlapping_ratio_[j]));
        } else {
          level_files_info[i].emplace_back(
              FileInfo(level_file[i][j]));
        }
      }
    }

    std::vector<std::pair<uint64_t, int>> pairs(
        file_overlapping_ratio.size());
    for (size_t i = 0; i < file_overlapping_ratio.size();
         ++i) {
      pairs[i] = {file_overlapping_ratio[i], i};
    }

    sort(pairs.begin(), pairs.end());
    for (size_t i = 0; i < pairs.size(); ++i) {
      if (pairs[i].second == static_cast<int>(index)) {
        choice_rank = i;
        break;
      }
    }
  }
};

class CS561Log {
  private:
  static std::string LOG_FILEPATH;
  static std::string MINIMUM_FILEPATH;
  static std::string VERSION_INFO_FILEPATH;
  static std::string TMP_RESULT_FILEPATH;
  static std::vector<std::string> DUMP_FILEPATHS;

  /**
   * Record all the compaction info of a run
   * @param target the target to write the log
   * @param compactions_info a vector records compaction
   * informatio of every compaction
   */
  static void LogCompactionsInfo(
      std::ofstream& target,
      const std::vector<CompactionInfo>& compactions_info);

  /**
   * Record compaction info of one compaction
   * @param target the target to write the log
   * @param compaction_info a compaction information object
   */
  static void LogCompactionInfo(
      std::ofstream& target,
      const CompactionInfo& compaction_info,
      int compaction_index);

  /**
   * Record compaction info of one compaction
   * @param target the target to write the log
   * @param compaction_info a compaction information object
   * @param level level of the files
   */
  static void LogCompactionInfoForLevel(
      std::ofstream& target,
      const CompactionInfo& compaction_info, int level);

  public:
  static std::string LOG_ROOT;

  enum LogLevel {
    INFO,
    WARNING,
    ERROR,
  };

  /**
   * Set the all log file root path
   * @param root_path root path of all log files
   */
  static void SetLogRootPath(const std::string& root_path);

  // TODO: Peixu
  /**
   * Log file selection of a certain version
   * @param content content to log
   */
  static void Log(const std::string& content,
                  LogLevel log_level = INFO);

  static void LogContent(std::ofstream& of, const std::string& content,
                  LogLevel log_level = INFO);

  /**
   * Record the total WA for a certain run
   * @param WA write amplification of a run
   * @param left_bytes left bytes when the WA is recorded
   */
  static void LogResult(size_t WA, size_t left_bytes);

  /**
   * Record the minimum WA so far and the compaction info
   * @param WA minimum write amplification so far
   */
  static void LogMinimum(
      size_t WA, size_t left_bytes,
      const std::vector<CompactionInfo>& compactions_info);

  /**
   * Load minimum WA so far
   */
  static std::pair<size_t, size_t> LoadMinimum();

  /**
   * Record a version to file
   * @param temp a vector of Fsize which contains the
   * filemetadata of a version
   * @param hash_value hash value of this version
   */
  static void LogVersion(const std::vector<Fsize>& temp,
                         size_t hash_value);

  /**
   * Log the compaction information for the run with the
   * minimum write amplification
   * @param compactions_info a vector records compaction
   * informatio of every compaction
   */
  static void LogMinimumCompactionInfo(
      const std::vector<CompactionInfo>& compactions_info);

  /**
   * Log the compaction information for a regular compaction
   * @param compactions_info a vector records compaction
   * informatio of every compaction
   * @param compaction_index the index of this compaction
   */
  static void LogRegularCompactionInfo(
      const CompactionInfo& compaction_info,
      int compaction_index);

  /**
   * Align the output to file according to a target with a
   * '\t'
   * @param of ofstream object
   * @param align_target alignment target
   * @param output output string
   */
  static void AlignOutput(std::ofstream& of,
                          const std::string& align_target,
                          const std::string& output);

  /**
   * Read the fully enumerated sign of the first version of
   * a level
   * @param level level
   */
  static std::string ReadFinishEnumeration(int level);

  /**
   * Log the write amplification of current run if the
   * current is the optimal one or is the baseline
   * @param WA write amplification of current run
   * @param type type of the current run(0: roundrobin, 1:
   * minoverlappingratio, 2: optimal)
   */
  static void LogWA(size_t WA, int type);

  /**
   * Log the simplified information of the optimal
   * compaction
   * @param of ofstream object
   * @param compaction_info a vector records compaction
   * informatio of every compaction
   */
  static void LogSimpleMinimumCompactionInfo(
      std::ofstream& of,
      const std::vector<CompactionInfo>& compactions_info);
};

}  // namespace ROCKSDB_NAMESPACE