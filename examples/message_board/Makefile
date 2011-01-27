DIRS=apps view

all:	mb.db locale/he/LC_MESSAGES/mb.mo
	make -C view
	make -C apps

clean:
	make -C view clean
	make -C apps clean
	rm -fr cppcms_rundir
	rm -f mb.db
	rm locale/he/LC_MESSAGES/mb.mo
	rm -f *.exe

mb.db:
	sqlite3 mb.db <model/sqlite3.sql


locale/he/LC_MESSAGES/mb.mo: locale/he/LC_MESSAGES/mb.po
	msgfmt locale/he/LC_MESSAGES/mb.po -o locale/he/LC_MESSAGES/mb.mo
