/***************************************************************************

  CHttpClient.c

  Advanced Network component

  (c) 2003-2008 Daniel Campos Fernández <dcamposf@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/
#define __CHTTPCLIENT_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/multi.h>

#include "main.h"
#include "gambas.h"
#include "CCurl.h"
#include "CHttpClient.h"
#include "CProxy.h"


/*****************************************************
 CURLM : a pointer to use curl_multi interface,
 allowing asynchrnous work without using threads
 in this class. Here also a pipe will be stablished
 to link with Gambas watching interface
 ******************************************************/
extern CURLM *CCURL_multicurl;
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
/*******************************************************************
####################################################################
	CALLBACKS FROM CURL LIBRARY
####################################################################
********************************************************************/
void http_parse_header(CHTTPCLIENT *mythis)
{
	/* Obtains HTTP return code and return string */
	int myloop;
	int mypos=0;
	int npos=0;
	int nposend=0;
	int len;
	char *buf;

	if (!mythis->buf_header)
		return;

	buf = mythis->buf_header[0];
	len = strlen(buf);

	for (myloop=4;myloop<len;myloop++)
	{
		if (buf[myloop]==' ')
		{
			mypos=myloop+1;
			break;
		}
	}
	if (!mypos) return;

	for (myloop=mypos;myloop<len;myloop++)
	{
		if (buf[myloop]==' ')
		{
			if (npos)
			{
				npos=myloop+1;
				break;
			}
		}
		else
		{
			if ( !((buf[myloop]>47) && (buf[myloop]<58)) )
			{
				if (buf[myloop] != 13)
					return;
				else
					return;
			}
			else
			{
				npos++;
				if (npos>3) return;
				mythis->ReturnCode*=10;
				mythis->ReturnCode+=(buf[myloop]-48);

			}
		}
	}

	GB.Alloc(POINTER(&mythis->ReturnString),(nposend+1)*sizeof(char));
	mythis->ReturnString[nposend]=0;
	for (myloop=npos;myloop<(nposend+npos);myloop++)
		mythis->ReturnString[myloop-npos]=buf[myloop];

}

int http_header_curl(void *buffer, size_t size, size_t nmemb, void *_object)
{
	if (!THIS_HTTP->len_header)
	{
		GB.Alloc((void**)POINTER(&THIS_HTTP->buf_header),sizeof(char*));
		GB.Alloc((void**)&THIS_HTTP->buf_header[0],nmemb+1);
	}
	else
	{
		GB.Realloc((void**)POINTER(&THIS_HTTP->buf_header),sizeof(char*)*(1+THIS_HTTP->len_header));
		GB.Alloc((void**)&(THIS_HTTP->buf_header[THIS_HTTP->len_header]),nmemb+1);
		THIS_HTTP->buf_header[THIS_HTTP->len_header][nmemb]=0;
	}
	strncpy(THIS_HTTP->buf_header[THIS_HTTP->len_header],buffer,nmemb);
	THIS_HTTP->len_header++;

	if ( (THIS_STATUS==6) && THIS->async )
	{
		THIS_STATUS=4;
		GB.Ref(THIS);
		GB.Post(CCURL_raise_connect,(long)THIS);
	}

	return nmemb;
}



int http_write_curl(void *buffer, size_t size, size_t nmemb, void *_object)
{

	if (!THIS_HTTP->ReturnCode) http_parse_header(THIS_HTTP);

	if (THIS_FILE)
	{
		return fwrite(buffer,size,nmemb,THIS_FILE);
	}
	else
	{
		if (!THIS->len_data)
			GB.Alloc((void**)POINTER(&THIS->buf_data),nmemb);
		else
			GB.Realloc((void**)POINTER(&THIS->buf_data),nmemb+THIS->len_data);
		memcpy(THIS->buf_data+THIS->len_data,buffer,nmemb);
		THIS->len_data+=nmemb;
	}

	if (THIS->async)
	{
		GB.Ref(THIS);
		GB.Post(CCURL_raise_read,(long)THIS);
	}
	
	return nmemb;
}

