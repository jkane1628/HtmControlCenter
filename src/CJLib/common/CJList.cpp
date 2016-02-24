// CJList.h : Header file
//

#include <stdio.h>

#include "CJTrace.h"
#include "CJAssert.h"
#include "CJSem.h"
#include "CJList.h"

// NOTE: This list object is a double linked list, it is NOT THREAD SAFE.

// Example of how list is structured:
// 
//    --------------
//         Head  
//             Next: NULL
// 
//             Prev: <--|
//    --------------    |   
//         Middle       |
//             Next: <--|
// 
//             Prev: <--|
//    --------------    |    
//         Middle       |
//             Next: <--|
// 
//             Prev: <--|
//    --------------    |     
//         Tail         |
//             Next: <--|
// 
//             Prev: NULL
//    --------------         


// Uncomment the CJLIST_CHECKING define to turn on list checking.  All add/remove operations will verify that the list pointers
// are all intact and the list count is correct.  Also, this will make the list return errors when items are added more than 
// once, or if they are being removed when they don't exist in the current list.  This can be helpful for finding invalid 
// list operations or finding memory corruptions.

//#define CJLIST_CHECKING



#ifdef CJLIST_CHECKING
  //#define VERIFY_LIST()
  #define VERIFY_LIST() VerifyList();
  #warning CJLIST_CHECKING is on, this hurts performance
#else
  #define VERIFY_LIST()
#endif

CJListElement::CJListElement()
{
   dpNextElement = NULL;
   dpPrevElement = NULL;   
}



CJList::CJList( char const* pListNameStr)
{
   dpHeadElement = NULL;
   dpTailElement = NULL;
   dNumElements = 0;
}

CJList::CJList( char const* pListNameStrBase, char const* pListNameStr)
{
   dpHeadElement = NULL;
   dpTailElement = NULL;
   dNumElements = 0;
}

CJList::~CJList( ) 
{}


void CJList::AddTail( CJListElement* pListElement)
{
#ifdef CJLIST_CHECKING
   if((pListElement->dpNextElement != NULL) || (pListElement->dpPrevElement != NULL))
   {
      CJTRACE(ERROR_LEVEL, "WARN: AddTail(%p) dpNext/PrevElement must be NULL, this element can only be on one list or Q at a time", pListElement);
   }
   if( DoesExistInList( pListElement) == TRUE)
   {
      CJTRACE(ERROR_LEVEL, "ERROR: Trying to add tail element that is already in list (%p)", pListElement);
   }
#endif
   CJTRACE(TRACE_DBG_LEVEL, "AddTail(%p) start dNumElements = %d", pListElement, dNumElements);



   d_lock_sem.Wait();
   if( dNumElements == 0)
   {
      // New element becomes the head and tail
      pListElement->dpNextElement = NULL;
      pListElement->dpPrevElement = NULL;
      dpTailElement = pListElement;
      dpHeadElement = pListElement;
   }
   else if( dNumElements == 1)
   {
      // New element becomes the tail
      pListElement->dpNextElement = dpHeadElement;
      pListElement->dpPrevElement = NULL;
      dpTailElement = pListElement;
      //dpHeadElement = Stays the same
      dpHeadElement->dpPrevElement = pListElement;
   }
   else
   {
      // New element gets added before the tail
      pListElement->dpNextElement = dpTailElement;
      pListElement->dpPrevElement = NULL;
      dpTailElement->dpPrevElement = pListElement;
      dpTailElement = pListElement;
   }
   dNumElements++;
   VERIFY_LIST()
   d_lock_sem.Post();
}


void CJList::AddHead( CJListElement* pListElement)
{
#ifdef CJLIST_CHECKING
   if((pListElement->dpNextElement != NULL) || (pListElement->dpPrevElement != NULL))
   {
      CJTRACE(ERROR_LEVEL, "WARN: AddHead(%p) dpNext/PrevElement must be NULL, this element can only be on one list or Q at a time", pListElement);
   }
   if( DoesExistInList( pListElement) == TRUE)
   {
      CJTRACE(ERROR_LEVEL, "ERROR: Trying to add head element that is already in list (%p)", pListElement);
   }
#endif
   if( pListElement == NULL)
      CJASSERT("ERROR: Cannot add a NULL element to the list AddHead");

   CJTRACE(TRACE_DBG_LEVEL, "AddHead(%p) start dNumElements = %d", pListElement, dNumElements);

   d_lock_sem.Wait();
   if( dNumElements == 0)
   {
      // New element becomes the head and tail
      pListElement->dpNextElement = NULL;
      pListElement->dpPrevElement = NULL;
      dpHeadElement = pListElement;
      dpTailElement = pListElement;
   }
   else if( dNumElements == 1)
   {
      // New element becomes the head
      pListElement->dpNextElement = NULL;
      pListElement->dpPrevElement = dpTailElement;
      dpHeadElement = pListElement;
      //dpTailElement = Stays the same
      dpTailElement->dpNextElement = pListElement;
   }
   else
   {
      // New element gets added after the head
      pListElement->dpNextElement = NULL;
      pListElement->dpPrevElement = dpHeadElement;
      dpHeadElement->dpNextElement = pListElement;
      dpHeadElement = pListElement;
   }
   dNumElements++;
   VERIFY_LIST()
   d_lock_sem.Post();
}


