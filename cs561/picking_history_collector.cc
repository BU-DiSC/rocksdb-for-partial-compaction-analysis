#include "cs561/picking_history_collector.h"

#include <fstream>
#include <iomanip>

#include "cs561/cs561_log.h"

using std::endl;

namespace ROCKSDB_NAMESPACE {

PickingHistoryCollector::PickingHistoryCollector()
    : DUMP_FILEPATH_LEVEL0(
          CS561Log::LOG_ROOT +
          "/history/picking_history_level0"),
      DUMP_FILEPATH_LEVEL1(
          CS561Log::LOG_ROOT +
          "/history/picking_history_level1"),
      forests(VersionForest(
          {DUMP_FILEPATH_LEVEL0, DUMP_FILEPATH_LEVEL1})) {
  // CS561Log::Log("Begin PickingHistoryCollector constructor");
  auto p = CS561Log::LoadMinimum();
  global_min_WA = p.first;
  global_min_WA_corresponding_left_bytes = p.second;
  WA = 0;
  left_bytes = 0;
  compaction_time = 0;
  // CS561Log::Log("Finish PickingHistoryCollector constructor");
}

PickingHistoryCollector::~PickingHistoryCollector() {
  // std::cout << "PickingHistoryCollector destructor" << std::endl;
  DumpToFile();
}

std::string PickingHistoryCollector::serialize_Fsize_vec(
    const std::vector<ROCKSDB_NAMESPACE::Fsize>& temp) {
  static FileSerializer serializer;
  static std::hash<std::string> hasher;

  size_t n = temp.size();

  std::vector<size_t> ordered_temp;
  ordered_temp.reserve(n);
  for (const auto& t : temp) {
    std::string ret = serializer.Serialize(t.file);
    ordered_temp.push_back(hasher(ret));
  }
  std::sort(ordered_temp.begin(), ordered_temp.end());

  std::string res;
  // size_t length(10/20) + split character(1)
  res.reserve(
      (std::to_string(std::numeric_limits<size_t>::max())
           .size() +
       1) *
      n);
  for (std::size_t i = 0; i < n; ++i) {
    res += std::to_string(ordered_temp[i]);
    res.push_back('A');
  }
  return res;
}

void PickingHistoryCollector::UpdateWA(size_t new_WA) {
  WA.fetch_add(new_WA);
  WA_corresponding_left_bytes.store(left_bytes.load());
}

void PickingHistoryCollector::UpdateCompactionTime(
    size_t new_compaction_time) {
  compaction_time += new_compaction_time;
}

bool PickingHistoryCollector::CheckContinue() {
  return (WA + WA_corresponding_left_bytes) <=
         (global_min_WA +
          global_min_WA_corresponding_left_bytes);
}

void PickingHistoryCollector::UpdateLeftBytes(
    size_t new_left_bytes) {
  left_bytes.store(new_left_bytes);
}

VersionForest& PickingHistoryCollector::GetVersionForest() {
  return forests;
}

void PickingHistoryCollector::DumpToFile() {
  // dump wa
  CS561Log::LogResult(WA, WA_corresponding_left_bytes);
  // compute minimum WA
  if ((WA + WA_corresponding_left_bytes) <=
      (global_min_WA +
       global_min_WA_corresponding_left_bytes)) {
    // dump minimum
    CS561Log::LogMinimum(WA, WA_corresponding_left_bytes,
                         compactions_info);
  }
  // log compaction latency
  CS561Log::Log("compaction_time: " +
                std::to_string(compaction_time) + "us");
}

void PickingHistoryCollector::DumpWAResult() {
  // dump wa
  CS561Log::LogResult(WA, WA_corresponding_left_bytes);
}

void PickingHistoryCollector::DumpWAMinimum() {
  // compute minimum WA
  if ((WA + WA_corresponding_left_bytes) <=
      (global_min_WA +
       global_min_WA_corresponding_left_bytes)) {
    // dump minimum
    CS561Log::LogMinimum(WA, WA_corresponding_left_bytes,
                         compactions_info);
  }
}

void PickingHistoryCollector::CollectCompactionInfo(
    int level, std::vector<FileMetaData*>* files,
    const std::vector<uint64_t>& file_overlapping_ratio,
    int num_level, size_t hash_value, size_t index) {
  compactions_info.emplace_back(
      CompactionInfo(level, files, file_overlapping_ratio,
                     num_level, hash_value, index));
}

std::vector<CompactionInfo>&
PickingHistoryCollector::GetCompactionsInfo() {
  return compactions_info;
}

size_t PickingHistoryCollector::GetWA() {
  return WA + WA_corresponding_left_bytes;
}

}  // namespace ROCKSDB_NAMESPACE
