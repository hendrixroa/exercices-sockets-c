#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
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
		comando = concat(comando," > FileMd5Server.txt");
		system(comando);
		FILE* fp;
		fp = fopen("FileMd5Server.txt","r");
		fread(buf, 32, 1, fp);
		fclose(fp);
		return buf;
	} 

	int main(int argc, char **argv){
	
	if(argc<2){
		printf("%s Indique un <puerto>\n",argv[0]);
		return 1;
	}
	
	char *dir="/CarpetaServidor";
	char *currentDir = getenv("PWD");
	//md5 = (char *)malloc(sizeof(char) * 32);
	
	currentDir = concat(currentDir,dir);
	
	//Variables para obtener los id's del servidor, cliente y el puerto especificado
	int conexion_servidor, conexion_cliente, puerto; 
	//Longitud del socket del cliente
	socklen_t longc; 
	//Estructuras del cliente y servidor
	struct sockaddr_in servidor, cliente;
	//Un buffer para el mensaje de entrada y otro de salida
	char bufferIn[100], bufferOut[100], dataBuf[BUFFER_SIZE]; 
	
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
	
	char *md5, *archMd5;
	md5 = (char *)malloc(sizeof(char) * 32);
	
	//Trataremos de 'bindear' y validaremos si es posible asociar el puerto al socket
	if(bind(conexion_servidor, (struct sockaddr *)&servidor, sizeof(servidor)) < 0){ 
		printf("Error al asociar el puerto a la conexion\n");
		close(conexion_servidor);
		return 1;
	}
	
	listen(conexion_servidor, 1); //Escuchemos si hay solicitudes con un maximo de 3 
	
	
	longc = sizeof(cliente); 
	//Una vez llegue un cliente aceptamos su conexion
	conexion_cliente = accept(conexion_servidor, (struct sockaddr *)&cliente, &longc); 
	
	if(conexion_cliente < 0){
		printf("Error al aceptar trafico\n");
		close(conexion_servidor);
		return 1;
	}
	
	char *name;
	int flag=0;
	int size = -1;
	
	while(1){
	
	memset( bufferIn, '\0', sizeof(char)*100);
	
	
	if(recv(conexion_cliente, bufferIn, 100, 0) < 0){ //Recibimos un mensaje de un cliente y lo guardamos en el buffer
		printf("Error al recibir los datos\n");
		close(conexion_servidor);
		return 1;
	}else{
		
		if(strcmp(bufferIn,"INCOMPLETE_SYNC") == 0){
			printf("INCOMPLETE_SYNC\n");
			send(conexion_cliente, "INCOMPLETE_SYNC" , 16 , 0);
			close(conexion_cliente);
			close(conexion_servidor);
			return 1;
		}
		
		if(strcmp(bufferIn,"PERFECT_SYNC") == 0){
			printf("PERFECT_SYNC\n");
			close(conexion_cliente);
			close(conexion_servidor);
			return 0;
		}
		
		printf("Comprobando archivo %s\n ",bufferIn);
		
		if ((fd1 = opendir(currentDir)) == NULL) {
			printf("La carpeta no se puede abrir \n");			
			return 1;
		}
	
		if ((fd2 = opendir(currentDir)) == NULL) {
			printf("La carpeta no se puede abrir \n");			
			return 1;
		}

		char *msj = strtok(bufferIn,":");
		char *fil = strtok(NULL,"| ");
		char *tam = strtok(NULL,"| ");

		char *msj2 = strtok(bufferIn,"|");
		char *fil2 = strtok(NULL,"| ");

		
		if(strcmp(msj,"Enviando")==0){
			printf("en servidor recibi %s %s %d\n",msj,fil,atoi(tam));
			
			size = atoi(tam);
			
			char *path = concat(currentDir,"/");
				 path = concat(path,fil);
			
			int fd,n,m=-1;

			if ((fd=open(path,O_CREAT|O_WRONLY,0600))==-1){
            	perror("open fail");
            	close(conexion_cliente);
            	exit (3);
        	}
			

			send(conexion_cliente, "Recibiendo" , 10 , 0);
			
			bzero(dataBuf,BUFFER_SIZE);
        
        	int restante = 0;
        	n=recv(conexion_cliente,dataBuf,BUFFER_SIZE,0);
	        
	       // printf("hola4 y size %d\n",size);

	        while(n) {
	        	
	        	//printf("data buf recibida %s %d\n",dataBuf, strlen(dataBuf));
	        	//printf("%d\n",strcmp(dataBuf,"\nFINALSEND"));
	        	restante += n; 

	            if(n==-1){
	                perror("recv fail");
	                exit(5);
	            }
	            if((m=write(fd,dataBuf,n))==-1){
	                perror("write fail");
	                exit (6);
	            }

	            if(restante==size){
	            	break;

	            }
	        	
	            //printf("segmentation1\n");
	            bzero(dataBuf,BUFFER_SIZE);
	            //printf("segmentation2\n");
	            n=recv(conexion_cliente,dataBuf,BUFFER_SIZE,0);

	        }
	        size = -1;
	        m=-1;

	        close(fd);
		} 

		if(strcmp(msj2,"Agregar_Archivo")==0){
			printf("en servidor recibi %s %s\n",msj2,fil2);
			
			
			char *path1 = concat(currentDir,"/");
				 path1 = concat(path1,fil2);
			
			int fd1,n1,m1=-1;

			if ((fd1=open(path1,O_CREAT|O_WRONLY,0600))==-1){
            	perror("open fail");
            	close(conexion_cliente);
            	exit (3);
        	}
			

			send(conexion_cliente, "Recibiendo" , 10 , 0);
			
			bzero(dataBuf,BUFFER_SIZE);
        
        	n1=recv(conexion_cliente,dataBuf,BUFFER_SIZE,0);
	        
	        while(n1) {
	     

	            if(n1==-1){
	                perror("recv fail");
	                exit(5);
	            }
	            if((m1=write(fd1,dataBuf,n1))==-1){
	                perror("write fail");
	                exit (6);
	            }

	        	
	            bzero(dataBuf,BUFFER_SIZE);
	            n1 = recv(conexion_cliente,dataBuf,BUFFER_SIZE,0);

	        }
	       
	        m1 = -1;

	        close(fd1);
	        close(conexion_cliente);
	        return 1;
		} 


		if(strcmp(msj2,"Borrar_Archivo")==0){
			printf("en servidor recibi %s %s\n",msj2,fil2);
			
			
			char *path3 = concat(currentDir,"/");
				 path3 = concat(path3,fil2);
			
			int status = unlink(path3);
       
	        close(conexion_cliente);
	        return 1;
		} 



	
	name = (char *)malloc(sizeof(char) * 32);
	while ((dp1 = readdir(fd1)) != NULL) {
	
		if (!strcmp(dp1->d_name, ".") || !strcmp(dp1->d_name, ".."))
	    	continue;
	    pos++;
	    if(strcmp(dp1->d_name,bufferIn)==0){
			strcpy(name,dp1->d_name);
			printf("Archivo encontrado %s\n",bufferIn);
			//le hago el md5 de una
			archMd5 = concat(currentDir,"/");
			archMd5 = concat(archMd5,bufferIn);
			md5 = ObtenerMd5(archMd5);
			md5[strlen(md5)] = '\0';
			send(conexion_cliente, md5 , 32 , 0);
			flag=1;
			bzero(archMd5,sizeof(archMd5));
			bzero(md5,sizeof(md5));
			bzero(name,sizeof(name));

		}    	   			
	}
	
			if(flag == 0){
				send(conexion_cliente, "No Existe", 9 , 0);
			}
			
		flag=0;
		
		
		
	}
}
	
	close(conexion_servidor);
	return 0;
}
