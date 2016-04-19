
#include "CJTrace.h"
#include "htm/NetworkManager.h"
#include "htm/InputSpace.h"
#include "htm/Classifier.h"
#include "htm/SDR.h"

#include "basicgraph.h"


BasicGraph::BasicGraph(QWidget *parent)
: QFrame(parent)
{
	ui.setupUi(this);
   OnReplot();

   dLastRecordedTime = 0;

   CJTRACE_SET_TRACEID_STRING(eTraceId_BasicGraph, "BG");
   //CJTRACE_REGISTER_CLI_COMMAND_OBJ(new CliGraphCommandSet(this));

   connect(this, SIGNAL(ExternalReplot()), this, SLOT(OnReplot()));
}

BasicGraph::~BasicGraph()
{

}

void BasicGraph::OnReplot()
{
   QRect localRect = frameRect();
   ui.basicGraph->resize(localRect.width(), localRect.height());
   ui.basicGraph->replot();
}

void BasicGraph::resizeEvent(QResizeEvent *event)
{
   OnReplot();
   QWidget::resizeEvent(event);
}

void BasicGraph::Demo()
{
   QCustomPlot* customPlot = ui.basicGraph;

   // add two new graphs and set their look:
   customPlot->addGraph();
   customPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
   customPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue
   customPlot->addGraph();
   customPlot->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph
   // generate some points of data (y0 for first, y1 for second graph):
   QVector<double> x(250), y0(250), y1(250);
   for (int i = 0; i<250; ++i)
   {
      x[i] = i;
      y0[i] = exp(-i / 150.0)*cos(i / 10.0); // exponentially decaying cosine
      y1[i] = exp(-i / 150.0); // exponential envelope
   }
   // configure right and top axis to show ticks but no labels:
   // (see QCPAxisRect::setupFullAxesBox for a quicker method to do this)
   customPlot->xAxis2->setVisible(true);
   customPlot->xAxis2->setTickLabels(false);
   customPlot->yAxis2->setVisible(true);
   customPlot->yAxis2->setTickLabels(false);
   // make left and bottom axes always transfer their ranges to right and top axes:
   connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
   connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));
   // pass data points to graphs:
   customPlot->graph(0)->setData(x, y0);
   customPlot->graph(1)->setData(x, y1);
   // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
   customPlot->graph(0)->rescaleAxes();
   // same thing for graph 1, but only enlarge ranges (in case graph 1 is smaller than graph 0):
   customPlot->graph(1)->rescaleAxes(true);
   // Note: we could have also just called customPlot->rescaleAxes(); instead
   // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
   customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

   

   ExternalReplot();
}



