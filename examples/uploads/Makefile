LIBS=-lcppcms -lbooster

all: clean uploader 

uploader: uploader.cpp view.cpp content.h
	$(CXX) -Wall $(CXXFLAGS) uploader.cpp view.cpp -o uploader $(LIBS)

view.cpp: view.tmpl content.h
	cppcms_tmpl_cc view.tmpl -o view.cpp

clean:
	rm -fr *.exe *.so view.cpp cppcms_rundir uploader
