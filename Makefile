SOURCES=src/Arbiter.cpp src/Version.cpp
OBJECTS=$(SOURCES:.cpp=.o)
LIBRARY=libArbiter.a

TEST_SOURCES=test/Test.cpp
TEST_RUNNER=test/main
TEST_INCLUDES=src/

CXX=clang++
CXXFLAGS=-std=c++14 -pedantic
LIBTOOL=libtool

all: $(LIBRARY)

check: $(TEST_RUNNER)
	$(TEST_RUNNER)

clean:
	rm -f $(LIBRARY) $(TEST_RUNNER)
	rm -f $(OBJECTS)

$(LIBRARY): $(OBJECTS)
	$(LIBTOOL) $(OBJECTS) -o $@

$(TEST_RUNNER): $(TEST_SOURCES) $(LIBRARY)
	$(CXX) $(CXXFLAGS) $(TEST_SOURCES) $(LIBRARY) -I$(TEST_INCLUDES) -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@
