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

enum eColumnState
{
   eColumnState_NotActive     = 0x00,
   eColumnState_Overlapped    = 0x40,  // TODO: NOT BEING SET, MIGHT WANT TO REMOVE
   eColumnState_Active        = 0x80,
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
#define VHTM_NUM_COLUMN_STATE_HISTORY_ENTRIES_DEFAULT 256
#define VHTM_NUM_CELL_STATE_HISTORY_ENTRIES_DEFAULT 256
#define VHTM_MIN_OVERLAP_THRESHOLD_DEFAULT 10

#define VHTM_INITIAL_PERMANENCE -71 //0.22
#define VHTM_CONNECTED_PERMANENCE -76 //0.2
#define VHTM_PERMANENCE_INCREASE 3  //0.015
#define VHTM_PERMANENCE_DECREASE -2 //0.01


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

class vInputSpace;

class vRegion
{
public:

   vRegion(vInputSpace* pInputSpace, eRegionSize dRegionSize);
   ~vRegion();

   bool CreateRegion();
   void FreeRegion();
   void Step();

private:
   
   void PerformSpatialPooling( bool allowLearning);
   void PerformTemporalPooling( bool allowLearning);

   vInputSpace* dpInputSpace;
   
   // Basic Dimensions
   int dXSize;
   int dYSize;
   int dZSize;
   int dNumCells;
   int dNumColumns;

   // Memories
   uint8_t*    dpColumnStateArray;     // eColumnState, size = dNumColumns * 1 byte * dNumColumnStateTimestepsInHistory = 256 * 256 steps = 64K (16x16x8)
   uint8_t*    dpCellStateArray;       // eCellState, size = numCells * 1 byte * dNumCellStateTimestepsInHistory = 2K * 256steps = 512 Kbytes (16x16x8)
   int8_t*     dpProximalSegmentPermanenceArray;   // columns * inputspaceX * inputspaceY * 1byte = 64K bytes (16x16x8 w/ 16x16 input)
   int8_t*     dpDistalSegmentPermanenceArray;     // cells x num seg/cell x receptive range x (1 perm + 1 update perm) = 2048x64x.25x2 = 128 MB (16x16x8)
   int*        dpColumnBoostArray;     // 4 bytes x num columns = 256x4 = 1K bytes (16x16x8)
   vColumn*    dpColumnArray;          // ~32 bytes x num columns = 256x32 = 8K bytes (16x16x8)
   vCell*      dpCellArray;            // ~32 bytes x num cells = 2048x32 = 64K bytes (16x16x8)
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



   int8_t*  GetProximalSegmentPermanenceArray(int columnIndex);
   int8_t*  GetDistalSegmentPermanenceArray(int cellIndex, int segmentIndex);
   
   

   
   


   // Column State Management
   int      GetTopCellIndex(int columnIndex);
   void     IncrementCurrentColumnStateArrayPtr();
   uint8_t* GetCurrentColumnStateArrayPtr(int columnIndex);
   uint8_t* GetOldestColumnStateArrayPtr(int columnIndex);
   uint8_t* GetHistoricalColumnStateArrayPtr(int columnIndex, int stepsBack);
   int      dNumColumnStateTimestepsInHistory;
   uint8_t* dpCurrentTimestepColumnStateArray;  // Always points at current timestep array of column states

   // Cell State Management
   void     IncrementCurrentCellStateArrayPtr();
   uint8_t* GetCurrentCellStateArrayPtr(int cellIndex);
   uint8_t* GetOldestCellStateArrayPtr(int cellIndex);
   uint8_t* GetHistoricalCellStateArrayPtr(int cellIndex, int stepsBack);
   int      dNumCellStateTimestepsInHistory;
   uint8_t* dpCurrentTimestepCellStateArray;    // Always points at current timestep array of cell states






   // Spatial Pooler internal variables
   int      dMinOverlapThreshold; // TODO Incorporate this into SP alg
   int      dInhibitionDistance;
   int      dDesiredNumActiveColumns;
   int      dColumnBoostIncrement;
   int8_t   dColumnPermanenceIncrement;
   int      dColumnBoostMax;
   int      dColumnBoostMin;

   int      dCurrentNumActiveColumns;
   int*     dpActiveColumnIndexArray;           // size = dNumColumns * 4 bytes = 256 * 4 = 1K (16x16x8) 
   
   void IncreaseProximalPermanences(int columnIndex);
   

   // Spatial Pooler Statistic Counters
   int*     dpColumnActiveDutyCycleTotals;      // size = dNumColumns * 4 bytes = 256 * 4 = 1K (16x16x8)
   int*     dpColumnOverlapDutyCycleTotals;     // size = dNumColumns * 4 bytes = 256 * 4 = 1K (16x16x8)

   // Spatial Pooler Configuration values
   struct SpatialPoolerConfig
   {
      int      dpProximalSegmentMinOverlap;
      uint8_t  dpConnectionThreshold;
   };
   
   int dTimestep;

   NetworkManager *dpNetworkManager;

};

