INCLUDE = -I../vrvService/src

LIB = -lthrift-0.9.1

CXXFLAGS = -O2 -m64 $(INCLUDE) -fPIC

OBJ = ./src/config_types.o ./src/config_constants.o ./src/ConfigService.o

all: libconfig_service_export.so

libconfig_service_export.so: $(OBJ)
	$(CXX) -shared $(CXXFLAGS) -o ./lib/$@ $^
	cp ./lib/$@ ../../lib/

src/config_types.o: src/config_types.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

src/config_constants.o: src/config_constants.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

src/ConfigService.o: src/ConfigService.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

.PHONY: clean

clean:
	$(RM) src/*.o lib/libconfig_service_export.so
