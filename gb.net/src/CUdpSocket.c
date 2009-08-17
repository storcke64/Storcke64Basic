/***************************************************************************

  CUdpSocket.c

  (c) 2003-2004 Daniel Campos Fernández <dcamposf@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CUDPSOCKET_C
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "main.h"
#include "tools.h"

#include "CUdpSocket.h"


GB_STREAM_DESC UdpSocketStream = {
	open: CUdpSocket_stream_open,
	close: CUdpSocket_stream_close,
	read: CUdpSocket_stream_read,
	write: CUdpSocket_stream_write,
	seek: CUdpSocket_stream_seek,
	tell: CUdpSocket_stream_tell,
	flush: CUdpSocket_stream_flush,
	eof: CUdpSocket_stream_eof,
	lof: CUdpSocket_stream_lof,
	handle: CUdpSocket_stream_handle
};


DECLARE_EVENT (CUDPSOCKET_Read);
DECLARE_EVENT (CUDPSOCKET_SocketError);

void CUdpSocket_post_data(intptr_t Param)
{
	CUDPSOCKET *t_obj;
	t_obj=(CUDPSOCKET*)Param;
	GB.Raise(t_obj,CUDPSOCKET_Read,0);
	GB.Unref(POINTER(&t_obj));
}
void CUdpSocket_post_error(intptr_t Param)
{
	CUDPSOCKET *t_obj;
	t_obj=(CUDPSOCKET*)Param;
	GB.Raise(t_obj,CUDPSOCKET_SocketError,0);
	GB.Unref(POINTER(&t_obj));
}

static void clear_buffer(CUDPSOCKET *_object)
{
	if (THIS->buffer)
	{
		GB.Free(POINTER(&THIS->buffer));
		THIS->buffer_pos = 0;
		THIS->buffer_len = 0;
	}
}

static void fill_buffer(CUDPSOCKET *_object)
{
	socklen_t host_len;
	int ret, block;
	char buffer[1];
	
	//fprintf(stderr, "fill_buffer\n");
	
	clear_buffer(THIS);
	
	host_len = sizeof(THIS->addr);

	block = GB.Stream.Block(&THIS->stream, TRUE);
	USE_MSG_NOSIGNAL(ret = recvfrom(THIS->Socket, (void*)buffer, sizeof(char), MSG_PEEK | MSG_NOSIGNAL, (struct sockaddr*)&THIS->addr, &host_len));
	GB.Stream.Block(&THIS->stream, block);
	
	if (ioctl(THIS->Socket, FIONREAD, &THIS->buffer_len))
		return;
	
	//fprintf(stderr, "buffer_len = %d\n", THIS->buffer_len);
	
	if (THIS->buffer_len)
		GB.Alloc(POINTER(&THIS->buffer), THIS->buffer_len);
	
	USE_MSG_NOSIGNAL(ret = recvfrom(THIS->Socket, (void *)THIS->buffer, THIS->buffer_len, MSG_NOSIGNAL, (struct sockaddr*)&THIS->addr, &host_len));

	//fprintf(stderr, "recvfrom() -> %d\n", ret);
	
	if (ret < 0)
	{
		CUdpSocket_stream_close(&THIS->stream);
		THIS->iStatus=-4;
		return;
	}

// 	THIS->sport = ntohs(host.sin_port);
// 	GB.FreeString(&THIS->shost);
// 	GB.NewString (&THIS->shost , inet_ntoa(host.sin_addr) ,0);
}

void CUdpSocket_CallBack(int t_sock,int type, intptr_t param)
{
	//struct sockaddr_in t_test;
	//unsigned int t_test_len;
	struct timespec mywait;
	CUDPSOCKET *_object = (CUDPSOCKET *)param;

	/*	Just sleeping a little to reduce CPU waste	*/
	mywait.tv_sec=0;
	mywait.tv_nsec=100000;
	nanosleep(&mywait,NULL);

	if (THIS->iStatus<=0) return;

	//t_test.sin_port=0;
	//t_test_len=sizeof(struct sockaddr);

	//USE_MSG_NOSIGNAL(numpoll=recvfrom(t_sock,POINTER(buf), sizeof(char), MSG_PEEK | MSG_NOSIGNAL, (struct sockaddr*)&t_test, &t_test_len));
	fill_buffer(THIS);
	
	if (THIS->buffer)
	{
		GB.Ref((void*)THIS);
		GB.Post(CUdpSocket_post_data, (intptr_t)THIS);
	}
}
/* not allowed methods */
int CUdpSocket_stream_open(GB_STREAM *stream, const char *path, int mode, void *data){return -1;}
int CUdpSocket_stream_seek(GB_STREAM *stream, int64_t pos, int whence){return -1;}
int CUdpSocket_stream_tell(GB_STREAM *stream, int64_t *pos)
{
	*pos=0;
	return -1; /* not allowed */
}

