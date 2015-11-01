/*
Actividad 6: Cliente Neutro
Vidal Sánchez José Antonio
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <iostream>
using namespace std;
int main()
{
    string nombreHost;
    string servicio;
    cout<<"Dame el nombre del host"<<endl;
    getline(cin,nombreHost);
    cout<<"Dame el puerto/servicio"<<endl;
    getline(cin,servicio);

    struct addrinfo hints;
    struct addrinfo *addrList,*addrPuntero;
    int sockDes;
    char resHostName[256];
    int status;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    status=getaddrinfo(nombreHost.c_str(), servicio.c_str(), &hints,&addrList);
    if (status<0) {
        perror("getaddrinfo error:");
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }
    for(addrPuntero=addrList;addrPuntero!=nullptr;addrPuntero=addrPuntero->ai_next)
    {
        getnameinfo((sockaddr*)(addrPuntero->ai_addr),addrPuntero->ai_addrlen,
            resHostName,sizeof resHostName,nullptr,0,NI_NUMERICHOST);
        cout<<"Intentando conectar a la ip: "<<resHostName<<"..."<<endl;
        sockDes=socket(addrPuntero->ai_family,SOCK_STREAM,addrPuntero->ai_protocol);
        if(sockDes>=0)
        {
            if(connect(sockDes,addrPuntero->ai_addr,addrPuntero->ai_addrlen)>=0)
            {
                cout<<"->Conneccion exitosa"<<endl;
                cout<<"Cerrando coneccion..."<<endl;
                if(close(sockDes)>=0)
                {
                    cout<<"->coneccion cerrada"<<endl;
                    cout<<"------------------------------"<<endl;
                }
                else
                {
                    perror("Error de cierre");
                }
            }
            else{
                perror("Error de coneccion ");
            }
        }
        else perror("Error en la creacion del socket. ");
    }
    freeaddrinfo(addrList);
    return 0;
}
