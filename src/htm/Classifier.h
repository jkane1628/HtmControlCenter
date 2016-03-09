#pragma once
#include <QtCore/QStringList>
#include "DataSpace.h"

class InputSpace;
class Region;
class SDR_Float;

class Classifier
	: public DataSpace
{
public:
	Classifier(QString &_id, int _numitems, QString _regionID, QString _inputspaceID, QStringList &_labels);
   ~Classifier(void);

	// Properties

	QString regionID, inputspaceID;
	InputSpace *inputspace;
	Region *region;
	int numItems;
	QStringList labels;

	// Methods
   void Classify();

	DataSpaceType GetDataSpaceType() {return DATASPACE_TYPE_CLASSIFIER;}

	int GetSizeX();
	int GetSizeY();
	int GetNumValues();
	int GetHypercolumnDiameter();

	bool GetIsActive(int _x, int _y, int _index);

	void SetInputSpace(InputSpace *_inputspace) {inputspace = _inputspace;}
	void SetRegion(Region *_region) {region = _region;}

   bool CreateProjectionSdr(int numSteps);

   static const int dOverlapArraySize = 7;
   int   dNumValuesWithOverlap;
   float dOverlapValueArray[dOverlapArraySize];
   float dOverlapAmountArray[dOverlapArraySize];
   float dNotOverlapAmountArray[dOverlapArraySize];
   float dMaxOverlapValue;
   float dPreviousMaxOverlapValue;

private:

   SDR_Float* dpProjectionSdr;

  

};

