LIBS=-lcppcms -lbooster

all: myapp

myapp: myapp.cpp my_skin.cpp content.h
	$(CXX) $(CXXFLAGS) -Wall myapp.cpp my_skin.cpp -o myapp ${LIBS}

my_skin.cpp: master.tmpl page.tmpl news.tmpl intro.tmpl content.h
	cppcms_tmpl_cc master.tmpl page.tmpl news.tmpl intro.tmpl -o my_skin.cpp

clean:
	rm -fr myapp *.exe my_skin.cpp cppcms_rundir

