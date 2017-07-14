INCLUDE = 

LIB = -lthrift-0.9.1

CXXFLAGS = -O2 -m64 $(INCLUDE) -fPIC

OBJ = ./src/vrv_types.o ./src/vrv_constants.o ./src/VRVService.o

all: libvrv_service_export.so

libvrv_service_export.so: $(OBJ)
	$(CXX) -shared $(CXXFLAGS) -o ./lib/$@ $^
	cp ./lib/$@ ../../lib

src/vrv_types.o: src/vrv_types.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

src/vrv_constants.o: src/vrv_constants.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

src/VRVService.o: src/VRVService.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

.PHONY: clean

clean:
	$(RM) src/*.o lib/libvrv_service_export.so
