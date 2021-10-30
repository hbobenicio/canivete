create table if not exists endpoints (
    id integer primary key autoincrement,
    host varchar(128) not null,
    port integer not null,

    -- unique(host, port)
    constraint unq_addr unique (host, port)
);
