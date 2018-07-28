#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include <unistd.h>

	char* concat(char *s1, char *s2){

		int tam = strlen(s1)+strlen(s2)+1;
	    char *result;
	    result = (char *)malloc(tam);
	    strcpy(result, s1);
	    strcat(result, s2);
	    return result;
	}
	
	char* getCurrentDir(){
		char cwd[1024];
		if (getcwd(cwd, sizeof(cwd)) != NULL)
			return cwd;
		else
			return NULL;
	}
	
	char* ObtenerMd5(char *file){
		char buf[32];
		char *comando = "";
		comando = concat(comando,"md5sum ");
		comando = concat(comando,file);
		comando = concat(comando," > File.txt");
		system(comando);
		FILE* fp;
		fp = fopen("File.txt","r");
		fread(buf, 32, 1, fp);
		fclose(fp);
		printf("Md5 %s\n",buf);
		return buf;
	} 

	int main(int argc, char **argv){
	
	if(argc<2){
		printf("%s Indique un <puerto>\n",argv[0]);
		return 1;
	}
	
	char *dir="/CarpetaServidor";
	char *currentDir = getCurrentDir();
	currentDir = concat(currentDir,dir);
	
	//Variables para obtener los id's del servidor, cliente y el puerto especificado
	int conexion_servidor, conexion_cliente, puerto; 
	//Longitud del socket del cliente
	socklen_t longc; 
	//Estructuras del cliente y servidor
	struct sockaddr_in servidor, cliente;
	//Un buffer para el mensaje de entrada y otro de salida
	char bufferIn[100], bufferOut[100]; 
	
	struct dirent *dp1, *dp2;
	DIR *fd1, *fd2;
	
	int pos=0;
	
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
	printf("Conectando con cliente\n");
  
	if(recv(conexion_cliente, bufferIn, 100, 0) < 0){ //Recibimos un mensaje de un cliente y lo guardamos en el buffer
		printf("Error al recibir los datos\n");
		close(conexion_servidor);
		return 1;
	}else{
		
		if ((fd1 = opendir(currentDir)) == NULL) {
			printf("La carpeta no se puede abrir \n");			
			return 1;
		}
	
	if ((fd2 = opendir(currentDir)) == NULL) {
		printf("La carpeta no se puede abrir \n");			
		return 1;
	}
	
	int yes = 0;
	
	while ((dp1 = readdir(fd1)) != NULL) {
	
		if (!strcmp(dp1->d_name, ".") || !strcmp(dp1->d_name, ".."))
	    	continue;
	    pos++;
	    if(strcmp(dp1->d_name,bufferIn)==0){
			yes=1;
		}    	   			
	}
	
	if(yes == 1){
		send(conexion_cliente, "Existe", sizeof("Existe") , 0);
	}else{
		send(conexion_cliente, "No Existe", sizeof("No Existe") , 0);
		close(conexion_servidor);
	}
		
		

	}
	close(conexion_servidor);
	return 0;
}
