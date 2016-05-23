#pragma once
#include <QtCore/qstring.h>

typedef int vDataSpaceType;
const vDataSpaceType VDATASPACE_TYPE_INPUTSPACE = 0;
const vDataSpaceType VDATASPACE_TYPE_REGION     = 1;
const vDataSpaceType VDATASPACE_TYPE_CLASSIFIER = 2;

struct vPosition
{
   vPosition() { x = -1; y = -1; z = -1; }
   vPosition(int _x, int _y, int _z) { x = _x; y = _y; z = _z; }
   int x; int y; int z;
};

class vDataSpace
{
public:

   vDataSpace(QString &_id) { id = _id; index = -1; SetSize(vPosition(0, 0, 0)); }

	const QString &GetID() {return id;}
	void SetID(QString &_id) {id = _id;}

	void SetIndex(int _index) {index = _index;}
	int  GetIndex() {return index;}

	virtual vDataSpaceType GetDataSpaceType()=0;


   void SetSize(vPosition pos) { dMaxPosition = pos; dNumColumns = dMaxPosition.x*dMaxPosition.y; dNumCells = dMaxPosition.x*dMaxPosition.y*dMaxPosition.z; };

	int GetSizeX() {return dMaxPosition.x;}
	int GetSizeY() { return dMaxPosition.y;}
   int GetSizeZ() { return dMaxPosition.z;}

   int GetIndex(int x, int y, int z) { return (x*dMaxPosition.y*dMaxPosition.z) + (y*dMaxPosition.z) + z; }
   vPosition GetPosition(int index) { vPosition pos; pos.x = index / (dMaxPosition.y*dMaxPosition.z); pos.y = (index % (dMaxPosition.y*dMaxPosition.z)) / dMaxPosition.z; pos.z = index % dMaxPosition.z; return pos; }

   int GetNumColumns() { return dNumCells; }
	int GetNumCells() { return dNumColumns;}
	
	QString id;
	int index;
   int dNumCells, dNumColumns;
   vPosition dMaxPosition;
};

