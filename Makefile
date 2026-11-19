# Build with one command:  make
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O3 -ffp-contract=off -Wall

heat: heat.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f heat

.PHONY: clean
