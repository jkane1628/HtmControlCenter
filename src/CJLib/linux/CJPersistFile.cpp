// CJPersistFile.cpp : Implementation File
//

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "CJConfig.h"
#include "CJTrace.h"
#include "CJAssert.h"

#include "CJPersistFile.h"


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

//PersistentTableElement ExampleParentTagDescriptorTable[] = {
//
//   // DATA TYPE                 TAG STRING               TAG ID,                           MAX NUM PER FILE   PARENT TAG ID
//   // -----------------------   -----------------------  -------------------------------   ----------------   ------------------        
//   {  ePERSISTTYPE_S64,         "someU64Tag",            eSOME64TAGID                      1                  eNONE     },
//   {  ePERSISTTYPE_STRING,      "someStringTag",         ePERSISTTYPE_32CHAR_STRING        3                  eNONE     },
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



CJPersistFile::CJPersistFile(char const* name) : CJObject("PERSISTFILE::", name)
{
   dBaseFilename[0] = 0;
   for( int i=0; i < MAX_NUM_UNIQUE_TAGS_PER_FILE; i++)
   {
      dpDataStringArray[i] = NULL;
   }
   memset( dLineBuffer, 0, MAX_PERSIST_FILE_LINE_SIZE);
   dCurrentRevisionNumber = 0;
   dpDescriptorTable = NULL;

}

CJPersistFile::~CJPersistFile( )
{
   for( int i=0; i < MAX_NUM_UNIQUE_TAGS_PER_FILE; i++)
   {
      if( dpDataStringArray[i] != NULL)
      {
         delete dpDataStringArray[i]; 
      }
   }
}

BOOL   CJPersistFile::Initialize( char const* baseFilename, PersistentTableElement* pDescriptorTable)
{
   strncpy( dBaseFilename, baseFilename, MAX_PERSIST_FILE_FILENAME_SIZE);

   dpDescriptorTable = pDescriptorTable;
   if( dpDescriptorTable == NULL)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Cannot initialize persitent table when it is NULL");
      return FALSE;
   }

   // Initialize the memory for the given descriptor table
   int entryCount =0;
   int maxSizeOfData;
   PersistentTableElement* pTempEntry = &pDescriptorTable[0];
   PersistentTableElement* pTempParentEntry;

   // Count up the entries in the table and alloc space for the data
   while (pTempEntry->dataType != ePERSISTTYPE_LAST_ENTRY)
   {
      if( pTempEntry->dataType == ePERSISTTYPE_PARENT_TAG)
      {
         if( pTempEntry->parentIdEnum != eNONE)
         {
            CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Cannot initialize a table that has parent tags with parents, only one level is allowed (parentTag=%s)", pTempEntry->paramTagString);
            return FALSE;
         }
         maxSizeOfData = 0;
      }
      else if( pTempEntry->parentIdEnum != eNONE)
      {
         pTempParentEntry = FindTableEntryById(pTempEntry->parentIdEnum);
         if( pTempParentEntry == NULL)
         {
            CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Cannot initialize persitent table, child tag specifies invalid parent (childTag=%s, invaidParent=%u)", pTempEntry->paramTagString, pTempEntry->parentIdEnum);
            return FALSE;
         }
         maxSizeOfData = pTempEntry->maxPerFile * pTempParentEntry->maxPerFile * GetDataTypeMaxStringSize(pTempEntry->dataType);
      }
      else
      {
         maxSizeOfData = pTempEntry->maxPerFile * GetDataTypeMaxStringSize(pTempEntry->dataType);
      }
      if(maxSizeOfData > 0)
      {
         dpDataStringArray[entryCount] = new char[maxSizeOfData];
         memset( dpDataStringArray[entryCount], 0, maxSizeOfData);
      }
      else
      {
         dpDataStringArray[entryCount] = NULL;
      }
      pTempEntry++;
      entryCount++;
      if(entryCount > MAX_NUM_TOTAL_TAGS_PER_FILE)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Exceeded max number of total tags per file (max=%u)", MAX_NUM_TOTAL_TAGS_PER_FILE);
         return FALSE;
      }
   }
   return TRUE;
}

