#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include <unistd.h>
using namespace std;

	 string ObtenerMd5(string file){
		string buf;
		string comando = "";
		comando.append("md5sum ");
		comando.append(file);
		comando.append(" > FileMd5Cliente.txt");
		system(comando.c_str());
		fstream fp;
		fp.open("FileMd5Cliente.txt", ios::in);
		fp >> buf;
		fp.close();
		return buf;
	}
	
	void enviarArchivo(string currentDir,string buf, int conexion){
		
		 string path;
				path.append(currentDir);
				path.append("/");
				path.append(buf);
									
			fstream  archivo;
			archivo.open(path.c_str(), ios::in);
			
			if(!archivo.is_open()){
				printf("Error al abrir el archivo a enviar\n");
				close(conexion);
				exit(1);
			}
			
			string aux;
			
			//bzero(aux,sizeof(char));
			aux.clear();
			
			while(!archivo.eof()){
				if(archivo.eof()){
					break;
				}
				getline(archivo,aux);
				cout <<"Envio" <<aux<<endl;
				if(write(conexion, aux.c_str() , aux.size())==-1){
					perror("Error al enviar el archivo");
				}
						
				//bzero(aux,sizeof(char));
				aux.clear();
			}
			
			archivo.close();
		
	}
	

	int main(int argc, char **argv){
		
	if(argc<2){
		printf("<host> <puerto>\n");
		return 1;
	}
	
	string dir="/CarpetaCliente";
	string currentDir = getenv("PWD");
	currentDir.append(dir);
	
	
	struct dirent *dp1, *dp2;
	DIR *fd1, *fd2;
	
	struct sockaddr_in cliente; 
	struct hostent *servidor;
	
	servidor = gethostbyname(argv[1]); //Obtenemos toda la data correspondiente al servidor
	
	if(servidor == NULL){ 
	printf("Host invalido\n");
	return 1;
	}
	
	int puerto, conexion,posicion=0;
	char buffer[32];
	string md5, archMd5;
	char opcion[1];
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
	
	if ((fd1 = opendir(currentDir.c_str())) == NULL) {
		printf("La carpeta no se puede abrir \n");			
		return 1;
	}
	
	if ((fd2 = opendir(currentDir.c_str())) == NULL) {
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
	    bzero(buf,sizeof(buf));
	    strcpy(buf,dp2->d_name);
	    write(conexion, buf, sizeof(buf));
	    bzero(buffer, 32);
	    read(conexion, buffer, 32);
	    //recibo el md5
	    
	    printf("recibo del server %s\n",buffer);
	    
		if(strcmp(buffer,"No Existe")!=0){
		//El archivo existe
		//Hacer mdsum
		printf("Archivo encontrado\n");
		
		archMd5.append(currentDir);
		archMd5.append("/");
		archMd5.append(dp2->d_name);
		md5 = ObtenerMd5(archMd5);
		
		cout << "md5 server" << md5<<endl;
		cout <<"md5 cliente"<< buffer<<endl;
		
		if(strcmp(buffer,md5.c_str())!=0){
			//caso donde el archivo se llama igual pero no contienen lo mismo
			//se tiene que renombrar el archivo file(1).txt para hacer la transferencia
			printf("INCOMPLETE_SYNC\n");
			write(conexion, "INCOMPLETE_SYNC", strlen("INCOMPLETE_SYNC"));
			bzero(buffer,32);
			read(conexion, buffer, 16);
			close(conexion);
			return 1;
		}
		
		archMd5.clear();
		md5.clear();	
				
		}else{
			//caso donde el archivo no existe
			printf("INCOMPLETE_SYNC\n");
			string msj = "";
			msj.append("Enviando|");
			msj.append(buf);
			write(conexion, msj.c_str() ,sizeof(msj));
			bzero(buffer,32);
			read(conexion, buffer, 16);
			enviarArchivo(currentDir,buf,conexion);		
		}
	    	
	}
	
	printf("PERFECT_SYNC\n");
	send(conexion, "PERFECT_SYNC", strlen("PERFECT_SYNC"), 0);
	close(conexion);
	return 0;
}
