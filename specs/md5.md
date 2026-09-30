# Spec: C++ MD5 Command-Line Utility

## Objective

Implement a self-contained C++ program that computes an MD5 digest from either standard input or one file. It is intended for coursework and learning, not security-sensitive integrity, authentication, or signature use.

### User-facing behavior

```text
./md5              # hash all raw bytes from stdin through EOF
./md5 <file-path>  # hash all raw bytes in the named file
```

- No arguments hashes every stdin byte through EOF, including whitespace, newlines, and NUL bytes.
- One argument is a literal file path, read in binary mode.
- More than one argument writes `Usage: ./md5 [file-path]` to standard error and returns nonzero.
- An unreadable file reports an error to standard error and returns nonzero.
- Success output is only a lowercase, 32-character MD5 digest followed by a newline; never a filename.
- `-` is an ordinary filename, not a stdin alias.

## Tech Stack

- C++17 standard library only; no external hashing library.
- `g++`, matching the existing project script.
- RFC 1321 algorithm and test vectors.

## Commands

Build:

```bash
make
```

Test stdin:

```bash
printf %s abc | ./md5
```

Expected output:

```text
900150983cd24fb0d6963f7d28e17f72
```

Test a file:

```bash
./md5 path/to/input
```

Build unit tests:

```bash
make md5_tests
./md5_tests
```

Run all checks:

```bash
make test
```

The equivalent shell runner is `./test_runner.sh`. Remove generated executables
with `make clean`.

## Project Structure

```text
main.cpp            Command-line parsing, stream selection, and output
md5.hpp             Public incremental MD5 interface
md5.cpp             MD5 block processing, padding, and hex encoding
Makefile            Builds the executable and runs tests
tests/md5_tests.cpp Algorithm and command-line behavior tests
test_runner.sh       Builds and runs the application and tests
specs/md5.md        This specification
```

## Code Style

- Use `std::uint32_t` and `std::uint64_t` for MD5 state and length arithmetic.
- Keep MD5 independent of files, arguments, and terminal I/O.
- Use `PascalCase` for `Md5`; use `snake_case` for functions and local variables.
- Stream fixed-size chunks rather than loading entire files.
- Use explicit little-endian helpers rather than host byte order.

```cpp
Md5 hash;
hash.update(buffer.data(), bytes_read);
const std::string digest = hash.finalize_hex();
```

## Algorithm Requirements

`Md5` retains four 32-bit state words, a partial 64-byte block, buffered-byte count, and total byte count. `update()` accepts arbitrary chunks and processes complete blocks as available.

The compression function must decode sixteen little-endian 32-bit words, run all four RFC 1321 rounds using their specified constants and rotation counts, rely only on defined unsigned 32-bit wraparound, and serialize the four final state words little-endian.

Finalization appends `0x80`, zero bytes until the message is 56 bytes modulo 64, then the original bit length as a 64-bit little-endian value modulo `2^64`. It returns lowercase hexadecimal. The class must document and test finalization reuse; preferred behavior is idempotent repeated finalization and rejection of subsequent updates.

## Testing Strategy

Tests reside in `tests/md5_tests.cpp` and use an assertion-based standard-library harness.

- RFC 1321 inputs: empty, `a`, `abc`, `message digest`, alphabet, alphanumeric sequence, and 80-character numeric sequence.
- Equivalent digests for one-shot and uneven incremental updates.
- Padding boundaries: 55, 56, 63, 64, and 65 bytes.
- Finalization behavior defined by the class interface.
- Stdin with no appended newline, plus a binary file containing a NUL byte and newline.
- Missing-path, unreadable-input, and multiple-argument CLI errors.
- A binary-file digest cross-check against `md5sum` when that command is available.

The test script must build application and unit-test executables separately, and invoke the application as `./md5` (not `md5`).

## Implementation Plan

1. Add `md5.hpp` with a small incremental `Md5` interface.
2. Implement block compression, buffering, padding, and hexadecimal output in `md5.cpp`; first verify RFC vectors.
3. Replace the placeholder `main.cpp` with positional input selection and chunked stream hashing.
4. Add tests and update `test_runner.sh` to build and run every check.

Tasks 1–3 are sequential: the CLI depends on the hash interface. Unit tests can accompany each step; command-line tests depend on the completed CLI.

## Boundaries

- Always: preserve every byte, validate argument count and file I/O, compile with warnings enabled, and run all tests before committing.
- Ask first: add dependencies, alter CI/container configuration, change the CLI contract, or introduce another build system.
- Never: claim MD5 is secure, use external hash implementations, append a filename to success output, or remove failing tests to get a pass.

## Success Criteria

- `printf %s abc | ./md5` prints `900150983cd24fb0d6963f7d28e17f72`.
- All RFC 1321 vectors pass.
- A binary-file digest is compared with `md5sum`, when available.
- Both modes process raw bytes incrementally and output only the digest.
- Invalid invocations and file failures return nonzero without a digest.
- The full test script builds and passes.

## Open Questions

None. The CLI and output contracts have been explicitly confirmed.
