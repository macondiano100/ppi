#ifndef URLANALIZER_H_INCLUDED
#define URLANALIZER_H_INCLUDED
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
using namespace std;
struct URLInfo
{
    string scheme;
    string domain;
    string port;
    string path;
};
bool validateScheme(string scheme);
size_t getScheme(string url,string &scheme);
bool validateIpv6Literal(string literal);
string servToPort(string service);
size_t getIpv6Literal(string url,string &literal,size_t pos=0);
size_t getIpv4Literal(string url,string &literal,size_t pos=0);
bool validateIpv4Literal(string iteral);
bool validateIpv4Literal(string literal);
bool validateDomain(string domain);
bool isHexDigit(char c);
size_t getDomain(string url,string &domain,size_t pos=0);
size_t getPath(string url,string &path,size_t pos=0);
size_t getPort(string url,string &port,size_t pos=0);
string portToService(int port);
bool getURLInfo(string url,URLInfo &info);


bool getURLInfo(string url,URLInfo &info)
{
    size_t aux=0;
    size_t aux2=0;
    aux=getScheme(url,info.scheme);
    if(aux>=url.size()) return false;
    aux2=aux;
    aux=getIpv6Literal(url,info.domain,aux2);
    if(aux==string::npos)
    {
        aux=getIpv4Literal(url,info.domain,aux2);
        if(aux==string::npos)
        {
            aux=getDomain(url,info.domain,aux2);
            if(aux==string::npos)
                return false;
        }
    }
    aux2=aux;
    if(aux2<url.size()&&url[aux2-1]==':')
    {
        aux=getPort(url,info.port,aux2);
        if(aux==string::npos||aux==aux2)return false;
    }
    aux2=aux;
    if(aux2<url.size()&&url[aux2-1]=='/')
    {
        aux=getPath(url,info.path,aux2);
        if(aux==string::npos) return false;
        aux2=aux;
    }
    if(info.path.empty()) info.path=R"(/)";
    if(info.port.empty()&&info.scheme.empty())
    {
        info.port="80";
        info.scheme="http";
    }
    else if(info.port.empty()) info.port=servToPort(info.scheme);
    else if(info.port.empty())info.scheme=portToService(stoi(info.port));
    return true;
}
size_t getPort(string url,string &port,size_t pos)
{
    size_t endpos;
    endpos=url.find_first_of(R"(/)",pos);
    string s=url.substr(pos,endpos-pos);
    if(s.empty()) return pos;
    for(auto c:s) if(!isdigit(c)) return string::npos;
    uint32_t portN=stoi(s);
    if (0<portN&&portN<0xffff)
    {
        port=s;
        return endpos+1;
    }
    else return string::npos;
}
size_t getPath(string url,string &path,size_t pos)
{
    string s=url.substr(pos,url.size());
    s='/'+s;;
    path=move(s);
    return url.size();
}
size_t getDomain(string url,string &domain,size_t pos)
{
    size_t endPos;
    endPos=url.find_first_of(R"(:/)",pos);
    if(endPos==string::npos) endPos=url.size();
    string s=url.substr(pos,endPos-pos);
    if(validateDomain(s))
    {
        domain=move(s);
        return endPos+1;
    }
    else return string::npos;
}
bool validateDomain(string domain)
{
    size_t nChars=0;
    size_t subDomainLen=0;
    for(auto c:domain)
    {
        if(isalnum(c)||c=='-')
        {
            subDomainLen++;
            if(subDomainLen>63) return false;
        }
        else if(c=='.')
        {
            if(subDomainLen>0)
            {
                subDomainLen=0;
            }
            else return false;
        }
        else return false;
        nChars++;
        if(nChars>255) return false;
    }
    return subDomainLen>0;
}
size_t getIpv4Literal(string url,string &literal,size_t pos)
{
    size_t endPos;
    endPos=url.find_first_of(R"(:/)",pos);
    if(endPos==string::npos) endPos=url.size();
    string s=url.substr(pos,endPos-pos);
    if(validateIpv4Literal(s))
    {
        literal=move(s);
        return endPos+1;
    }
    else return string::npos;
}
bool validateIpv6Literal(string literal)
{
    int sectionCount=0,digitCount=0;
    bool ceroGroupUsed=false;
    char aux=0;
    for(auto c:literal)
    {
        if(isHexDigit(c))
        {
            digitCount++;
            if(digitCount>4) return false;
        }
        else if(c==':')
        {
            if(aux==':')
            {
                if(ceroGroupUsed) return false;
                else
                {
                    ceroGroupUsed=true;
                }
            }
            sectionCount++;
            if(sectionCount>=8) return false;
            digitCount=0;
        }
        else return false;
        aux=c;
    }
    return sectionCount>0;
}
size_t getIpv6Literal(string url,string &literal,size_t pos)
{
    size_t beginPos,endPos;
    beginPos=url.find_first_of('[',pos);
    endPos=url.find_first_of(']',beginPos);
    if(beginPos==string::npos||endPos==string::npos) return string::npos;
    else
    {
        string s;
        s=url.substr(beginPos+1,endPos-beginPos-1);
        if(validateIpv6Literal(s))
        {
            literal=move(s);
            return endPos+1;
        }
        else return string::npos;
    }
}
bool isHexDigit(char c)
{
    c=toupper(c);
    return isdigit(c)||c=='A'||c=='B'||c=='C'||c=='D'||c=='E'||c=='F';
}
size_t getScheme(string url,string &scheme)
{
    size_t pos;
    string s;
    if((pos=url.find(R"(://)"))==string::npos)
    {
        return 0;
    }
    else
    {
        s=url.substr(0,pos);
        if(validateScheme(s))
        {
            scheme=move(s);
            return pos+3;
        }
        else return string::npos;
    }
}
bool validateScheme(string scheme)
{
    if(isalpha(scheme[0]))
    {
        int i,j;
        for(i=1,j=scheme.length(); i<j; i++)
        {
            if(!(isalnum(scheme[i])||scheme[i]=='-'||
                    scheme[i]=='+'||scheme[i]=='.'))
                return false;
        }
        return true;
    }
    else return false;
}
bool validateIpv4Literal(string literal)
{
    size_t auxPos=0,auxPos2=0;
    int byteCount=0;
    for(auto c:literal) if(!(isdigit(c)||c=='.')) return false;
    while(auxPos2<literal.size())
    {
        auxPos2=literal.find('.',auxPos);
        if(stoi(literal.substr(auxPos,auxPos2-auxPos).c_str())<0xFF)
        {
            byteCount++;
            if(byteCount>4)return false;
            else
            {
                auxPos=auxPos2+1;
            }
        }
        else return false;

    }
    return byteCount==4;
}
string portToService(int port)
{

    struct servent *appl_name;
    appl_name = getservbyport(htons(port),"TCP");
    if (!appl_name)
        return "";
    else
    {
        return appl_name->s_name;
    }
}
string servToPort(string service)
{

    struct servent *appl_name;
    appl_name = getservbyname(service.c_str(),NULL);
    if (!appl_name)
        return "";
    else
    {
        return to_string(ntohs(appl_name->s_port));
    }
}
#endif // URLANALIZER_H_INCLUDED
