CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic

.PHONY: all test clean

all: app

app: main.cpp md5.cpp md5.hpp
	$(CXX) $(CXXFLAGS) main.cpp md5.cpp -o $@

md5_tests: tests/md5_tests.cpp md5.cpp md5.hpp
	$(CXX) $(CXXFLAGS) -I. tests/md5_tests.cpp md5.cpp -o $@

test: app md5_tests
	./md5_tests

clean:
	rm -f app md5_tests
