//Estudiante: Vidal Sanchez Jose Antonio
//Servidor Cliente
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
#include <cctype>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
static constexpr int MAX_HOSTNAME_LEN=255;
static constexpr int MAX_DNS_REQ_LEN=1500;
static constexpr uint16_t A_QTYPE=1;
static constexpr uint16_t AAAA_QTYPE=28;
static constexpr uint16_t CNAME_QTYPE=5;
static constexpr uint16_t IN_QCLASS=1;
void toQname(char *query,char *formatedQuery);
int createRequest( char *addr,char *DNSRequest,int type);
int interpretaQueriesSection(char* dnsPackage);
int interpretaAnswersSection(char* dnsPackage,int index);
int interpretaQname(char* dnsPackage,int index,char *hostName);
using  std::endl;
using  std::cout;
using  std::cin;
void debugDNSPack(char *package,int length)
{
    for(int i=0; i<length; i++,package++)
    {
        if(isalnum(*package))
        {
            std::cout<<*package;
        }
        else
        {
            std::cout<<(unsigned int)*package;
        }
        std::cout<<",";
    }
    std::cout<<std::endl;
    fflush(stdout);
}
void myGetAddrInfo( char *addr,int type);
int main()
{
    char addr[MAX_HOSTNAME_LEN+1]={};
    int op;
    cout<<"Dame el nombre del host: ";
    cin>>addr;
    cout<<"Tipo de pregunta?"<<endl;
    cout<<"1) A"<<endl;
    cout<<"2) AAAA"<<endl;
    cout<<"3) CNAME"<<endl;
    cin>>op;
    if(op==1)
    {
        myGetAddrInfo(addr,A_QTYPE);
    }
    else if(op==2)
    {
        myGetAddrInfo(addr,AAAA_QTYPE);
    }
    else if(op==3)
    {
        myGetAddrInfo(addr,CNAME_QTYPE);
    }
    return 0;
}
void myGetAddrInfo(char *addr,int type)
{
    char DNSRequest[MAX_DNS_REQ_LEN]="";
    int requestLen=createRequest(addr,DNSRequest,type);
    int sd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(sd<0)
    {
        perror("Error en la creacion de socket: ");
    }
    sockaddr_in bind_addr;
    bind_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    bind_addr.sin_port=htons(12345);
    bind_addr.sin_family=AF_INET;
    bind(sd,(sockaddr*)&bind_addr,sizeof(bind_addr));
    sockaddr_in dns_addr;
    dns_addr.sin_port=htons(53);
    dns_addr.sin_family=AF_INET;
    if(inet_aton("8.8.8.8",&(dns_addr.sin_addr))<0)
    {
        perror("Error traduciendo la direccion del DNS ");
    }
    if(sendto(sd,DNSRequest,requestLen,0,(sockaddr*)&dns_addr,sizeof(sockaddr_in))<0)
    {
        perror("Error en el envio de la peticion");
    }
    char DNSAnswer[MAX_DNS_REQ_LEN]="";
    socklen_t socklen=sizeof(sockaddr_in);
    int bytesLeidos;
    if((bytesLeidos=recvfrom(sd,DNSAnswer,MAX_DNS_REQ_LEN,0,(sockaddr*)&dns_addr,&socklen))<0)
    {
        perror("Error en el envio de la peticion");
    }

    uint16_t answerId;
    memcpy(&answerId,DNSAnswer,2);
    answerId=answerId;

    uint16_t flags;
    memcpy(&flags,DNSAnswer+2,2);

    cout<<"-----------------Cabecera-----------------------"<<endl;
    cout<<"Id de la respuesta: "<<answerId<<endl;
    cout<<"Recursion disponible en el DNS de Google: "<<((flags|(1<<8))?"Disponible":"No disponible")<<endl;
    cout<<"Respuesta autoritativa: "<<((flags|(1<<10))?"SI":"NO")<<endl;
    uint16_t currentByte=
        interpretaQueriesSection(DNSAnswer);
    interpretaAnswersSection(DNSAnswer,currentByte);


}
int interpretaAnswersSection(char* dnsPackage,int index)
{
    uint16_t answersCount;
    uint16_t currentByte;
    uint16_t qType;
    uint32_t ttl;
    uint16_t rLen;
    char mensajeRespuesta[MAX_HOSTNAME_LEN+1]= {};
    currentByte=6;
    memcpy(&answersCount,dnsPackage+currentByte,2);
    answersCount=ntohs(answersCount);
    currentByte=index;
    char name[MAX_HOSTNAME_LEN+1]= {};

    cout<<"---------------------Respuestas-----------"<<endl;
    cout<<"Respuestas recibidas: "<<answersCount<<endl;
    for(int i=1; i<=answersCount; i++)
    {
        memset(name,0,sizeof(name));
        currentByte=interpretaQname(dnsPackage,currentByte,name);
        memcpy(&qType,dnsPackage+currentByte,2);
        qType=ntohs(qType);
        currentByte+=2;
        currentByte+=2;
        memcpy(&ttl,dnsPackage+currentByte,4);
        ttl=ntohl(ttl);
        currentByte+=4;
        memcpy(&rLen,dnsPackage+currentByte,2);
        rLen=ntohs(rLen);
        currentByte+=2;
        memset(mensajeRespuesta,0,sizeof(mensajeRespuesta));
        if(qType==A_QTYPE)
        {
            in_addr dir= {};
            memcpy(&(dir.s_addr),dnsPackage+currentByte,rLen);
            inet_ntop(AF_INET,&dir,mensajeRespuesta,sizeof(mensajeRespuesta));
        }
        else if(qType==AAAA_QTYPE)
        {
            in6_addr dir= {};
            memcpy(&(dir.s6_addr),dnsPackage+currentByte,rLen);
            inet_ntop(AF_INET6,&dir,mensajeRespuesta,sizeof(mensajeRespuesta));
        }
        else if(qType==CNAME_QTYPE)
        {
            interpretaQname(dnsPackage,currentByte,mensajeRespuesta);
        }
        currentByte+=rLen;
        cout<<"---->Respuesta #"<<i<<endl;
        cout<<"Dominio (NAME): "<<name<<endl;
        cout<<"Tipo de Respuesta: ";
        if(qType==A_QTYPE) cout<<"A"<<endl;
        else if(qType==AAAA_QTYPE)cout<<"AAAA"<<endl;
        else if(qType==CNAME_QTYPE)cout<<"CNAME "<<endl;
        cout<<"Repuesta Valida por: "<<ttl<<endl;
        cout<<"Rlen:"<<rLen<<endl;
        cout<<"Respuesta: "<<mensajeRespuesta;

    }
    return currentByte;
}
int interpretaQueriesSection(char* dnsPackage)
{
    int currentByte=4;
    uint16_t qCount;

    memcpy(&qCount,dnsPackage+currentByte,2);
    qCount=ntohs(qCount);
    currentByte+=2;
    char addr[MAX_HOSTNAME_LEN+1];
    uint16_t qType;
    uint16_t qClass;
    currentByte=12;
    cout<<"-----------------Preguntas----------------------"<<endl;
    cout<<"Preguntas enviadas: "<<qCount<<endl;
    for(int i=1; i<=qCount; i++)
    {
        cout<<"--->Pregunta #"<<i<<endl;
        memset(addr,0,sizeof(addr));
        currentByte=interpretaQname(dnsPackage,currentByte,addr);
        memcpy(&qType,dnsPackage+currentByte,2);
        qType=ntohs(qType);
        currentByte+=2;
        memcpy(&qClass,dnsPackage+currentByte,2);
        qClass=ntohs(qClass);
        currentByte+=2;
        cout<<"Direccion preguntada: "<<addr<<endl;
        cout<<"Tipo de pregunta: ";
        if(qType==A_QTYPE) cout<<"A"<<endl;
        else if(qType==AAAA_QTYPE)cout<<"AAAA"<<endl;
        else if(qType==CNAME_QTYPE)cout<<"CNAME "<<endl;
        cout<<"QClass: ";
        if(qClass==IN_QCLASS) cout<<"Internet"<<endl;
    }
    return currentByte;
}
int interpretaQname(char* dnsPackage,int index,char *hostName)
{
    char sizeField=dnsPackage[index];
    if(sizeField&(0xC000))
    {
        uint16_t offset;
        memcpy(&offset,dnsPackage+index,2);
        offset ^= 0xC000;
        offset=ntohs(offset);
        interpretaQname(dnsPackage,dnsPackage[index+1],hostName);
        return index+2;
    }
    else
    {
        if(sizeField!=0)
        {
            strncpy(hostName,
                    dnsPackage+index+1,
                    sizeField);
            hostName[sizeField]='.';
            hostName+=sizeField+1;
            index+=sizeField+1;
            return interpretaQname(dnsPackage,index,hostName);
        }
        else{
        *(hostName-1)=0;
        return index+1;
        }
    }

}
int createRequest(char *addr,char *DNSRequest,int type)
{
    char qName[MAX_HOSTNAME_LEN]="";
    uint16_t requestID=1;
    uint16_t flags=0;
    memset(DNSRequest,0,MAX_DNS_REQ_LEN);
    memcpy(DNSRequest,&requestID,2);//id
    flags=htons(flags|(1<<8));
    memcpy(&DNSRequest[2],&flags,sizeof(flags));
    DNSRequest[5]=1;
    toQname(addr,qName);
    int qname_size=strlen(qName)+1;
    memcpy(&DNSRequest[12],qName,qname_size);
    int16_t qType=htons(type);
    int byteActual=12+qname_size;
    memcpy(&DNSRequest[byteActual],&qType,sizeof(qType));
    byteActual+=sizeof(qType);
    int16_t qClass=htons(1);
    memcpy(&DNSRequest[byteActual],&qClass,sizeof(qClass));
    byteActual+=sizeof(qType);
    return byteActual;
}
void toQname(char *query,char *formatedQuery)
{
    static char point='.';
    char *sizeFieldPointer=formatedQuery;
    char *queryPointer=query;
    char *formatedQueryPointer=formatedQuery+1;
    char wordLen=0;
    formatedQuery++;
    strcat(query,&point);
    for(; *queryPointer!=0; queryPointer++,formatedQueryPointer++)
    {
        if(*queryPointer=='.')
        {
            *sizeFieldPointer=wordLen;
            sizeFieldPointer=formatedQueryPointer;
            wordLen=0;
        }
        else
        {
            *formatedQueryPointer=*queryPointer;
            wordLen++;
        }
    }
    *formatedQueryPointer=0;
}
