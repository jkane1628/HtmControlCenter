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




class MainWindow;
class NetworkManager;
class Region;
class InputSpace;

class ControlWidget : public QTabWidget
{
   Q_OBJECT
public:
   ControlWidget(MainWindow* pMainWindow, NetworkManager *pNetworkManager);
   ~ControlWidget();

   void InitForNewNetworkLoad();
   void SetUpdateWhileRunning(bool update) { updateWhileRunning = update; }
   bool RunToStopTime(int newStopTimeVal, bool increment = false);

   //void keyPressEvent(QKeyEvent* e);
   void timerEvent(QTimerEvent *event);

   void UpdateNetworkInfo();
   void UpdateSelectedInfo();
   void SetSelected(Region *_region, InputSpace *_input, int _colX, int _colY, int _cellIndex, int _segmentIndex);

private slots:
   void Pause();
   void Step();
   void Run();

   void SelectSegment(const QModelIndex & current, const QModelIndex & previous);
   void DeselectSegment();

   void CurrentTabChanged(int _currentTabIndex);


private:
   NetworkManager* dpNetworkManager;
   QFrame* dpHTMNetworkFrame;
   QFrame* dpHTMSelectedFrame;

   void createHTMNetworkFrame();
   void createHTMSelectedFrame();

   QLabel *networkName, *networkTime, *networkInfo, *selectedInfo;
   QLineEdit *stopTime;
   QPushButton *pauseButton, *stepButton, *runButton;
   QTableWidget *segmentsTable;
   QPushButton *deselectSegButton;

   Region *selRegion;
   InputSpace *selInput;
   int selColX, selColY, selCellIndex, selSegmentIndex;

   MainWindow* dpMainWindow;
    
   bool running;
   bool updateWhileRunning;
   bool networkFrameRequiresUpdate, selectedFrameRequiresUpdate;
   int  stopTimeVal;
   QBasicTimer timer;
   
   
   
   

};

#endif // CONTROLWIDGET_H
