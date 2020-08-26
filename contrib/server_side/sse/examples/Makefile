all: chat ticker

chat: ../server_sent_events.cpp  ../server_sent_events.h chat.cpp
	g++ -I .. -Wall -Wextra -O2 -std=c++11 -g chat.cpp ../server_sent_events.cpp -lcppcms -lbooster -o chat

ticker: ../server_sent_events.cpp  ../server_sent_events.h ticker.cpp
	g++ -I .. -Wall -Wextra -std=c++11 -O2 -g ticker.cpp ../server_sent_events.cpp -lcppcms -lbooster -o ticker

clean:
	rm chat ticker