void http_initialize_curl_handle(void *_object)
{
	if (THIS_CURL)
	{
		if (Adv_Comp ( THIS->user.userpwd,THIS->user.user,THIS->user.pwd))
		{
			CCURL_stop(_object);
			http_reset(_object);
			THIS_CURL=curl_easy_init();
		}
	}
	else
	{
		THIS_CURL=curl_easy_init();
		
	}
	
	if (!THIS->async)
	{
		curl_easy_setopt(THIS_CURL, CURLOPT_NOSIGNAL,1);
		curl_easy_setopt(THIS_CURL, CURLOPT_TIMEOUT,THIS->TimeOut);
	}
	
	curl_easy_setopt(THIS_CURL, CURLOPT_PRIVATE,(char*)_object);
	curl_easy_setopt(THIS_CURL, CURLOPT_USERAGENT, THIS_HTTP->sUserAgent);
	curl_easy_setopt(THIS_CURL, CURLOPT_ENCODING, THIS_HTTP->encoding);
	curl_easy_setopt(THIS_CURL, CURLOPT_HEADERFUNCTION, http_header_curl);
	curl_easy_setopt(THIS_CURL, CURLOPT_WRITEFUNCTION, http_write_curl);
	curl_easy_setopt(THIS_CURL, CURLOPT_WRITEDATA, _object);
	curl_easy_setopt(THIS_CURL, CURLOPT_WRITEHEADER, _object);
	curl_easy_setopt(THIS_CURL, CURLOPT_COOKIEFILE, THIS_HTTP->cookiesfile);
	
	if (THIS_HTTP->updatecookies)
		curl_easy_setopt(THIS_CURL, CURLOPT_COOKIEJAR, THIS_HTTP->cookiesfile);
	else
		curl_easy_setopt(THIS_CURL, CURLOPT_COOKIEJAR, NULL);

	Adv_proxy_SET (&THIS->proxy.proxy,THIS_CURL);
	Adv_user_SET  (&THIS->user, THIS_CURL);
	curl_easy_setopt(THIS_CURL, CURLOPT_URL,THIS_URL);

	THIS_HTTP->ReturnCode=0;
	if (THIS_HTTP->ReturnString)
	{
		GB.Free((void**)POINTER(&THIS_HTTP->ReturnString));
		THIS_HTTP->ReturnString=NULL;
	}
	http_reset(_object);
	THIS_STATUS=6;
	
	CCURL_init_stream(THIS);
}

/*************************************************
 GET HTTP method
 *************************************************/
int http_get (void *_object)
{
	if (THIS_STATUS > 0) return 1;

	THIS->iMethod=0;
	http_initialize_curl_handle(_object);
	curl_easy_setopt(THIS_CURL,CURLOPT_HTTPGET,1);
	
	if (THIS->async)
	{
		curl_multi_add_handle(CCURL_multicurl,THIS_CURL);
		CCURL_init_post();
		return 0;
	}
	
	CCURL_Manage_ErrCode(_object,curl_easy_perform(THIS_CURL));
	return 0;
}

/*************************************************
 POST HTTP method
 *************************************************/
int http_post (void *_object,char *sContent,char *sData,int lendata)
{
	int mylen;
	struct curl_slist *headers=NULL;
	int myloop;

	if (THIS_STATUS > 0) return 1;
	if (!sContent) return 2;
	if (!sData) return 3;
	for (myloop=0;myloop<strlen(sContent);myloop++)
		if (sContent[myloop]<32) return 1;


	http_initialize_curl_handle(_object);

	mylen=strlen(sContent) + strlen("Content-Type: ") + 1;
	GB.Alloc((void*)&THIS_HTTP->sContentType,mylen);
	GB.Alloc((void*)&THIS_HTTP->sPostData,lendata+1);
	strncpy(THIS_HTTP->sPostData,sData,lendata);
	THIS_HTTP->sContentType[0]=0;
	strcpy(THIS_HTTP->sContentType,"Content-Type: ");
	strcat(THIS_HTTP->sContentType,sContent);

	THIS->iMethod=1;
	headers=curl_slist_append(headers,THIS_HTTP->sContentType );
	curl_easy_setopt(THIS_CURL,CURLOPT_POSTFIELDS,THIS_HTTP->sPostData);
	curl_easy_setopt(THIS_CURL,CURLOPT_POSTFIELDSIZE,lendata);
	curl_easy_setopt(THIS_CURL,CURLOPT_HTTPHEADER,headers);


	if (THIS->async)
	{
		curl_multi_add_handle(CCURL_multicurl,THIS_CURL);
		CCURL_init_post();
		return 0;
	}
	
	CCURL_Manage_ErrCode(_object,curl_easy_perform(THIS_CURL));
	return 0;
}

////////////////////////////////////////////////////////////////////////
//########################## Other properties ##########################
////////////////////////////////////////////////////////////////////////
/*****************************************************************
 URL to work with
 *****************************************************************/


////////////////////////////////////////////////////////////////////////
//########################## Cookies ###################################
////////////////////////////////////////////////////////////////////////
BEGIN_PROPERTY ( CHttpClient_UpdateCookies )

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(THIS_HTTP->updatecookies);
		return;
	}

	if (THIS_STATUS > 0)
	{
		GB.Error ("UpdateCookies property can not be changed while working");
		return;
  	}

	if (VPROP(GB_BOOLEAN))
		THIS_HTTP->updatecookies=1;
	else
		THIS_HTTP->updatecookies=0;

