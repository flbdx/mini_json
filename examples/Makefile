OBJECTS_create_json = \
	main_create_json.o
OBJECTS_read_json = \
	main_read_json.o
OBJECTS_tester = \
	main_tester.o

OBJECTS=$(sort $(OBJECTS_create_json))

CPPFLAGS=-I../lib/
CXXFLAGS+=-Wall -Wextra -std=c++17

all: create_json read_json tester

-include $(OBJECTS:.o=.d)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD $(TARGET_ARCH) -c $(OUTPUT_OPTION) $<

create_json: $(OBJECTS_create_json)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@
read_json: $(OBJECTS_read_json)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@
tester: $(OBJECTS_tester)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f $(OBJECTS) $(OBJECTS:.o=.d) create_json read_json

