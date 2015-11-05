#include <iostream>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <algorithm>
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
#include <fstream>
#include <poll.h>
#include "urlAnalizer.h"
using namespace std;
using SOCKET=int;
constexpr int BUFFSIZE=1055;
constexpr char* NOMBRE_ARCHIVO="result.txt";
class HTTPHeader
{
private:
    string startLine;
    unordered_map<string,string> fields;
public:
    void setStartLine(string line)
    {
        startLine=line;
    }
    void setStartLine(string method,string uri,string httpVersion)
    {
        startLine=method+" "+uri+" "+R"(HTTP/)"+httpVersion;
    }
    bool fieldExists(string field)
    {
        return fields.count(field);
    }
    string getField(string field)
    {
        return fields.at(field);
    }
    string getStartLine()
    {
        return startLine;
    }
    void setField(string field,string value)
    {
        fields[field]=value;
    }
    vector<string> getFieldNames()
    {
        vector<string> fieldNames;
        transform(fields.cbegin(),fields.cend(),fieldNames.begin(),
                  [](pair<string,string> t)->string {return t.first;});
        return fieldNames;
    }
    string toSendableString()
    {
        stringstream s;
        s<<startLine<<"\r\n";
        for(auto p:fields)
        {
            s<<p.first<<":"<<p.second<<"\r\n";
        }
        s<<"\r\n";
        return s.str();
    }
    string toString()
    {
        stringstream s;
        s<<startLine<<endl;
        for(auto p:fields)
        {
            s<<p.first<<":"<<p.second<<endl;
        }
        return s.str();
    }
};
string getResponseType(HTTPHeader h);
void salirError(char const* error);
int createClientSocket(const char * const nombreHost,const char* const port="80");
void createRequestHeader(URLInfo info,HTTPHeader &h);
void stringToHeader(string s,HTTPHeader &h);
bool getResponse(SOCKET sock,HTTPHeader &responseHeader,ofstream &archivo)
{
    char responseBuff[BUFFSIZE+1];
    string aux;
    size_t pos=0;
    int nLeidos;
    bool readingHeader=true,okStatus,continuarLeyendo=true;
    int i,pollRes;
    string fileName;
    pollfd revisar {};
    revisar.fd=sock;
    revisar.events=POLLIN;
    archivo.open(NOMBRE_ARCHIVO,ios::binary|ios::out);
    while(continuarLeyendo)
    {
        pollRes=poll(&revisar,1,40);
        if(pollRes<0)
        {
            perror("Error de Polling: ");
        }
        else if(pollRes>0)
        {

            if(revisar.revents&(POLLERR|POLLHUP))
            {
                perror("Error");
            }
            else if(revisar.revents&POLLIN)
            {
                nLeidos=read(sock,responseBuff,BUFFSIZE);
                continuarLeyendo=nLeidos;
                responseBuff[nLeidos]='\0';
                if(readingHeader)
                {
                    aux.append(responseBuff);
                    pos=aux.find("\r\n\r\n",pos);
                    if(pos!=string::npos)
                    {
                        stringToHeader(aux.substr(0,pos+4),responseHeader);
                        if(getResponseType(responseHeader)=="OK")
                        {
                            archivo<<aux.substr(pos+4,aux.size());
                            okStatus=true;
                            readingHeader=false;
                        }
                        else
                        {
                            okStatus=false;
                            break;
                        }
                    }
                    pos=aux.size()-4;
                }
                else
                {
                    for(i=0; i<nLeidos; i++) archivo<<responseBuff[i];
                }
            }
        }
    }
    return okStatus;
}
int main(int args,char* argv[])
{
    HTTPHeader requestHeader;
    URLInfo urlInfo;
    string direccion;
    if(args==2) direccion=argv[1];
    else
    {
        cout<<"Deme la URL"<<endl;
        cin>>direccion;
    }
    getURLInfo(direccion.c_str(),urlInfo);
    createRequestHeader(urlInfo,requestHeader);
    SOCKET sock=createClientSocket(urlInfo.domain.c_str());
    string message=requestHeader.toSendableString().c_str();
    if(sock<0) salirError("Error de connecion");
    if(write(sock,message.c_str(),message.size())<0)salirError("Error al enviar el mensaje");
    HTTPHeader responseHeader;
    ofstream archivo;
    if(getResponse(sock,responseHeader,archivo))
    {
        cout<<"Mensaje recibido correctamente"<<endl;
        cout<<"Guardado en el archivo: "<<NOMBRE_ARCHIVO<<endl;
    }
    else{
        cout<<"Error de http"<<endl;
    }
        cout<<"----Cabecera de la respuesta-----"<<endl;
        cout<<responseHeader.toString();
    return 0;
}


void stringToHeader(string s,HTTPHeader &h)
{
    string line;
    size_t pos1,pos2,aux;
    pos1=pos2=0;
    pos1=s.find("\r\n");
    line=s.substr(0,pos1);
    pos1+=2;
    h.setStartLine(line);
    while(1)
    {
        pos2=s.find("\r\n",pos1);
        if(pos2>=s.size()||pos2==pos1) break;
        line=s.substr(pos1,pos2-pos1);
        aux=line.find(':');
        h.setField(line.substr(0,aux),line.substr(aux+1,line.size()));
        pos1=pos2+2;
    }
}
string getResponseType(HTTPHeader h)
{
    size_t pos;
    string startLine=h.getStartLine();
    pos=startLine.find(" ");
    pos=startLine.find(" ",pos+1);
    return (pos!=string::npos)? startLine.substr(pos+1,startLine.size()):"";
}
int createClientSocket(const char * const nombreHost,const char* const port)
{
    struct addrinfo hints;
    struct addrinfo *addrList,*addrPuntero;
    int sockDes;
    char resHostName[256];
    int status;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    status=getaddrinfo(nombreHost, port, &hints,&addrList);
    if (status<0) salirError("getaddrinfo error:");
    for(addrPuntero=addrList; addrPuntero!=nullptr; addrPuntero=addrPuntero->ai_next)
    {
        getnameinfo((sockaddr*)(addrPuntero->ai_addr),addrPuntero->ai_addrlen,
                    resHostName,sizeof resHostName,nullptr,0,NI_NUMERICHOST);
        sockDes=socket(addrPuntero->ai_family,SOCK_STREAM,addrPuntero->ai_protocol);
        if(sockDes>=0)
        {
            if(connect(sockDes,addrPuntero->ai_addr,addrPuntero->ai_addrlen)>=0) break;
        }
    }
    freeaddrinfo(addrList);
    return sockDes;
}
void salirError(char const* error)
{
    perror(error);
    exit(1);
}
void createRequestHeader(URLInfo info,HTTPHeader &h)
{
    h.setStartLine("GET",info.path,"1.1");
    h.setField("Host",info.domain+":80");
    h.setField("Accept-Encoding","None");
    h.setField("Connection","close");
}
