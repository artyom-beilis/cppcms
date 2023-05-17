LIBS=-lcppcms -lbooster


all: rpc

rpc: rpc.cpp
	$(CXX) $(CXXFLAGS) -std=c++11 -O2 -Wall -g rpc.cpp -o rpc ${LIBS}

clean:
	rm -fr rpc rpc.exe cppcms_rundir
