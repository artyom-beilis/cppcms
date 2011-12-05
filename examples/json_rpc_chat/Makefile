LIBS=-lcppcms -lbooster


all: chat

chat: chat.cpp
	$(CXX) -O2 -Wall $(CXXFLAGS) -g chat.cpp -o chat ${LIBS}

clean:
	rm -fr chat chat.exe cppcms_rundir
