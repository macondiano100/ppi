/*
    Vidal Sanchez Jose Antonio
    Codigo; 213496757
    Multicast IPV6
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "util.h"

int main(int argc, char *argv[])
{
    char*     multicastIP,*puertoMulticast,*mensaje;
    struct addrinfo hints = {};
    struct addrinfo* multicastAddr;
    SOCKET sock;

    if(argc==3)
    {
        multicastIP   = argv[1];
        puertoMulticast = argv[2];
    }
    else if(argc==1)
    {
        multicastIP   = "ff02::1";
        puertoMulticast = "12345";
    }
    else salirError("Argumentos incorrectos");
    mensaje="Mensaje de prueba de multicast";
    hints.ai_family   = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_NUMERICHOST;
    if ((getaddrinfo(multicastIP, puertoMulticast, &hints, &multicastAddr))<0 )
    {
        salirError("Error de getAddrInfo");
    }
    if ( (sock = socket(multicastAddr->ai_family, multicastAddr->ai_socktype, 0)) < 0 )
        salirError("Error al crear el socket");
    in_addr_t inaddr_any=INADDR_ANY;
    if(setsockopt (sock,
                   multicastAddr->ai_family == PF_INET6 ? IPPROTO_IPV6 : IPPROTO_IP,
                   multicastAddr->ai_family == PF_INET6 ? IPV6_MULTICAST_IF : IP_MULTICAST_IF,
                   (char*)&inaddr_any, sizeof(inaddr_any))<0)
        salirError("Error de setSokOpt");
    if ( sendto(sock,mensaje,strlen(mensaje)+1,0,
                multicastAddr->ai_addr, multicastAddr->ai_addrlen)<0)
        salirError("Error al enviar el mensaje");
    return 0;
}