BOOL BasicGraph::LoadCSVDataFile(QString filename, TimeSeriesData* pOutputData)
{
   int rc;
   char tempString[64];
   FILE* pFile;
   // errno_t err = fopen_s(&pFile, filename.toLatin1().data(), "rt");
   errno_t err = fopen_s(&pFile, "C:/trainingdata/8286/TimeSeries/Average_Z.csv", "rt");
   if (err == 13)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to open file: PERMISSION DENIED", filename.toLatin1().data());
      return FALSE;
   }
   else if (err != 0)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to open file. err=%d, file=%s", err, filename.toLatin1().data());
      return FALSE;
   }
   pOutputData->filename = filename;

   // Get data name
   fscanf_s(pFile, "%s", tempString, 64);
   pOutputData->name.fromLatin1(tempString);
   pOutputData->name.remove(0, 5);
   CJTRACE(TRACE_ERROR_LEVEL, "Loading data file: %s", filename.toLatin1().data());
   CJTRACE(TRACE_ERROR_LEVEL, "   Name:  %s", pOutputData->name.toLatin1().data());

   // Get time delta in X axis
   fscanf_s(pFile, "%s", tempString, 64);
   pOutputData->timedelta = atof(&tempString[6]);
   CJTRACE(TRACE_ERROR_LEVEL, "   Delta: %.10lf (sec)", pOutputData->timedelta);

   // Get first X axis value
   fscanf_s(pFile, "%s", tempString, 64);
   pOutputData->timestart = atof(&tempString[5]);
   CJTRACE(TRACE_ERROR_LEVEL, "   Start: %.10lf (sec)", pOutputData->timestart);

   // Get X axis label
   fscanf_s(pFile, "%s", tempString, 64);
   pOutputData->Xaxislabel.fromLatin1(tempString);
   pOutputData->Xaxislabel.remove(0, 7);
   CJTRACE(TRACE_ERROR_LEVEL, "   X Axis: %s", pOutputData->Xaxislabel.toLatin1().data());

   // Get Y axis label
   fscanf_s(pFile, "%s", tempString, 64);
   pOutputData->Yaxislabel.fromLatin1(tempString);
   pOutputData->Yaxislabel.remove(0, 7);
   CJTRACE(TRACE_ERROR_LEVEL, "   Y Axis: %s", pOutputData->Yaxislabel.toLatin1().data());

   while (strcmp(tempString, "Values") != 0)
   {
      fscanf_s(pFile, "%s", tempString, 64);  // Data Notes (skip for now)
   }
   
   // Get data values
   rc = 0;
   int count = 0;
   QVector<double> time(0); 
   double nextTime = pOutputData->timestart;

   while (rc != EOF)
   {
      rc = fscanf_s(pFile, "%s", tempString, 64);
      pOutputData->pData.append(atof(tempString));
      time.append(nextTime);
      nextTime += pOutputData->timedelta;
      count++;
      if ((count < 50) || (count % 500 == 0))
      CJTRACE(TRACE_ERROR_LEVEL, "   Value: %d, %.10lf", count, pOutputData->timedelta);
   }
   CJTRACE(TRACE_ERROR_LEVEL, "Data File Read (%d entries)", count);

   QCustomPlot* customPlot = ui.basicGraph;

   // add two new graphs and set their look:
   customPlot->addGraph();
   customPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
   // configure right and top axis to show ticks but no labels:
   // (see QCPAxisRect::setupFullAxesBox for a quicker method to do this)
   customPlot->xAxis2->setVisible(true);
   customPlot->xAxis2->setTickLabels(false);
   customPlot->yAxis2->setVisible(true);
   customPlot->yAxis2->setTickLabels(false);
   // make left and bottom axes always transfer their ranges to right and top axes:
   connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
   connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));
   // pass data points to graphs:
   customPlot->graph(0)->setData(time, pOutputData->pData);

   // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
   customPlot->graph(0)->rescaleAxes();
   // same thing for graph 1, but only enlarge ranges (in case graph 1 is smaller than graph 0):
   //customPlot->graph(1)->rescaleAxes(true);
   // Note: we could have also just called customPlot->rescaleAxes(); instead
   // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
   customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

   ExternalReplot();

   return TRUE;
}


InputspacePredictionGraph::InputspacePredictionGraph(QWidget *pParent, NetworkManager* pNetworkManager)
: BasicGraph(pParent),
dpNetworkManager(pNetworkManager)
{
   
}

void InputspacePredictionGraph::InitializeGraph()
{
   QCustomPlot* pPlot = ui.basicGraph;

   pPlot->yAxis->setRange(0, 175);


   // Setup Invalid Prediction Shading
   QPen blankPen;
   //blankPen.setColor();
   blankPen.setStyle(Qt::NoPen);

   pPlot->addGraph();
   pPlot->graph(0)->setPen(blankPen);
   pPlot->graph(0)->setBrush(QBrush(QColor(255, 0, 0, 64)));
   pPlot->graph(0)->setName("Error");


   // Setup Input Graph Line
   QPen blueDotPen;
   blueDotPen.setColor(QColor(30, 40, 255, 150));
   blueDotPen.setStyle(Qt::DotLine);
   blueDotPen.setWidthF(4);

   pPlot->addGraph();
   pPlot->graph(1)->setPen(blueDotPen); // line color blue for first graph
   pPlot->graph(1)->setName("Input");

   // Setup Prediction Graph Line
   QPen greenSolidPen;
   greenSolidPen.setColor(Qt::green);
   greenSolidPen.setStyle(Qt::SolidLine);
   greenSolidPen.setWidthF(2);

   pPlot->addGraph();
   pPlot->graph(2)->setPen(greenSolidPen);
   pPlot->graph(2)->setName("Prediction");

   // Setup Error Graph Line
   QPen greySolidPen;
   greySolidPen.setColor(Qt::gray);
   greySolidPen.setStyle(Qt::SolidLine);
   greySolidPen.setWidthF(2);

   pPlot->addGraph();
   pPlot->graph(3)->setPen(greySolidPen);
   pPlot->graph(3)->setName("Error");



}


