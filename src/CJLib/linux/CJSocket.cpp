// CJSocket.cpp : Implementation file
//

#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "CJTypes.h"
#include "CJConfig.h"
#include "CJObject.h"
#include "CJTrace.h"
#include "CJSocket.h"



CJSocket::CJSocket(char const* pSocketNameStr) : CJObject("LIB::SK::", pSocketNameStr)
{ 
    dConnectedSocketFd = -1;
    dServerSocketFd = -1;
    dState = eSocketState_NotInitialized;
}

CJSocket::~CJSocket( )
{
    if ( dConnectedSocketFd > 0)  close(dConnectedSocketFd);
    if ( dServerSocketFd > 0)     close(dServerSocketFd);
}

eSocketError CJSocket::InitializeClientSocket()
{
    dConnectedSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (dConnectedSocketFd < 0) 
    {
       CJTRACE( TRACE_ERROR_LEVEL, "ERROR: Failed to create client socket");
       return eSocket_CreateFailed;
    }
    dState = eSocketState_Initialized;
    return eSocket_Success;
}

eSocketError CJSocket::Connect(char* hostname, int port)
{
    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) {
        CJTRACE( TRACE_ERROR_LEVEL, "ERROR: No such hostname");
        return eSocket_NoSuchHostname;
    }

    bzero((char *) &dConnectedAddr, sizeof(dConnectedAddr));
    dConnectedAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&dConnectedAddr.sin_addr.s_addr, server->h_length);
    dConnectedAddr.sin_port = htons(port);
    if (connect( dConnectedSocketFd,(struct sockaddr *) &dConnectedAddr, sizeof(dConnectedAddr)) < 0) 
        CJTRACE( TRACE_ERROR_LEVEL, "ERROR: Failed to connect");
    dState = eSocketState_Connected;
    return eSocket_Success;
}

eSocketError CJSocket::InitializeServerSocket(int portNumber)
{
    struct sockaddr_in servAddr;

    if ( dState == eSocketState_NotInitialized) 
    {
        dServerSocketFd = socket(AF_INET, SOCK_STREAM, 0);
        if (dServerSocketFd < 0) 
        {
           CJTRACE( TRACE_ERROR_LEVEL, "ERROR: Failed to create server socket");
           return eSocket_CreateFailed;
        }

        bzero((char *) &servAddr, sizeof(servAddr));
        dPortNumber = portNumber;

        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = INADDR_ANY;
        servAddr.sin_port = htons(dPortNumber);

        if (bind(dServerSocketFd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) 
        {
            CJTRACE( TRACE_ERROR_LEVEL, "ERROR: Failed to bind server socket");
            return eSocket_BindingError;
        }
        dState = eSocketState_Initialized;
    }

    if ( dState == eSocketState_Initialized)
    {
        listen(dServerSocketFd,5);
        dConnectedSocketFd = -1;
        dState = eSocketState_Listening;
        return eSocket_Success;
    }
    return eSocket_InvalidSocketState;
}

eSocketError CJSocket::AcceptConnection()
{
    socklen_t remotelen = sizeof( dConnectedAddr);
    dConnectedSocketFd = accept( dServerSocketFd, (struct sockaddr *) &dConnectedAddr, &remotelen);
    if (dConnectedSocketFd < 0) 
    {
       CJTRACE( TRACE_ERROR_LEVEL, "ERROR: Failed to accept connection");
       return eSocket_AcceptFailure;
    }
    dState = eSocketState_Connected;
    return eSocket_Success;
}

int CJSocket::ReceiveData( char* dataBuffer, int dataBufferLength)
{
    int numBytes;
    if ( dState != eSocketState_Connected)
    {
        CJTRACE( TRACE_ERROR_LEVEL, "ERROR: Socket not connected");
        return -1;
    }
    numBytes = read( dConnectedSocketFd, dataBuffer, dataBufferLength);
    if (numBytes < 0) 
    {
        CJTRACE( TRACE_ERROR_LEVEL, "ERROR: Failed to read socket");
    }
    return numBytes;
}

int CJSocket::SendData( char* dataBuffer, int dataBufferLength)
{
    int bytesSent;
    if ( dState != eSocketState_Connected)
    {
        return -1;
    }
    bytesSent = write( dConnectedSocketFd, dataBuffer, dataBufferLength);
    if( dataBufferLength != bytesSent)
    {
        CJTRACE( TRACE_ERROR_LEVEL, "ERROR: Failed to write socket");
    }
    return bytesSent;
}
