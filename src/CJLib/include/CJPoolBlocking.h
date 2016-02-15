// CJPoolBlocking.h : Header file
//

#ifndef __CJPOOLBLOCKING_H_
#define __CJPOOLBLOCKING_H_

#include "CJTypes.h"
#include "CJObject.h"
//#include "CJList.h"


class CJSem;


class CJPoolBlocking : public CJObject
{
public:
    CJPoolBlocking( char const* pPoolNameStr, U32 maxNumObjects);
    ~CJPoolBlocking( );

    BOOL AddElementToPool( void* pNewPoolObject);
    //BOOL RemoveElementFromPool();

    void*          ReserveElement();
    BOOL           ReleaseElement( void* pObjectToRelease);
    //void           ReleaseAllElements();

    U32 GetFreeCount();
    U32 GetTotalCount() {return dNumTotalElements;}
    U32 GetReservedCount();
 
    //void* GetFirstReserved();
    //void* GetNextReserved( void* pPoolObject);
    //void* GetLastReserved();
    //void* GetPrevReserved( void* pPoolObject);


private:

    void**          dppFreeArray;
    U32             dNumTotalElements;
    U32             dMaxNumObjects;
    CJSem*          dpLockSem;
    CJSem*          dpFreeCountSem;

    //U32             dNumReservedElements;
    U32             dNumFreeElements;
};


#endif // __CJPOOLBLOCKING_H_
