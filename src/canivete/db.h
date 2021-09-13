#ifndef CANIVETE_DB_H
#define CANIVETE_DB_H

struct canivete;

int canivete_db_migrate(struct canivete* app);
int canivete_db_endpoints_insert(struct canivete* app, const char* host, int port);
int canivete_db_endpoints_list(struct canivete* app);

#endif