END_PROPERTY

BEGIN_PROPERTY ( CHttpClient_CookiesFile )

	if (READ_PROPERTY)
	{
		GB.ReturnNewString(THIS_HTTP->cookiesfile,0);
		return;
	}

	if (THIS_STATUS > 0)
	{
		GB.Error ("CookiesFile property can not be changed while working");
		return;
  	}

	if (THIS_HTTP->cookiesfile)
	{
		GB.Free((void**)POINTER(&THIS_HTTP->cookiesfile));
		THIS_HTTP->cookiesfile=NULL;
	}
	if ( strlen(GB.ToZeroString(PROP(GB_STRING))) )
	{
		GB.Alloc( (void**)POINTER(&THIS_HTTP->cookiesfile), \
		          (strlen(GB.ToZeroString(PROP(GB_STRING)))+1)*sizeof(char));
		strcpy(THIS_HTTP->cookiesfile,GB.ToZeroString(PROP(GB_STRING)));
	}

END_PROPERTY


/***********************************************
 HTTP authentication : Plain, GSS negotiation, Digest or NTLM
************************************************/
BEGIN_PROPERTY (CHttpClient_Auth)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS_HTTP->auth);
		return;
	}

	if (THIS_STATUS > 0)
	{
		GB.Error ("Auth property can not be changed while working");
		return;
  	}

	if (Adv_user_SETAUTH (&THIS->user,VPROP(GB_INTEGER)) )
		GB.Error ("Unknown authentication method");
	else
		THIS_HTTP->auth=VPROP(GB_INTEGER);


END_PROPERTY
/*********************************************
 User Agent string to be sent to the server
 *********************************************/
BEGIN_PROPERTY ( CHttpClient_UserAgent )

	if (READ_PROPERTY)
	{
		GB.ReturnString(THIS_HTTP->sUserAgent);
		return;
	}

	if (THIS_STATUS > 0)
	{
		GB.Error ("UserAgent property can not be changed while working");
		return;
  	}
	GB.StoreString(PROP(GB_STRING), &THIS_HTTP->sUserAgent);


END_PROPERTY

BEGIN_PROPERTY(CHttpClient_Encoding)

	if (READ_PROPERTY)
		GB.ReturnString(THIS_HTTP->encoding);
	else
	{
		if (THIS_STATUS > 0)
			GB.Error("Encoding property can not be changed while working");
		else
			GB.StoreString(PROP(GB_STRING), &THIS_HTTP->encoding);
	}

END_PROPERTY

/*********************************************
 Return code received from Http Server
 *********************************************/
BEGIN_PROPERTY ( CHttpClient_ReturnCode )

	GB.ReturnInteger(THIS_HTTP->ReturnCode);

END_PROPERTY
/*********************************************
 Return string received from Http Server
 *********************************************/
BEGIN_PROPERTY ( CHttpClient_ReturnString )

	GB.ReturnNewZeroString(THIS_HTTP->ReturnString);

END_PROPERTY

/*********************************************
 Header of data received from Http Server
 *********************************************/
BEGIN_PROPERTY ( CHttpClient_Headers )

	GB_ARRAY retval;
	char *element;
	int myloop;

	if ( (THIS_STATUS==4) || (THIS_STATUS==0) )
	{
		if (THIS_HTTP->len_header)
		{
			GB.Array.New(&retval,GB_T_STRING,THIS_HTTP->len_header);
			for (myloop=0;myloop<THIS_HTTP->len_header;myloop++)
			{
			  GB.NewString(&element,THIS_HTTP->buf_header[myloop],strlen(THIS_HTTP->buf_header[myloop]));
			  *((char **)GB.Array.Get(retval,myloop)) = element;
			}
			GB.ReturnObject ( retval );
			return;
		}

	}


END_PROPERTY

//*************************************************************************
//#################### INITIALIZATION AND DESTRUCTION #####################
//*************************************************************************
/*************************************************
 Gambas object "Constructor"
 *************************************************/
BEGIN_METHOD_VOID(CHTTPCLIENT_new)

	char *tmp=NULL;
	
	GB.Alloc((void**)POINTER(&tmp),sizeof(char)*(1+strlen("http://127.0.0.1:80")));
	strcpy(tmp,"http://127.0.0.1:80");
	THIS_URL=tmp;
	GB.NewString(&THIS_HTTP->sUserAgent,"Gambas Http/1.0",0);
	
	tmp=NULL;
	GB.Alloc((void**)POINTER(&tmp),8);
	strcpy(tmp,"http://");
	THIS_PROTOCOL=tmp;

END_METHOD

/*************************************************
 Gambas object "Destructor"
 *************************************************/


