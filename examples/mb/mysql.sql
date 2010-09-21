drop table if exists threads;
drop table if exists messages;

create table threads (
	id integer auto_increment primary key not null,
	title varchar(256) not null
) Engine = InnoDB;

create table messages (
	id integer auto_increment primary key not null,
	reply_to integer not null,
	thread_id integer not null,
	author varchar(256) not null,
	content text not null
) Engine = InnoDB;


