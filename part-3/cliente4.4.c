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
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 512

	char* concat(char *s1, char *s2){

		int tam = strlen(s1)+strlen(s2)+1;
	    char *result;
	    result = (char *)malloc(tam);
	    strcpy(result, s1);
	    strcat(result, s2);
	    return result;
	}
	
	
	char* ObtenerMd5(char *file){
		char *buf;
		buf = (char *)malloc(sizeof(char) * 32);
		char *comando = "";
		comando = concat(comando,"md5sum ");
		comando = concat(comando,file);
		comando = concat(comando," > FileMd5Cliente.txt");
		system(comando);
		FILE* fp;
		fp = fopen("FileMd5Cliente.txt","r");
		fread(buf, 32, 1, fp);
		fclose(fp);
		return buf;
	}
	

	int main(int argc, char **argv){
		
	if(argc<2){
		printf("<host> <puerto>\n");
		return 1;
	}
	
	char *dir="/CarpetaCliente";
	char *currentDir = getenv("PWD");
	currentDir = concat(currentDir,dir);
	
	
	struct dirent *dp1, *dp2;
	DIR *fd1, *fd2;
	
	struct sockaddr_in cliente; 
	struct hostent *servidor;
	struct stat fileSize;
	
	servidor = gethostbyname(argv[1]); //Obtenemos toda la data correspondiente al servidor
	
	if(servidor == NULL){ 
	printf("Host invalido\n");
	return 1;
	}
	
	int puerto, conexion,posicion=0;
	char buffer[32], *md5, *archMd5;
	char opcion[1], dataBuf[BUFFER_SIZE];
	md5 = (char *)malloc(sizeof(char) * 32);
	
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

	if(strcmp(opcion,"2")==0){

			char ruta[BUFFER_SIZE];

			scanf("%s", ruta);
			

			int fd1,size1=0;

			char *msj1 = "";
			msj1 = concat("Agregar_Archivo| ",ruta);
			//envio nombre de archivo e instruccion de que debe crearlo


			send(conexion, msj1 , strlen(msj1) , 0);

			bzero(buffer,32);
			recv(conexion, buffer, 10, 0);

			char *ruta_completa="";

			ruta_completa = concat(ruta_completa,currentDir);
			ruta_completa = concat(ruta_completa,"/");
			ruta_completa = concat(ruta_completa,ruta);
				
			if ((fd1 = open(ruta_completa,O_RDONLY))==-1){
				perror("fallo al abrir archivo a enviar");
				return EXIT_FAILURE;
			}
    		
    		bzero(dataBuf,BUFFER_SIZE);
    		
			int m1;
			int n1 = read(fd1,dataBuf,BUFFER_SIZE);
			
			while(n1){
				if(n1==-1){
					perror("read fails");
					return EXIT_FAILURE;
				}
				//printf("en cliente enviando %s\n", dataBuf);
				m1 = sendto(conexion,dataBuf,n1,0,(struct sockaddr*)&cliente,sizeof(struct sockaddr_in));

			if(m1==-1){
				perror("send error");
				close(conexion);
				return EXIT_FAILURE;

			}
				bzero(dataBuf,BUFFER_SIZE);
        		n1=read(fd1,dataBuf,BUFFER_SIZE);
			}
			//m = sendto(conexion,dataBuf,0,0,(struct sockaddr*)&cliente,sizeof(struct sockaddr_in));
			//para desbloquear el servidor
			close(fd1);
			close(conexion);
			return 1;

	}

	if(strcmp(opcion,"3")==0){

			char ruta3[BUFFER_SIZE];

			scanf("%s", ruta3);
			
			char *msj3 = "";
			msj3 = concat("Borrar_Archivo| ",ruta3);

			send(conexion, msj3 , strlen(msj3) , 0);

			close(conexion);
			return 1;

	}
	
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
	
	
	printf("Comprobando archivos | %d archivos del cliente \n",posicion);
	
	while ((dp2 = readdir(fd2)) != NULL) {
	
		if (!strcmp(dp2->d_name, ".") || !strcmp(dp2->d_name, ".."))
	    	continue;
	    printf("Comprobando archivo: %s\n",dp2->d_name);
	    char buf[strlen(dp2->d_name)];
	    strcpy(buf,dp2->d_name);
	    send(conexion, buf, strlen(buf), 0);
	    bzero(buffer, 32);
	    recv(conexion, buffer, 32, 0);

	    //recibo el md5
		if(strcmp(buffer,"No Existe")!=0){
		//El archivo existe 
		//Hacer mdsum
		buffer[32] = '\0';	
		printf("Archivo encontrado\n");
		
		archMd5 = concat(currentDir,"/");
		archMd5 = concat(archMd5,dp2->d_name);
		md5 = ObtenerMd5(archMd5);
		
		
		if(strcmp(buffer,md5)!=0){
			//el archivo existe en el server pero no contienen lo mismo
			printf("INCOMPLETE_SYNC\n");
			send(conexion, "INCOMPLETE_SYNC", strlen("INCOMPLETE_SYNC"), 0);
			recv(conexion, buffer, 16, 0);
			close(conexion);
			return 1;
		}
				
		}else{
			//el archivo no existe hay que crearlo

			char *path = concat(currentDir,"/");
				 path = concat(path,buf);
			
			printf("path %s\n",path);

			int fd,size=0;

			char *msj = "";
			msj = concat("Enviando: ",buf);
			//envio nombre de archivo e instruccion de que debe crearlo

			//tama#o del archivo
    		if (stat(path,&fileSize)==-1){
				perror("stat fail");
				return EXIT_FAILURE;
			}
			else
				size = fileSize.st_size;

			char numSize[BUFFER_SIZE];
			sprintf(numSize, "%d", size);
			msj = concat(msj," | ");
			msj = concat(msj,numSize);

			send(conexion, msj , strlen(msj) , 0);

			bzero(buffer,32);
			recv(conexion, buffer, 10, 0);
	
			if ((fd = open(path,O_RDONLY))==-1){
				perror("fallo al abrir archivo a enviar");
				return EXIT_FAILURE;
			}
    		
    		bzero(dataBuf,BUFFER_SIZE);

			int m;
			int n = read(fd,dataBuf,BUFFER_SIZE);
			size = size-n;
			while(n){
				if(n==-1){
					perror("read fails");
					return EXIT_FAILURE;
				}
				//printf("en cliente enviando %s\n", dataBuf);
				m = sendto(conexion,dataBuf,n,0,(struct sockaddr*)&cliente,sizeof(struct sockaddr_in));

			if(m==-1){
				perror("send error");
				close(conexion);
				return EXIT_FAILURE;

			}
				bzero(dataBuf,BUFFER_SIZE);
        		n=read(fd,dataBuf,BUFFER_SIZE);
			}
			//m = sendto(conexion,dataBuf,0,0,(struct sockaddr*)&cliente,sizeof(struct sockaddr_in));
			//para desbloquear el servidor
			//printf("llegue al final cliente\n");
		}
	    	
	}

	//el cliente termino de revisar sus archivos ahora le toca al servidor revisar en sentido contrario
	
	close(conexion);
	return 0;
}
