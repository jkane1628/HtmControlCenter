// CJPersistFile.h : Header file
//

#ifndef __CJPERSISTFILE_H_
#define __CJPERSISTFILE_H_


#include "CJTypes.h"
#include "CJObject.h"

#define MAX_PERSIST_FILE_LINE_SIZE 512
#define MAX_PERSIST_FILE_FILENAME_SIZE (256-16) //Save 16 chars for NULL, 10 digit revision number, '_', and a '.xml'
#define MAX_NUM_UNIQUE_TAGS_PER_FILE 1024

enum PersistElementDataType
{                                  // Max String Len (including NULL terminator)
   ePERSISTTYPE_U8,                // 4
   ePERSISTTYPE_U16,               // 6
   ePERSISTTYPE_U32,               // 11
   ePERSISTTYPE_U64,               // 21
   ePERSISTTYPE_S8,                // 4
   ePERSISTTYPE_S16,               // 6
   ePERSISTTYPE_S32,               // 11
   ePERSISTTYPE_S64,               // 21
   ePERSISTTYPE_CHAR,              // 2
   ePERSISTTYPE_16CHAR_STRING,     // 17
   ePERSISTTYPE_32CHAR_STRING,     // 33
   ePERSISTTYPE_64CHAR_STRING,     // 65
   ePERSISTTYPE_256CHAR_STRING,    // 257
   ePERSISTTYPE_PARENT_TAG,        // NA
   ePERSISTTYPE_LAST_ENTRY         // NA
};

#define STRING_SIZE_U8    4
#define STRING_SIZE_U16   6
#define STRING_SIZE_U32   11
#define STRING_SIZE_U64   21
#define STRING_SIZE_S8    4
#define STRING_SIZE_S16   6
#define STRING_SIZE_S32   11
#define STRING_SIZE_S64   21
#define STRING_SIZE_CHAR  2
#define STRING_SIZE_16CHAR_STRING  17
#define STRING_SIZE_32CHAR_STRING  33
#define STRING_SIZE_64CHAR_STRING  65
#define STRING_SIZE_256CHAR_STRING 257


struct PersistentTableElement
{
   PersistElementDataType dataType;
   char const*            paramTagString;
   U32                    idEnum;
   U32                    maxPerFile;
   U32                    parentIdEnum;
};

#define eNONE 0


/////////////////////////////
// EXAMPLE TABLE
/////////////////////////////
//enum ExampleTagIdEnum
//{
//   eNONE,  // The Zero tag id value must be reserved for top level tags
//
//   // Parent Tags
//   eSOME64TAGID,
//   eSOMESTRINGTAGID,
//   eSOMEPARENTTAGID
//
//   // EXAMPLE PARENT TAG CHILD VALUES
//   eSOMECHILD64TAGID,
//   eSOMECHILDCHARTAGID
//};
//
//
//PersistentTableElement ExampleParentTagDescriptorTable[] = {
//
//   // DATA TYPE                 TAG STRING               TAG ID,                           MAX NUM PER FILE   PARENT TAG ID
//   // -----------------------   -----------------------  -------------------------------   ----------------   ------------------        
//   {  ePERSISTTYPE_S64,         "someU64Tag",            eSOME64TAGID                      1                  eNONE     },
//   {  ePERSISTTYPE_STRING,      "someStringTag",         ePERSISTTYPE_32CHAR_STRING        1                  eNONE     },
//                                                                                                                                    
//   {  ePERSISTTYPE_PARENT_TAG,  "someParentTag",         eSOMEPARENTTAGID                  1                  eNONE     },
//   {    ePERSISTTYPE_S64,       "someChildU64Tag",       eSOMECHILD64TAGID                 5/*per parent tag*/eSOMEPARENTTAGID  },
//   {    ePERSISTTYPE_CHAR,      "someChildCharTag",      eSOMECHILDCHARTAGID               1                  eSOMEPARENTTAGID  },
//
//
//   {  ePERSISTTYPE_LAST_ENTRY,  "", 0, 0, 0, 0} 
//};
/////////////////////////////
// END OF EXAMPLE TABLE
/////////////////////////////



class CJPersistFile : public CJObject
{
public:
    CJPersistFile(char const* name);
    ~CJPersistFile( );

    BOOL   Initialize( char const* baseFilename, PersistentTableElement* pDescriptorTable);
    BOOL   LoadFile();
    BOOL   SaveFile();



    U8     GetU8Param( U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    BOOL   SetU8Param( U8 value,  U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    U16    GetU16Param( U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    BOOL   SetU16Param( U16 value,  U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    U32    GetU32Param( U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    BOOL   SetU32Param( U32 value,  U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    U64    GetU64Param( U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    BOOL   SetU64Param( U64 value,  U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);

    S8     GetS8Param( U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    BOOL   SetS8Param( S8 value,  U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    S16    GetS16Param( U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    BOOL   SetS16Param( S16 value,  U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    S32    GetS32Param( U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    BOOL   SetS32Param( S32 value,  U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    S64    GetS64Param( U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    BOOL   SetS64Param( S64 value,  U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);

    char   GetCharParam( U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    BOOL   SetCharParam( char value,  U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    char*  Get16CharStrParam( U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    BOOL   Set16CharStrParam( char const* value,  U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    char*  Get32CharStrParam( U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    BOOL   Set32CharStrParam( char const* value,  U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    char*  Get64CharStrParam( U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    BOOL   Set64CharStrParam( char const* value,  U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    char*  Get256CharStrParam( U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
    BOOL   Set256CharStrParam( char const* value,  U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);


private:

   // BOOL ParseLine()

   char* GetTableValueStr( U32 parentIdValue, U32 parentIndex, U32 childIdValue, U32 childIndex);
   BOOL  SetTableValueStr( char* valueString, U32 parentIdValue, U32 parentIndex, U32 childIdValue, U32 childIndex);
   PersistentTableElement* FindTableEntryByTag( char* tagString);
   PersistentTableElement* FindTableEntryById( U32 idValue);
   PersistentTableElement* GetNextEntry( PersistentTableElement* pEntry, U32 parentIdTag);
   int GetDataTypeMaxStringSize( PersistElementDataType dataType);

   BOOL   SetStrParam( char const* value, U32 maxStringSize, U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);
   char*  GetStrParam( U32 parentId, U32 parentIndex=0, U32 childId=eNONE, U32 childIndex=0);


   char                      dBaseFilename[MAX_PERSIST_FILE_FILENAME_SIZE];
   PersistentTableElement*   dpDescriptorTable;
   char                      dLineBuffer[MAX_PERSIST_FILE_LINE_SIZE];
   U32                       dCurrentRevisionNumber;
   char*                     dpDataStringArray[MAX_NUM_UNIQUE_TAGS_PER_FILE];

};


#endif // __CJPERSISTFILE_H_
