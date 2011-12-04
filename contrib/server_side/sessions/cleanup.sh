#!/bin/bash

cleanall()
{
	pushd $1
	rm -fr CMakeCache.txt CMakeFiles cmake_install.cmake  Makefile
	popd
}

cleanall .
cleanall berkeley_db
cleanall cppdb
cleanall sqlite3

rm -f *.db
rm -fr db/*

mysql -u root --password=root test <<EOF
drop table cppdb_sessions;
EOF

psql test <<EOF
drop table cppdb_sessions;
EOF

