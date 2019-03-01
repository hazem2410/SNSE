CC = x86_64-w64-mingw32-g++
TGT = $(SRC:%.cpp=%)
CXX_DEBUG_FLAGS = -g
CXX_RELEASE_FLAGS = -O3

release: ABM_Disasters.exe

ABM_Disasters.exe: ABM_Disasters.cpp
	$(CC) -std=c++11 $(CXX_RELEASE_FLAGS) -static -o ABM_Disasters.exe ABM_Disasters.cpp


