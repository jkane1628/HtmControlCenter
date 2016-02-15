// CJPersistFile.h : Header file
//

#ifndef __CJPERSISTFILE_H_
#define __CJPERSISTFILE_H_


#include "CJTypes.h"
#include "CJObject.h"

#define MAX_PERSIST_FILE_LINE_SIZE 512
#define MAX_PERSIST_FILE_FILENAME_SIZE 256
#define MAX_NUM_UNIQUE_TAGS_PER_FILE 1024

enum PersistFileDataType
{
   ePERSISTTYPE_U8,
   ePERSISTTYPE_U16,
   ePERSISTTYPE_U32,
   ePERSISTTYPE_U64,
   ePERSISTTYPE_S8,
   ePERSISTTYPE_S16,
   ePERSISTTYPE_S32,
   ePERSISTTYPE_S64,
   ePERSISTTYPE_CHAR,
   ePERSISTTYPE_16CHAR_STRING,
   ePERSISTTYPE_32CHAR_STRING,
   ePERSISTTYPE_64CHAR_STRING,
   ePERSISTTYPE_256CHAR_STRING,
   ePERSISTTYPE_PARENT_TAG,
   ePERSISTTYPE_LAST_ENTRY
};


struct PersistentFileElement
{
   char*               paramTagString;
   PersistFileDataType dataType;
   U32                 idEnum;
   U32                 maxPerFile;
   U32                 parentIdEnum;
}



/////////////////////////////
// EXAMPLE TABLE
/////////////////////////////
enum ExampleTagIdEnum
{
   eNONE,  // The Zero tag id value must be reserved for top level tags

   // Parent Tags
   eSOME64TAGID,
   eSOMESTRINGTAGID,
   eSOMEPARENTTAGID

   // EXAMPLE PARENT TAG CHILD VALUES
   eSOMECHILD64TAGID,
   eSOMECHILDCHARTAGID
};


PersistentTableElement ExampleParentTagDescriptorTable[] = {

   // DATA TYPE                 TAG STRING               TAG ID,                           MAX NUM PER FILE   PARENT TAG ID
   // -----------------------   -----------------------  -------------------------------   ----------------   ------------------        
   {  ePERSISTTYPE_S64,         "someU64Tag",            eSOME64TAGID                      1                  eNONE     },
   {  ePERSISTTYPE_STRING,      "someStringTag",         ePERSISTTYPE_32CHAR_STRING        1                  eNONE     },
                                                                                                                                    
   {  ePERSISTTYPE_PARENT_TAG,  "someParentTag",         eSOMEPARENTTAGID                  1                  eNONE     },
   {    ePERSISTTYPE_S64,       "someChildU64Tag",       eSOMECHILD64TAGID                 5/*per parent tag*/eSOMEPARENTTAGID  },
   {    ePERSISTTYPE_CHAR,      "someChildCharTag",      eSOMECHILDCHARTAGID               1                  eSOMEPARENTTAGID  },


   {  ePERSISTTYPE_LAST_ENTRY,  "", 0, 0, 0, 0} 
};

   
/////////////////////////////
// END OF EXAMPLE TABLE
/////////////////////////////



class CJPersistFile : public CJObject
{
public:
    CJPersistFile();
    ~CJPersistFile( );

    BOOL   Initialize( char const* baseFilename, PersistentTableElement* pDescriptorTable);
    BOOL   LoadFile();
    BOOL   SaveFile();


    U64    GetU64Param( U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0)
    {
       char temp64ValString[STRING_SIZE_U64];
       char* pTempStr = GetTableValueStr( parentId, parentIndex, childId, childIndex);
       return atoll( pTempStr);
    }

    BOOL   SetU64Param( U64 value,  U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0)
    {
       char temp64ValString[STRING_SIZE_U64];
       sprintf( temp64ValString, "%llu", value);
       return SetTableValueStr(temp64ValString, parentId, parentIndex, childId, childIndex);
    }
    



private:

   char*   GetTableValueStr( U32 parentIdValue, U32 parentIndex, U32 childIdValue, U32 childIndex)
   {

      // Use the table index to get the correct pointer

      // Check the indexes to make sure we are lower than the max allowed for this table entry

      // Use the parent and child index and dataType to find the correct string in the array






   }


   BOOL SetTableValueStr( char* valueString, U32 parentIdValue, U32 parentIndex, U32 childIdValue, U32 childIndex)
   {
      char* pTempString = GetTableValueStr( parentIdValue, parentIndex, childIdValue, childIndex);
      if( pTempString == NULL)
      {
         return FALSE;
      }
      strcpy( pTempString, valueString);
      return TRUE;
   }



   PersistentTableElement* FindTableEntryByTag( char* tagString)
   {
      PersistentTableElement* pTempElement = &dpDescriptorTable[0];
      while( pTempElement->dataType != ePERSISTTYPE_LAST_ENTRY)
      {
         if( strcmp(pTempElement->paramTagString, tagString) == 0)
         {
            // Found the element
            return pTempElement;
         }
         pTempElement++;
      }
      return NULL;
   }
   



   PersistentTableElement* FindTableEntryById( U32 idValue);
   {
      PersistentTableElement* pTempElement = &dpDescriptorTable[0];
      while( pTempElement->dataType != ePERSISTTYPE_LAST_ENTRY)
      {
         if( pTempElement->idEnum == idValue)
         {
            return pTempElement;
         }
         pTempElement++;
      }
      return NULL;
   }




   // BOOL ParseLine()




   char* baseFilename[MAX_PERSIST_FILE_FILENAME_SIZE];
   PersistentFileDescriptor* dpDescriptorTable;
   char dLineBuffer[MAX_PERSIST_FILE_LINE_SIZE];
   U32 dCurrentRevisionNumber;


   char* dpDataElementArray[MAX_NUM_UNIQUE_TAGS_PER_FILE];

   PersistentFileDescriptor* dpDescriptorTable;



};


#endif // __CJPERSISTFILE_H_
