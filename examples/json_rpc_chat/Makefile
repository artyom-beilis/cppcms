LIBS=-lcppcms -lbooster


all: chat

chat: chat.cpp
	$(CXX) -std=c++11 -O2 -Wall $(CXXFLAGS) -g chat.cpp -o chat ${LIBS}

clean:
	rm -fr chat chat.exe cppcms_rundir
