#include "cs561/cs561_log.h"

#include <chrono>
#include <iomanip>

namespace ROCKSDB_NAMESPACE {

std::string CS561Log::LOG_ROOT = "experiment";
std::string CS561Log::LOG_FILEPATH = "log.txt";
std::string CS561Log::MINIMUM_FILEPATH = "minimum.txt";
std::string CS561Log::VERSION_INFO_FILEPATH =
    "version_info.txt";
std::string CS561Log::TMP_RESULT_FILEPATH =
    "tmp_result.txt";
std::vector<std::string> CS561Log::DUMP_FILEPATHS = {
    "history/picking_history_level0",
    "history/picking_history_level1"};

void CS561Log::SetLogRootPath(
    const std::string& root_path) {
  LOG_ROOT = root_path;
}

void CS561Log::Log(const std::string& content,
                   LogLevel log_level) {
  std::ofstream log_file(LOG_ROOT + "/" + LOG_FILEPATH,
                         std::ios::app);
  LogContent(log_file, content, log_level);
}

void CS561Log::LogContent(std::ofstream& of, const std::string& content,
                   LogLevel log_level) {
  const auto now = std::chrono::system_clock::to_time_t(
      std::chrono::system_clock::now());
  const auto local_time = std::localtime(&now);
  std::string log_type;
  switch (log_level) {
    case INFO:
      log_type = "INFO";
      break;
    case WARNING:
      log_type = "WARNING";
      break;
    case ERROR:
      log_type = "ERROR";
      break;
    default:
      log_type = "NONE";
      break;
  }
  of << std::put_time(local_time, "%Y-%m-%d %H:%M:%S")
           << " --- "
           << "(" << log_type << ")" << content
           << std::endl;
  of.flush();
}

void CS561Log::LogResult(size_t WA, size_t left_bytes) {
  Log("total written bytes: " + std::to_string(WA) +
      ", left bytes: " + std::to_string(left_bytes));
  // std::ofstream content_file(LOG_ROOT + "/content.txt",
  //                        std::ios::app);
  // LogContent(content_file, "total written bytes: " + std::to_string(WA));
}

void CS561Log::LogMinimum(
    size_t WA, size_t left_bytes,
    const std::vector<CompactionInfo>& compactions_info) {
  // Log minimum written bytes
  std::ofstream f(LOG_ROOT + "/" + MINIMUM_FILEPATH);
  f << WA << " " << left_bytes << std::endl;
  f.close();
  // Log corresponding compaction info
  LogMinimumCompactionInfo(compactions_info);
}

std::pair<size_t, size_t> CS561Log::LoadMinimum() {
  std::ifstream f(LOG_ROOT + "/" + MINIMUM_FILEPATH);
  size_t wa, left_bytes;
  f >> wa >> left_bytes;
  f.close();
  return {wa, left_bytes};
}

void CS561Log::LogVersion(const std::vector<Fsize>& temp,
                          size_t hash_value) {
  std::ofstream version_info_file(
      LOG_ROOT + "/" + VERSION_INFO_FILEPATH,
      std::ios::app);
  version_info_file << "Hash value: " << std::setfill('0')
                    << std::setw(sizeof(size_t) * 2)
                    << std::hex << hash_value << std::dec
                    << std::endl;
  version_info_file << "Number of files: " << temp.size()
                    << std::endl;
  version_info_file << "Information of files: "
                    << std::endl;
  version_info_file << "File No."
                    << "\t"
                    << "Smallest key"
                    << "\t"
                    << "Largest key"
                    << "\t"
                    << "Entry number"
                    << "\t"
                    << "Deletion number"
                    << "\t"
                    << "Raw key size"
                    << "\t"
                    << "Raw value size"
                    << "\t" << std::endl;
  for (size_t i = 0; i < temp.size(); ++i) {
    AlignOutput(version_info_file, "File No.",
                std::to_string(i + 1));
    AlignOutput(
        version_info_file, "Smallest key",
        temp[i].file->smallest.user_key().ToString());
    AlignOutput(
        version_info_file, "Largest key",
        temp[i].file->largest.user_key().ToString());
    AlignOutput(version_info_file, "Entry number",
                std::to_string(temp[i].file->num_entries));
    AlignOutput(
        version_info_file, "Deletion number",
        std::to_string(temp[i].file->num_deletions));
    AlignOutput(version_info_file, "Raw key size",
                std::to_string(temp[i].file->raw_key_size));
    AlignOutput(
        version_info_file, "Raw value size",
        std::to_string(temp[i].file->raw_value_size));
    version_info_file << std::endl;
  }
  version_info_file << std::endl;
}

void CS561Log::LogCompactionsInfo(
    std::ofstream& target,
    const std::vector<CompactionInfo>& compactions_info) {
  target << std::endl;
  for (size_t i = 0; i < compactions_info.size(); ++i) {
    LogCompactionInfo(target, compactions_info[i], static_cast<int>(i + 1));
  }
}

void CS561Log::LogMinimumCompactionInfo(
    const std::vector<CompactionInfo>& compactions_info) {
  std::ofstream f(LOG_ROOT + "/" + MINIMUM_FILEPATH,
                  std::ios::app);
  LogCompactionsInfo(f, compactions_info);
}

void CS561Log::LogRegularCompactionInfo(
    const CompactionInfo& compaction_info,
    int compaction_index) {
  std::ofstream f(LOG_ROOT + "/" + LOG_FILEPATH,
                  std::ios::app);
  LogCompactionInfo(f, compaction_info, compaction_index);
}

void CS561Log::LogCompactionInfo(
    std::ofstream& target,
    const CompactionInfo& compaction_info,
    int compaction_index) {
  target << "Compaction " << compaction_index
         << " in level " << compaction_info.level
         << " Choice: file No." << compaction_info.index
         << std::endl;
  target << "Hash value: " << std::setfill('0')
         << std::setw(sizeof(size_t) * 2) << std::hex
         << compaction_info.hash_value << std::dec
         << std::endl;
  for (size_t i = 0;
       i < compaction_info.level_files_info.size(); ++i) {
    LogCompactionInfoForLevel(target, compaction_info, static_cast<int>(i));
  }
}

void CS561Log::LogCompactionInfoForLevel(
    std::ofstream& target,
    const CompactionInfo& compaction_info, int level) {
  target << "Information of files in level " << level
         << ": " << std::endl;
  target << "File No."
         << "\t"
         << "Smallest key"
         << "\t"
         << "Largest key"
         << "\t"
         << "Entry number"
         << "\t"
         << "Deletion number"
         << "\t"
         << "Raw key size"
         << "\t"
         << "Raw value size"
         << "\t"
         << "File size"
         << "\t"
         << "Overlapping ratio"
         << "\t" << std::endl;
  for (size_t j = 0;
       j < compaction_info.level_files_info[level].size();
       ++j) {
    if (j == compaction_info.index && level != 0 &&
        level == 1) {  // when the file is chosen for
                       // compaciton in this level and this
                       // level is not level 0
      AlignOutput(target, "File No.",
                  std::to_string(j) + "*");
    } else {
      AlignOutput(target, "File No.", std::to_string(j));
    }
    AlignOutput(target, "Smallest key",
                compaction_info.level_files_info[level][j]
                    .smallest);
    AlignOutput(
        target, "Largest key",
        compaction_info.level_files_info[level][j].largest);
    AlignOutput(
        target, "Entry number",
        std::to_string(
            compaction_info.level_files_info[level][j]
                .num_entries));
    AlignOutput(
        target, "Deletion number",
        std::to_string(
            compaction_info.level_files_info[level][j]
                .num_deletions));
    AlignOutput(
        target, "Raw key size",
        std::to_string(
            compaction_info.level_files_info[level][j]
                .raw_key_size));
    AlignOutput(
        target, "Raw value size",
        std::to_string(
            compaction_info.level_files_info[level][j]
                .raw_value_size));
    AlignOutput(
        target, "File size",
        std::to_string(
            compaction_info.level_files_info[level][j]
                .file_size));
    AlignOutput(
        target, "Overlapping ratio",
        std::to_string(compaction_info.level_files_info[level][j]
                .overlapping_bytes));
    target << std::endl;
  }
  target << std::endl;
}

void CS561Log::AlignOutput(std::ofstream& of,
                           const std::string& align_target,
                           const std::string& output) {
  size_t align_length = (floor(static_cast<double>(align_target.length()) / 4) + 1) * 4;
  size_t number_of_extra_tab = ceil(static_cast<double>(align_length - output.length()) / 4);
  if (align_length <= output.length()) {
    number_of_extra_tab = 1;
  }
  of << output;
  for (size_t i = 0; i < number_of_extra_tab; i++) {
    of << "\t";
  }
}

std::string CS561Log::ReadFinishEnumeration(int level) {
  std::ifstream f(LOG_ROOT + "/" + DUMP_FILEPATHS[level]);
  size_t line = 4;
  size_t column = 5;
  // the fully enumerating sign is on the fourth line, fifth
  // value
  std::string tmp;
  for (size_t i = 0; i < line - 1; i++) {
    std::getline(f, tmp);
  }
  for (size_t i = 0; i < column - 1; i++) {
    f >> tmp;
  }
  f >> tmp;
  return tmp;
}

void CS561Log::LogWA(size_t WA, int type) {
  std::ifstream f(LOG_ROOT + "/" + TMP_RESULT_FILEPATH,
                  std::ios::app);
  // roundrobin wa, minoverlappingratio wa and optimal wa
  size_t rbwa, mowa, opwa;
  f >> rbwa >> mowa >> opwa;
  if (type == 0) {
    rbwa = WA;
  } else if (type == 1) {
    mowa = WA;
  } else if (type == 2) {
    opwa = WA;
  }
  // write back
  std::ofstream f2(LOG_ROOT + "/" + TMP_RESULT_FILEPATH);
  f2 << rbwa << " " << mowa << " " << opwa << std::endl;
}

void CS561Log::LogSimpleMinimumCompactionInfo(
    std::ofstream& of,
    const std::vector<CompactionInfo>& compactions_info) {
  of << "Optimal compaction choice rank: ";
  for (size_t i = 0; i < compactions_info.size(); ++i) {
    if (i != 0) {
      of << " ";
    }
    of << compactions_info[i].choice_rank;
  }
  of << std::endl;
  of << std::endl;
}

}  // namespace ROCKSDB_NAMESPACE
