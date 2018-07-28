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
		printf("<host> <puerto>\n");
		return 1;
	}
	
	char *dir="/CarpetaCliente";
	char *currentDir = getCurrentDir();
	currentDir = concat(currentDir,dir);
	
	
	struct dirent *dp1, *dp2;
	DIR *fd1, *fd2;
	
	struct sockaddr_in cliente; 
	struct hostent *servidor;
	
	servidor = gethostbyname(argv[1]); //Obtenemos toda la data correspondiente al servidor
	
	if(servidor == NULL){ 
	printf("Host invalido\n");
	return 1;
	}
	
	int puerto, conexion,posicion;
	char *buffer, *md5, *archMd5;
	char opcion[1];
	char **paragraph;
	paragraph = (char**)malloc(1*sizeof(char*));
	
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
	//printf("Conectado con el server \n");
	//printf("Escribe un mensaje al servidor: ");
	printf("1) Comprobar Archivo \n");
	printf("2) Agregar Archivo \n");
	printf("3) Eliminar Archivo \n");
	
	scanf("%s", opcion);
	
	if ((fd1 = opendir(currentDir)) == NULL) {
		printf("La carpeta no se puede abrir \n");			
		return 1;
	}
	
	if ((fd2 = opendir(currentDir)) == NULL) {
		printf("La carpeta no se puede abrir \n");			
		return 1;
	}
	
	while ((dp1 = readdir(fd1)) != NULL) {
	
		if (!strcmp(dp1->d_name, ".") || !strcmp(dp1->d_name, ".."))
	    	continue;
	    posicion++;
	    	   			
	}
	
	
	//printf("Comprobando archivos | %d archivos del cliente",posicion);
	
	while ((dp2 = readdir(fd2)) != NULL) {
	
		if (!strcmp(dp2->d_name, ".") || !strcmp(dp2->d_name, ".."))
	    	continue;
	    
	    send(conexion, dp2->d_name, sizeof(dp2->d_name), 0);
	    recv(conexion, buffer, sizeof(buffer), 0);
		if(strcmp(buffer,"Existe")==0){
		//El archivo existe
		//Hacer mdsum
		archMd5 = concat(currentDir,"/");
		archMd5 = concat(archMd5,dp2->d_name);
		md5 = ObtenerMd5(archMd5);
		send(conexion, md5, sizeof(md5), 0);
		bzero((char *)&buffer, sizeof(buffer));
		recv(conexion, buffer, sizeof(buffer), 0);
		
			if(strcmp(buffer,"Igual")==0){
			//El archivo es igual
			}else{
				printf("No sincronizado");
				return 1;
			}
			
		}else{
		//no existe
			printf("No sincronizado");
			return 1;
			
		} 
	    
	    	   	
		printf("file %s\n",archMd5);		
	}
	
	//while(i < posicion){
		//envio el nombre primero para saber si esta
		//send(conexion, paragraph[i], sizeof(paragraph[i]), 0); 
		//recibo para saber si esta o no
		//recv(conexion, buffer, sizeof(buffer), 0);
		//if(strcmp(buffer,"Existe")==0){
		//El archivo existe
		//Hacer mdsum
		//archMd5 = concat(currentDir,"/");
		//archMd5 = concat(archMd5,paragraph[i]);
		//md5 = ObtenerMd5(archMd5);
		
		
		//}else{
		//no existe
			
		//} 
		//Envio el checksum
		//recibo feedback
		//i++;
	//}
	
	
	
	
	
	/*printf("Hemos recibido del servidor: %s\n", buffer);*/
	
	return 0;
}
