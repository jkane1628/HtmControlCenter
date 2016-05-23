#pragma once

#include <QtGui/QColor>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include "DataSpace.h"
#include <map>

class QStyleOptionGraphicsItem;
class vRegion;
class vInputSpace;
//class Column;
class vView;
//class Synapse;
class SynapseRecord;

class vColumnDisp : public QGraphicsItem
{
public:
    vColumnDisp(vView *_view, vDataSpace *_dataSpace, int _x, int _y);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

		int DetermineCellIndex(float _x, float _y);
		QColor DetermineSynapseColor(float _permanence, float _connectedPerm);

		void SetSelection(bool _colSelected, int _selCellIndex);

      void SelectSynapse(int _cellIndex, float permanence, float connectedPermanence);
		void ClearSelectedSynapses();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

public:
		float imageVal;
		float *imageVals;
		int imageValCount;
private:
   int colX, colY, numItems, numRows;
   float itemDist, itemSize, itemSpacer, itemMargin, synapseInfoMargin, markMargin;
		
   vView*       dpView;
   vDataSpace*  dpDataSpace;
   vInputSpace* dpInputSpace;
   vRegion*     dpRegion;
   //Column *column;
   QColor color;
   QVector<QPointF> stuff;
   bool colSelected;
   int selCellIndex;
   std::map<int,SynapseRecord> sel_synapses;
};

class SynapseRecord
{
public:
	SynapseRecord() {cellIndex = -1; permanence = 0; connectedPerm = 0;}
	SynapseRecord(int _cellIndex, float _permanence, float _connectedPerm) {cellIndex = _cellIndex; permanence = _permanence; connectedPerm = _connectedPerm;}

	int cellIndex;
	float permanence, connectedPerm;
};
