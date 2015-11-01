#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

using SOCKET=int;
void salirError(const char* errorMessage)
{
    perror(errorMessage);
    exit(EXIT_FAILURE);
}


#endif // UTIL_H_INCLUDED
