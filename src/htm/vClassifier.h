#pragma once
#include <QtCore/QStringList>
#include "DataSpace.h"

class vInputSpace;
class vRegion;
class SDR_Float;

class vClassifier	: public vDataSpace
{
public:
	vClassifier(QString &id, QString regionID, QString inputspaceID);
   ~vClassifier(void);

	// Properties
	QString dRegionID, dInputspaceID;
	

	// Methods
   void Classify();

	DataSpaceType GetDataSpaceType() {return DATASPACE_TYPE_CLASSIFIER;}

	void SetInputSpace(vInputSpace *pInputspace) { dpInputspace = pInputspace;}
	void SetRegion(vRegion *pRegion) { dpRegion = pRegion;}

   bool CreateProjectionSdr(int numSteps);

   static const int dOverlapArraySize = 7;
   int   dNumValuesWithOverlap;
   float dOverlapValueArray[dOverlapArraySize];
   float dOverlapAmountArray[dOverlapArraySize];
   float dNotOverlapAmountArray[dOverlapArraySize];
   float dMaxOverlapValue;
   float dPreviousMaxOverlapValue;

private:

   vInputSpace* dpInputspace;
   vRegion* dpRegion;

   SDR_Float* dpProjectionSdr;
};

