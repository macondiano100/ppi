//Estudiante: Vidal Sanchez Jose Antonio
//Practica: 5
//Cliente con polling
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <poll.h>
using namespace std;
int main()
{
    constexpr int TAMANIO_BUFFER=255;
    int sockDes;
    string nombreHost;
    short puerto;
    int errorCode;
    struct hostent* hostInfo;
    sockaddr_in dir_servidor;
    /*cout<<"Dame el nombre del host"<<endl;
    getline(cin,nombreHost);
    cout<<"Dame el numero de puerto"<<endl;
    cin>>puerto;*/
    nombreHost="miles.gatuno.mx";
    puerto=12346;
    sockDes=socket(AF_INET,SOCK_STREAM,0);
    if(sockDes<0)
    {
        perror("Error al abrir el socket");
        exit(1);
    }
    if(!memset(&dir_servidor,0,sizeof(sockaddr_in)))
    {
        perror("Error creando el cliente");
        exit(1);
    }
    hostInfo=gethostbyname(nombreHost.c_str());
    if(hostInfo==nullptr)
    {
        printf("Error resolviendo el host");
        exit(1);
    }

    dir_servidor.sin_addr.s_addr=((struct in_addr*)(hostInfo->h_addr_list[0]))->s_addr;
    dir_servidor.sin_port=htons(puerto);
    dir_servidor.sin_family=AF_INET;
    errorCode=connect(sockDes,(sockaddr *)&dir_servidor,sizeof(sockaddr_in));

    if(errorCode<0)
    {
        perror("Error de conexion");
    }

    pollfd revisar;
    char buffer[TAMANIO_BUFFER+1];
    int bytesLeidos;
    int n;
    string mensaje;
    bool continuarLeyendo=true;
    struct timeval tiempoInicio,tiempoActual;
    long tiempoTranscurrido;
    revisar.fd=sockDes;
    revisar.events=POLLIN;
    gettimeofday(&tiempoInicio,nullptr);
    cout<<"Esperando transmision de: "<<nombreHost<<endl;
    while(continuarLeyendo)
    {
        n=poll(&revisar,1,40);
        if(n<0)
        {
            perror("Error de Polling: ");
        }
        else if(n>0)
        {

            if(revisar.revents&(POLLERR|POLLHUP))
            {
                perror("Error");
            }
            else if(revisar.revents&POLLIN)
            {
                bytesLeidos=read(sockDes,buffer,TAMANIO_BUFFER);
                continuarLeyendo=bytesLeidos;
                if(bytesLeidos>0) buffer[bytesLeidos]='\0';
                cout<<buffer;
            }
        }
        else
        {
            gettimeofday(&tiempoActual,nullptr);
            tiempoTranscurrido=
                (tiempoActual.tv_sec - tiempoInicio.tv_sec) * 1000 + (tiempoActual.tv_usec - tiempoInicio.tv_usec)/1000;
            if(tiempoTranscurrido>500)
            {
                cout<<".";
                cout.flush();
                gettimeofday(&tiempoInicio,nullptr);
            }
        }
    }
    cout<<"Fin de las transmisiones";
    if(close(sockDes)<0)
    {
        perror("Error al cerrar el socket");
    }
    return 0;
}
