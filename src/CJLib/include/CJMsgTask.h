// CJMsgTask.h : Header file
//

#ifndef __CJMSGTASK_H_
#define __CJMSGTASK_H_

#include "CJTypes.h"
#include "CJLibObject.h"
#include "CJList.h"

class CJThread;


#define CMD_HDR_FLAG_NONE              0x00000000
#define CMD_HDR_FLAG_CMD_BLOCK         0x00000001
#define CMD_HDR_FLAG_RSP_BLOCK         0x00000002
#define CMD_HDR_FLAG_REMOTE_CMD        0x00000004
#define CMD_HDR_FLAG_NO_DATA           0x00000008
#define CMD_HDR_FLAG_DATA_XFER_IN_CMD  0x00000010
#define CMD_HDR_FLAG_DATA_XFER_IN_RSP  0x00000020

#define CMD_STATE_READY                0x00000000
#define CMD_STATE_COMMAND_SENT         0x00000001
#define CMD_STATE_RECEIVED             0x00000002
#define CMD_STATE_WAITING_ON_PEERS     0x00000003
#define CMD_STATE_RESPONSE_SENT        0x00000004
#define CMD_STATE_COMPLETE             0x00000005


struct MsgHdr
{
   U32 opcode;
   U32 flags;
   U32 state;
};

class CJMsg : public CJListElement
{
public:
   CJMsg( U32 opcode, U32 flags);

   BOOL SendCommand();
   BOOL SendResponse();

   U32 opcode;
   MsgHdr dCmdHdr;

   void*  dpCmdBlkBuffer;
   void*  dpRspBlkBuffer;

   CJMsgTask* dpSourceMsgTask;
   CJMsgTask* dpDestMsgTask;

private:
   U32  dpCmdBlkBufferSize;
   U32  dpRspBlkBufferSize;

};



class CJMsgQueue : public CJList
{
public:
   CJMsgQueue( char* pMsgQueueName);
   //~CJMsgQueue( );

   void   Queue( CJMsg* pMsg);
   CJMsg* Dequeue();

private:

};



class CJMsgTask : public CJObject
{
public:
   CJMsgTask( char* pPoolNameStr);
   //~CJMsgTask( );

   void HandleMessage( CJMsg* pMsg) = 0;
   void HandleResponse( CJMsg* pMsg) = 0;


    
private:

   CJThread*   dpMsgThread;
   CJMsgQueue* dpMsgQueue;

   
};


#endif // __CJMSGTASK_H_