BOOL CJPersistFile::LoadFile()
{

   return TRUE;
}

BOOL CJPersistFile::SaveFile()
{
   char filename[256];
   FILE* pFile;
   int bytesWritten;
   char* tempDataStr;
   U32 i,j;
   PersistentTableElement* pCurrentParentEntry;
   PersistentTableElement* pCurrentChildEntry;

   // Increment the version number
   dCurrentRevisionNumber++;

   // Open the file to save
   sprintf(filename, "%s_%u.xml", dBaseFilename, dCurrentRevisionNumber);
   pFile = fopen(filename, "w");
   if( pFile == NULL)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to open file for save operation (filename=%s, rev=%u, errno=%s)", dBaseFilename, dCurrentRevisionNumber, strerror(errno));
      return FALSE;
   }

   // Write out the header information
   bytesWritten = fprintf(pFile, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n\n");
   if( bytesWritten <= 0)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to write to file (filename=%s, rev=%u, errno=%s)", dBaseFilename, dCurrentRevisionNumber, strerror(errno));
      fclose(pFile);
      return FALSE;
   }

   // Cycle through the tags and write out data
   pCurrentParentEntry = GetNextEntry(NULL, 0);
   while( pCurrentParentEntry->dataType != ePERSISTTYPE_LAST_ENTRY)
   {
      // Is this a parent tag entry?
      if(pCurrentParentEntry->dataType == ePERSISTTYPE_PARENT_TAG)
      {   
         for( i=0; i < pCurrentParentEntry->maxPerFile; i++)
         {
            // Write out the open parent tag
            bytesWritten = fprintf(pFile, "<%s_%u>\n", pCurrentParentEntry->paramTagString, i);
            if( bytesWritten <= 0)
            {
               CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to write to file (filename=%s, rev=%u, errno=%s)", dBaseFilename, dCurrentRevisionNumber, strerror(errno));
               fclose(pFile);
               return FALSE;
            }   
   
            // Write out the associated child values
            pCurrentChildEntry = GetNextEntry(NULL, pCurrentParentEntry->idEnum);
            while( pCurrentChildEntry->dataType != ePERSISTTYPE_LAST_ENTRY)
            {
               for( j=0; j < pCurrentChildEntry->maxPerFile; j++)
               {
                  tempDataStr = GetTableValueStr( pCurrentParentEntry->idEnum, i, pCurrentChildEntry->idEnum, j);
                  if( tempDataStr[0] != 0)
                  {
                     bytesWritten = fprintf(pFile, "   <%s_%u>%s</%s_%u>\n", pCurrentChildEntry->paramTagString, j, tempDataStr, pCurrentChildEntry->paramTagString, j);
                     if( bytesWritten <= 0)
                     {
                        CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to write data string to file (filename=%s, rev=%u, errno=%s)", dBaseFilename, dCurrentRevisionNumber, strerror(errno));
                        fclose(pFile);
                        return FALSE;
                     }
                  }
               }
               pCurrentChildEntry = GetNextEntry( pCurrentChildEntry, pCurrentParentEntry->idEnum);
            }   
   
            // Write out the close parent tag
            bytesWritten = fprintf(pFile, "</%s_%u>\n", pCurrentParentEntry->paramTagString, i);
            if( bytesWritten <= 0)
            {
               CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to write to file (filename=%s, rev=%u, errno=%s)", dBaseFilename, dCurrentRevisionNumber, strerror(errno));
               fclose(pFile);
               return FALSE;
            }
         }
      }
      else
      {
         // This is a top level value, write it out
         for( i=0; i < pCurrentParentEntry->maxPerFile; i++)
         {
            tempDataStr = GetTableValueStr( pCurrentParentEntry->idEnum, i, 0, 0);
            if( tempDataStr[0] != 0)
            {
               bytesWritten = fprintf(pFile, "<%s_%u>%s</%s_%u>\n", pCurrentParentEntry->paramTagString, i, tempDataStr, pCurrentParentEntry->paramTagString, i);
               if( bytesWritten <= 0)
               {
                  CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to write data string to file (filename=%s, rev=%u, errno=%s)", dBaseFilename, dCurrentRevisionNumber, strerror(errno));
                  fclose(pFile);
                  return FALSE;
               }
            }
         }
      }
      pCurrentParentEntry = GetNextEntry(pCurrentParentEntry, 0);
   }
   fclose(pFile);
   return TRUE;
}