int CUdpSocket_stream_flush(GB_STREAM *stream)
{
	return 0; /* OK */
}

int CUdpSocket_stream_handle(GB_STREAM *stream)
{
	void *_object = stream->tag;
	return THIS->Socket;
}

int CUdpSocket_stream_close(GB_STREAM *stream)
{
	void *_object = stream->tag;

	if ( !_object ) return -1;
	stream->desc=NULL;
	if (THIS->iStatus > 0)
	{
		GB.Watch (THIS->Socket,GB_WATCH_NONE,(void *)CUdpSocket_CallBack,(intptr_t)THIS);
		close(THIS->Socket);
		THIS->iStatus=0;
	}
	GB.FreeString(&THIS->thost);
	GB.FreeString(&THIS->tpath);
	
	if (THIS->path)
	{
		unlink(THIS->path);
		GB.FreeString(&THIS->path);
	}
	
	THIS->tport=0;
	THIS->iStatus=0;
	clear_buffer(THIS);
	return 0;
}

int CUdpSocket_stream_lof(GB_STREAM *stream, int64_t *len)
{
	void *_object = stream->tag;
	*len = THIS->buffer_len - THIS->buffer_pos;
	return 0;
}

int CUdpSocket_stream_eof(GB_STREAM *stream)
{
	void *_object = stream->tag;
	return THIS->buffer_pos >= THIS->buffer_len;
}

int CUdpSocket_stream_read(GB_STREAM *stream, char *buffer, int len)
{
	void *_object = stream->tag;
	int len_max;

	if ( !_object ) return TRUE;
	
	len_max = THIS->buffer_len - THIS->buffer_pos;
	
	if (len_max <= 0)
		return TRUE;
	
	if (len > len_max)
		len = len_max;
		
	memcpy(buffer, &THIS->buffer[THIS->buffer_pos], len);
	THIS->buffer_pos += len;
	
	GB.Stream.SetBytesRead(stream, len);
	
	return 0;
}

int CUdpSocket_stream_write(GB_STREAM *stream, char *buffer, int len)
{
	void *_object = stream->tag;
	int retval;
	struct in_addr dest_ip;
	NET_ADDRESS dest;
	size_t size;

	if (!THIS) return -1;
	
	CLEAR(&dest);

	if (THIS->tpath && *THIS->tpath)
	{
		dest.un.sun_family = PF_UNIX;
		strcpy(dest.un.sun_path, THIS->tpath);
		size = sizeof(struct sockaddr_un);
	}
	else
	{
		if (!inet_aton((const char*)THIS->thost, &dest_ip))
			return -1;
			
		dest.in.sin_family = PF_INET;
		dest.in.sin_addr.s_addr = dest_ip.s_addr;
		dest.in.sin_port = htons(THIS->port);
		size = sizeof(struct sockaddr_in);
		//bzero(&(THIS->addr.in.sin_zero), 8);
	}

	USE_MSG_NOSIGNAL(retval = sendto(THIS->Socket, (void*)buffer, len, MSG_NOSIGNAL, (struct sockaddr*)&dest, size));

	if (retval >= 0) 
		return 0;
	
	CUdpSocket_stream_close(stream);
	THIS->iStatus= -5;
	return -1;
}

/************************************************************************************************
################################################################################################
--------------------UDPSOCKET CLASS GAMBAS INTERFACE IMPLEMENTATION------------------------------
################################################################################################
***********************************************************************************************/

static bool update_broadcast(CUDPSOCKET *_object)
{
	if (THIS->Socket == 0)
		return FALSE;

	if (setsockopt(THIS->Socket, SOL_SOCKET, SO_BROADCAST, (char *)&THIS->broadcast, sizeof(int)) < 0)
	{
		GB.Error("Cannot set broadcast socket option");
		return TRUE;
	}
	else
		return FALSE;
}

