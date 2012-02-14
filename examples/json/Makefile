LIBS=-lcppcms 


all: json

json: json.cpp
	$(CXX) $(CXXFLAGS) -O2 -Wall -g json.cpp -o json ${LIBS}

clean:
	rm -fr json