void InputspacePredictionGraph::UpdateGraphData(int time)
{
   dpInputSpace = dpNetworkManager->inputSpaces[0];
   dpClassifier = dpNetworkManager->classifiers[0];

   QCustomPlot* pPlot = ui.basicGraph;

   if (time != dLastRecordedTime+1)
   {      
      pPlot->graph(0)->clearData();
      pPlot->graph(1)->clearData();
   }
   dLastRecordedTime = time;

   // Invalid Prediction Data
   float invalid;
   invalid = (dpClassifier->dNumValuesWithOverlap == 0) ? 175 : 0;
   pPlot->graph(0)->addData(time, invalid);
   pPlot->graph(0)->removeDataBefore(time - 100);

   // Input Data
   pPlot->graph(1)->addData(time, dpInputSpace->dCurrentInputValue);
   pPlot->graph(1)->removeDataBefore(time - 100);
      
   // Prediction Data
   pPlot->graph(2)->addData(time+1, dpClassifier->dMaxOverlapValue);
   pPlot->graph(2)->removeDataBefore(time - 100);
   
   // Error Data
   float error;
   if (dpClassifier->dPreviousMaxOverlapValue == -99999) error = 0;
   else error = dpInputSpace->dCurrentInputValue - dpClassifier->dPreviousMaxOverlapValue;
   pPlot->graph(3)->addData(time, error);
   pPlot->graph(3)->removeDataBefore(time - 100);
}

void InputspacePredictionGraph::ReplotGraph()
{
   ui.basicGraph->xAxis->setRange(dLastRecordedTime - 100, dLastRecordedTime + 10);
   ui.basicGraph->yAxis->setRange(dpInputSpace->dpSdrEncoder->dMinValue, dpInputSpace->dpSdrEncoder->dMaxValue);
   OnReplot();
}



///////////////////////////////////////////////////////////////////////////////////////////////////////
//InputspaceOverlapGraph
///////////////////////////////////////////////////////////////////////////////////////////////////////

InputspaceOverlapGraph::InputspaceOverlapGraph(QWidget *pParent, NetworkManager* pNetworkManager)
: BasicGraph(pParent),
dpNetworkManager(pNetworkManager)
{
   dpPlot = ui.basicGraph;
   dpBarsOverlap = new QCPBars(dpPlot->xAxis, dpPlot->yAxis);   
   dpBarsNotOverlap = new QCPBars(dpPlot->xAxis, dpPlot->yAxis);
}

void InputspaceOverlapGraph::InitializeGraph()
{   
   QPen pen;

   dpPlot->addPlottable(dpBarsOverlap);
   dpPlot->addPlottable(dpBarsNotOverlap);

   dpBarsOverlap->setName("Overlap");
   pen.setColor(QColor(1, 92, 191));
   dpBarsOverlap->setPen(pen);
   dpBarsOverlap->setBrush(QColor(1, 92, 191, 50));

   dpBarsNotOverlap->setName("Not Overlap");
   pen.setColor(QColor(255, 25, 0));
   dpBarsNotOverlap->setPen(pen);
   dpBarsNotOverlap->setBrush(QColor(255, 25, 0, 50));

   dpBarsNotOverlap->moveBelow(dpBarsOverlap);
}

void InputspaceOverlapGraph::UpdateGraphData(int time)
{
   dpInputSpace = dpNetworkManager->inputSpaces[0];
   dpClassifier = dpNetworkManager->classifiers[0];
}

void InputspaceOverlapGraph::ReplotGraph()
{
   int itemsToGraph = MIN(dpClassifier->dNumValuesWithOverlap, dpClassifier->dOverlapArraySize);

   float overlapMax = 0;
   float overlapMin = 10000000;
   float notMax = 0;
   float notMin = 10000000;

   // Normalize the data
   for (int i = 0; i < itemsToGraph; i++)
   {
      overlapMax = MAX(overlapMax, dpClassifier->dOverlapAmountArray[i]);
      overlapMin = MIN(overlapMin, dpClassifier->dOverlapAmountArray[i]);
      notMax = MAX(notMax, dpClassifier->dNotOverlapAmountArray[i]);
      notMin = MIN(notMin, dpClassifier->dNotOverlapAmountArray[i]);
   }

   float overlapMult = 10 / (overlapMax - overlapMin);
   float overlapOffset = 20;
   float notMult = 10 / (notMax - notMin);
   float notOffset = 5;  // Shift results down to start at 5 
   float overlapAdj;
   float notAdj;

   // No Normalization
   overlapMult = 1;
   overlapOffset = 0;
   notMult = 1;
   notOffset = 0;


   dpBarsOverlap->clearData();
   dpBarsNotOverlap->clearData();
   for (int i = 0; i < itemsToGraph; i++)
   {
      overlapAdj = ((dpClassifier->dOverlapAmountArray[i] - overlapMin) * overlapMult) + overlapOffset;
      notAdj = ((dpClassifier->dNotOverlapAmountArray[i] - notMin) * notMult) + notOffset;
      dpBarsOverlap->addData(dpClassifier->dOverlapValueArray[i], overlapAdj - notAdj);
      dpBarsNotOverlap->addData(dpClassifier->dOverlapValueArray[i], notAdj);
   }
   dpPlot->rescaleAxes();
   OnReplot();
}