PersistentTableElement* CJPersistFile::GetNextEntry( PersistentTableElement* pEntry, U32 parentIdTag)
{
   PersistentTableElement* pReturnVal;

   if( pEntry == NULL)
   {
      pReturnVal = &dpDescriptorTable[0];
   }
   else if( pEntry->dataType == ePERSISTTYPE_LAST_ENTRY)
   {
      return pEntry;
   }
   else
   {
      pReturnVal = pEntry + 1;
   }

   while((pReturnVal->dataType != ePERSISTTYPE_LAST_ENTRY) && (pReturnVal->parentIdEnum != parentIdTag))
   {
      pReturnVal++;
   }
   return pReturnVal;
}



U8    CJPersistFile::GetU8Param( U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char* pTempStr = GetTableValueStr( parentId, parentIndex, childId, childIndex);
   return (U8)atol(pTempStr);
}
BOOL   CJPersistFile::SetU8Param( U8 value,  U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char tempValString[STRING_SIZE_U8];
   sprintf( tempValString, "%u", value);
   return SetTableValueStr(tempValString, parentId, parentIndex, childId, childIndex);
}
U16    CJPersistFile::GetU16Param( U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char* pTempStr = GetTableValueStr( parentId, parentIndex, childId, childIndex);
   return (U16)atol(pTempStr);
}
BOOL   CJPersistFile::SetU16Param( U16 value,  U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char tempValString[STRING_SIZE_U16];
   sprintf( tempValString, "%u", value);
   return SetTableValueStr(tempValString, parentId, parentIndex, childId, childIndex);
}
U32    CJPersistFile::GetU32Param( U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char* pTempStr = GetTableValueStr( parentId, parentIndex, childId, childIndex);
   return atol(pTempStr);
}
BOOL   CJPersistFile::SetU32Param( U32 value,  U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char tempValString[STRING_SIZE_U32];
   sprintf( tempValString, "%u", value);
   return SetTableValueStr(tempValString, parentId, parentIndex, childId, childIndex);
}
U64    CJPersistFile::GetU64Param( U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char* pTempStr = GetTableValueStr( parentId, parentIndex, childId, childIndex);
   return atoll( pTempStr);
}
BOOL   CJPersistFile::SetU64Param( U64 value,  U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char tempValString[STRING_SIZE_U64];
   sprintf( tempValString, "%llu", value);
   return SetTableValueStr(tempValString, parentId, parentIndex, childId, childIndex);
}


S8    CJPersistFile::GetS8Param( U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char* pTempStr = GetTableValueStr( parentId, parentIndex, childId, childIndex);
   return (S8)atoi(pTempStr);
}
BOOL   CJPersistFile::SetS8Param( S8 value,  U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char tempValString[STRING_SIZE_S8];
   sprintf( tempValString, "%d", value);
   return SetTableValueStr(tempValString, parentId, parentIndex, childId, childIndex);
}
S16    CJPersistFile::GetS16Param( U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char* pTempStr = GetTableValueStr( parentId, parentIndex, childId, childIndex);
   return (S16)atoi(pTempStr);
}
BOOL   CJPersistFile::SetS16Param( S16 value,  U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char tempValString[STRING_SIZE_S16];
   sprintf( tempValString, "%d", value);
   return SetTableValueStr(tempValString, parentId, parentIndex, childId, childIndex);
}
S32    CJPersistFile::GetS32Param( U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char* pTempStr = GetTableValueStr( parentId, parentIndex, childId, childIndex);
   return atoi(pTempStr);
}
BOOL   CJPersistFile::SetS32Param( S32 value,  U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char tempValString[STRING_SIZE_S32];
   sprintf( tempValString, "%d", value);
   return SetTableValueStr(tempValString, parentId, parentIndex, childId, childIndex);
}
S64    CJPersistFile::GetS64Param( U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char* pTempStr = GetTableValueStr( parentId, parentIndex, childId, childIndex);
   return atoi( pTempStr);
}
BOOL   CJPersistFile::SetS64Param( S64 value,  U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char tempValString[STRING_SIZE_S64];
   sprintf( tempValString, "%lld", value);
   return SetTableValueStr(tempValString, parentId, parentIndex, childId, childIndex);
}
    
