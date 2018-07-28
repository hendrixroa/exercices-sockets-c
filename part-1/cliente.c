#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

	int main(int argc, char **argv){
		
	if(argc<2){
		printf("<host> <puerto>\n");
		return 1;
	}
	
	struct sockaddr_in cliente; 
	struct hostent *servidor;
	
	servidor = gethostbyname(argv[1]); //Obtenemos toda la data correspondiente al servidor
	
	if(servidor == NULL){ 
	printf("Host invalido\n");
	return 1;
	}
	
	int puerto, conexion;
	char buffer[100];
	
	conexion = socket(AF_INET, SOCK_STREAM, 0); //Establecemos el socket
	
	puerto=(atoi(argv[2]));
	
	bzero((char *)&cliente, sizeof((char *)&cliente)); 
	
	cliente.sin_family = AF_INET;
	cliente.sin_port = htons(puerto);
	bcopy((char *)servidor->h_addr, (char *)&cliente.sin_addr.s_addr, sizeof(servidor->h_length));
	//Intentaremos conectarnos 
	if(connect(conexion,(struct sockaddr *)&cliente, sizeof(cliente)) < 0){ 
		printf("Error conectando con el host\n");
		close(conexion);
		return 1;
	}
	//Ya nos conectamos y escribimos mensajes
	printf("Conectado con %s:%d\n",inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));
	printf("Escribe un mensaje al servidor: ");
	fgets(buffer, 100, stdin);
	send(conexion, buffer, 100, 0); 
	bzero(buffer, 100);
	recv(conexion, buffer, 100, 0); 
	printf("Hemos recibido del servidor: %s\n", buffer);
	
	return 0;
}
