#include "cs561/all_files_enumerator.h"

#include "cs561/cs561_log.h"

namespace ROCKSDB_NAMESPACE {

AllFilesEnumerator* AllFilesEnumerator::instance = nullptr;

AllFilesEnumerator::AllFilesEnumerator()
    : collector(),
      entry_num_collector(INFO_COLLECTOR_SIZE),
      deletion_num_collector(INFO_COLLECTOR_SIZE),
      file_size_collector(INFO_COLLECTOR_SIZE),
      log_level(1),
      compaction_counter(0) {
  // CS561Log::Log("AllFilesEnumerator is created");
  }

int AllFilesEnumerator::GetPickingFile(
    std::vector<rocksdb::Fsize>& temp, int level, 
    const std::vector<uint64_t>& file_overlapping_ratio) {
  size_t index = std::numeric_limits<size_t>::max();
  // compute hash of the current version
  static std::hash<std::vector<ROCKSDB_NAMESPACE::Fsize>>
      hasher;
  std::size_t hash_value = hasher(temp);
  ++compaction_counter;
  // if this is a new version, record the file information
  // of this version to disk
  if (!collector.GetVersionForest().IsVersionExist(level, hash_value)) {
    CS561Log::LogVersion(temp, hash_value);
  }
  // log
  if (log_level == 1) {
    CS561Log::Log("Level: " + std::to_string(level) +
                  ", Version: " + std::to_string(hash_value) +
                  ", File index: " + std::to_string(index));
  }
  // select a file according to the history
  index = collector.GetVersionForest().GetCompactionFile(
      level, hash_value, static_cast<int>(temp.size()));
  
  // check whether we find a file to compact
  if (index == std::numeric_limits<size_t>::max()) {
    // we can terminate the program because we don't need to continue
    Terminate();
  }
  // record this compaction
  chosen_file_index_ = index;
  // record the involved bytes
  involved_bytes_.clear();
  for (size_t i = 0; i < temp.size(); ++i) {
    double estimated_bytes = temp[i].file->fd.file_size * (1.0 + static_cast<double>(file_overlapping_ratio[i]) / 1024);
    involved_bytes_.push_back(static_cast<uint64_t>(estimated_bytes));
  }
  return static_cast<int>(index);
}

int AllFilesEnumerator::MaybeSelectLastSimilarFile(const std::vector<FileMetaData*>& files, const std::vector<uint64_t>& file_overlapping_ratio) {
  // std::cout << "MaybeSelectLastSimilarFile" << std::endl;
  // std::cout << "files.size(): " << files.size() << ", file_overlapping_ratio.size(): " << file_overlapping_ratio.size() << std::endl;

  // find the file with the smallest overlapping ratio
  uint64_t min_overlap_index = std::min_element(file_overlapping_ratio.begin(), file_overlapping_ratio.end()) - file_overlapping_ratio.begin();
  // std::cout << "min_overlap_index: " << min_overlap_index << std::endl;
  // we keep the same strategy as KMinOverlappingRatio if the min overlapping ratio is 0
  if (file_overlapping_ratio[min_overlap_index] == 0) {
    return static_cast<int>(min_overlap_index);
  }
  // the overlapping ratio threshold is 1.05 * the smallest overlapping ratio
  uint64_t threshold = static_cast<uint64_t>(1.05 * file_overlapping_ratio[min_overlap_index]);
  // std::cout << "threshold: " << threshold << std::endl;
  // find the files with overlapping ratio less than the threshold
  std::vector<size_t> candidate_files;
  for(size_t i = 0; i < file_overlapping_ratio.size(); ++i) {
    if (file_overlapping_ratio[i] < threshold) {
      candidate_files.push_back(i);
    }
  }
  // std::cout << "candidate_files.size(): " << candidate_files.size() << std::endl;
  // for (size_t i = 0; i < candidate_files.size(); ++i) {
  //   std::cout << "candidate_files[" << i << "]: " << candidate_files[i] << std::endl;
  // }
  if (candidate_files.size() == 1) {
    return static_cast<int>(min_overlap_index);
  }

  CS561Log::Log("Encounter a case in SelectLastSimilarFile");

  // If the candidate file has no next file, directly select it
  if (candidate_files.back() == files.size() - 1) {
    if (min_overlap_index == candidate_files.back()) {
      CS561Log::Log("Select the same file as MinOverlappingRatio in SelectLastSimilarFile");
    }
    return static_cast<int>(candidate_files.back());
  }

  // select the file whose next file has the largest overlapping ratio. 
  uint64_t min_combined_or = std::numeric_limits<uint64_t>::max();
  size_t selected_file_index = 0;
  for(size_t i = 0; i < candidate_files.size(); ++i) {
    if (file_overlapping_ratio[candidate_files[i]] * 0.05 - file_overlapping_ratio[candidate_files[i]+1] < min_combined_or) {
      min_combined_or = file_overlapping_ratio[candidate_files[i]] * 0.05 - file_overlapping_ratio[candidate_files[i]+1];
      selected_file_index = candidate_files[i];
    }
  }

  if (min_overlap_index == selected_file_index) {
    CS561Log::Log("Select the same file as MinOverlappingRatio in SelectLastSimilarFile");
  }

  // std::cout << "selected_file_index: " << selected_file_index << std::endl;
  return static_cast<int>(selected_file_index);
}


int AllFilesEnumerator::GetEstimatedFile(const std::vector<FileMetaData *>* files, int level, int num_level, const MutableCFOptions& op, const InternalKeyComparator& icmp){
  // print the information of the files
  for (int i = 0; i < num_level; ++i) {
    std::cout << "level: " << i << ", file number: " << files[i].size() << std::endl;
    for (size_t j = 0; j < files[i].size(); ++j) {
      std::cout << "level: " << i << ", file: " << j << ", skey: " << KeyToNumber(files[i].at(j)->smallest) << ", lkey: " << KeyToNumber(files[i].at(j)->largest);
      std::cout << ", file size: " << files[i].at(j)->fd.file_size << ", entry number: " << files[i].at(j)->num_entries << std::endl;
    }
  }

  // build simulated file from FileMetaData
  std::vector<std::vector<std::shared_ptr<SimulatedFileMetaData>>> simulated_files(num_level);
  for(int i = 0; i < num_level; ++i) {
    for(size_t j = 0; j < files[i].size(); ++j) {
      simulated_files[i].emplace_back(std::make_shared<SimulatedFileMetaData>(files[i][j]));
      std::cout << "simulated_files[" << i << "][" << j << "]: " << "num_entries: " << simulated_files[i][j]->num_entries << ", file_size: " << simulated_files[i][j]->file_size << std::endl;
    }
  }
  
  const std::vector<std::shared_ptr<SimulatedFileMetaData>>& level_files = simulated_files[level];

  std::vector<uint64_t> estimated_written_bytes(level_files.size(), 0);

  // for each file in this level, we compute the estimated cost in terms of written bytes if we choose the file to compact
  for(size_t i = 0; i < level_files.size(); ++i) {
    std::cout << "estimating file " << i << " in level " << level << std::endl;
    // compute the written bytes to the next level of each file
    estimated_written_bytes[i] += ComputeWrittenBytesForCompaction({level_files[i]}, simulated_files[level+1], icmp);
    std::cout << "estimated_written_bytes[i]: " << estimated_written_bytes[i] << std::endl;

    // remove level_files[i] from this level since we compact it
    std::vector<std::shared_ptr<SimulatedFileMetaData>> simulated_level_files_after_compaction;
    simulated_level_files_after_compaction.reserve(level_files.size()-1);
    simulated_level_files_after_compaction.insert(simulated_level_files_after_compaction.end(), level_files.begin(), level_files.begin() + i);
    simulated_level_files_after_compaction.insert(simulated_level_files_after_compaction.end(), level_files.begin() + i + 1, level_files.end());

    // estimate the next level in the next compaction (i.e. after compacting level_files[i] to it)
    std::vector<std::shared_ptr<SimulatedFileMetaData>> simulated_next_level_files_in_next_compaction = Compaction({level_files[i]}, simulated_files[level+1], level+1, icmp, op);
    std::cout << "simulated_next_level_files_in_next_compaction.size(): " << simulated_next_level_files_in_next_compaction.size() << std::endl;
    for (size_t j = 0; j < simulated_next_level_files_in_next_compaction.size(); ++j) {
      std::cout << "\tfile " << j << " in next level after compaction: ";
      std::cout << "skey: " << KeyToNumber(simulated_next_level_files_in_next_compaction[j]->smallest);
      std::cout << " lkey: " << KeyToNumber(simulated_next_level_files_in_next_compaction[j]->largest) << std::endl;
    }

    // compute the estimated overlapping bytes from the previous level if we remove level_files[i] in the next compaction
    std::vector<std::shared_ptr<SimulatedFileMetaData>> simulated_prev_level_files_in_next_compaction = simulated_files[level-1];
    // if the number of files is less than op.level0_file_num_compaction_trigger, assume there will be op.level0_file_num_compaction_trigger files. 
    for(size_t j = 0; j < static_cast<size_t>(op.level0_file_num_compaction_trigger) - simulated_files[level-1].size(); ++j) {
      simulated_prev_level_files_in_next_compaction.emplace_back(MakeEstimatedLevel0File());
    }
    estimated_written_bytes[i] += ComputeWrittenBytesForCompaction(simulated_prev_level_files_in_next_compaction, simulated_level_files_after_compaction, icmp);
    std::cout << "estimated_written_bytes[i]: " << estimated_written_bytes[i] << std::endl;
    
    // estimate this level after the next compaction
    std::vector<std::shared_ptr<SimulatedFileMetaData>> simulated_level_files_in_next_compaction = Compaction(simulated_prev_level_files_in_next_compaction, simulated_level_files_after_compaction, level, icmp, op);
    std::cout << "simulated_level_files_in_next_compaction.size(): " << simulated_level_files_in_next_compaction.size() << std::endl;
    for (size_t j = 0; j < simulated_level_files_in_next_compaction.size(); ++j) {
      std::cout << "\tfile " << j << " in level after compaction: ";
      std::cout << "skey: " << KeyToNumber(simulated_level_files_in_next_compaction[j]->smallest);
      std::cout << " lkey: " << KeyToNumber(simulated_level_files_in_next_compaction[j]->largest) << std::endl;
    }

    // find the file with the smallest overlapping bytes and use it to compact
    uint64_t smallest_overlapping_bytes = std::numeric_limits<uint64_t>::max();
    size_t file_index = 0;
    for(size_t j = 0; j < simulated_level_files_in_next_compaction.size(); ++j) {
      uint64_t overlapping_bytes = ComputeOverlappingBytesForFile(simulated_level_files_in_next_compaction[j], simulated_next_level_files_in_next_compaction, icmp);
      if (overlapping_bytes < smallest_overlapping_bytes) {
        smallest_overlapping_bytes = overlapping_bytes;
        file_index = j;
      }
    }
    std::cout << "file_index: " << file_index << std::endl;
    estimated_written_bytes[i] += ComputeWrittenBytesForCompaction({simulated_level_files_in_next_compaction[file_index]}, simulated_next_level_files_in_next_compaction, icmp);
    std::cout << "estimated_written_bytes[i]: " << estimated_written_bytes[i] << std::endl;

    // build a vector for this level after the next compaction
    std::vector<std::shared_ptr<SimulatedFileMetaData>> simulated_level_files_after_next_compaction;
    simulated_level_files_after_next_compaction.reserve(level_files.size()-1);
    simulated_level_files_after_next_compaction.insert(simulated_level_files_after_next_compaction.end(), simulated_level_files_in_next_compaction.begin(), simulated_level_files_in_next_compaction.begin() + file_index);
    simulated_level_files_after_next_compaction.insert(simulated_level_files_after_next_compaction.end(), simulated_level_files_in_next_compaction.begin() + file_index + 1, simulated_level_files_in_next_compaction.end());
    std::cout << "simulated_level_files_after_next_compaction.size(): " << simulated_level_files_after_next_compaction.size() << std::endl;
    for (size_t j = 0; j < simulated_level_files_after_next_compaction.size(); ++j) {
      std::cout << "\tfile " << j << " in level after next compaction: ";
      std::cout << "skey: " << KeyToNumber(simulated_level_files_after_next_compaction[j]->smallest);
      std::cout << " lkey: " << KeyToNumber(simulated_level_files_after_next_compaction[j]->largest) << std::endl;
    }

    // estimate the overlapping bytes again for the previous level to this level
    // now all files in previous needed to be estimated
    std::vector<std::shared_ptr<SimulatedFileMetaData>> simulated_prev_level_files_after_next_compaction;
    simulated_prev_level_files_after_next_compaction.reserve(static_cast<size_t>(op.level0_file_num_compaction_trigger));
    // if the number of files is less than op.level0_file_num_compaction_trigger, assume there will be op.level0_file_num_compaction_trigger files. 
    for(size_t j = 0; j < static_cast<size_t>(op.level0_file_num_compaction_trigger); ++j) {
      simulated_prev_level_files_after_next_compaction.emplace_back(MakeEstimatedLevel0File());
    }
    std::cout << "simulated_prev_level_files_after_next_compaction.size(): " << simulated_prev_level_files_after_next_compaction.size() << std::endl;
    for (size_t j = 0; j < simulated_prev_level_files_after_next_compaction.size(); ++j) {
      std::cout << "\tfile " << j << " in previous level after next compaction: ";
      std::cout << "skey: " << KeyToNumber(simulated_prev_level_files_after_next_compaction[j]->smallest);
      std::cout << " lkey: " << KeyToNumber(simulated_prev_level_files_after_next_compaction[j]->largest) << std::endl;
    }
    estimated_written_bytes[i] += ComputeWrittenBytesForCompaction(simulated_prev_level_files_after_next_compaction, simulated_level_files_after_next_compaction, icmp);
    std::cout << "estimated_written_bytes[i]: " << estimated_written_bytes[i] << std::endl;
  }
  
  uint64_t smallest_overlapping_bytes = std::numeric_limits<uint64_t>::max();
  size_t optimal_file_index = 0;
  for(size_t i = 0; i < level_files.size(); ++i) {
    if (estimated_written_bytes[i] < smallest_overlapping_bytes) {
      smallest_overlapping_bytes = estimated_written_bytes[i];
      optimal_file_index = i;
    }
  }
  std::cout << "optimal_file_index: " << optimal_file_index << std::endl;
  // exit(0);
  return static_cast<int>(optimal_file_index);
}

int AllFilesEnumerator::GetEstimatedFile(const std::vector<std::vector<std::shared_ptr<SimulatedFileMetaData>>>& simulated_files, int level, int num_level, const MutableCFOptions& op, const InternalKeyComparator& icmp){  
  (void)num_level;
  const std::vector<std::shared_ptr<SimulatedFileMetaData>>& level_files = simulated_files[level];

  std::vector<uint64_t> estimated_written_bytes(level_files.size(), 0);

  // for each file in this level, we compute the estimated cost in terms of written bytes if we choose the file to compact
  for(size_t i = 0; i < level_files.size(); ++i) {
    // compute the written bytes to the next level of each file
    estimated_written_bytes[i] += ComputeWrittenBytesForCompaction({level_files[i]}, simulated_files[level+1], icmp);

    // remove level_files[i] from this level since we compact it
    std::vector<std::shared_ptr<SimulatedFileMetaData>> simulated_level_files_after_compaction;
    simulated_level_files_after_compaction.reserve(level_files.size()-1);
    simulated_level_files_after_compaction.insert(simulated_level_files_after_compaction.end(), level_files.begin(), level_files.begin() + i);
    simulated_level_files_after_compaction.insert(simulated_level_files_after_compaction.end(), level_files.begin() + i + 1, level_files.end());

    // estimate the next level in the next compaction (i.e. after compacting level_files[i] to it)
    std::vector<std::shared_ptr<SimulatedFileMetaData>> simulated_next_level_files_in_next_compaction = Compaction({level_files[i]}, simulated_files[level+1], level+1, icmp, op);
    std::cout << "simulated_next_level_files_in_next_compaction.size(): " << simulated_next_level_files_in_next_compaction.size() << std::endl;
    for (size_t j = 0; j < simulated_next_level_files_in_next_compaction.size(); ++j) {
      std::cout << "file " << j << " in next level after compaction: ";
      std::cout << "skey: " << KeyToNumber(simulated_next_level_files_in_next_compaction[j]->smallest);
      std::cout << "lkey: " << KeyToNumber(simulated_next_level_files_in_next_compaction[j]->largest) << std::endl;
    }

    // compute the estimated overlapping bytes from the previous level if we remove level_files[i] in the next compaction
    std::vector<std::shared_ptr<SimulatedFileMetaData>> simulated_prev_level_files_in_next_compaction = simulated_files[level-1];
    // if the number of files is less than op.level0_file_num_compaction_trigger, assume there will be op.level0_file_num_compaction_trigger files. 
    for(size_t j = 0; j < static_cast<size_t>(op.level0_file_num_compaction_trigger) - simulated_files[level-1].size(); ++j) {
      simulated_prev_level_files_in_next_compaction.emplace_back(MakeEstimatedLevel0File());
    }
    estimated_written_bytes[i] += ComputeWrittenBytesForCompaction(simulated_prev_level_files_in_next_compaction, simulated_level_files_after_compaction, icmp);
    std::cout << "estimated_written_bytes[i]: " << estimated_written_bytes[i] << std::endl;
    
    // estimate this level after the next compaction
    std::vector<std::shared_ptr<SimulatedFileMetaData>> simulated_level_files_in_next_compaction = Compaction(simulated_prev_level_files_in_next_compaction, simulated_level_files_after_compaction, level, icmp, op);
    std::cout << "simulated_level_files_in_next_compaction.size(): " << simulated_level_files_in_next_compaction.size() << std::endl;
    for (size_t j = 0; j < simulated_level_files_in_next_compaction.size(); ++j) {
      std::cout << "file " << j << " in level after compaction: ";
      std::cout << "skey: " << KeyToNumber(simulated_level_files_in_next_compaction[j]->smallest);
      std::cout << "lkey: " << KeyToNumber(simulated_level_files_in_next_compaction[j]->largest) << std::endl;
    }

    // find the file with the smallest overlapping bytes and use it to compact
    uint64_t smallest_overlapping_bytes = std::numeric_limits<uint64_t>::max();
    size_t file_index = 0;
    for(size_t j = 0; j < simulated_level_files_in_next_compaction.size(); ++j) {
      uint64_t overlapping_bytes = ComputeOverlappingBytesForFile(simulated_level_files_in_next_compaction[j], simulated_next_level_files_in_next_compaction, icmp);
      if (overlapping_bytes < smallest_overlapping_bytes) {
        smallest_overlapping_bytes = overlapping_bytes;
        file_index = j;
      }
    }
    estimated_written_bytes[i] += ComputeWrittenBytesForCompaction({simulated_level_files_in_next_compaction[file_index]}, simulated_next_level_files_in_next_compaction, icmp);

    // build a vector for this level after the next compaction
    std::vector<std::shared_ptr<SimulatedFileMetaData>> simulated_level_files_after_next_compaction;
    simulated_level_files_after_next_compaction.reserve(level_files.size()-1);
    simulated_level_files_after_next_compaction.insert(simulated_level_files_after_next_compaction.end(), simulated_level_files_in_next_compaction.begin(), simulated_level_files_in_next_compaction.begin() + file_index);
    simulated_level_files_after_next_compaction.insert(simulated_level_files_after_next_compaction.end(), simulated_level_files_in_next_compaction.begin() + file_index + 1, simulated_level_files_in_next_compaction.end());

    // estimate the overlapping bytes again for the previous level to this level
    // now all files in previous needed to be estimated
    std::vector<std::shared_ptr<SimulatedFileMetaData>> simulated_prev_level_files_after_next_compaction;
    simulated_prev_level_files_after_next_compaction.reserve(static_cast<size_t>(op.level0_file_num_compaction_trigger));
    // if the number of files is less than op.level0_file_num_compaction_trigger, assume there will be op.level0_file_num_compaction_trigger files. 
    for(size_t j = 0; j < static_cast<size_t>(op.level0_file_num_compaction_trigger); ++j) {
      simulated_prev_level_files_after_next_compaction.emplace_back(MakeEstimatedLevel0File());
    }
    estimated_written_bytes[i] += ComputeWrittenBytesForCompaction(simulated_prev_level_files_after_next_compaction, simulated_level_files_after_next_compaction, icmp);
  }
  
  uint64_t smallest_overlapping_bytes = std::numeric_limits<uint64_t>::max();
  size_t optimal_file_index = 0;
  for(size_t i = 0; i < level_files.size(); ++i) {
    if (estimated_written_bytes[i] < smallest_overlapping_bytes) {
      smallest_overlapping_bytes = estimated_written_bytes[i];
      optimal_file_index = i;
    }
  }

  return static_cast<int>(optimal_file_index);
}

uint64_t AllFilesEnumerator::ComputeOverlappingBytesForFile(
  const std::shared_ptr<SimulatedFileMetaData>& file, 
  const std::vector<std::shared_ptr<SimulatedFileMetaData>>& next_level_files,
  const InternalKeyComparator& icmp) {
  
  auto next_level_it = next_level_files.begin();
  uint64_t overlapping_bytes = 0;
  // Skip files in next level that is smaller than current file
  while (next_level_it != next_level_files.end() &&
         icmp.Compare((*next_level_it)->largest,
                      file->smallest) < 0) {
    next_level_it++;
  }

  // iterate until the file's smallest is larger than largest key
  while (next_level_it != next_level_files.end() &&
         icmp.Compare((*next_level_it)->smallest,
                      file->largest) < 0) {
    overlapping_bytes += (*next_level_it)->file_size;

    if (icmp.Compare((*next_level_it)->largest,
                     file->largest) > 0) {
      // next level file cross large boundary of current file.
      break;
    }
    next_level_it++;
  }

  return overlapping_bytes;
}

uint64_t AllFilesEnumerator::ComputeOverlappingEntriesForFile(
    const std::shared_ptr<SimulatedFileMetaData>& file, 
    const std::vector<std::shared_ptr<SimulatedFileMetaData>>& next_level_files,
    const InternalKeyComparator& icmp) {
  
  uint64_t overlapping_entries = 0;

  auto next_level_it = next_level_files.begin();
  // Skip files in next level that is smaller than current file
  while (next_level_it != next_level_files.end() &&
         icmp.Compare((*next_level_it)->largest,
                      file->smallest) < 0) {
    next_level_it++;
  }

  // iterate until the file's smallest is larger than largest key
  while (next_level_it != next_level_files.end() &&
         icmp.Compare((*next_level_it)->smallest,
                      file->largest) < 0) {
    overlapping_entries += (*next_level_it)->num_entries;

    if (icmp.Compare((*next_level_it)->largest,
                     file->largest) > 0) {
      // next level file cross large boundary of current file.
      break;
    }
    next_level_it++;
  }

  return overlapping_entries;
}

std::vector<size_t> AllFilesEnumerator::FindOverlappingFiles(
    const std::shared_ptr<SimulatedFileMetaData>& file, 
    const std::vector<std::shared_ptr<SimulatedFileMetaData>>& next_level_files,
    const InternalKeyComparator& icmp) {

  std::vector<size_t> overlapping_files;

  auto next_level_it = next_level_files.begin();
  // Skip files in next level that is smaller than current file
  while (next_level_it != next_level_files.end() &&
         icmp.Compare((*next_level_it)->largest,
                      file->smallest) < 0) {
    next_level_it++;
  }

  // iterate until the file's smallest is larger than largest key
  while (next_level_it != next_level_files.end() &&
         icmp.Compare((*next_level_it)->smallest,
                      file->largest) < 0) {
    overlapping_files.push_back(std::distance(next_level_files.begin(), next_level_it));

    if (icmp.Compare((*next_level_it)->largest,
                     file->largest) > 0) {
      // next level file cross large boundary of current file.
      break;
    }
    next_level_it++;
  }

  return overlapping_files;
}

uint64_t AllFilesEnumerator::ComputeWrittenBytesForCompaction(
    const std::vector<std::shared_ptr<SimulatedFileMetaData>>& input_level_files,
    const std::vector<std::shared_ptr<SimulatedFileMetaData>>& target_level_files,
    const InternalKeyComparator& icmp) {

  // find overlapping files
  std::unordered_set<size_t> overlapping_files_set;
  uint64_t overlapping_entry_num = 0;
  for(size_t i = 0; i < input_level_files.size(); ++i) {
    std::vector<size_t> overlapping_files = FindOverlappingFiles(input_level_files[i], target_level_files, icmp);
    for(size_t j = 0; j < overlapping_files.size(); ++j) {
      overlapping_files_set.insert(overlapping_files[j]);
      overlapping_entry_num += target_level_files[overlapping_files[j]]->num_entries;
    }
  }

  // merge all input files and overlapping files in target level
  uint64_t unique_entry_num = 0;
  for(size_t j = 0; j < input_level_files.size(); ++j) {
    unique_entry_num = Merge(unique_entry_num, input_level_files[j]->num_entries);
  }
  unique_entry_num = Merge(unique_entry_num, overlapping_entry_num);

  double avg_bytes_per_entry = 0;
  if (target_level_files.empty()) {  // this will be a trivial move, so the written bytes is 0
    return 0;
  } else if (input_level_files.empty()) {
    std::cout << "error: input_level_files is empty when computing written bytes for compaction" << std::endl;
  } else {
    avg_bytes_per_entry = (ComputeAvgBytesPerEntry(input_level_files) + ComputeAvgBytesPerEntry(target_level_files)) / 2;
  }
  // std::cout << "unique_entry_num: " << unique_entry_num << ", avg_bytes_per_entry: " << avg_bytes_per_entry << std::endl;
  uint64_t written_bytes = static_cast<uint64_t>(unique_entry_num * avg_bytes_per_entry);
  return written_bytes;
}

double AllFilesEnumerator::ComputeAvgBytesPerEntry(
    const std::vector<std::shared_ptr<SimulatedFileMetaData>>& input_level_files) {

  if (input_level_files.empty()) {
    return 0.0;
  }

  uint64_t total_bytes = 0;
  uint64_t total_entries = 0;

  // std::cout << "input_level_files.size(): " << input_level_files.size() << std::endl;
  for(size_t i = 0; i < input_level_files.size(); ++i) {
    // std::cout << "input_level_files[" << i << "]: " << "num_entries: " << input_level_files[i]->num_entries << ", file_size: " << input_level_files[i]->file_size << std::endl;
    total_bytes += input_level_files[i]->file_size;
    total_entries += input_level_files[i]->num_entries;
  }
  // std::cout << "total_bytes: " << total_bytes << ", total_entries: " << total_entries << std::endl;
  return static_cast<double>(total_bytes) / total_entries;
}

std::shared_ptr<SimulatedFileMetaData> AllFilesEnumerator::MakeEstimatedLevel0File() {
  return std::make_shared<SimulatedFileMetaData>(smallest_key_, largest_key_, entry_num_collector.Avg(), deletion_num_collector.Avg(), file_size_collector.Avg());
}

uint64_t AllFilesEnumerator::KeyToNumber(const InternalKey& key) {
  uint64_t num = 0;
  uint64_t base = 1;
  std::string_view key_str = key.user_key().ToStringView();
  for(int i = static_cast<int>(key_str.length()-1); i >= 0; --i) {
    num += static_cast<uint64_t>(static_cast<unsigned char>(key_str[i])) * base;
    base *= 256;
  }
  return num;
}

InternalKey AllFilesEnumerator::NumberToKey(uint64_t num) {
  std::string key_str;
  uint64_t base = std::pow(256, 7);
  bool started = false;
  for(size_t i = 0; i < 8; ++i) {
    uint64_t c = num / base;
    num -= c * base;
    base /= 256;
    if (c == 0 && !started) {
      continue;
    }
    started = true;
    key_str.push_back(static_cast<char>(c));
  }
  InternalKey key(Slice(key_str), 0, kTypeValue);
  return key;
}

uint64_t AllFilesEnumerator::Unique(uint64_t request_num) {
  return key_space_size_ * (1 - std::pow(1 - 1.0 / key_space_size_, request_num));
}

uint64_t AllFilesEnumerator::UniqueInversed(uint64_t unique_num) {
  return static_cast<uint64_t>(std::log(static_cast<double>(key_space_size_) / (key_space_size_-unique_num)) / std::log(static_cast<double>(key_space_size_) / (key_space_size_-1)));
}

uint64_t AllFilesEnumerator::Merge(uint64_t unique_num1, uint64_t unique_num2) {
  return Unique(UniqueInversed(unique_num1) + UniqueInversed(unique_num2));
}

void AllFilesEnumerator::SetKeySpaceSize(uint64_t size) {
  key_space_size_ = size;
}

void AllFilesEnumerator::CollectInfo(uint64_t entry_num, uint64_t deletion_num, uint64_t file_size) {
  entry_num_collector.Add(entry_num);
  deletion_num_collector.Add(deletion_num);
  file_size_collector.Add(file_size);
}

std::vector<std::shared_ptr<SimulatedFileMetaData>> AllFilesEnumerator::Compaction(
  const std::vector<std::shared_ptr<SimulatedFileMetaData>>& input_files, 
  const std::vector<std::shared_ptr<SimulatedFileMetaData>>& target_level_files,
  size_t target_level, const InternalKeyComparator& icmp, const MutableCFOptions& op) {
  
  // std::cout << "input_files.size(): " << input_files.size() << std::endl;
  // std::cout << "target_level_files.size(): " << target_level_files.size() << std::endl;

  // compute the average entry size in all files
  uint64_t avg_entry_size = 0;
  uint64_t total_size = 0;
  uint64_t total_entry_num = 0;
  for(const std::shared_ptr<SimulatedFileMetaData>& f: input_files) {
    total_size += f->file_size;
    total_entry_num += f->num_entries;
  }
  avg_entry_size = total_size / total_entry_num;

  // there is only two cases now
  // one is that the input files are from level-0 which are overlapping with each other, so we can simply merge them.
  // the other is that the input files are from level-1 which should be only one file
  std::shared_ptr<SimulatedFileMetaData> input_file;
  if (input_files.size() == 1) { // from level-1
    input_file = input_files[0];
  } else {
    uint64_t merged_entry_num = 0;
    uint64_t merged_deletion_num = 0;
    InternalKey skey = largest_key_;
    InternalKey lkey = smallest_key_;
    for(const std::shared_ptr<SimulatedFileMetaData>& f: input_files) {
      // std::cout << "f->num_entries: " << f->num_entries << std::endl;
      merged_entry_num = Merge(merged_entry_num, f->num_entries);
      merged_deletion_num = Merge(merged_deletion_num, f->num_deletions);
      if (icmp.Compare(skey, f->smallest) > 0) {
        skey = f->smallest;
      }
      if (icmp.Compare(lkey, f->largest) < 0) {
        lkey = f->largest;
      }
    }

    // std::cout << "skey: " << skey.DebugString(false) << std::endl;
    // std::cout << "lkey: " << lkey.DebugString(false) << std::endl;
    // std::cout << "merged_entry_num: " << merged_entry_num << std::endl;

    input_file = std::make_shared<SimulatedFileMetaData>(skey, lkey, merged_entry_num, merged_deletion_num, merged_entry_num * avg_entry_size);
  }
  // std::cout << "input_file: " << "skey: " << KeyToNumber(input_file->smallest) << ", lkey: " << KeyToNumber(input_file->largest) << ", num_entries: " << input_file->num_entries << ", file_size: " << input_file->file_size << std::endl;

  // compute average number of entries, deletions in target level
  uint64_t avg_num_entries = 0;
  uint64_t avg_num_deletions = 0;
  for(const std::shared_ptr<SimulatedFileMetaData>& f: target_level_files) {
    avg_num_entries += f->num_entries;
    avg_num_deletions += f->num_deletions;
  }
  avg_num_entries /= target_level_files.size();
  avg_num_deletions /= target_level_files.size();
  // std::cout << "avg_num_entries: " << avg_num_entries << std::endl;
  // std::cout << "avg_num_deletions: " << avg_num_deletions << std::endl;

  // now there is only one file to merge
  std::vector<size_t> overlapping_files = FindOverlappingFiles(input_file, target_level_files, icmp);
  // std::cout << "overlapping_files.size(): " << overlapping_files.size() << std::endl;
  if (overlapping_files.size() == 0) {  // if there is no overlapping files, we just insert the input file to the target level
    // find the first file in target level that is smaller than input file
    auto it = std::upper_bound(target_level_files.begin(), target_level_files.end(), input_file, 
      [&icmp](const std::shared_ptr<SimulatedFileMetaData>& f1, const std::shared_ptr<SimulatedFileMetaData>& f2) {
        return icmp.Compare(f1->largest, f2->smallest) < 0;
      });
    // std::cout << "it: " << std::distance(target_level_files.begin(), it) << std::endl;
    // insert the input file to the target place
    std::vector<std::shared_ptr<SimulatedFileMetaData>> files_after_compaction = target_level_files;
    if (it == target_level_files.end()) {
      files_after_compaction.emplace_back(input_file);
    } else {
      files_after_compaction.insert(it, input_file);
    }
    return files_after_compaction;
  }
  uint64_t total_file_size_after_compaction = ComputeWrittenBytesForCompaction({input_file}, target_level_files, icmp);
  uint64_t file_size = op.target_file_size_base * std::pow(op.target_file_size_multiplier, target_level);
  int file_num = std::ceil(static_cast<double>(total_file_size_after_compaction) / file_size);
  // for (size_t i = 0; i < overlapping_files.size(); ++i) {
  //   std::cout << "overlapping_files[" << i << "]: " << overlapping_files[i];
  //   std::cout << " skey: " << KeyToNumber(target_level_files[overlapping_files[i]]->smallest);
  //   std::cout << " lkey: " << KeyToNumber(target_level_files[overlapping_files[i]]->largest);
  //   std::cout << " size: " << target_level_files[overlapping_files[i]]->file_size << std::endl;
  // }
  // std::cout << "total_file_size_after_compaction: " << total_file_size_after_compaction << std::endl;
  // std::cout << "file_size: " << file_size << std::endl;
  // std::cout << "file_num: " << file_num << std::endl;

  uint64_t input_start_key = KeyToNumber(input_file->smallest);
  uint64_t input_end_key = KeyToNumber(input_file->largest);
  uint64_t input_file_size = input_file->file_size;
  uint64_t target_start_key = KeyToNumber(target_level_files[overlapping_files[0]]->smallest);
  uint64_t target_end_key = KeyToNumber(target_level_files[overlapping_files[0]]->largest);
  uint64_t target_file_size = target_level_files[overlapping_files[0]]->file_size;
  size_t target_file_index = overlapping_files[0];

  // some variables for computing
  std::vector<std::shared_ptr<SimulatedFileMetaData>> files_after_compaction;
  for(int i = 0; i < file_num; ++i) {
    // std::cout << "generating file " << i << " after compaction" << std::endl;
    // std::cout << "input_start_key: " << input_start_key << std::endl;
    // std::cout << "input_end_key: " << input_end_key << std::endl;
    // std::cout << "target_start_key: " << target_start_key << std::endl;
    // std::cout << "target_end_key: " << target_end_key << std::endl;
    uint64_t input_file_key_distance = input_end_key - input_start_key;
    // std::cout << "input_file_key_distance: " << input_file_key_distance << std::endl;
    uint64_t target_file_key_distance = target_end_key - target_start_key;
    // std::cout << "target_file_key_distance: " << target_file_key_distance << std::endl;
    double a = static_cast<double>(input_file_key_distance) / input_file_size;
    double b = file_size * a;
    // compute the possible end key
    double target_file_ratio = static_cast<double>(b + input_start_key - target_start_key) / (target_file_key_distance + target_file_size * a);
    double input_file_ratio = (file_size - target_file_size * target_file_ratio) / input_file_size;

    // std::cout << "target_file_ratio: " << target_file_ratio << std::endl;
    // std::cout << "input_file_ratio: " << input_file_ratio << std::endl;
    
    if (target_file_ratio < 1 && input_file_ratio < 1) {  // a normal merge
      uint64_t end_key = static_cast<uint64_t>(target_file_ratio * target_file_key_distance) + target_start_key;
      // make a file
      InternalKey skey = NumberToKey(std::min(input_start_key, target_start_key));
      InternalKey lkey = NumberToKey(end_key);
      files_after_compaction.emplace_back(std::make_shared<SimulatedFileMetaData>(skey, lkey, avg_num_entries, avg_num_deletions, target_file_size));

      input_start_key = end_key + 1;
      target_start_key = end_key + 1;
      input_file_size = static_cast<uint64_t>((1 - input_file_ratio) * input_file_size);
      target_file_size = static_cast<uint64_t>((1 - target_file_ratio) * target_file_size);
    } else if (target_file_ratio >= 1 && input_file_ratio >= 1) {  // the last merge
      InternalKey skey = NumberToKey(std::min(input_start_key, target_start_key));
      InternalKey lkey = NumberToKey(std::max(input_end_key, target_end_key));
      files_after_compaction.emplace_back(std::make_shared<SimulatedFileMetaData>(skey, lkey, avg_num_entries, avg_num_deletions, total_file_size_after_compaction%file_size));
      break;
    } else if (target_file_ratio >= 1) {  // target file is used out
      // compute the left file size
      uint64_t start_key = std::min(input_start_key, target_start_key);
      uint64_t left_size = file_size - (static_cast<uint64_t>(static_cast<double>(target_end_key - input_start_key) / input_file_key_distance * input_file_size) + target_file_size);
      // std::cout << "left_size: " << left_size << std::endl;
      // update input file info
      input_file_size = static_cast<uint64_t>((1 - static_cast<double>(target_end_key - input_start_key) / input_file_key_distance) * input_file_size);
      input_start_key = target_end_key + 1;
      // switch to the next target file
      target_file_index++;
      // std::cout << "target_file_index: " << target_file_index << std::endl;
      // check whether all target files are used out
      if (target_file_index == overlapping_files.back() + 1) {
        // use the input file to fill the left size
        uint64_t end_key = static_cast<double>(left_size) / input_file_size * input_file_key_distance + target_end_key;
        InternalKey skey = NumberToKey(start_key);
        InternalKey lkey = NumberToKey(end_key);
        files_after_compaction.emplace_back(std::make_shared<SimulatedFileMetaData>(skey, lkey, avg_num_entries, avg_num_deletions, file_size));
        // use the left input file to make the last file
        skey = NumberToKey(end_key+1);
        lkey = NumberToKey(input_end_key);
        // std::cout << "input_file_size-left_size: " << input_file_size-left_size << std::endl;
        files_after_compaction.emplace_back(std::make_shared<SimulatedFileMetaData>(skey, lkey, avg_num_entries, avg_num_deletions, input_file_size-left_size));
        break;
      }
      target_start_key = KeyToNumber(target_level_files[target_file_index]->smallest);
      target_end_key = KeyToNumber(target_level_files[target_file_index]->largest);
      target_file_size = target_level_files[target_file_index]->file_size;
      // std::cout << "target_file_index: " << target_file_index << std::endl;
      // std::cout << "input_start_key: " << input_start_key << std::endl;
      // std::cout << "input_end_key: " << input_end_key << std::endl;
      // std::cout << "target_start_key: " << target_start_key << std::endl;
      // std::cout << "target_end_key: " << target_end_key << std::endl;
      input_file_key_distance = input_end_key - input_start_key;
      target_file_key_distance = target_end_key - target_start_key;
      // check whether the target_start_key has surpassed left_size
      uint64_t size_before_target_file = (static_cast<double>(target_start_key - input_start_key) / input_file_key_distance) * input_file_size;
      // std::cout << "size_before_target_file: " << size_before_target_file << std::endl;
      if (size_before_target_file > left_size) {
        uint64_t end_key = static_cast<double>(left_size) / input_file_size * input_file_key_distance + input_start_key;
        // make a file
        InternalKey skey = NumberToKey(start_key);
        InternalKey lkey = NumberToKey(end_key);
        files_after_compaction.emplace_back(std::make_shared<SimulatedFileMetaData>(skey, lkey, avg_num_entries, avg_num_deletions, target_file_size));
        // update
        input_file_size = static_cast<uint64_t>((1 - static_cast<double>(end_key - input_start_key) / input_file_key_distance) * input_file_size);
        input_start_key = end_key + 1;
        continue;
      }
      // compute the ratio again with the new target file and new file size
      a = static_cast<double>(input_file_key_distance) / input_file_size;
      b = left_size * a;
      // compute the possible end key
      target_file_ratio = static_cast<double>(b + input_start_key - target_start_key) / (target_file_key_distance + target_file_size * a);
      input_file_ratio = (left_size - target_file_size * target_file_ratio) / input_file_size;
      // std::cout << "target_file_ratio: " << target_file_ratio << std::endl;
      // std::cout << "input_file_ratio: " << input_file_ratio << std::endl;

      // we still need to check whether the input file is used out
      if (input_file_ratio < 1) {
        uint64_t end_key = static_cast<uint64_t>(target_file_ratio * target_file_key_distance) + target_start_key;
        // make a file
        InternalKey skey = NumberToKey(start_key);
        InternalKey lkey = NumberToKey(end_key);
        files_after_compaction.emplace_back(std::make_shared<SimulatedFileMetaData>(skey, lkey, avg_num_entries, avg_num_deletions, file_size));
        // update
        input_start_key = end_key + 1;
        target_start_key = end_key + 1;
        input_file_size = static_cast<uint64_t>((1 - input_file_ratio) * input_file_size);
        target_file_size = static_cast<uint64_t>((1 - target_file_ratio) * target_file_size);
      } else {
        // compute the left file size
        left_size = left_size - (static_cast<uint64_t>(static_cast<double>(input_end_key - target_start_key) / target_file_key_distance * target_file_size) + input_file_size);
        // compute the end key in target file with the left file size
        uint64_t end_key = static_cast<double>(left_size) / target_file_size * target_file_key_distance + input_end_key;
        // make a file
        InternalKey skey = NumberToKey(start_key);
        InternalKey lkey = NumberToKey(end_key);
        files_after_compaction.emplace_back(std::make_shared<SimulatedFileMetaData>(skey, lkey, avg_num_entries, avg_num_deletions, file_size));
        // make the last file
        skey = NumberToKey(end_key+1);
        lkey = NumberToKey(target_end_key);
        files_after_compaction.emplace_back(std::make_shared<SimulatedFileMetaData>(skey, lkey, avg_num_entries, avg_num_deletions, target_file_size-left_size));
        break;
      }
    } else {  // input file is used out
      // compute the left file size
      uint64_t start_key = std::min(input_start_key, target_start_key);
      uint64_t left_size = file_size - (static_cast<uint64_t>(static_cast<double>(input_end_key - target_start_key) / target_file_key_distance * target_file_size) + input_file_size);
      // compute the end key in target file with the left file size
      uint64_t end_key = static_cast<double>(left_size) / target_file_size * target_file_key_distance + input_end_key;
      // make a file
      InternalKey skey = NumberToKey(start_key);
      InternalKey lkey = NumberToKey(end_key);
      files_after_compaction.emplace_back(std::make_shared<SimulatedFileMetaData>(skey, lkey, avg_num_entries, avg_num_deletions, file_size));
      // make the last file
      skey = NumberToKey(end_key+1);
      lkey = NumberToKey(target_end_key);
      files_after_compaction.emplace_back(std::make_shared<SimulatedFileMetaData>(skey, lkey, avg_num_entries, avg_num_deletions, target_file_size-left_size));
      break;
    }
  }

  files_after_compaction.insert(files_after_compaction.begin(), target_level_files.begin(), target_level_files.begin()+overlapping_files[0]);
  files_after_compaction.insert(files_after_compaction.end(), target_level_files.begin()+overlapping_files.back()+1, target_level_files.end());

  return files_after_compaction;
}

void AllFilesEnumerator::CollectCompactionInfo(
    std::vector<FileMetaData*>* files,
    const std::vector<uint64_t>& file_overlapping_ratio,
    int num_level, int level, int index) {
  // compute hash of the current version
  static std::hash<std::vector<ROCKSDB_NAMESPACE::Fsize>>
      hasher;
  std::vector<Fsize> temp(files[level].size());
  for (size_t i = 0; i < files[level].size(); i++) {
    temp[i].index = i;
    temp[i].file = files[level][i];
  }
  std::size_t hash_value = hasher(temp);
  // collect this seletion
  collector.CollectCompactionInfo(
      level, files, file_overlapping_ratio, num_level,
      hash_value, index);
  // log
  if (log_level == 2) {
    CS561Log::LogRegularCompactionInfo(
        collector.GetCompactionsInfo().back(),
        static_cast<int>(collector.GetCompactionsInfo().size()));
  }
}

int AllFilesEnumerator::NextChoiceForManual() {
  return static_cast<int>(manual_list[compaction_counter++]);
}

void AllFilesEnumerator::SetManualList(
    const std::vector<size_t>& ml) {
  manual_list = ml;
}

PickingHistoryCollector&
AllFilesEnumerator::GetCollector() {
  return collector;
}

void AllFilesEnumerator::Terminate() {
  // Before termination, dump version forests to file
  CS561Log::Log("Start to dump version forests to file");
  collector.GetVersionForest().DumpToFile();
  // record WA and minimum in this run
  // collector.DumpWAResult();
  // Log
  // CS561Log::Log("Terminate program due to early stop");
  // Terminate program
  CS561Log::Log("Terminate program");
  _exit(1);
}

void AllFilesEnumerator::Pruning() {
  // Check whether current WA already exceeds global min
  if (!collector.CheckContinue()) {
    // Set the current version to be fully enumerated (no need to explore further)
    collector.GetVersionForest().GetLevelVersionTree(1).SetCurrentVersionFullyEnumerated(chosen_file_index_);
    // set the flag of the current node
    CS561Log::Log("Terminate reason: current WA already exceeds the minimum");
    // check other file choices in the same compaction
    uint64_t threshold = involved_bytes_[chosen_file_index_];
    for (size_t i = chosen_file_index_ + 1; i < involved_bytes_.size(); i++) {
      if (involved_bytes_[i] >= threshold) {
        // set the flag of the current node
        CS561Log::Log("Set file " + std::to_string(i) + " of last compaction to be fully enumerated");
        // Set the current version to be fully enumerated (no need to explore further)
        collector.GetVersionForest().GetLevelVersionTree(1).SetCurrentVersionFullyEnumerated(i);
      }
    }
    Terminate();
  }
}

int AllFilesEnumerator::GetLogLevel() {
  return log_level;
}

void AllFilesEnumerator::SetLogLevel(int log_level_) {
  log_level = log_level_;
}

bool AllFilesEnumerator::CheckFinishEnumeration() {
  return false;
}

void AllFilesEnumerator::SetKeyRange(const InternalKey& smallest_key,
                                     const InternalKey& largest_key) {
  smallest_key_ = smallest_key;
  largest_key_ = largest_key;
}

}  // namespace ROCKSDB_NAMESPACE