
#include "CJTrace.h"
#include "CJCli.h"
#include "CJConsole.h"
#include "basicgraph.h"


BasicGraph* CliGraphCommandSet::dpBasicGraph = NULL;

CliCommand CliGraphCommandSet::CommandSet[] =
{
   { "lg",
     "Loads a datafile to the graph window",
     "setmotor motorId dir speed\n"\
     "   PARAMS:"\
     "     filename - filename of .csv datafile",
     eCliAccess_Guest,
     CliGraphCommandSet::CliCommand_loadGraph },


   {"","","",eCliAccess_Hidden} // This must be the last command
};

CliReturnCode CliGraphCommandSet::CliCommand_loadGraph(CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams)
{
   pConsole->Printf("Load Graph Command...\n");
   
   //if (pParams->numParams != 4)
   //   return eCliReturn_InvalidParam;
   
   TimeSeriesData timeseriesdata;
   timeseriesdata.filename = QString::fromUtf8((const char*)pParams->str[1]);
   timeseriesdata.filename.remove(0, 8);
   if (dpBasicGraph->LoadCSVDataFile(timeseriesdata.filename, &timeseriesdata) == FALSE)
   {
      return eCliReturn_Failed;
   }
   return eCliReturn_Success;   
}


BasicGraph::BasicGraph(QWidget *parent)
: QFrame(parent), CJObject("BG","")
{
	ui.setupUi(this);
   OnReplot();

   CJTRACE_REGISTER_TRACE_OBJ(this);
   CJTRACE_REGISTER_CLI_COMMAND_OBJ(new CliGraphCommandSet(this));

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



