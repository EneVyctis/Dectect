# include <sys/socket.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include "client.h"

# define SERVERPORT 8080
# define LOCALHOST "127.0.0.1"
# define MYMSGLEN  2048

void send_to_server ( int sock, char * string )
{
	int code, length ;

	// Send the length of the string.
	length = strlen ( string ) + 1 ;
	
	// Send the string of characters.
	code = send ( sock, string, length, 0 ) ;
	if ( code == -1 ) {
		perror ( "send" ) ;
		exit ( 1 ) ;
	}
	printf ( "Data sent to server ..... \n " ) ;
}

int connecting ( )
{
	// Socket creation.
	int sock ;
	sock = socket ( AF_INET, SOCK_STREAM, 0 ) ;
	if ( sock == -1 ) {
		perror ( "socket" ) ;
		exit ( 1 ) ;
	}

	// Initialisation of the sockaddr_in data structure

	struct sockaddr_in addr ;
	socklen_t len ;
	
	struct sockaddr_in newAddr ;
	struct sockaddr_in peerAddr ;
	
	memset ( & addr, 0, sizeof ( struct sockaddr_in ) ) ;
	addr . sin_family = AF_INET ;
	addr . sin_port = htons(SERVERPORT) ;
	addr . sin_addr . s_addr = inet_addr(LOCALHOST) ;

	// Name the socket.
	int code ;
	code = connect ( sock, ( struct sockaddr * ) & addr, sizeof ( struct sockaddr_in ) ) ;
	if ( code == -1 ) {
        printf("hello\n");
		perror ( "connect" ) ;
		close ( sock ) ;
		exit ( 1 ) ;
	}

	len = sizeof ( struct sockaddr_in ) ;
	code = getsockname ( sock, ( struct sockaddr * ) & newAddr, & len ) ;
	if ( code == -1 ) {
	      perror ( "getsockname" ) ;
	      close ( sock ) ;
		
	      exit ( 1 ) ;
	}
	len = sizeof ( struct sockaddr_in ) ;
	code = getpeername ( sock, ( struct sockaddr * ) & peerAddr, & len ) ;
	if ( code == -1 ) {
	      perror ( "getpeername" ) ;
	      close ( sock ) ;
	      exit ( 1 ) ;
	}
		
	printf ( " The local address bound to the current socket --> %s:%d \n" , inet_ntoa ( newAddr.sin_addr ), ntohs ( newAddr.sin_port )  ) ;
	printf ( " The peer address bound to the peer socket --> %s:%d \n" , inet_ntoa ( peerAddr.sin_addr ), ntohs ( peerAddr.sin_port )  ) ;
	// Return the socket ID.
	return sock ;
}
