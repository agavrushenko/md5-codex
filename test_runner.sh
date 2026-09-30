#!/bin/bash

set -euo pipefail

g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp md5.cpp -o app
g++ -std=c++17 -Wall -Wextra -Wpedantic -I. tests/md5_tests.cpp md5.cpp -o md5_tests
./md5_tests
