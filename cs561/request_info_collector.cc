#include "cs561/request_info_collector.h"

namespace rocksdb {

RequestInfoCollector::RequestInfoCollector(size_t queue_length) : queue_length_(queue_length) {}

void RequestInfoCollector::Add(uint64_t e) {
  q_.push(e);
  if (q_.size() > queue_length_) {
    q_.pop();
  }
}

uint64_t RequestInfoCollector::Avg() {
  std::queue<uint64_t> q = q_;
  uint64_t sum = 0;
  while (!q.empty()) {
      sum += q.front();
      q.pop();
  }
  sum /= q_.size();
  return sum;
}

}
