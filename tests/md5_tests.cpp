#include "md5.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::string digest(const std::string& input) {
  Md5 hash;
  hash.update(reinterpret_cast<const std::uint8_t*>(input.data()), input.size());
  return hash.finalize_hex();
}

std::string incremental_digest(const std::string& input) {
  Md5 hash;
  const std::array<std::size_t, 5> chunk_sizes{1, 7, 2, 19, 3};
  std::size_t offset = 0;
  std::size_t chunk_index = 0;

  while (offset < input.size()) {
    const std::size_t length = std::min(chunk_sizes[chunk_index % chunk_sizes.size()],
                                        input.size() - offset);
    hash.update(reinterpret_cast<const std::uint8_t*>(input.data() + offset), length);
    offset += length;
    ++chunk_index;
  }

  return hash.finalize_hex();
}

int run_command(const std::string& command, const std::filesystem::path& output_path) {
  return std::system((command + " > \"" + output_path.string() + "\" 2>&1").c_str());
}

std::string read_file(const std::filesystem::path& path) {
  std::ifstream input(path, std::ios::binary);
  std::ostringstream content;
  content << input.rdbuf();
  return content.str();
}

void test_rfc_vectors() {
  const std::vector<std::pair<std::string, std::string>> vectors{
      {"", "d41d8cd98f00b204e9800998ecf8427e"},
      {"a", "0cc175b9c0f1b6a831c399e269772661"},
      {"abc", "900150983cd24fb0d6963f7d28e17f72"},
      {"message digest", "f96b697d7cb7938d525a2f31aaf161d0"},
      {"abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b"},
      {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
       "d174ab98d277d9f5a5611c2c9f419d9f"},
      {"12345678901234567890123456789012345678901234567890123456789012345678901234567890",
       "57edf4a22be3c955ac49da2e2107b67a"},
  };

  for (const auto& [input, expected] : vectors) {
    assert(digest(input) == expected);
    assert(incremental_digest(input) == expected);
  }
}

void test_padding_boundaries_and_finalization() {
  for (const std::size_t length : {55U, 56U, 63U, 64U, 65U}) {
    const std::string input(length, 'x');
    assert(digest(input) == incremental_digest(input));
  }

  Md5 hash;
  hash.update(reinterpret_cast<const std::uint8_t*>("abc"), 3);
  const std::string first_digest = hash.finalize_hex();
  assert(first_digest == hash.finalize_hex());

  bool rejected_update = false;
  try {
    hash.update(reinterpret_cast<const std::uint8_t*>("x"), 1);
  } catch (const std::logic_error&) {
    rejected_update = true;
  }
  assert(rejected_update);
}

void test_command_line() {
  const std::filesystem::path temporary_directory = "tests/.md5_test_tmp";
  std::filesystem::create_directories(temporary_directory);
  const std::filesystem::path binary_input = temporary_directory / "binary input.bin";
  const std::filesystem::path command_output = temporary_directory / "command output.txt";

  {
    std::ofstream output(binary_input, std::ios::binary);
    const std::array<char, 5> bytes{'a', '\0', 'b', '\n', 'c'};
    output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
  }

  assert(run_command("printf %s abc | ./app", command_output) == 0);
  assert(read_file(command_output) == "900150983cd24fb0d6963f7d28e17f72\n");

  assert(run_command("./app \"" + binary_input.string() + "\"", command_output) == 0);
  assert(read_file(command_output) == "2a35356f1148b99c7da3553648c10ec6\n");

  assert(run_command("./app tests/.md5_test_tmp/missing", command_output) != 0);
  assert(!read_file(command_output).empty());

  assert(run_command("./app one two", command_output) != 0);
  assert(read_file(command_output).find("Usage: ./app [file-path]") != std::string::npos);

  std::filesystem::remove_all(temporary_directory);
}

}  // namespace

int main() {
  test_rfc_vectors();
  test_padding_boundaries_and_finalization();
  test_command_line();
  std::cout << "All MD5 tests passed.\n";
  return 0;
}
