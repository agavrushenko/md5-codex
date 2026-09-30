# cpp-container-template

## Getting Started

This repository is compatible with [cpp-container](https://github.com/ChicoState/cpp-container). If not already built on your machine, clone and build it.

Run the container:

```bash
docker run -v "$(pwd)":/usr/src -it cpp-container
```

Run the application interactively in a shell:

```bash
docker run -v "$(pwd)":/usr/src -it cpp-container sh
```

## MD5 Utility

This project is being developed as a self-contained C++17 implementation of
the MD5 message-digest algorithm for coursework and learning. MD5 is
cryptographically broken and must not be used for security-sensitive
integrity, authentication, or signature verification.

Build the program with:

```bash
make
```

Use no arguments to hash raw standard-input bytes through EOF, or provide one
file path to hash that file's raw bytes:

```bash
printf %s abc | ./md5
./md5 path/to/input
```

Successful commands print only the lowercase 32-character MD5 digest and a
newline. The program accepts at most one argument; file paths are read in
binary mode, and stdin preserves all bytes, including newlines and NUL bytes.

Run the full test suite with `make test` (or `./test_runner.sh`).

The complete requirements, test strategy, and implementation plan are in
[specs/md5.md](specs/md5.md).

## Structure

* `.agents` - AI agent configurations and skills (in `/skills` subdirectory) for this project
* `.` - The root directory contains the C++ code, `Makefile`, and necessary scripts
* `specs` - Specification documentation
* `tests` - Test code
