#pragma once

#include <vector>
#include "vDataSpace.h"
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
   eColumnState_Overlapped    = 0x40,  
   eColumnState_Active        = 0x80,
};


class vDistalSegment
{
   vDistalSegment(int8_t* permanenceArray, int8_t* permanenceUpdateArray);
   int8_t* dpPermanenceArray;   // 1 byte * receptiveRange (numCells*.25) ~= 512B for 2K cells
   int8_t* dpPermanenceUpdate;  // 1 byte * receptiveRange (numCells*.25) ~= 512B for 2K cells
};


#define VHTM_MEMORY_ALIGNMENT_VALUE 32
#define VHTM_SEGMENTS_PER_DISTAL_SEGMENT_GROUP 32
#define VHTM_MAX_NUM_DISTAL_SEGMENT_GROUPS 4
#define VHTM_MAX_DISTAL_SEGMENTS_PER_CELL (VHTM_SEGMENTS_PER_DISTAL_SEGMENT_GROUP*VHTM_MAX_NUM_DISTAL_SEGMENT_GROUPS)
#define VHTM_NOMINAL_DISTAL_SEGMENTS_PER_CELL (VHTM_MAX_DISTAL_SEGMENTS_PER_CELL/2)
#define VHTM_DISTAL_SEGMENT_RECEPTIVE_RANGE_DIVISOR 4   // If the receptive range divisor is 4, then that means that the receptive range will be 1/4th of the cells in this region


#define VHTM_NUM_COLUMN_STATE_HISTORY_ENTRIES_DEFAULT 256
#define VHTM_NUM_CELL_STATE_HISTORY_ENTRIES_DEFAULT 256
#define VHTM_MIN_OVERLAP_THRESHOLD_DEFAULT 10

#define VHTM_INITIAL_PERMANENCE -71 //0.22
#define VHTM_CONNECTED_PERMANENCE -76 //0.2
#define VHTM_PERMANENCE_INCREASE 3  //0.015
#define VHTM_PERMANENCE_DECREASE -2 //0.01


struct CellSegment
{
   int             numSegments;
   int             pSegmentGroupIndexArray[VHTM_MAX_NUM_DISTAL_SEGMENT_GROUPS];   // Index of the segment group in the memory allocation (applies to both the perm array, and perm update array)
};

enum eRegionSize
{
   eRegionSize_INVALID,
   eRegionSize_4x4x4,
   eRegionSize_16x16x8,
   eRegionSize_32x32x4
};

class vInputSpace;


class vRegion : public vDataSpace
{
public:
   friend class vCell;   // This class is used to interface into vRegion by the GUI
   friend class vColumn; // This class is used to interface into vRegion by the GUI


   vRegion(QString id);
   ~vRegion();

   vDataSpaceType GetDataSpaceType() { return VDATASPACE_TYPE_REGION; }
   bool SetRegionSize(eRegionSize regionSize) { dRegionSize = regionSize; return true; }

   bool AttachInputDataSpace(vDataSpace* pInputSpace);  // Using vDataSpace because it could be a region or inputSpace
   bool CreateRegion();
   void FreeRegion();
   void Step();

   vInputSpace* GetInputSpace() { return dpInputSpace; }
   int GetProximalSegmentReceptiveRange();


   // Spatial Pooler Configuration values
   struct SpatialPoolerConfig
   {
      int      desiredNumActiveColumns;
      int      noOverlapThresholdMin;
      int8_t   noOverlapPermanenceIncrement;
      int      inhibitionDistance;
      int      boostIncrement;
      int      boostMax;
      int      boostMin;



   };
   SpatialPoolerConfig GetSpatialPoolerConfig() { return dSpatialPoolerConfig;}
   bool SetSpatialPoolerConfig( SpatialPoolerConfig* pSpatialPoolerConfig);



   struct TemporalPoolerConfig
   {
      int      dpSampleConfig;
   };
   TemporalPoolerConfig GetTemporalPoolerConfig() { return dTemporalPoolerConfig;}
   bool SetTemporalPoolerConfig(TemporalPoolerConfig* pTemporalPoolerConfig);


   std::vector<QString> dConfiguredInputIDs;

private:
   
   void PerformSpatialPooling( bool allowLearning);
   void PerformTemporalPooling( bool allowLearning);

   vInputSpace* dpInputSpace;
   
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //   Overall Region
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   eRegionSize dRegionSize;   
   int dTimestep;

