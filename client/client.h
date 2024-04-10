#ifndef CLIENT_H
#define CLIENT_H

void send_to_server ( int sock, char * string );
int connecting ( );

# define SERVERPORT 8080
# define LOCALHOST "127.0.0.1"
# define MYMSGLEN  2048

#endif

