LIBS=-lcppcms 


all: json

json: json.cpp
	$(CXX) $(CXXFLAGS) -std=c++11 -O2 -Wall -g json.cpp -o json ${LIBS}

clean:
	rm -fr json
