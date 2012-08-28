all: chat ticker

chat: ../server_sent_events.cpp  ../server_sent_events.h chat.cpp
	g++ -I .. -Wall -Wextra -O2 -g chat.cpp ../server_sent_events.cpp -lcppcms -lbooster -o chat

ticker: ../server_sent_events.cpp  ../server_sent_events.h ticker.cpp
	g++ -I .. -std=c++0x -Wall -Wextra -O2 -g ticker.cpp ../server_sent_events.cpp -lcppcms -lbooster -o ticker

clean:
	rm chat ticker