char   CJPersistFile::GetCharParam( U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char* pTempStr = GetTableValueStr( parentId, parentIndex, childId, childIndex);
   return pTempStr[0];
}
BOOL   CJPersistFile::SetCharParam( char value,  U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char tempValString[STRING_SIZE_CHAR];
   sprintf( tempValString, "%c", value);
   return SetTableValueStr(tempValString, parentId, parentIndex, childId, childIndex);
}

char*  CJPersistFile::Get16CharStrParam( U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)  { return GetStrParam(parentId, parentIndex=0, childId=eNONE, childIndex);}
BOOL   CJPersistFile::Set16CharStrParam( char const* value,  U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)  { return SetStrParam( value, 16, parentId, parentIndex=0, childId, childIndex);}
char*  CJPersistFile::Get32CharStrParam( U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)  { return GetStrParam(parentId, parentIndex=0, childId=eNONE, childIndex);}
BOOL   CJPersistFile::Set32CharStrParam( char const* value,  U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)  { return SetStrParam( value, 32, parentId, parentIndex=0, childId, childIndex);}
char*  CJPersistFile::Get64CharStrParam( U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)  { return GetStrParam(parentId, parentIndex=0, childId=eNONE, childIndex);}
BOOL   CJPersistFile::Set64CharStrParam( char const* value,  U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)  { return SetStrParam( value, 64, parentId, parentIndex=0, childId, childIndex);}
char*  CJPersistFile::Get256CharStrParam( U32 parentId, U32 parentIndex, U32 childId, U32 childIndex) { return GetStrParam(parentId, parentIndex=0, childId=eNONE, childIndex);}
BOOL   CJPersistFile::Set256CharStrParam( char const* value,  U32 parentId, U32 parentIndex, U32 childId, U32 childIndex) { return SetStrParam( value, 64, parentId, parentIndex=0, childId, childIndex);}

BOOL   CJPersistFile::SetStrParam( char const* value, U32 maxStringSize,  U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   char tempValString[maxStringSize+1];
   strncpy( tempValString, value, maxStringSize);
   return SetTableValueStr(tempValString, parentId, parentIndex, childId, childIndex);
}
char*  CJPersistFile::GetStrParam( U32 parentId, U32 parentIndex, U32 childId, U32 childIndex)
{
   return GetTableValueStr( parentId, parentIndex, childId, childIndex);
}


// BOOL CJPersistFile::ParseLine()



