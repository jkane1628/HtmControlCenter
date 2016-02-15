// CJSocket.h : Header file
//

#ifndef __CJSOCKET_H_
#define __CJSOCKET_H_

#include <netinet/in.h>

#include "CJTypes.h"
#include "CJConfig.h"
#include "CJObject.h"


enum eSocketError
{
    eSocket_Success,
    eSocket_CreateFailed,
    eSocket_BindingError,
    eSocket_AcceptFailure,
    eSocket_NoSuchHostname,
    eSocket_NotConnected,
    eSocket_SendFailure,
    eSocket_InvalidSocketState
};

enum eSocketState
{
    eSocketState_NotInitialized,
    eSocketState_Initialized,
    eSocketState_Listening,
    eSocketState_Connected,
    eSocketState_Error
};

class CJSocket : public CJObject
{
public:
   CJSocket(char const* pClientSocketNameStr);
   virtual ~CJSocket( );

   eSocketError InitializeClientSocket();
   eSocketError InitializeServerSocket( int portNumber);
   eSocketError Connect( char* hostname, int port);
   eSocketError AcceptConnection();

   int          ReceiveData( char* dataBuffer, int dataBufferLength);
   int          SendData( char* dataBuffer, int dataBufferLength);

private:
    
   int                dConnectedSocketFd;
   int                dServerSocketFd;
   int                dPortNumber;
   struct sockaddr_in dConnectedAddr;
   eSocketState       dState;
};


#endif // __CJSOCKET_H_
