// CJQueue.h : Header file
//

#ifndef __CJQUEUE_H_
#define __CJQUEUE_H_

#include <semaphore.h>
#include "CJList.h"
  
class CJQueue : public CJList
{
public:
    CJQueue( char const* pQueueNameStr, U32 maxQdepth, BOOL blockOnMaxQueued);
    //~CJQueue( );

    BOOL Queue( CJListElement* pListElement);
    BOOL Dequeue( CJListElement** ppListElement, int timeout_ms=-1);
    CJListElement* Dequeue( int timeout_ms=-1);

    U32  GetCount() {return dElementCount;}

    // Override this and make them error out, it is too easy to call these from a Queue object, it makes for tricky bugs
    void AddTail( CJListElement* pListElement);
    void AddTail( CJListObject* pListObject);
    void AddHead( CJListElement* pListElement);
    void AddHead( CJListObject* pListObject);


private:
   U32  dElementCount;
   U32  dMaxQdepth;
   BOOL dBlockOnMaxQueued;

   sem_t d_max_limit_sem;
   sem_t d_count_sem;
   sem_t d_lock_sem;
};


#endif // __CJQUEUE_H_
