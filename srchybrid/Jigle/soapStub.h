/* soapStub.h
   Generated by gSOAP 2.2.3b from JigleService-1.0.h
   Copyright (C) 2001-2003 Genivia inc.
   All Rights Reserved.
*/
#ifndef soapStub_H
#define soapStub_H

/* Types With Custom (De)serializers: */

/* Enumerations */

/* Classes and Structs */

#ifndef _SOAP_api__versionResponse
#define _SOAP_api__versionResponse
class SOAP_CMAC api__versionResponse
{ public:
	char *_r;
	virtual void soap_default(struct soap*); 
	virtual void soap_serialize(struct soap*) const;
	virtual void soap_mark(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*); 
};
#endif

#ifndef _SOAP_api__File
#define _SOAP_api__File
class SOAP_CMAC api__File
{ public:
	LONG64 s;
	char *h;
	int a;
	int t;
	int r;
	class FilenameArray *n;
	class StringArray *v;
	virtual void soap_default(struct soap*); 
	virtual void soap_serialize(struct soap*) const;
	virtual void soap_mark(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*); 
};
#endif

#ifndef _SOAP_FilenameArray
#define _SOAP_FilenameArray
/* Array of api:Filename schema type: */
class SOAP_CMAC FilenameArray
{ public:
	class api__Filename *__ptr;
	int __size;
	int __offset;
	virtual void soap_default(struct soap*); 
	virtual void soap_serialize(struct soap*) const;
	virtual void soap_mark(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*); 
};
#endif

#ifndef _SOAP_StringArray
#define _SOAP_StringArray
/* Array of xsd:string schema type: */
class SOAP_CMAC StringArray
{ public:
	char **__ptr;
	int __size;
	int __offset;
	virtual void soap_default(struct soap*); 
	virtual void soap_serialize(struct soap*) const;
	virtual void soap_mark(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*); 
};
#endif

#ifndef _SOAP_api__SearchResult
#define _SOAP_api__SearchResult
class SOAP_CMAC api__SearchResult
{ public:
	int t;
	int o;
	class FileArray *m;
	virtual void soap_default(struct soap*); 
	virtual void soap_serialize(struct soap*) const;
	virtual void soap_mark(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*); 
};
#endif

#ifndef _SOAP_FileArray
#define _SOAP_FileArray
/* Array of api:File schema type: */
class SOAP_CMAC FileArray
{ public:
	api__File *__ptr;
	int __size;
	int __offset;
	virtual void soap_default(struct soap*); 
	virtual void soap_serialize(struct soap*) const;
	virtual void soap_mark(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*); 
};
#endif

#ifndef _SOAP_api__searchFileResponse
#define _SOAP_api__searchFileResponse
class SOAP_CMAC api__searchFileResponse
{ public:
	api__File *_r;
	virtual void soap_default(struct soap*); 
	virtual void soap_serialize(struct soap*) const;
	virtual void soap_mark(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*); 
};
#endif

#ifndef _SOAP_api__searchResponse
#define _SOAP_api__searchResponse
class SOAP_CMAC api__searchResponse
{ public:
	api__SearchResult *_r;
	virtual void soap_default(struct soap*); 
	virtual void soap_serialize(struct soap*) const;
	virtual void soap_mark(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*); 
};
#endif

#ifndef _SOAP_api__Filename
#define _SOAP_api__Filename
class SOAP_CMAC api__Filename
{ public:
	char *t;
	int a;
	virtual void soap_default(struct soap*); 
	virtual void soap_serialize(struct soap*) const;
	virtual void soap_mark(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*); 
};
#endif

#ifndef _SOAP_api__version
#define _SOAP_api__version
struct api__version
{
};
#endif

#ifndef _SOAP_api__searchFile
#define _SOAP_api__searchFile
struct api__searchFile
{
	LONG64 s;
	char *h;
	int o;
};
#endif

#ifndef _SOAP_api__search
#define _SOAP_api__search
struct api__search
{
	char *p;
	char *e;
	int a;
	LONG64 l;
	LONG64 u;
	int f;
	int m;
	int o;
};
#endif

#ifndef _SOAP_SOAP_ENV__Header
#define _SOAP_SOAP_ENV__Header
/* SOAP Header: */
struct SOAP_ENV__Header
{
	void *dummy;	/* transient */
};
#endif

#ifndef _SOAP_SOAP_ENV__Code
#define _SOAP_SOAP_ENV__Code
/* SOAP Fault Code: */
struct SOAP_ENV__Code
{
	char *SOAP_ENV__Value;
	char *SOAP_ENV__Node;
	char *SOAP_ENV__Role;
};
#endif

#ifndef _SOAP_SOAP_ENV__Fault
#define _SOAP_SOAP_ENV__Fault
/* SOAP Fault: */
struct SOAP_ENV__Fault
{
	char *faultcode;
	char *faultstring;
	char *faultactor;
	char *detail;
	struct SOAP_ENV__Code *SOAP_ENV__Code;
	char *SOAP_ENV__Reason;
	char *SOAP_ENV__Detail;
};
#endif

/* Typedefs */
typedef char *xsd__string;
typedef int xsd__int;
typedef LONG64 xsd__long;

/* Variables */

/* Remote Methods */

SOAP_FMAC1 int SOAP_FMAC2 api__version(struct soap*, api__versionResponse *);

SOAP_FMAC1 int SOAP_FMAC2 api__searchFile(struct soap*, LONG64, char *, int, api__searchFileResponse *);

SOAP_FMAC1 int SOAP_FMAC2 api__search(struct soap*, char *, char *, int, LONG64, LONG64, int, int, int, api__searchResponse *);

/* Stubs */

SOAP_FMAC1 int SOAP_FMAC2 soap_call_api__version(struct soap*, const char*, const char*, api__versionResponse *);

SOAP_FMAC1 int SOAP_FMAC2 soap_call_api__searchFile(struct soap*, const char*, const char*, LONG64, char *, int, api__searchFileResponse *);

SOAP_FMAC1 int SOAP_FMAC2 soap_call_api__search(struct soap*, const char*, const char*, char *, char *, int, LONG64, LONG64, int, int, int, api__searchResponse *);

/* Skeletons */

SOAP_FMAC1 int SOAP_FMAC2 soap_serve(struct soap*);

SOAP_FMAC1 int SOAP_FMAC2 soap_serve_api__version(struct soap*);

SOAP_FMAC1 int SOAP_FMAC2 soap_serve_api__searchFile(struct soap*);

SOAP_FMAC1 int SOAP_FMAC2 soap_serve_api__search(struct soap*);
#endif

/* end of soapStub.h */
