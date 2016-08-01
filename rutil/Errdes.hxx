#ifndef ERRDES_HXX
#define ERRDES_HXX

#include <iostream>
#include <map>
#include <string>
#include <errno.h>

#include <openssl/ssl.h>
#include <openssl/x509.h>

#ifdef _WIN32
#include <winsock.h>
#include <windows.h>
#endif

using namespace std;

class AbstractError
{

};

class NumericError : public AbstractError
{
public:
	string SearchErrorMsg(int Error, int ClassCode);
};

class ErrnoError : public NumericError
{
public:
	void CreateMappingErrorMsg();
};

class OpenSSLError : public NumericError
{
public:
	void CreateMappingErrorMsg();
};

class X509Error : public NumericError
{
public:
	void CreateMappingErrorMsg();
};

#endif
