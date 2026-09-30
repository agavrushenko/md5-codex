CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic

.PHONY: all test clean

all: md5

md5: main.cpp md5.cpp md5.hpp
	$(CXX) $(CXXFLAGS) main.cpp md5.cpp -o $@

md5_tests: tests/md5_tests.cpp md5.cpp md5.hpp
	$(CXX) $(CXXFLAGS) -I. tests/md5_tests.cpp md5.cpp -o $@

test: md5 md5_tests
	./md5_tests

clean:
	rm -f md5 md5_tests
