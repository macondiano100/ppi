//Estudiante: Vidal Sanchez Jose Antonio
//Practica: 3
//Servidor TCP con nombre de cliente
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
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
    sockaddr_in infoServer,info_cliente;
    constexpr long port=12345;
    constexpr int TAMANIO_MENSAJE=255;
    char mensaje[TAMANIO_MENSAJE+1]="Yo no sufro\n";
    serverDes=socket(AF_INET,SOCK_STREAM,0);
    if(serverDes<0)
    {
        perror("Error al crear socket");
        exit(1);
    }
    memset(&infoServer,0,sizeof(sockaddr_in));
    infoServer.sin_addr.s_addr=htonl (INADDR_ANY);
    infoServer.sin_port=htons(port);
    infoServer.sin_family=AF_INET;
    int optval=1;
    if (setsockopt(serverDes, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))<0)//evitar el TIME_WAIT
    {
        perror("setsockopt");
        exit(1);
    }
    if(bind(serverDes,(struct sockaddr *)&infoServer,sizeof(infoServer))<0)
    {
        perror("Error vincular el socket con el puerto/direccion");
        exit(1);
    }
    if(listen(serverDes,5)<0)
    {
        perror("Error al intentar escuchar conecciones");
        exit(1);
    }
    socklen_t size_sockaddr=sizeof(sockaddr_in);
    bool continuarServidor=true;
    hostent *hostentCliente=nullptr;
    while(continuarServidor)
    {
        cout<<"Esperando conecciones"<<endl;
        clientDes=accept(serverDes,(struct sockaddr *)&info_cliente,&size_sockaddr);
        if(clientDes<0)
        {
            perror("Error al aceptar coneccion entrante:");
            exit(1);
        }
        hostentCliente=gethostbyaddr(&(info_cliente.sin_addr),sizeof(info_cliente.sin_addr),AF_INET);
        cout<<"Conección entrante de "<<inet_ntoa(*((in_addr*)hostentCliente->h_addr))
        <<" '"<<hostentCliente->h_name<<"'"
        <<" con puerto: "<<ntohs(info_cliente.sin_port)<<endl;
        write(clientDes,mensaje,TAMANIO_MENSAJE);
        close(clientDes);
        cout<<"-->Presione 'x' para cerrar el servidor."<<endl;
        cout<<"   Cualquier otra tecla para esperar más conecciones:";
        if(::tolower(cin.get())=='x') continuarServidor=false;
    }

    close(serverDes);
    return 0;
}