BOOL CJList::InsertAfter( CJListElement* pLocationListElement, CJListElement* pNewListElement)
{
#ifdef CJLIST_CHECKING
   if((pNewListElement->dpNextElement != NULL) || (pNewListElement->dpPrevElement != NULL))
   {
      CJTRACE(ERROR_LEVEL, "WARN: InsertAfter(%p) dpNext/PrevElement must be NULL, this element can only be on one list or Q at a time", pNewListElement);
   }
   if( DoesExistInList( pNewListElement) == TRUE)
   {
      sem_post(&d_lock_sem);
      CJTRACE(ERROR_LEVEL, "ERROR: Trying to insert element that is already in list (%p)", pNewListElement);
      return FALSE;
   }
#endif
   if((pLocationListElement == NULL) || (pNewListElement == NULL))
      CJASSERT("ERROR: Cannot insert a NULL element to the list InsertAfter");

   d_lock_sem.Wait();
   CJTRACE(TRACE_DBG_LEVEL, "InsertAfter(remove %p after %p) start dNumElements = %d", pNewListElement, pLocationListElement, dNumElements);
   if((dNumElements == 0) || (pLocationListElement == NULL) || (pNewListElement == NULL))
   {
      d_lock_sem.Post();
      return FALSE;
   }
   else
   {
      // New element gets added after the location element
      pNewListElement->dpNextElement = pLocationListElement->dpNextElement;
      pNewListElement->dpPrevElement = pLocationListElement;

      if( pNewListElement->dpNextElement == NULL)
      {
         // New element becomes the new head
         dpHeadElement = pNewListElement;
      }
      else
      {
         pNewListElement->dpNextElement->dpPrevElement = pNewListElement;
      }
      pLocationListElement->dpNextElement = pNewListElement;      
   }
   dNumElements++;
   VERIFY_LIST()
   d_lock_sem.Post();
   return TRUE;
}


BOOL CJList::InsertBefore( CJListElement* pLocationListElement, CJListElement* pNewListElement)
{
#ifdef CJLIST_CHECKING
   if((pNewListElement->dpNextElement != NULL) || (pNewListElement->dpPrevElement != NULL))
   {
      CJTRACE(ERROR_LEVEL, "WARN: InsertAfter(%p) dpNext/PrevElement must be NULL, this element can only be on one list or Q at a time", pNewListElement);
   }
   if( DoesExistInList( pNewListElement) == TRUE)
   {
      sem_post(&d_lock_sem);
      CJTRACE(ERROR_LEVEL, "ERROR: Trying to insert element that is already in list (%p)", pNewListElement);
      return FALSE;
   }
#endif
   if((pLocationListElement == NULL) || (pNewListElement == NULL))
      CJASSERT("ERROR: Cannot insert a NULL element to the list InsertBefore");

   CJTRACE(TRACE_DBG_LEVEL, "InsertBefore(remove %p before %p) start dNumElements = %d", pNewListElement, pLocationListElement, dNumElements);

   d_lock_sem.Wait();
   if((dNumElements == 0) || (pLocationListElement == NULL) || (pNewListElement == NULL))
   {
      d_lock_sem.Post();
      return FALSE;
   }
   else
   {
      // New element gets added before the location element
      pNewListElement->dpNextElement = pLocationListElement;
      pNewListElement->dpPrevElement = pLocationListElement->dpPrevElement;

      if( pNewListElement->dpPrevElement == NULL)
      {
         // New element becomes the new tail
         dpTailElement = pNewListElement;
      }
      else
      {
         pNewListElement->dpPrevElement->dpNextElement = pNewListElement;
      }
      pLocationListElement->dpPrevElement = pNewListElement;      
   }
   dNumElements++;
   VERIFY_LIST()
   d_lock_sem.Post();
   return TRUE;
}