static void dgram_start(CUDPSOCKET *_object)
{
	sa_family_t domain;
	size_t size;
	struct stat info;

	if (THIS->iStatus > 0)
	{
		GB.Error("Socket is active");
		return;
	}
	
	if (THIS->path && *THIS->path)
	{
		domain = PF_UNIX;
		if (strlen(THIS->path) >= NET_UNIX_PATH_MAX)
		{
			GB.Error("Socket path is too long");
			return;
		}
	}
	else
	{
		domain = PF_INET;
		if (THIS->port < 0 || THIS->port > 65535)
		{
			GB.Error("Invalid port number");
			return;
		}
	}

	if ((THIS->Socket = socket(domain, SOCK_DGRAM, 0)) < 0)
	{
		THIS->iStatus = -2;
		GB.Ref(THIS);
		GB.Post(CUdpSocket_post_error, (intptr_t)THIS);
		return;
	}

	if (update_broadcast(THIS))
	{
		THIS->iStatus = -2;
		GB.Ref(THIS);
		GB.Post(CUdpSocket_post_error,(intptr_t)THIS);
		return;
	}

	CLEAR(&THIS->addr);

	if (domain == PF_UNIX)
	{
		if (stat(THIS->path, &info) >= 0 && S_ISSOCK(info.st_mode))
			unlink(THIS->path);
		THIS->addr.un.sun_family = domain;
		strcpy(THIS->addr.un.sun_path, THIS->path);
		size = sizeof(struct sockaddr_un);
	}
	else
	{
		THIS->addr.in.sin_family = domain;
		THIS->addr.in.sin_addr.s_addr = htonl(INADDR_ANY);
		THIS->addr.in.sin_port = htons(THIS->port);
		size = sizeof(struct sockaddr_in);
		//bzero(&(THIS->addr.in.sin_zero), 8);
	}
	
	if (bind(THIS->Socket, (struct sockaddr*)&THIS->addr, size) < 0)
	{
		close(THIS->Socket);
		THIS->iStatus = -10;
		GB.Ref(THIS);
		GB.Post(CUdpSocket_post_error, (intptr_t)THIS);
		return;
	}

	THIS->iStatus = 1;
	THIS->stream.desc = &UdpSocketStream;
	GB.Stream.SetSwapping(&THIS->stream, htons(0x1234) != 0x1234);
	
	GB.Watch(THIS->Socket, GB_WATCH_READ, (void *)CUdpSocket_CallBack, (intptr_t)THIS);
}


/**********************************************************
This property gets status : 0 --> Inactive, 1 --> Working
**********************************************************/

BEGIN_PROPERTY ( CUDPSOCKET_Status )

	GB.ReturnInteger(THIS->iStatus);

END_PROPERTY

// 	THIS->sport = ntohs(host.sin_port);
// 	GB.FreeString(&THIS->shost);
// 	GB.NewString (&THIS->shost , inet_ntoa(host.sin_addr) ,0);

BEGIN_PROPERTY(CUDPSOCKET_SourceHost)

	if (THIS->addr.a.sa_family == PF_INET)
		GB.ReturnNewZeroString(inet_ntoa(THIS->addr.in.sin_addr));
	else
		GB.ReturnNull();

END_PROPERTY

BEGIN_PROPERTY(CUDPSOCKET_SourcePort)

	if (THIS->addr.a.sa_family == PF_INET)
		GB.ReturnInteger(ntohs(THIS->addr.in.sin_port));
	else
		GB.ReturnInteger(0);

END_PROPERTY

BEGIN_PROPERTY(CUDPSOCKET_SourcePath)

	if (THIS->addr.a.sa_family == PF_UNIX)
		GB.ReturnNewZeroString(THIS->addr.un.sun_path);
	else
		GB.ReturnNull();

END_PROPERTY

BEGIN_PROPERTY ( CUDPSOCKET_TargetHost )

		char *strtmp;
		struct in_addr rem_ip;
		if (READ_PROPERTY)
		{
			GB.ReturnString(THIS->thost);
		return;
		}

		strtmp=GB.ToZeroString(PROP(GB_STRING));
		if ( !inet_aton(strtmp,&rem_ip) )
	{
		GB.Error ("Invalid IP address");
		return;
	}
		GB.StoreString(PROP(GB_STRING), &THIS->thost);

END_PROPERTY

BEGIN_PROPERTY ( CUDPSOCKET_TargetPort )

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->tport);
	else
	{
		int port = VPROP(GB_INTEGER);
		
		if (port < 1 || port > 65535)
		{
			GB.Error("Invalid port number");
			return;
		}
	
		THIS->tport = port;
	}

END_PROPERTY

BEGIN_PROPERTY(CUDPSOCKET_TargetPath)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->tpath);
	else
	{
		if (PLENGTH() >= NET_UNIX_PATH_MAX)
		{
			GB.Error("Socket path is too long");
			return;
		}
		GB.StoreString(PROP(GB_STRING), &THIS->tpath);
	}

END_PROPERTY

/*************************************************
Gambas object "Constructor"
*************************************************/
BEGIN_METHOD_VOID(CUDPSOCKET_new)

	THIS->stream.tag = _object;

END_METHOD

/*************************************************
Gambas object "Destructor"
*************************************************/
BEGIN_METHOD_VOID(CUDPSOCKET_free)

	CUdpSocket_stream_close(&THIS->stream);

