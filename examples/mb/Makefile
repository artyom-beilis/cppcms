DIRS=src view

all:	mb.db locale/he/LC_MESSAGES/mb.mo
	cd view ; make 
	cd src ; make

clean:
	cd src ; make clean
	cd view ; make clean
	rm -fr cppcms_rundir
	rm -f mb.db
	rm locale/he/LC_MESSAGES/mb.mo
	rm -f *.exe

mb.db:
	sqlite3 mb.db <sqlite3.sql


locale/he/LC_MESSAGES/mb.mo: locale/he/LC_MESSAGES/mb.po
	msgfmt locale/he/LC_MESSAGES/mb.po -o locale/he/LC_MESSAGES/mb.mo