char* CJPersistFile::GetTableValueStr( U32 parentIdValue, U32 parentIndex, U32 childIdValue, U32 childIndex)
{
   PersistentTableElement* pTempEntry = &dpDescriptorTable[0];
   PersistentTableElement* pTempParentEntry;

   int descriptorIndex=0;
   char* pReturnString = NULL;

   if(childIdValue == 0)
   {
      //Search for a PARENT tag using the parent ID
      pTempEntry = FindTableEntryById(parentIdValue);
      if( pTempEntry == NULL)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Parent id tag not found (parentId=%u, parentIndex=%u)", parentIdValue, parentIndex);
         return NULL;
      }
      // Check the index to make sure we are lower than the max allowed for this table entry
      if( parentIndex >= pTempEntry->maxPerFile)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Parent index too high (parentId=%u, parentIndex=%u, parentTag=%s)", parentIdValue, parentIndex, pTempEntry->paramTagString);
         return NULL;
      }   
      // Use the parent index and dataType to find the correct string in the array
      descriptorIndex = pTempEntry - &dpDescriptorTable[0];
      pReturnString = dpDataStringArray[descriptorIndex] + (parentIndex * GetDataTypeMaxStringSize(pTempEntry->dataType));
      return pReturnString;
   }
   else
   {
      //Search for a CHILD tag using the ID value
      pTempEntry = FindTableEntryById(childIdValue);
      if( pTempEntry == NULL)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Child id tag not found (parentId/Index=%u, childId/index=%u/%u)", parentIdValue, parentIndex, childIdValue, childIndex);
         return NULL;
      }
      pTempParentEntry = FindTableEntryById(parentIdValue);
      if( pTempParentEntry == NULL)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Parent of child id tag not found (parentId/Index=%u, childId/index=%u/%u)", parentIdValue, parentIndex, childIdValue, childIndex);
         return NULL;
      }

      // Check the indexes to make sure we are lower than the max allowed for this table entry
      if( childIndex >= pTempEntry->maxPerFile)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Child index too high (childId=%u, childIndex=%u, childTag=%s)", childIdValue, childIndex, pTempEntry->paramTagString);
         return NULL;
      }
      if( parentIndex >= pTempParentEntry->maxPerFile)
      {
         pTempParentEntry = FindTableEntryById(parentIdValue);
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Parent of child index too high (parentId=%u, parentIndex=%u, parentTag=%s)", parentIdValue, parentIndex, pTempEntry->paramTagString);
         return NULL;
      }
      // Check to make sure a valid parent was given
      if( pTempEntry->parentIdEnum != parentIdValue)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Parent id value for child tag is invalid (parentId=%u, tableParentId=%u, childTag=%s)", parentIdValue, pTempEntry->parentIdEnum, pTempEntry->paramTagString);
         return NULL;
      }

      // Use the child index and dataType to find the correct string in the array
      descriptorIndex = pTempEntry - &dpDescriptorTable[0];
      pReturnString = dpDataStringArray[descriptorIndex] + (parentIndex * pTempEntry->maxPerFile * GetDataTypeMaxStringSize(pTempEntry->dataType)) + 
                                                           (childIndex * GetDataTypeMaxStringSize(pTempEntry->dataType));
      return pReturnString;            
   }

   CJASSERT("Invalid code path");
   return NULL;
}



BOOL CJPersistFile::SetTableValueStr( char* valueString, U32 parentIdValue, U32 parentIndex, U32 childIdValue, U32 childIndex)
{
   char* pTempString = GetTableValueStr( parentIdValue, parentIndex, childIdValue, childIndex);
   if( pTempString == NULL)
   {
      return FALSE;
   }
   strcpy( pTempString, valueString);
   return TRUE;
}



PersistentTableElement* CJPersistFile::FindTableEntryByTag( char* tagString)
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




PersistentTableElement* CJPersistFile::FindTableEntryById( U32 idValue)
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


int CJPersistFile::GetDataTypeMaxStringSize( PersistElementDataType dataType)
{
   switch(dataType)
   {
   case ePERSISTTYPE_U8:              return STRING_SIZE_U8;              
   case ePERSISTTYPE_U16:             return STRING_SIZE_U16;             
   case ePERSISTTYPE_U32:             return STRING_SIZE_U32;             
   case ePERSISTTYPE_U64:             return STRING_SIZE_U64;             
   case ePERSISTTYPE_S8:              return STRING_SIZE_S8;              
   case ePERSISTTYPE_S16:             return STRING_SIZE_S16;             
   case ePERSISTTYPE_S32:             return STRING_SIZE_S32;             
   case ePERSISTTYPE_S64:             return STRING_SIZE_S64;             
   case ePERSISTTYPE_CHAR:            return STRING_SIZE_CHAR;           
   case ePERSISTTYPE_16CHAR_STRING:   return STRING_SIZE_16CHAR_STRING; 
   case ePERSISTTYPE_32CHAR_STRING:   return STRING_SIZE_32CHAR_STRING; 
   case ePERSISTTYPE_64CHAR_STRING:   return STRING_SIZE_64CHAR_STRING; 
   case ePERSISTTYPE_256CHAR_STRING:  return STRING_SIZE_256CHAR_STRING;
   case ePERSISTTYPE_PARENT_TAG:      return 0;
   default:
      CJASSERT("Unkown Data Type (type=%d)", dataType);
   }
   return 0x7FFFFFFF;         
}



