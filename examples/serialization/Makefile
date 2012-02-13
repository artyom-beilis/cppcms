LIBS=-lcppcms -lbooster


all: serialization external_serialization

serialization: serialization.cpp
	$(CXX) $(CXXFLAGS) -O2 -Wall -g serialization.cpp -o serialization ${LIBS}

external_serialization: external_serialization.cpp
	$(CXX) $(CXXFLAGS) -O2 -Wall -g external_serialization.cpp -o external_serialization ${LIBS} -lboost_serialization

clean:
	rm -f *.exe serialization external_serialization
