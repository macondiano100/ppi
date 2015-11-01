/*
    Vidal Sanchez Jose Antonio
    Codigo; 213496757
    Multicast IPV6
*/
#include <net/if.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "util.h"
constexpr int BUFF_SIZE=1055;


int main(int argc, char* argv[])
{
    char      *multicastIP,*puertoMulticast;
    char      buffer[BUFF_SIZE+1];
    struct addrinfo *multicastAddr,*localAddr;
    SOCKET     sockDes;
    struct addrinfo   hints  = { 0 };
    if(argc==1)
    {
        multicastIP   = "ff02::1";
        puertoMulticast = "12345";
    }
    else if(argc==3)
    {
        multicastIP   = argv[1];
        puertoMulticast = argv[2];
    }
    else salirError("Argumentos incorrectos");
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags  = AI_NUMERICHOST;
    int status;
    if ((status = getaddrinfo(multicastIP, NULL, &hints, &multicastAddr)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        salirError("Error al resolver la ip dada");
    }

    hints.ai_family   = multicastAddr->ai_family;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_PASSIVE;
    if (getaddrinfo(NULL, puertoMulticast, &hints, &localAddr) != 0 )
        salirError("Error intentando obtener IP para conectarse");

    if ((sockDes = socket(localAddr->ai_family, localAddr->ai_socktype, 0)) < 0 )
        salirError("socket() failed");

    int yes=1;
    if (setsockopt(sockDes,SOL_SOCKET,SO_REUSEADDR,(char*)&yes,sizeof(int))<0) // evitar addres already in use
        salirError("Error en setSockOpt");

    if ( bind(sockDes, localAddr->ai_addr, localAddr->ai_addrlen) != 0 )
        salirError("Error al vincular el cliente con la IP");
    if ( multicastAddr->ai_family  == PF_INET &&
            multicastAddr->ai_addrlen == sizeof(struct sockaddr_in) ) /* IPv4 */
    {
        struct ip_mreq multicastRequest;

        multicastRequest.imr_multiaddr=((struct sockaddr_in*)(multicastAddr->ai_addr))->sin_addr;
        multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
        if ( setsockopt(sockDes, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                        (char*) &multicastRequest, sizeof(multicastRequest)))
            salirError("Error de setsockOpt");
    }
    else if ( multicastAddr->ai_family  == PF_INET6 &&
              multicastAddr->ai_addrlen == sizeof(struct sockaddr_in6) ) /* IPv6 */
    {
        struct ipv6_mreq multicastRequest;
        memcpy(&multicastRequest.ipv6mr_multiaddr,
               &((struct sockaddr_in6*)(multicastAddr->ai_addr))->sin6_addr,
               sizeof(multicastRequest.ipv6mr_multiaddr));
        multicastRequest.ipv6mr_interface = 0;
        if ( setsockopt(sockDes, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
                        (char*) &multicastRequest, sizeof(multicastRequest)) != 0 )
            salirError("Error de setsockOpt");
    }
    else
    {
        fprintf(stderr,"Protocolo de Internet no conocido");
        exit(EXIT_FAILURE);
    }
    printf("A la escucha de mensajes multicast usando el protocolo de internet: IP%s\n",
           multicastAddr->ai_family == PF_INET6 ? "v6" : "v4");
    sockaddr_storage senderAddr;
    socklen_t sockSize=sizeof(sockaddr_storage);
    char cadenaIP[256]= {};
    int bytesLeidos = 0;
    while(1)
    {
        if ((bytesLeidos = recvfrom(sockDes, buffer, BUFF_SIZE, 0,(sockaddr*)&senderAddr,&sockSize )) < 0)
            salirError("Error al recibir bytes");
        buffer[bytesLeidos]=0;
        if(getnameinfo((sockaddr*)&senderAddr,sockSize,cadenaIP,sizeof(cadenaIP),0,0,NI_NUMERICHOST)<0)
            salirError("Error al obtener informacion del enviante del mensaje");
        printf("Mensaje entrante de %s\n",cadenaIP);
        printf("Recibido el mensaje %s\n",buffer);
    }
        if(close(sockDes)<0)
        {
            salirError("Error al cerrar el socket");
        }
    freeaddrinfo(localAddr);
    freeaddrinfo(multicastAddr);
    exit(EXIT_SUCCESS);
}