   // Region Memories
   uint8_t*    dpColumnStateArray;                 // eColumnState, size = dNumColumns * 1 byte * dNumColumnStateTimestepsInHistory = 256 * 256 steps = 64K (16x16x8)
   uint8_t*    dpCellStateArray;                   // eCellState, size = numCells * 1 byte * dNumCellStateTimestepsInHistory = 2K * 256steps = 512 Kbytes (16x16x8)
   CellSegment* dpCellSegmentGroups;               // size = numCells * sizeof(CellSegment) = numCells * 5 * 4bytes = 2K * 20B = 40 Kbytes (16x16x8)
   int8_t*     dpProximalSegmentPermanenceArray;   // columns * inputspaceX * inputspaceY * 1byte = 64K bytes (16x16x8 w/ 16x16 input)
   int8_t*     dpDistalSegmentPermanenceArray;       // cells * num seg/cell * cells * receptive range = 2048x64x2048x.25 = 64 MB (16x16x8)
   int8_t*     dpDistalSegmentPermanenceUpdateArray; // cells * num seg/cell * cells * receptive range = 2048x64x2048x.25 = 64 MB (16x16x8)
   int*        dpColumnBoostArray;                 // 4 bytes * num columns = 256*4 = 1K bytes (16x16x8)
   int*        dpColumnActivationTotalArray;       // 4 bytes * num columns = 256*4 = 1K bytes (16x16x8)
   // Spacial pooler memories: dpActiveColumnIndexArray,dpColumnActiveDutyCycleTotals,dpColumnOverlapDutyCycleTotals = 3K (16x16x8) -> Declared Below in spacial pooler area

                                                   //  Total Mem = 64K + 512K + 64K + 1 + 1 + 3 + 128MB = 645KB + 128MB

   // Segment Groups/Lists -  The segments are split into groups so that each cell can hold up to 4 groups, 
   //                         segments are allocated to cells on demand.  The target is for each cell to have 
   //                         32-128 segments/cell, so they are allcated 32 segments at a time.  2 groups for a cell
   //                         would put it at the target of 64 segs/cell.  Because segments are not distributed evenly,
   //                         we allow upto 4 "groups" to be assigned to a single cell.  This allow a cell to have 0,32,64,96, or 128
   //                         segments assigned to it by using the global segment groups
   int      dDistalSegmentSize;
   int      dDistalSegmentGroupsTotal;
   int      dDistalSegmentGroupsRemaining;
   int      dDistalSegmentGroupsSize;
   int8_t*  dpDistalSegmentGroupsNext;

   int8_t*  GetProximalSegmentPermanenceArray(int columnIndex);
   int8_t*  GetDistalSegmentPermanenceArray(int cellIndex, int segmentIndex);
   int8_t*  GetDistalSegmentPermanenceUpdateArray(int cellIndex, int segmentIndex);

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




   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //   Spatial Pooler
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   // Spatial Pooler internal variables
   SpatialPoolerConfig dSpatialPoolerConfig;
   int      dCurrentNumActiveColumns;
   int*     dpActiveColumnIndexArray;           // size = dNumColumns * 4 bytes = 256 * 4 = 1K (16x16x8) 
   
   
   // Spatial Pooler Statistic Counters
   int*     dpColumnActiveDutyCycleTotals;      // size = dNumColumns * 4 bytes = 256 * 4 = 1K (16x16x8)
   int*     dpColumnOverlapDutyCycleTotals;     // size = dNumColumns * 4 bytes = 256 * 4 = 1K (16x16x8)

   // Spacial Pooler Helper Functions
   void IncreaseProximalPermanences(int columnIndex);


   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //   Temporal Pooler
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   // Temporal Pooler internal variables
   TemporalPoolerConfig dTemporalPoolerConfig;

   // Temporal Pooler Statistic Counters

   // Temporal Pooler Helper Functions


   

   NetworkManager *dpNetworkManager;

};



class vCell
{
public:
   vCell(vRegion* pRegion, int x, int y, int z);
   vCell(vRegion* pRegion, int index);
   int  GetCellIndex();
   vPosition GetCellPosition();
   bool IsCellActive();
   bool IsCellPredicted();
   int  GetNumPredictionSteps();
   bool IsCellLearning();

   int  GetNumDistalSegments();
   int8_t* GetDistalSegmentPermanenceArray(int segmentIndex);
   int8_t* GetDistalSegmentPermanenceUpdateArray(int segmentIndex);
   int GetGetDistalSegmentReceptiveRange();
   int GetDistalSynaspeRemoteConnectionIndex(int synapseIndex);
   vPosition GetDistalSynaspeRemoteConnectionPosition(int synapseIndex);
   
private:
   int dIndex;
   vRegion* dpRegion;
};

class vColumn
{
public:
   vColumn(vRegion* pRegion, int x, int y);
   vColumn(vRegion* pRegion, int index);
   int GetColumnIndex();
   float GetBoostValue();
   bool GetIsActive();
   bool GetIsOverlapped();   
   bool GetIsAnyCellPredicting(int numSteps);
   vCell GetCellByIndex(int cellIndex);
   int8_t* GetProximalSegmentPermanenceArray();
   int GetProximalSegmentReceptiveRange();

   int       GetProximalSynapseConnectionInputIndex(int synaspeIndex);
   vPosition GetProximalSynapseConnectionInputPosition(int synaspeIndex);

private:
   int dIndex;
   vRegion* dpRegion;
};
