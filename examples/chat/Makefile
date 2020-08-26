LIBS=-lcppcms -lbooster


all: chat

chat: chat.cpp
	$(CXX) $(CXXFLAGS) -std=c++11 -O2 -Wall -g chat.cpp -o chat ${LIBS}

clean:
	rm -fr chat chat.exe cppcms_rundir
