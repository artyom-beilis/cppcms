-include ../Makefile.spec
TEMPLATES= master.tmpl forums.tmpl thread.tmpl
TARGET=simple
NAMESPACE=simple



#
# For Cygwin
#
# TARGET_NAME = cyg$(TARGET).dll
#
# For Windows/GCC
# TMPL_CC_FLAGS = -u 
#

TARGET_NAME = lib$(TARGET).so
TMPL_CC_FLAGS = 

all: .deps $(TARGET_NAME)

.deps:
	mkdir .deps

$(TARGET_NAME): $(TARGET).cpp 
	$(CXX) $(CXXFLAGS) -shared -MD -MF .deps/$@.deps $< -o $@ $(LIBS)

$(TARGET).cpp: $(TEMPLATES)
	$(CTMPL) -o $@ $(TMPL_CC_FLAGS) -d $(GETTEXT_DOMAIN) -s $(NAMESPACE) $(TEMPLATES)

clean:
	rm -fr *.cpp $(TARGET_NAME) .deps *.o

-include .deps/$(DLL).deps
