[
	{
		"so" : "./sqlite3/libcppcms_session_sqlite3.so",
		"test" : "Sqlite3 native",
		"db" : "cppdb.db",
		"clean" : "rm cppdb.db",
		"run_gc" : true
	}, 
	{
		"so" : "./berkeley_db/libcppcms_session_bdb.so",
		"test" : "Berkeley DB",
		"directory" : "./db/",
		"clean" : "rm -f ./db/*"
	}, 
	{
		"so" : "./cppdb/libcppcms_session_cppdb.so",
		"test" : "sqlite3 acid",
		"connection_string" : "sqlite3:db=cppdb.db;busy_timeout=10000",
		"clean" : "rm cppdb.db",
		"transactivity" : "acid",
		"no_performance" : true,
	}, 
	{
		"so" : "./cppdb/libcppcms_session_cppdb.so",
		"test" : "sqlite3 relaxed ",
		"connection_string" : "sqlite3:db=cppdb.db;busy_timeout=10000",
		"clean" : "rm cppdb.db",
		"transactivity" : "relaxed",
		"no_performance" : true,
	}, 
	{
		"so" : "./cppdb/libcppcms_session_cppdb.so",
		"test" : "sqlite3 non_durable ",
		"connection_string" : "sqlite3:db=cppdb.db;busy_timeout=10000",
		"clean" : "rm cppdb.db",
		"transactivity" : "non_durable",
		"no_performance" : true,
	},
	{
		"so" : "./cppdb/libcppcms_session_cppdb.so",
		"test" : "mysql acid",
		"connection_string" : "mysql:database=test;user=root;password=root;host=localhost",
		"clean" : "mysql -u root --password=root test -e 'drop table cppdb_sessions;'",
		"transactivity" : "acid"
	}, 
	{
		"so" : "./cppdb/libcppcms_session_cppdb.so",
		"test" : "mysql relaxed",
		"connection_string" : "mysql:database=test;user=root;password=root;host=localhost",
		"clean" : "mysql -u root --password=root test -e 'drop table cppdb_sessions;'",
		"transactivity" : "relaxed"
	}, 
	{
		"so" : "./cppdb/libcppcms_session_cppdb.so",
		"test" : "postgresql acid",
		"connection_string" : "postgresql:dbname=test;@blob=bytea",
		"clean" : "psql test -c 'drop table cppdb_sessions;'",
		"transactivity" : "acid"
	}, 
	{
		"so" : "./cppdb/libcppcms_session_cppdb.so",
		"test" : "postgresql relaxed",
		"connection_string" : "postgresql:dbname=test;@blob=bytea",
		"clean" : "psql test -c 'drop table cppdb_sessions;'",
		"transactivity" : "relaxed"
	}, 
]
