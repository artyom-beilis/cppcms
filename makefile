TR=test.fcgi
SRC=main.cpp textstream.cpp worker_thread.cpp FCgiIO.cpp main_thread.cpp mysql_db.cpp thread_pool.cpp
OBJ := $(patsubst %.cpp,%.o,$(SRC))

LIBS = -lmysqlclient -lfcgi++ -lcgicc
CXX = g++
CFLAGS = -Wall -g 


all: $(TR)

$(TR) : $(OBJ)
	$(CXX) -o $@ $^ $(LIBS)

.cpp.o:
	$(CXX) -c $(CFLAGS) $<

clean:
	rm -f $(OBJ) $(TR) .depend

depend:
	$(CXX) -M $(SRC) > .depend

install: $(TR)
	/etc/init.d/lighttpd stop
	sleep 1
	cp $(TR) /home/artik/Projects/www/c/
	chown artik:artik /home/artik/Projects/www/c/$(TR)
	/etc/init.d/lighttpd start

-include .depend