BEGIN_METHOD_VOID(CHTTPCLIENT_free)

	http_reset(THIS);
	
	GB.FreeString(&THIS_HTTP->sUserAgent);
	GB.FreeString(&THIS_HTTP->encoding);
	GB.Free(POINTER(&THIS_HTTP->cookiesfile));
	GB.Free(POINTER(&THIS_HTTP->ReturnString));

END_METHOD



//*************************************************************************
//#################### METHODS ############################################
//*************************************************************************



BEGIN_METHOD(CHTTPCLIENT_Get,GB_STRING TargetHost;)

	if (!MISSING(TargetHost))
	{
		if (THIS_STATUS > 0)
		{
			GB.Error("Still active");
			return;
		}
		THIS_FILE=fopen(GB.ToZeroString(ARG(TargetHost)),"w");
		if (!THIS_FILE)
		{
			GB.Error("Unable to open file for writing");
			return;
		}
	}

	if (http_get(THIS))
	{
		GB.Error("Still active");
		return;
	}

END_METHOD



BEGIN_METHOD(CHTTPCLIENT_Post,GB_STRING sContentType;GB_STRING sData;GB_STRING TargetHost;)

	if (!MISSING(TargetHost))
	{
		if (THIS_STATUS > 0)
		{
			GB.Error("Still active");
			return;
		}
		THIS_FILE=fopen(GB.ToZeroString(ARG(TargetHost)),"w");
		if (!THIS_FILE)
		{
			GB.Error("Unable to open file for writing");
			return;
		}
	}

	switch(http_post (THIS,GB.ToZeroString(ARG(sContentType)),STRING(sData),LENGTH(sData)) )
	{
		case 1:
			GB.Error("Still active");
			return;
		case 2:
			GB.Error("Invalid content type");
			return;
		case 3:
			GB.Error("Invalid data");
			return;
	}

END_METHOD




void http_reset(void *_object)
{
	int myloop;

	if (THIS->buf_data)
	{
		GB.Free((void**)POINTER(&THIS->buf_data));
		THIS->buf_data=NULL;
	}
	if (THIS_HTTP->buf_header)
	{
		for (myloop=0;myloop < THIS_HTTP->len_header;myloop++)
			GB.Free((void**)&THIS_HTTP->buf_header[myloop]);
		GB.Free((void**)POINTER(&THIS_HTTP->buf_header));
		THIS_HTTP->buf_header=NULL;
	}
	if (THIS_HTTP->sContentType)
	{
		GB.Free((void**)POINTER(&THIS_HTTP->sContentType));
		THIS_HTTP->sContentType=NULL;
	}
	if (THIS_HTTP->sPostData)
	{
		GB.Free((void**)POINTER(&THIS_HTTP->sPostData));
		THIS_HTTP->sPostData =NULL;
	}
	THIS->len_data=0;
	THIS_HTTP->len_header=0;
}



BEGIN_METHOD_VOID(CHTTPCLIENT_Stop)

	CCURL_stop(THIS);
	http_reset(_object);

END_METHOD

//*************************************************************************
//#################### GAMBAS INTERFACE ###################################
//*************************************************************************
GB_DESC CHttpClientDesc[] =
{

  GB_DECLARE("HttpClient", sizeof(CHTTPCLIENT)),

  GB_INHERITS("Curl"),

  GB_METHOD("_new", NULL, CHTTPCLIENT_new, NULL),
  GB_METHOD("_free", NULL, CHTTPCLIENT_free, NULL),
  GB_METHOD("Stop", NULL, CHTTPCLIENT_Stop, NULL),
  GB_METHOD("Get", NULL, CHTTPCLIENT_Get, "[(TargetFile)s]"),
  GB_METHOD("Post", NULL, CHTTPCLIENT_Post, "(ContentType)s(Data)s[(TargetFile)s]"),

  GB_PROPERTY("Auth","i<Net,AuthNone,AuthBasic,AuthNTLM,AuthDIGEST,AuthGSSNEGOTIATE>",CHttpClient_Auth),
  GB_PROPERTY("CookiesFile", "s",CHttpClient_CookiesFile),
  GB_PROPERTY("UpdateCookies", "b",CHttpClient_UpdateCookies),
  GB_PROPERTY("Headers", "String[]", CHttpClient_Headers),
  GB_PROPERTY("UserAgent", "s", CHttpClient_UserAgent),
  GB_PROPERTY("Encoding", "s", CHttpClient_Encoding),

  GB_PROPERTY_READ("Code","i",CHttpClient_ReturnCode),
  GB_PROPERTY_READ("Reason","s",CHttpClient_ReturnString),
  
  GB_CONSTANT("_Properties", "s", HTTP_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Read"),

  GB_END_DECLARE
};

