#include "md5.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <stdexcept>

namespace {

constexpr std::array<std::uint32_t, 64> k_constants{
    0xd76aa478U, 0xe8c7b756U, 0x242070dbU, 0xc1bdceeeU, 0xf57c0fafU, 0x4787c62aU,
    0xa8304613U, 0xfd469501U, 0x698098d8U, 0x8b44f7afU, 0xffff5bb1U, 0x895cd7beU,
    0x6b901122U, 0xfd987193U, 0xa679438eU, 0x49b40821U, 0xf61e2562U, 0xc040b340U,
    0x265e5a51U, 0xe9b6c7aaU, 0xd62f105dU, 0x02441453U, 0xd8a1e681U, 0xe7d3fbc8U,
    0x21e1cde6U, 0xc33707d6U, 0xf4d50d87U, 0x455a14edU, 0xa9e3e905U, 0xfcefa3f8U,
    0x676f02d9U, 0x8d2a4c8aU, 0xfffa3942U, 0x8771f681U, 0x6d9d6122U, 0xfde5380cU,
    0xa4beea44U, 0x4bdecfa9U, 0xf6bb4b60U, 0xbebfbc70U, 0x289b7ec6U, 0xeaa127faU,
    0xd4ef3085U, 0x04881d05U, 0xd9d4d039U, 0xe6db99e5U, 0x1fa27cf8U, 0xc4ac5665U,
    0xf4292244U, 0x432aff97U, 0xab9423a7U, 0xfc93a039U, 0x655b59c3U, 0x8f0ccc92U,
    0xffeff47dU, 0x85845dd1U, 0x6fa87e4fU, 0xfe2ce6e0U, 0xa3014314U, 0x4e0811a1U,
    0xf7537e82U, 0xbd3af235U, 0x2ad7d2bbU, 0xeb86d391U,
};

constexpr std::array<unsigned int, 64> k_rotations{
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21,
};

std::uint32_t load_le32(const std::uint8_t* bytes) {
  return static_cast<std::uint32_t>(bytes[0]) |
         (static_cast<std::uint32_t>(bytes[1]) << 8U) |
         (static_cast<std::uint32_t>(bytes[2]) << 16U) |
         (static_cast<std::uint32_t>(bytes[3]) << 24U);
}

void store_le32(std::uint8_t* bytes, std::uint32_t value) {
  for (std::size_t index = 0; index < 4; ++index) {
    bytes[index] = static_cast<std::uint8_t>(value >> (index * 8U));
  }
}

void store_le64(std::uint8_t* bytes, std::uint64_t value) {
  for (std::size_t index = 0; index < 8; ++index) {
    bytes[index] = static_cast<std::uint8_t>(value >> (index * 8U));
  }
}

std::uint32_t rotate_left(std::uint32_t value, unsigned int amount) {
  return (value << amount) | (value >> (32U - amount));
}

}  // namespace

Md5::Md5()
    : state_{0x67452301U, 0xefcdab89U, 0x98badcfeU, 0x10325476U},
      buffer_{},
      buffered_bytes_(0),
      total_bytes_(0),
      finalized_(false) {}

void Md5::update(const std::uint8_t* data, std::size_t length) {
  if (finalized_) {
    throw std::logic_error("cannot update a finalized MD5 digest");
  }
  if (length == 0) {
    return;
  }
  if (data == nullptr) {
    throw std::invalid_argument("MD5 input cannot be null when length is nonzero");
  }

  total_bytes_ += static_cast<std::uint64_t>(length);
  std::size_t offset = 0;

  if (buffered_bytes_ > 0) {
    const std::size_t copied = std::min(length, 64U - buffered_bytes_);
    std::memcpy(buffer_ + buffered_bytes_, data, copied);
    buffered_bytes_ += copied;
    offset += copied;
    if (buffered_bytes_ == 64) {
      process_block(buffer_);
      buffered_bytes_ = 0;
    }
  }

  while (length - offset >= 64) {
    process_block(data + offset);
    offset += 64;
  }

  const std::size_t remaining = length - offset;
  if (remaining > 0) {
    std::memcpy(buffer_, data + offset, remaining);
    buffered_bytes_ = remaining;
  }
}

void Md5::process_block(const std::uint8_t block[64]) {
  std::array<std::uint32_t, 16> words{};
  for (std::size_t index = 0; index < words.size(); ++index) {
    words[index] = load_le32(block + index * 4);
  }

  std::uint32_t a = state_[0];
  std::uint32_t b = state_[1];
  std::uint32_t c = state_[2];
  std::uint32_t d = state_[3];

  for (std::size_t index = 0; index < 64; ++index) {
    std::uint32_t function = 0;
    std::size_t word_index = 0;
    if (index < 16) {
      function = (b & c) | (~b & d);
      word_index = index;
    } else if (index < 32) {
      function = (d & b) | (~d & c);
      word_index = (5 * index + 1) % 16;
    } else if (index < 48) {
      function = b ^ c ^ d;
      word_index = (3 * index + 5) % 16;
    } else {
      function = c ^ (b | ~d);
      word_index = (7 * index) % 16;
    }

    const std::uint32_t next_d = d;
    d = c;
    c = b;
    b += rotate_left(a + function + k_constants[index] + words[word_index], k_rotations[index]);
    a = next_d;
  }

  state_[0] += a;
  state_[1] += b;
  state_[2] += c;
  state_[3] += d;
}

std::string Md5::finalize_hex() {
  if (finalized_) {
    return digest_;
  }

  const std::uint64_t bit_length = total_bytes_ * 8U;
  const std::uint8_t first_padding_byte = 0x80U;
  update(&first_padding_byte, 1);

  const std::array<std::uint8_t, 64> zeros{};
  const std::size_t padding_length = buffered_bytes_ <= 56 ? 56 - buffered_bytes_ : 120 - buffered_bytes_;
  update(zeros.data(), padding_length);

  std::array<std::uint8_t, 8> length_bytes{};
  store_le64(length_bytes.data(), bit_length);
  update(length_bytes.data(), length_bytes.size());

  std::array<std::uint8_t, 16> digest_bytes{};
  for (std::size_t index = 0; index < 4; ++index) {
    store_le32(digest_bytes.data() + index * 4, state_[index]);
  }

  constexpr char hex[] = "0123456789abcdef";
  digest_.reserve(32);
  for (const std::uint8_t byte : digest_bytes) {
    digest_ += hex[byte >> 4U];
    digest_ += hex[byte & 0x0fU];
  }
  finalized_ = true;
  return digest_;
}
