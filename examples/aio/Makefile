LIBS=-lbooster


all: echo_server

echo_server: echo_server.cpp
	$(CXX) $(CXXFLAGS) -std=c++0x -O2 -Wall -g echo_server.cpp -o echo_server ${LIBS}

clean:
	rm -fr echo_server echo_server.exe cppcms_rundir
