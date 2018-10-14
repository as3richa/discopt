CPPFLAGS=-Wall -Wextra -Werror -std=c++11 -pedantic
SOURCES=$(shell echo *.cpp)
EXECUTABLES=$(SOURCES:%.cpp=bin/%)

.PHONY: clean

all: $(EXECUTABLES)

bin/%: %.cpp
	$(CXX) $(CPPFLAGS) -o $@ $<

clean:
	rm $(EXECUTABLES)
