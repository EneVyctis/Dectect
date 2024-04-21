#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "esp_log.h"

#include "client.h"

#define ADRESSE_IP_SERVER CONFIG_SERVER_ADDRESS
#define PORT_SERVER CONFIG_SERVER_PORT

/**
 * Fonction d'etablissement d'une socket avec le server
 * 
 * renvoit l'identifiant de la socket de transmission
*/
int establish_connexion() 
{
	int sock;
	struct sockaddr_in server;

	/**
	 * Création de la socket
	*/

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == -1) {
		printf("Could not create socket \n");
		return -1;
	}
	ESP_LOGI("Socket", "Socket créée\n");

	/**
	 * Enregistrement des infos du serveur
	*/

	server.sin_addr.s_addr = inet_addr(ADRESSE_IP_SERVER);
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT_SERVER);

	/**
	 * Tentative de connexion au serveur
	*/

	int res;
	if ((res = connect(sock, (struct sockaddr *) &server, sizeof(server)))<0) {
		ESP_LOGI("Socket", "Connect failed error\n");
		close(sock);
		return -1;
	}
	
	ESP_LOGI("Socket", "Return value, %d\n", res);
	ESP_LOGI("Socket", "Connection establishec, waiting to be accepted\n");
	return sock;
}

void send_message(int sock, char* message) 
{
	ESP_LOGI("Send Message", "%s %d", message, strlen(message));
	send(sock, message, strlen(message), 0);
}
