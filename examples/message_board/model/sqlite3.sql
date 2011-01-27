begin;
drop table if exists threads;
drop table if exists messages;

create table threads (
    id integer primary key autoincrement not null,
    title varchar(256) not null
);

create table messages (
    id integer primary key autoincrement not null,
    reply_to integer not null,
    thread_id integer not null,
    author varchar(256) not null,
    content text not null
);

commit;
