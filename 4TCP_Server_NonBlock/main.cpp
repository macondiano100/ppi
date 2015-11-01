//Estudiante: Vidal Sanchez Jose Antonio
//Practica: 4
//Servidor TCP No bloqueante
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
using namespace std;
int main()
{
    int serverDes,clientDes;
    sockaddr_in6 infoServer,info_cliente;
    constexpr long port=12345;
    constexpr int TAMANIO_MENSAJE=255;
    char mensaje[TAMANIO_MENSAJE+1]="Yo no sufro\n";
    serverDes=socket(AF_INET6,SOCK_STREAM,0);
    if(serverDes<0)
    {
        perror("Error al crear socket");
        exit(1);
    }
    memset(&infoServer,0,sizeof(sockaddr_in6));
    infoServer.sin6_addr=(in6addr_any);
    infoServer.sin6_port=htons(port);
    infoServer.sin6_flowinfo=0;
    infoServer.sin6_family=AF_INET6;
    if(bind(serverDes,(struct sockaddr *)&infoServer,sizeof(infoServer))<0)
    {
        perror("Error al vincular el servidor");
        exit(1);
    }
    listen(serverDes,5);
    socklen_t size_sockaddr=sizeof(sockaddr_in);
    struct timeval tiempoInicio,tiempoActual;
    long tiempoTranscurrido;
    bool continuarServidor=true;
    cout<<"Esperando conecciones:"<<endl;
    gettimeofday(&tiempoInicio,nullptr);
    clientDes=accept(serverDes,(struct sockaddr *)&info_cliente,&size_sockaddr);
    if(clientDes<0)
    {
    }

    if(close(serverDes)<0)
    {
        cout<<"Error al cerrar el servidor";
        exit(1);
    }
    return 0;
}
