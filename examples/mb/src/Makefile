LDFLAGS=-export-dynamic
SRC=forums.cpp mb.cpp thread.cpp main.cpp
OBJ=$(patsubst %.cpp,%.o,$(SRC)) ../view/simple.o
DEPS=$(patsubst %.o,.deps/%.o.dep,$(OBJ))

TARGET=../mb

all: .deps $(TARGET)

.deps:
	mkdir -p .deps

$(TARGET): $(OBJ)
	$(CXX)  $(EXEC_LDFLAGS) -o $@ $(OBJ) $(EXEC_LIBS)

.cpp.o:
	$(CXX) $(EXEC_CXXFLAGS) -MD -MF .deps/$@.dep -c -o $@ $<

clean:
	rm -fr .deps *.o $(TARGET)

-include $(DEPS)
-include ../Makefile.spec
