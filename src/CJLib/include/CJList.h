// CJList.h : Header file
//

#ifndef __CJLIST_H_
#define __CJLIST_H_

#include "CJTypes.h"
#include "CJObject.h"
#include "CJSem.h"

class CJListElement
{
public:
    CJListElement();
    CJListElement* dpNextElement;
    CJListElement* dpPrevElement;   
};

class CJListObject : public CJObject , public CJListElement
{
public:
    CJListObject( char const* pNameBaseStr, char const* pNameUserStr) : CJObject(pNameBaseStr,pNameUserStr){} 
};   


class CJList : public CJObject
{
public:
    CJList( char const* pListNameStr);
    CJList( char const* pListNameStrBase, char const* pListNameStr);
    virtual ~CJList( );

    virtual void AddTail( CJListElement* pListElement);
    virtual void AddTail( CJListObject* pListObject);
    virtual void AddHead( CJListElement* pListElement);
    virtual void AddHead( CJListObject* pListObject);

    BOOL InsertAfter( CJListElement* pLocationListElement, CJListElement* pNewListElement);
    BOOL InsertAfter( CJListObject* pLocationListObject, CJListObject* pNewListObject);
    BOOL InsertBefore( CJListElement* pLocationListElement, CJListElement* pNewListElement);
    BOOL InsertBefore( CJListObject* pLocationListObject, CJListObject* pNewListObject);

    BOOL Remove( CJListElement* pListElement);
    BOOL Remove( CJListObject* pListObject);
    CJListElement* RemoveHead();
    CJListObject* RemoveHeadObj();
    CJListElement* RemoveTail();
    CJListObject* RemoveTailObj();

    CJListElement* GetNext( CJListElement* pListElement);  // Get next starts with tail and move to head
    CJListObject*  GetNext( CJListObject* pListObject);
    CJListElement* GetPrev( CJListElement* pListElement);  // Get next starts with head and move to tail
    CJListObject*  GetPrev( CJListObject* pListObject);
    CJListElement* GetHead();
    CJListElement* GetTail();

    U32 GetNumElements() {return dNumElements;}

    BOOL DoesExistInList( CJListElement* pListElement);

protected:

   BOOL VerifyList();

   CJListElement* dpHeadElement;
   CJListElement* dpTailElement;
   U32 dNumElements;
   CJSem d_lock_sem;
};


#endif // __CJLIST_H_
