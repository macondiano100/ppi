#ifndef HTTPHEADER_H
#define HTTPHEADER_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <sstream>
using namespace std;
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


#endif // HTTPHEADER_H
