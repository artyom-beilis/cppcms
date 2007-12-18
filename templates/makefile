PREFIX="T"
INTERFACE=look


###################################
# Don't change after this comment #
###################################

all: 
	cc_templ_d -i $(INTERFACE).def -M $(INTERFACE).template
	make build

build: $(INTERFACE).template $(INTERFACE).h

.SUFFIXES: .tmpl .op 

$(INTERFACE).h: $(INTERFACE).def
	cc_templ_d -p $(PREFIX) -i $< -h $@

.tmpl.op: 
	cc_templ_d -i $(INTERFACE).def $< -o $@

$(INTERFACE).template: 
	ln_templ $(INTERFACE).def

clean:
	rm -f *.op *.template *.h .deps

-include .deps

