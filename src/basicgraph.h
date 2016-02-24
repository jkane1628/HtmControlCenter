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


class BasicGraph : public QFrame
{
	Q_OBJECT

public:
   BasicGraph(QWidget *parent);
   ~BasicGraph();

   void Demo();
   BOOL LoadCSVDataFile(QString filename, TimeSeriesData* pOutputData);

signals:
   void ExternalReplot();

public slots:
   void OnReplot();

protected:
   void resizeEvent(QResizeEvent *event);

private:
   Ui::BasicGraph ui;
};

/*
#include "CJCli.h"
class CliGraphCommandSet : public CliCommandSetBase
{
public:
   CliGraphCommandSet(BasicGraph* pBasicGraph) { dpCommandSet = CommandSet; dpBasicGraph = pBasicGraph; }

   // CLI Command Functions
   static CliReturnCode CliCommand_loadGraph(CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);

private:
   static CliCommand CommandSet[];
   static BasicGraph* dpBasicGraph;
};
*/

#endif // BASICGRAPH_H
