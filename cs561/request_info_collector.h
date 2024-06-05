#pragma once

#include <queue>
#include <cstdint>

namespace rocksdb {

class RequestInfoCollector{
public:
  RequestInfoCollector(size_t queue_length);
  void Add(uint64_t e);
  uint64_t Avg();

private:
  size_t queue_length_;
  std::queue<uint64_t> q_;
};

}
