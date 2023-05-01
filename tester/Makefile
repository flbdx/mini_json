tester_OBJECTS=\
	main_tester.o

OBJECTS=$(sort $(tester_OBJECTS))

CPPFLAGS=-I../lib/
CXXFLAGS+=-Wall -Wextra -std=c++17

all: tester

-include $(OBJECTS:.o=.d)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD $(TARGET_ARCH) -c $(OUTPUT_OPTION) $<

tester: $(tester_OBJECTS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f $(OBJECTS) $(OBJECTS:.o=.d) tester
