// CJPool.h : Header file
//

#ifndef __CJPOOL_H_
#define __CJPOOL_H_

#include "CJTypes.h"
#include "CJObject.h"
//#include "CJList.h"

//class CJPool;
class CJSem;


class CJPool : public CJObject
{
public:
    CJPool( char const* pPoolNameStr, U32 maxNumObjects);
    ~CJPool( );

    BOOL AddElementToPool( void* pNewPoolObject);
    //BOOL RemoveElementFromPool();

    void*          ReserveElement();
    BOOL           ReleaseElement( void* pObjectToRelease);
    //void           ReleaseAllElements();

    U32 GetFreeCount() {return dNumFreeElements;}
    U32 GetTotalCount() {return dNumTotalElements;}
    U32 GetReservedCount(){return dNumReservedElements;}
 
    //void* GetFirstReserved();
    //void* GetNextReserved( void* pPoolObject);
    //void* GetLastReserved();
    //void* GetPrevReserved( void* pPoolObject);


private:

    void**          dppFreeArray;
    U32             dNumTotalElements;
    U32             dNumReservedElements;
    U32             dNumFreeElements;
    U32             dMaxNumObjects;

    CJSem*          dpLockSem;
};


#endif // __CJPOOL_H_
