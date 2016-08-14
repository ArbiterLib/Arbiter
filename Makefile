SOURCES=src/Arbiter.cpp src/Requirement.cpp src/Version.cpp
OBJECTS=$(SOURCES:.cpp=.o)
LIBRARY=libArbiter.a

GTEST_DIR=external/googletest/googletest

TEST_SOURCES=test/VersionTest.cpp $(GTEST_DIR)/src/gtest-all.cc $(GTEST_DIR)/src/gtest_main.cc
TEST_RUNNER=test/main
TEST_INCLUDES=-isystem $(GTEST_DIR)/include -Isrc/ -I$(GTEST_DIR)

CXX=clang++
CXXFLAGS=-std=c++14 -pedantic -Wall -Wextra
LIBTOOL=libtool

all: $(LIBRARY)

.PHONY: check $(TEST_RUNNER)

check: $(TEST_RUNNER)
	$(TEST_RUNNER)

clean:
	rm -f $(LIBRARY) $(TEST_RUNNER)
	rm -f $(OBJECTS)

$(LIBRARY): $(OBJECTS)
	$(LIBTOOL) $(OBJECTS) -o $@

$(TEST_RUNNER): $(TEST_SOURCES) $(LIBRARY)
	$(CXX) $(CXXFLAGS) $(TEST_SOURCES) $(LIBRARY) -pthread $(TEST_INCLUDES) -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@
