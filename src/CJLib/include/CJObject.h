// CJObject.h : Header file
//

#ifndef __CJOBJECT_H_
#define __CJOBJECT_H_

#include "CJConfig.h"
#include "CJTypes.h"


class CJObject
{
public:
    CJObject();
    CJObject( char const* pNameBaseStr, char const* pNameUserStr);

    void  SetObjectStr( char const* pObjString);
    char const* GetObjectStr() {return dObjectNameStr;}

    U32        GetTraceId() {return dTraceId;}
    void       SetTraceLevel( U32 newLevel) {dTraceLevel = newLevel;}
    inline U32 GetTraceLevel() {return dTraceLevel;}


private:

    static U32 gNextTraceId;

    char dObjectNameStr[MAX_OBJ_STR_LEN];
    U32  dTraceId;
    U32  dTraceLevel;
};

#endif // __CJOBJECT_H_
