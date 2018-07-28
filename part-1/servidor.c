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
		printf("%s Indique un <puerto>\n",argv[0]);
		return 1;
	}
	
	//Variables para obtener los id's del servidor, cliente y el puerto especificado
	int conexion_servidor, conexion_cliente, puerto; 
	//Longitud del socket del cliente
	socklen_t longc; 
	//Estructuras del cliente y servidor
	struct sockaddr_in servidor, cliente;
	//Un buffer para el mensaje de entrada y otro de salida
	char bufferIn[100], bufferOut[100]; 
	
	puerto = atoi(argv[1]); //Convertimos el puerto de entrada a entero
	
	if(puerto <= 1022 || puerto >= 49152){
		printf("Error, el puerto no puede ser usado. \n");
		return 1;
	}
	
	conexion_servidor = socket(AF_INET, SOCK_STREAM, 0); //Establecemos la creacion del socket
	bzero((char *)&servidor, sizeof(servidor));  //Por buenas practicas llenamos la estructura en 0's
	servidor.sin_family = AF_INET;  //Especificamos que estamos utilizando ipv4
	servidor.sin_port = htons(puerto); //Realizamos la conversion del puerto y se la asignamos a la estructura
	
	servidor.sin_addr.s_addr = INADDR_ANY; //Tomamos la direccion de la maquina en donde se ejecuta.
	
	//Trataremos de 'bindear' y validaremos si es posible asociar el puerto al socket
	if(bind(conexion_servidor, (struct sockaddr *)&servidor, sizeof(servidor)) < 0){ 
		printf("Error al asociar el puerto a la conexion\n");
		close(conexion_servidor);
		return 1;
	}
	
	listen(conexion_servidor, 3); //Escuchemos si hay solicitudes con un maximo de 3 
	
	printf("A la escucha en el puerto %d\n", ntohs(servidor.sin_port));
	
	longc = sizeof(cliente); 
	//Una vez llegue un cliente aceptamos su conexion
	conexion_cliente = accept(conexion_servidor, (struct sockaddr *)&cliente, &longc); 
	
	if(conexion_cliente < 0){
		printf("Error al aceptar trafico\n");
		close(conexion_servidor);
		return 1;
	}
	printf("Conectando con %s:%d\n", inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));
  
	if(recv(conexion_cliente, bufferIn, 100, 0) < 0){ //Recibimos un mensaje de un cliente y lo guardamos en el buffer
		printf("Error al recibir los datos\n");
		close(conexion_servidor);
		return 1;
	}else{
		printf("%s\n", bufferIn);//imprimos lo recibido
		bzero((char *)&bufferIn, sizeof(bufferIn));
		printf("Escribe un mensaje al cliente: ");
		fgets(bufferOut, 100, stdin);
		send(conexion_cliente, bufferOut, sizeof(bufferOut) , 0);//Le indicamos al cliente que recibimos el mensaje
	}
	close(conexion_servidor);
	return 0;
}
