// CJList.h : Header file
//

#ifndef __CJLIST_H_
#define __CJLIST_H_

#include "CJTypes.h"
#include "CJSem.h"

class CJListElement
{
public:
    CJListElement();
    CJListElement* dpNextElement;
    CJListElement* dpPrevElement;   
};  


class CJList 
{
public:
    CJList( char const* pListNameStr);
    CJList( char const* pListNameStrBase, char const* pListNameStr);
    virtual ~CJList( );

    virtual void AddTail( CJListElement* pListElement);
    virtual void AddHead( CJListElement* pListElement);
    
    BOOL InsertAfter( CJListElement* pLocationListElement, CJListElement* pNewListElement);
    BOOL InsertBefore( CJListElement* pLocationListElement, CJListElement* pNewListElement);
    
    BOOL Remove( CJListElement* pListElement);
    CJListElement* RemoveHead();
    CJListElement* RemoveTail();
    
    CJListElement* GetNext( CJListElement* pListElement);  // Get next starts with tail and move to head
    CJListElement* GetPrev( CJListElement* pListElement);  // Get next starts with head and move to tail
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
