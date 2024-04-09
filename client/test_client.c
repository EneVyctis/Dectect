#include "client.h"
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>

int main ( int argc, char * argv [ ] )
{
	// Ask for the string of characters.
	char string [ MYMSGLEN ];
	
	

	// Connect to the server.
	int sock ;
	sock = connecting ( ) ;

	// Ask the question and wait for the answer.
	while ( 1 )
	{
	  memset ( string, 0, sizeof( string ) );
	  printf ( "Type a string to send: " ) ;
	  scanf ( "%s", string ) ;
	  send_to_server ( sock, string ) ;
	}
	
	return ( 0 ) ;
}