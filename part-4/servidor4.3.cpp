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
		comando.append(" > FileMd5Server.txt");
		system(comando.c_str());
		fstream fp;
		fp.open("FileMd5Server.txt", ios::in);
		fp >> buf;
		fp.close();
		return buf;
	}
	
	void procesarArchivo(string currentDir, string fil, int conexion_servidor, int conexion_cliente){
		
			string path;
			path.append(currentDir);
			path.append("/");
			path.append(fil);
			fstream archivo;
			archivo.open( path.c_str(), ios::out | ios::app | ios::binary);
			if(!archivo.is_open()){
				printf("Error al crear el archivo enviado\n");
				close(conexion_servidor);
				exit(1);
			}
			
			write(conexion_cliente, "holaholaholahola" , 16);
			
			char aux[100];
			//aux.clear();
			bzero(aux,sizeof(aux));
			while(read(conexion_cliente, aux, sizeof(aux)) > 0){
				cout << "recibo "<< aux<<endl;
				archivo << aux;
				//aux.clear();
				bzero(aux,sizeof(aux));
			}
			bzero(aux,sizeof(aux));
			printf("Recibo ultimo de cliente %s\n",aux);
			archivo.close();
	} 

	int main(int argc, char **argv){
	
	if(argc<2){
		printf("%s Indique un <puerto>\n",argv[0]);
		return 1;
	}
	
	string dir="/CarpetaServidor";
	string currentDir = getenv("PWD");
	//md5 = (char *)malloc(sizeof(char) * 32);
	
	currentDir.append(dir);
	
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
	
	string md5, archMd5;
	
	//Trataremos de 'bindear' y validaremos si es posible asociar el puerto al socket
	if(bind(conexion_servidor, (struct sockaddr *)&servidor, sizeof(servidor)) < 0){ 
		printf("Error al asociar el puerto a la conexion\n");
		close(conexion_servidor);
		return 1;
	}
	
	listen(conexion_servidor, 3); //Escuchemos si hay solicitudes con un maximo de 3 
	
	
	longc = sizeof(cliente); 
	//Una vez llegue un cliente aceptamos su conexion
	conexion_cliente = accept(conexion_servidor, (struct sockaddr *)&cliente, &longc); 
	
	if(conexion_cliente < 0){
		printf("Error al aceptar trafico\n");
		close(conexion_servidor);
		return 1;
	}
	
	string name;
	int flag=0;
	
	bzero(bufferIn,sizeof(bufferIn));
	
	while(1){
		
	if(read(conexion_cliente, bufferIn, sizeof(bufferIn)) < 0){ //Recibimos un mensaje de un cliente y lo guardamos en el buffer
		printf("Error al recibir los datos\n");
		close(conexion_servidor);
		return 1;
	}else{
		
		if(bufferIn == "INCOMPLETE_SYNC"){
			printf("INCOMPLETE_SYNC\n");
			send(conexion_cliente, "INCOMPLETE_SYNC" , 16 , 0);
			close(conexion_servidor);
			return 1;
		}
		
		if( bufferIn == "PERFECT_SYNC"){
			printf("PERFECT_SYNC\n");
			close(conexion_servidor);
			return 0;
		}
		
		printf("Comprobando archivo %s\n",bufferIn);
		
		if ((fd1 = opendir(currentDir.c_str())) == NULL) {
			printf("La carpeta no se puede abrir \n");			
			return 1;
		}
	
		if ((fd2 = opendir(currentDir.c_str())) == NULL) {
			printf("La carpeta no se puede abrir \n");			
			return 1;
		}

		std::string buff = std::string(bufferIn);
		string msj = buff.substr(0, buff.find("|"));
		string fil = buff.substr(buff.find("|") +1 , buff.length());
		
		if(msj == "Enviando"){
			cout << "en servidor recibi " << msj << fil <<endl;
			
			procesarArchivo(currentDir,fil,conexion_servidor,conexion_cliente);
		}else{ 
	
			while ((dp1 = readdir(fd1)) != NULL) {
			
				if (!strcmp(dp1->d_name, ".") || !strcmp(dp1->d_name, ".."))
					continue;
				pos++;
				if(strcmp(dp1->d_name,bufferIn)==0){
					name = dp1->d_name;
					cout << "Archivo encontrado " << bufferIn << endl;
					//le hago el md5 de una
					archMd5.append(currentDir);
					archMd5.append("/");
					archMd5.append(bufferIn);
					md5 = ObtenerMd5(archMd5);
					write(conexion_cliente, md5.c_str() , 32);
					flag=1;
				}    	   			
			}
			
					if(flag == 0){
						cout<<"archivo no existe "<<endl;
						write(conexion_cliente, "No Existe", 9 );
					}
					
				flag=0;
				archMd5="";
				md5="";
				name="";
				
		}
	
	}
}
	
	close(conexion_servidor);
	
	return 0;
}
