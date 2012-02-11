LIBS=-lcppcms -lbooster

all: clean myapp

myapp: myapp.cpp
	$(CXX) $(CXXFLAGS) -Wall myapp.cpp -o myapp $(LIBS)

clean:
	rm -fr myapp *.exe *.so cppcms_rundir
