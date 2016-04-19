#pragma once

class NetworkManager;


enum eCellState
{
   eCellState_NotActive       = 0x00,
   eCellState_Predicting      = 0x01, 
   eCellState_Active          = 0x02,   
   eCellState_Learning        = 0x04,
   eCellState_LearningSegment = 0x08
};


class vDistalSegment
{
   vDistalSegment(int8_t* permanenceArray, int8_t* permanenceUpdateArray);
   int8_t* dpPermanenceArray;   // 1 byte * receptiveRange (numCells*.25) ~= 512B for 2K cells
   int8_t* dpPermanenceUpdate;  // 1 byte * receptiveRange (numCells*.25) ~= 512B for 2K cells
};


#define VHTM_MEMORY_ALIGNMENT_VALUE 32
#define VHTM_TARGET_SEGMENTS_PER_CELL 64
#define VHTM_MAX_SEGMENTS_PER_CELL (VHTM_TARGET_SEGMENTS_PER_CELL*2)
#define VHTM_MAX_NUM_SEGMENT_GROUPS 4
#define VHTM_SEGMENTS_PER_GROUP (VHTM_TARGET_SEGMENTS_PER_CELL/VHTM_MAX_NUM_SEGMENT_GROUPS)
#define VHTM_RECEPTIVE_RANGE_DIVISOR 4   // If the receptive range divisor is 4, then that means that the receptive range will be 1/4th of the cells in this region

#define VHTM_INITIAL_PERMANENCE -71 //0.22
#define VHTM_CONNECTED_PERMANENCE -76 //0.2
#define VHTM_PERMANENCE_INCREASE 3  //0.015
#define VHTM_PERMANENCE_DECREASE 2 //0.01


class vCell
{
public:
   vCell();
   int             dNumSegments;
   int8_t*         dpSegmentGroupArray[VHTM_MAX_NUM_SEGMENT_GROUPS];
   int8_t*         GetDistalSegmentPermanenceArray( int segmentIndex);
};

class vColumn
{
public:
   vColumn();
   uint8_t  dBoost;
};

enum eRegionSize
{
   eRegionSize_4x4x4,
   eRegionSize_16x16x8,
   eRegionSize_32x32x4
};


class vRegion
{
public:

   vRegion(vInputSpace* pInputSpace, eRegionSize dRegionSize);
   ~vRegion();

   bool CreateRegion();
   void FreeRegion();
   void Step();




private:
   
   void PerformSpacialPooling( bool allowLearning);
   void PerformTemporalPooling( bool allowLearning);

   vInputSpace* dpInputSpace;
   
   // Basic Dimensions
   int dXSize;
   int dYSize;
   int dZSize;
   int dNumCells;
   int dNumColumns;

   // Memories
   uint8_t*    dpCellStateArray0;   // eCellState, size = numCells * 1 byte = 2K bytes (16x16x8)
   uint8_t*    dpCellStateArray1;   // One array for current timestep, one for previous
   int8_t*     dpProximalSegmentPermanenceArray;   // columns * inputspaceX * inputspaceY * 1byte = 64K bytes (16x16x8 w/ 16x16 input)
   int8_t*     dpDistalSegmentPermanenceArray;     // cells x num seg/cell x receptive range x (1 perm + 1 update perm) = 2048x64x.25x2 = 128 MB (16x16x8)
   vColumn*    dpColumnArray;       // ~32 bytes x num columns = 256x32 = 8K bytes (16x16x8)
   vCell*      dpCellArray;         // ~32 bytes x num cells = 2048x32 = 64K bytes (16x16x8)
                                    //  Total Mem = 2 + 2 + 64 + 8 + 64 + 128MB = 128MB + 140K

   // Segment Groups/Lists - The segments are split into groups so that each cell can hold up to 4 groups, 
   //                         segments are allocated to cells on demand.  The target is for each cell to have 
   //                         64 segments/cell, so they are allcated 32 segments at a time.  2 groups for a cell
   //                         would put it at the target of 64 segs/cell.  Because segments are not distributed evenly,
   //                         we allow upto 4 "groups" to be assigned to a single cell.  This allow a cell to have 0,32,64,96, or 128
   //                         segments assigned to it by using the global segment groups
   int      dDistalSegmentSize;
   int      dDistalSegmentGroupsTotal;
   int      dDistalSegmentGroupsRemaining;
   int      dDistalSegmentGroupsSize;
   int8_t*  dpDistalSegmentGroupsNext;

   vColumn* GetColumnObject(int columnIndex);
   vCell*   GetCellObject(int columnIndex);

   int      GetTopCellIndex(int columnIndex);
   uint8_t* GetCurrentTopCellStateArrayPtr(int columnIndex);
   uint8_t* GetPreviousTopCellStateArrayPtr(int columnIndex);
   int8_t*  GetProximalSegmentPermanenceArray(int columnIndex);
   int8_t*  GetDistalSegmentPermanenceArray(int cellIndex, int segmentIndex);
   

   // Spacial Pooler Configuration values
   struct SpacialPoolerConfig
   {
      int      dpProximalSegmentMinOverlap;
      uint8_t  dpConnectionThreshold;
   };
   
   int dTimestep;

   NetworkManager *dpNetworkManager;

};