END_METHOD


BEGIN_METHOD_VOID (CUDPSOCKET_Peek)

	char *sData=NULL;
	size_t host_len;
	int retval=0;
	//int NoBlock=0;
	int peeking;
	int bytes=0;
	if (THIS->iStatus <= 0)
	{
		GB.Error ("Inactive");
		return;
	}


	peeking = MSG_NOSIGNAL | MSG_PEEK;

	ioctl(THIS->Socket,FIONREAD,&bytes);
	if (bytes)
	{
		GB.Alloc( POINTER(&sData),bytes*sizeof(char) );
		host_len = sizeof(THIS->addr);
		//ioctl(THIS->Socket,FIONBIO,&NoBlock);
		USE_MSG_NOSIGNAL(retval=recvfrom(THIS->Socket,(void*)sData,1024*sizeof(char) \
					,peeking,(struct sockaddr*)&THIS->addr, &host_len));
		if (retval<0)
		{
			GB.Free(POINTER(&sData));
			CUdpSocket_stream_close(&THIS->stream);
			THIS->iStatus=-4;
			GB.Raise(THIS,CUDPSOCKET_SocketError,0);
			GB.ReturnNewString(NULL,0);
			return;
		}
		//NoBlock++;
		//ioctl(THIS->Socket,FIONBIO,&NoBlock);
		if (retval>0)
			GB.ReturnNewString(sData,retval);
		else
			GB.ReturnNewString(NULL,0);
		GB.Free(POINTER(&sData));
	}
	else
	{
		GB.ReturnNull();
	}

END_METHOD

BEGIN_METHOD_VOID(CUDPSOCKET_Bind)

	dgram_start(THIS);

END_METHOD


BEGIN_PROPERTY(CUDPSOCKET_broadcast)

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(THIS->broadcast);
	}
	else
	{
		THIS->broadcast = VPROP(GB_BOOLEAN);
		update_broadcast(THIS);
	}

END_PROPERTY

BEGIN_PROPERTY(CUDPSOCKET_Port)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->port);
	else
	{
		int port = VPROP(GB_INTEGER);
		if (port < 0 || port > 65535)
		{
			GB.Error("Invalid port value");
			return;
		}
		if (THIS->iStatus > 0)
		{
			GB.Error("Socket is active");
			return;
		}
		THIS->port = port;
	}

END_PROPERTY

BEGIN_PROPERTY(CUDPSOCKET_Path)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->path);
	else
	{
		if (THIS->iStatus > 0)
		{
			GB.Error("Socket is active");
			return;
		}
		GB.StoreString(PROP(GB_STRING), &THIS->path);
	}

END_PROPERTY


/***************************************************************
Here we declare the public interface of UdpSocket class
***************************************************************/
GB_DESC CUdpSocketDesc[] =
{
	GB_DECLARE("UdpSocket", sizeof(CUDPSOCKET)),

	GB_INHERITS("Stream"),

	GB_EVENT("Error", NULL, NULL, &CUDPSOCKET_SocketError),
	GB_EVENT("Read", NULL, NULL, &CUDPSOCKET_Read),

	GB_METHOD("_new", NULL, CUDPSOCKET_new, NULL),
	GB_METHOD("_free", NULL, CUDPSOCKET_free, NULL),
	GB_METHOD("Bind", NULL, CUDPSOCKET_Bind, NULL),
	GB_METHOD("Peek","s",CUDPSOCKET_Peek,NULL),

	GB_PROPERTY_READ("Status", "i", CUDPSOCKET_Status),
	GB_PROPERTY_READ("SourceHost", "s", CUDPSOCKET_SourceHost),
	GB_PROPERTY_READ("SourcePort", "i", CUDPSOCKET_SourcePort),
	GB_PROPERTY_READ("SourcePath", "i", CUDPSOCKET_SourcePath),
	GB_PROPERTY("TargetHost", "s", CUDPSOCKET_TargetHost),
	GB_PROPERTY("TargetPort", "i", CUDPSOCKET_TargetPort),
	GB_PROPERTY("TargetPath", "s", CUDPSOCKET_TargetPath),

	GB_PROPERTY("Port", "i", CUDPSOCKET_Port),
	GB_PROPERTY("Path", "s", CUDPSOCKET_Path),
	
	GB_PROPERTY("Broadcast", "b", CUDPSOCKET_broadcast),

	GB_CONSTANT("_Properties", "s", "Port{Range:0;65535},Path,TargetHost,TargetPort,Broadcast"),
	GB_CONSTANT("_DefaultEvent", "s", "Read"),

	GB_END_DECLARE
};


