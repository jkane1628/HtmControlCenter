#pragma once
/**
*                              (c) Copyright 2013-2014, KFS, Westminster, CO
*                                           All Rights Reserved
* @file mainwindow.h
*
* @section LICENSE
*    HydroidOS is provided in source form for FREE evaluation, for hobby use, for educational use, or for peaceful research.
*    If you plan on using HydroidOS in a commercial product you need to contact KFS to properly license
*    its use in your product. We provide ALL the source code for your convenience and to help you experience
*    HydroidOS.  The fact that the source is provided does NOT mean that you can use it without paying a
*    licensing fee.
*
* @section DESCRIPTION
*    TODO: ADD TEXT
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "htm/View.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QTextEdit;
QT_END_NAMESPACE

class BasicGraph;
class InputspacePredictionGraph;
class InputspaceOverlapGraph;
class ConsoleDock;
class View;
class NetworkManager;
class ControlWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
   MainWindow();

   void UpdateUIDataForNetworkStep(int time);
   void UpdateUIForNetworkExecution(int time);
   void UpdateUIForNetworkLoad();
   void UpdateUIForInfoSelection(Region *_region, InputSpace *_input, int _colX, int _colY, int _cellIndex, int _segmentIndex);

public slots:
   void cliLoadNetworkFile(QString fileName, CliReturnCode* pCliRc);
   void cliStepHtmNetwork(int numSteps, CliReturnCode* pCliRc);

   

private slots:
   void MouseMode_Select();
   void MouseMode_Drag();

   void ViewMode_UpdateWhileRunning();
   void menuLoadNetworkFile();
   
   void loadDataFile();
   void saveDataFile();

private:
    void createActions();
    void createMenus();
    void createStatusBar();
    void createDockWindows();
    void createRightSideTabs();
    void createMainFrame();

    QTextEdit *textEdit; 
	
	 QAction *aboutAct;
	 QAction *aboutQtAct;
    QAction *quitAct;

    // Application Objects
    NetworkManager*  dpHtmNetworkManager;
    View*            dpHtmView1;
    View*            dpHtmView2;
    ConsoleDock*     dpConsoleDock;
    InputspacePredictionGraph*   dpBasicGraph1;
    InputspaceOverlapGraph*      dpBasicGraph2;
    ControlWidget*   pControlWidget;

    // Menus
    QMenu   *helpMenu;
    QMenu   *fileMenu, *viewMenu, *mouseMenu;
    QAction *loadNetworkAct, *loadDataAct, *saveDataAct;
    QAction *exitAct;
    QAction *viewDuringRunAct;
    QAction *mouseModeSelectAct, *mouseModeDragAct;

    
    void SetMouseMode(MouseMode _mouseMode);
    MouseMode mouseMode;

};

#include "CJCli.h"
class CliMainWindowCommandSet : public QObject
{
   Q_OBJECT
public:
   CliMainWindowCommandSet() {}

   void Initialize(MainWindow* pMainWindow)
   {
      dpMainWindow = pMainWindow;
      connect(this, &CliMainWindowCommandSet::LoadNetworkFileSignal, dpMainWindow, &MainWindow::cliLoadNetworkFile, Qt::BlockingQueuedConnection);
      connect(this, &CliMainWindowCommandSet::StepNetworkSignal, dpMainWindow, &MainWindow::cliStepHtmNetwork, Qt::BlockingQueuedConnection);
   }
   static CliCommand CommandDescriptorArray[];

   // CLI Command Functions
   CliReturnCode CliCommand_loadNetwork(CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
   CliReturnCode CliCommand_stepNetwork(CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
   CliReturnCode CliCommand_vectorTest(CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
   


signals:
   void LoadNetworkFileSignal(QString fileName, CliReturnCode* pCliRc);
   void StepNetworkSignal(int numSteps, CliReturnCode* pCliRc);

private:

   MainWindow* dpMainWindow;
};

#endif
