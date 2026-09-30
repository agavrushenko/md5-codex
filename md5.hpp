#ifndef MD5_HPP
#define MD5_HPP

#include <cstddef>
#include <cstdint>
#include <string>

class Md5 {
 public:
  Md5();

  // Adds raw message bytes. Throws std::logic_error after finalization.
  void update(const std::uint8_t* data, std::size_t length);

  // Finalizes once and returns the same lowercase digest on later calls.
  std::string finalize_hex();

 private:
  void process_block(const std::uint8_t block[64]);

  std::uint32_t state_[4];
  std::uint8_t buffer_[64];
  std::size_t buffered_bytes_;
  std::uint64_t total_bytes_;
  bool finalized_;
  std::string digest_;
};

#endif
