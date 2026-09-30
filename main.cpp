#include "md5.hpp"

#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <istream>
#include <string>

namespace {

bool hash_stream(std::istream& input, Md5& hash) {
  std::array<char, 8192> buffer{};
  while (input.read(buffer.data(), static_cast<std::streamsize>(buffer.size())) || input.gcount() > 0) {
    const auto bytes_read = static_cast<std::size_t>(input.gcount());
    hash.update(reinterpret_cast<const std::uint8_t*>(buffer.data()), bytes_read);
  }
  return input.eof();
}

}  // namespace

int main(int argc, char* argv[]) {
  if (argc > 2) {
    std::cerr << "Usage: ./md5 [file-path]\n";
    return 1;
  }

  Md5 hash;
  if (argc == 2) {
    const std::string path = argv[1];
    std::ifstream file(path, std::ios::binary);
    if (!file) {
      std::cerr << "Unable to read file: " << path << '\n';
      return 1;
    }
    if (!hash_stream(file, hash)) {
      std::cerr << "Error while reading file: " << path << '\n';
      return 1;
    }
  } else if (!hash_stream(std::cin, hash)) {
    std::cerr << "Error while reading standard input\n";
    return 1;
  }

  std::cout << hash.finalize_hex() << '\n';
  return 0;
}
