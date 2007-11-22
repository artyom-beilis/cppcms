all: look.template look.h

look.h: look.def
	cc_templ_d -i look.def -h look.h -p "T"

look.template: dologin.op main.op post.op master.op 
	ln_templ look.def

main.op: look.def main.tmpl
	cc_templ_d -i look.def main.tmpl

dologin.op: look.def dologin.tmpl
	cc_templ_d -i look.def dologin.tmpl

post.op: look.def post.tmpl
	cc_templ_d -i look.def post.tmpl

master.op: look.def master.tmpl
	cc_templ_d -i look.def master.tmpl

clean:
	rm -f *.op *.template *.h
