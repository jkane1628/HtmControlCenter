#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H

#include <QtWidgets/QTabWidget>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QPushButton>
#include <QBasicTimer>

#include "mainwindow.h"

class MainWindow;
#ifdef USE_HTM_VECTOR_CLASSES

class vNetworkManager;
class vRegion;
class vInputSpace;
#else
class NetworkManager;
class Region;
class InputSpace;
#endif



class ControlWidget : public QTabWidget
{
   Q_OBJECT
public:
#ifdef USE_HTM_VECTOR_CLASSES
   ControlWidget(MainWindow* pMainWindow, vNetworkManager *pNetworkManager);
#else
   ControlWidget(MainWindow* pMainWindow, NetworkManager *pNetworkManager);
#endif
   ~ControlWidget();

   void InitForNewNetworkLoad();
   void SetUpdateWhileRunning(bool update) { updateWhileRunning = update; }
   bool RunToStopTime(int newStopTimeVal, bool increment = false);

   //void keyPressEvent(QKeyEvent* e);
   void timerEvent(QTimerEvent *event);

   void UpdateNetworkInfo();
   void UpdateSelectedInfo();

#ifdef USE_HTM_VECTOR_CLASSES
   void SetSelected(vRegion *_region, vInputSpace *_input, int _colX, int _colY, int _cellIndex, int _segmentIndex);
   //class Region;
   //class InputSpace;
   //void SetSelected(Region *_region, InputSpace *_input, int _colX, int _colY, int _cellIndex, int _segmentIndex) {}
#else
   void SetSelected(Region *_region, InputSpace *_input, int _colX, int _colY, int _cellIndex, int _segmentIndex);
   //class vRegion;
   //class vInputSpace;
   //void SetSelected(vRegion *_region, vInputSpace *_input, int _colX, int _colY, int _cellIndex, int _segmentIndex) {}
#endif

private slots:
   void Pause();
   void Step();
   void Run();

   void SelectSegment(const QModelIndex & current, const QModelIndex & previous);
   void DeselectSegment();

   void CurrentTabChanged(int _currentTabIndex);


private:
#ifdef USE_HTM_VECTOR_CLASSES
   vNetworkManager* dpNetworkManager;
   vRegion *selRegion;
   vInputSpace *selInput;
#else
   NetworkManager* dpNetworkManager;
   Region *selRegion;
   InputSpace *selInput;
#endif

   QFrame* dpHTMNetworkFrame;
   QFrame* dpHTMSelectedFrame;

   void createHTMNetworkFrame();
   void createHTMSelectedFrame();

   QLabel *networkName, *networkTime, *networkInfo, *selectedInfo;
   QLineEdit *stopTime;
   QPushButton *pauseButton, *stepButton, *runButton;
   QTableWidget *segmentsTable;
   QPushButton *deselectSegButton;


   int selColX, selColY, selCellIndex, selSegmentIndex;

   MainWindow* dpMainWindow;
    
   bool running;
   bool updateWhileRunning;
   bool networkFrameRequiresUpdate, selectedFrameRequiresUpdate;
   int  stopTimeVal;
   QBasicTimer timer;
   
   
   
   

};

#endif // CONTROLWIDGET_H
