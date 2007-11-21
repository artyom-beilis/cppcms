TR=test.fcgi
SRC=main.cpp textstream.cpp worker_thread.cpp FCgiIO.cpp main_thread.cpp mysql_db.cpp thread_pool.cpp \
    global_config.cpp url.cpp

LSRC=textstream.cpp worker_thread.cpp FCgiIO.cpp mysql_db.cpp thread_pool.cpp global_config.cpp
OBJ := $(patsubst %.cpp,%.o,$(SRC))
LOBJ := $(patsubst %.cpp,%.o,$(LSRC))

LIBS = -lmysqlclient -lfcgi++ -lcgicc -lboost_regex -lboost_signals
CXX = g++
;CFLAGS = -Wall -g -DFCGX_API_ACCEPT_ONLY_EXISTS
CFLAGS = -Wall -g -O2


all: $(TR)

lib: libcppcms.a

$(TR) : $(OBJ)
	$(CXX) -o $@ $^ $(LIBS)

libcppcms.a :  $(LOBJ)
	ar rvu libcppcms.a $(LOBJ)
	ranlib libcppcms.a

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
