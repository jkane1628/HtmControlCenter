#ifndef BASICGRAPH_H
#define BASICGRAPH_H

#include <QFrame>
#include <QWidget>
#include "mainwindow.h"
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

#ifdef USE_HTM_VECTOR_CLASSES
class vNetworkManager;
class vInputSpace;
class vClassifier;
#else
class NetworkManager;
class InputSpace;
class Classifier;
#endif

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

#ifdef USE_HTM_VECTOR_CLASSES
   InputspacePredictionGraph(QWidget *parent, vNetworkManager* pNetworkManager);
#else
   InputspacePredictionGraph(QWidget *parent, NetworkManager* pNetworkManager);
#endif
   ~InputspacePredictionGraph() {}

   void InitializeGraph();
   void UpdateGraphData(int time);
   void ReplotGraph();

private:
#ifdef USE_HTM_VECTOR_CLASSES
   vNetworkManager* dpNetworkManager;
   vInputSpace* dpInputSpace;
   vClassifier* dpClassifier;
#else
   NetworkManager* dpNetworkManager;
   InputSpace* dpInputSpace;
   Classifier* dpClassifier;
#endif
   

};

class InputspaceOverlapGraph : public BasicGraph
{
public:

#ifdef USE_HTM_VECTOR_CLASSES
   InputspaceOverlapGraph(QWidget *parent, vNetworkManager* pNetworkManager);
#else
   InputspaceOverlapGraph(QWidget *parent, NetworkManager* pNetworkManager);
#endif
   ~InputspaceOverlapGraph() {}

   void InitializeGraph();
   void UpdateGraphData(int time);
   void ReplotGraph();

private:
#ifdef USE_HTM_VECTOR_CLASSES
   vNetworkManager* dpNetworkManager;
   vInputSpace* dpInputSpace;
   vClassifier* dpClassifier;
#else
   NetworkManager* dpNetworkManager;
   InputSpace* dpInputSpace;
   Classifier* dpClassifier;
#endif
   

   QCustomPlot* dpPlot;
   QCPBars *dpBarsOverlap;
   QCPBars *dpBarsNotOverlap;
};

#endif // BASICGRAPH_H
