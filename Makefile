CXXFLAGS += -std=c++23 -O3

cpp-combinators: combinators.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^
