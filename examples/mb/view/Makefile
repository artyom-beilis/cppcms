-include ../Makefile.spec
TEMPLATES= master.tmpl forums.tmpl thread.tmpl
TARGET=simple
NAMESPACE=simple


all: .deps $(TARGET).o

.deps:
	mkdir .deps

$(TARGET).o: $(TARGET).cpp
	$(CXX) -c -MD -MF .deps/$@.deps $(VIEW_CXXFLAGS) -o $@ $<

$(TARGET).cpp: $(TEMPLATES)
	$(CTMPL) -o $@ -d $(GETTEXT_DOMAIN) -s $(NAMESPACE) $(TEMPLATES)

clean:
	rm -fr *.cpp $(DLL) .deps *.o

-include .deps/$(DLL).deps