BOOL CJList::Remove( CJListElement* pListElement)
{
#ifdef CJLIST_CHECKING
   if( DoesExistInList( pListElement) == FALSE)
   {
      sem_post(&d_lock_sem);

      VerifyList();


      CJTRACE(ERROR_LEVEL, "ERROR: Trying to remove element that is not in list (%p)", pListElement);
      return FALSE;
   }
#endif
   if(pListElement == NULL)
      CJASSERT("ERROR: Cannot remove a NULL element from the list");

   CJTRACE(TRACE_DBG_LEVEL, "Removing(%p) start dNumElements = %d", pListElement, dNumElements);

   d_lock_sem.Wait();
   if((pListElement == NULL) || (dNumElements == 0))
   {
      d_lock_sem.Post();
      return FALSE;
   }
   if( dpHeadElement == pListElement)
   {
      dpHeadElement = dpHeadElement->dpPrevElement;
   }
   if( dpTailElement == pListElement)
   {
      dpTailElement = dpTailElement->dpNextElement;
   }
   if( pListElement->dpNextElement != NULL)
   {
      pListElement->dpNextElement->dpPrevElement = pListElement->dpPrevElement;
   }
   if( pListElement->dpPrevElement != NULL)
   {
      pListElement->dpPrevElement->dpNextElement = pListElement->dpNextElement;
   }

   pListElement->dpNextElement = NULL;
   pListElement->dpPrevElement = NULL;

   dNumElements--;
   VERIFY_LIST()
   d_lock_sem.Post();
   return TRUE;
}


CJListElement* CJList::RemoveHead()
{
   VERIFY_LIST()
   CJListElement* pReturnElement = dpHeadElement;
   if( pReturnElement != NULL)
   {
      Remove( dpHeadElement);
   }
   return pReturnElement;
}


CJListElement* CJList::RemoveTail()
{
   VERIFY_LIST()
   CJListElement* pReturnElement = dpTailElement;
   if( pReturnElement != NULL)
   {
      Remove( dpTailElement);
   }
   return pReturnElement;
}


CJListElement* CJList::GetNext( CJListElement* pListElement)
{
   d_lock_sem.Wait();
   if( pListElement == NULL)
   {
      d_lock_sem.Post();
      return dpTailElement;
   }
   d_lock_sem.Post();
   return pListElement->dpNextElement;
}


CJListElement* CJList::GetPrev( CJListElement* pListElement)
{
   d_lock_sem.Wait();
   if( pListElement == NULL)
   {
      d_lock_sem.Post();
      return dpHeadElement;
   }
   d_lock_sem.Post(); 
   return pListElement->dpPrevElement;
}


CJListElement* CJList::GetHead()
{
   return dpHeadElement;
}


CJListElement* CJList::GetTail()
{
   return dpTailElement;
}


BOOL CJList::DoesExistInList( CJListElement* pListElement)
{
   d_lock_sem.Wait();
   
   CJListElement* pCurrent = dpTailElement;
   while( pCurrent)
   {
      if( pCurrent == pListElement)
      {
         d_lock_sem.Post();
         return TRUE;
      }
      pCurrent = pCurrent->dpNextElement;
   }
   d_lock_sem.Post();
   return FALSE;
}


BOOL CJList::VerifyList()
{
   CJListElement* pCurrent = dpHeadElement;
   U32 forwardListCount = 0;
   U32 backwardsListCount = 0;

   // Walk the list and make sure there are the correct number of elements
   CJTRACE(TRACE_DBG_LEVEL, "LIST: Walking List Backwards", forwardListCount, pCurrent);
   while( pCurrent)
   {
      CJTRACE(TRACE_DBG_LEVEL, "LIST: Backwards element %d, %p", backwardsListCount, pCurrent);
      backwardsListCount++;
      pCurrent = pCurrent->dpPrevElement;
      if(backwardsListCount > dNumElements+5)
         break;
   }

   CJTRACE(TRACE_DBG_LEVEL, "LIST: Walking List Forward", forwardListCount, pCurrent);
   pCurrent = dpTailElement;
   while( pCurrent)
   {
      CJTRACE(TRACE_DBG_LEVEL, "LIST: Forward element %d, %p", forwardListCount, pCurrent);
      forwardListCount++;
      pCurrent = pCurrent->dpNextElement;
      if(forwardListCount > dNumElements+5)
         break;
   }
   if( forwardListCount != dNumElements)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "LIST: ERROR - Verify List FAILED.  forwardListCount=%d, dNumElements=%d", forwardListCount, dNumElements);
      return FALSE;
   }
   if( backwardsListCount != dNumElements)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "LIST: ERROR - Verify List FAILED.  backwardsListCount=%d, dNumElements=%d", backwardsListCount, dNumElements);
      return FALSE;
   }
   CJTRACE(TRACE_DBG_LEVEL, "LIST: List Check Completed (OK).  fCount=%d, bCount=%d, dNumElements=%d", forwardListCount, backwardsListCount, dNumElements);
   return TRUE;
}




