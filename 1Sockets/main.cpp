#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
using namespace std;
unsigned long cadenaIptoInt(char *cadenaIp );
int main()
{
    constexpr int TAMANIO_BUFFER=1;
    int sockDes;
    //char *cadenaIP="208.94.246.184";
    char *cadenaIP="127.0.0.1";
    short puerto=12345;
    int errorCode;
    sockaddr_in dir_servidor;
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

    dir_servidor.sin_addr.s_addr=htonl(cadenaIptoInt(cadenaIP));
    dir_servidor.sin_port=htons(puerto);
    dir_servidor.sin_family=AF_INET;
    errorCode=connect(sockDes,(sockaddr *)&dir_servidor,sizeof(sockaddr_in));

    if(errorCode<0)
    {
        perror("Error de conexion");
        exit(1);
    }

    printf("waitin\n");
    char buffer[TAMANIO_BUFFER+1];
    int bytesLeidos;
    while((bytesLeidos=read(sockDes,buffer,TAMANIO_BUFFER))>0)
    {
        buffer[bytesLeidos]='\0';
        printf("%s%d",buffer,bytesLeidos);
    }
    if(close(sockDes)<0)
    {
        perror("Error al cerrar el socket");
        exit(1);
    }
    return 0;
}
unsigned long cadenaIptoInt(char *cadenaIp )
{
    char buffer[3];
    unsigned long ipLong=0;
    unsigned long multiplier=0x01000000;

    for(int i,j=0; (*cadenaIp)!=0&&j<4; j++)
    {
        i=0;
        while(*cadenaIp!='.'&&*cadenaIp!='\0')
        {
            if(i>3) break;
            buffer[i]=*cadenaIp;
            i++;
            cadenaIp++;
        }
        buffer[i]=0;
        i=0;
        cadenaIp++;
        ipLong+=atol(buffer)*multiplier;
        multiplier>>=8;
    }
    return ipLong;

}
