#ifndef BASICGRAPH_H
#define BASICGRAPH_H

#include <QFrame>
#include <QWidget>
#include "ui_BasicGraph.h"

class CJCli;

struct TimeSeriesData
{
   QString filename;
   QString name;
   double timedelta;
   double timestart;
   QString Xaxislabel;
   QString Yaxislabel;
   QString label;
   QVector<double> pData;
};

class NetworkManager;
class InputSpace;
class Classifier;

class BasicGraph : public QFrame
{
	Q_OBJECT

public:
   BasicGraph(QWidget *parent);
   ~BasicGraph();

   virtual void InitializeGraph() {};
   virtual void UpdateGraph(int time, bool replot) {};

   void Demo();
   BOOL LoadCSVDataFile(QString filename, TimeSeriesData* pOutputData);

signals:
   void ExternalReplot();

public slots:
   void OnReplot();

protected:
   void resizeEvent(QResizeEvent *event);
   Ui::BasicGraph ui;
   int dLastRecordedTime;

private:
};

class InputspacePredictionGraph : public BasicGraph
{
public:

   InputspacePredictionGraph(QWidget *parent, NetworkManager* pNetworkManager);
   ~InputspacePredictionGraph() {}

   void InitializeGraph();
   void UpdateGraphData(int time);
   void ReplotGraph();

private:
   NetworkManager* dpNetworkManager;
   InputSpace* dpInputSpace;
   Classifier* dpClassifier;

};

class InputspaceOverlapGraph : public BasicGraph
{
public:

   InputspaceOverlapGraph(QWidget *parent, NetworkManager* pNetworkManager);
   ~InputspaceOverlapGraph() {}

   void InitializeGraph();
   void UpdateGraphData(int time);
   void ReplotGraph();

private:
   NetworkManager* dpNetworkManager;
   InputSpace* dpInputSpace;
   Classifier* dpClassifier;

   QCustomPlot* dpPlot;
   QCPBars *dpBarsOverlap;
   QCPBars *dpBarsNotOverlap;
};

#endif // BASICGRAPH_H
