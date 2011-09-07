LIBS=-lcppcms -lbooster


all: rpc

rpc: rpc.cpp
	$(CXX) $(CXXFLAGS) -O2 -Wall -g rpc.cpp -o rpc ${LIBS}

clean:
	rm -fr rpc rpc.exe cppcms_rundir
