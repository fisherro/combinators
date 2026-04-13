CXXFLAGS += -std=c++23

cpp-combinators: combinators.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^
