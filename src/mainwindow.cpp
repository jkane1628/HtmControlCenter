/**
*                              (c) Copyright 2013-2014, KFS, Westminster, CO
*                                           All Rights Reserved
* @file actuator.h
*
* @section LICENSE
*    HydroidOS is provided in source form for FREE evaluation, for hobby use, for educational use, or for peaceful research.
*    If you plan on using HydroidOS in a commercial product you need to contact KFS to properly license
*    its use in your product. We provide ALL the source code for your convenience and to help you experience
*    HydroidOS.  The fact that the source is provided does NOT mean that you can use it without paying a
*    licensing fee.
*
* @section DESCRIPTION
*    Definition of the actuator control functions.  These functions include control of the speed controls or servos attached to the system.  This module allows
*    the user to initialize, set, get, and report settings, values, and the status of each actuator.
*/


#include <QtWidgets>

#include "CJTrace.h"
#include "CJCli.h"

#include "htm/View.h"
#include "htm/NetworkManager.h"
#include "mousecontrols.h"
#include "controlwidget.h"
#include "consoledock.h"
#include "basicgraph.h"


#include "mainwindow.h"


#undef CJTRACE_MODULE_ID
#define CJTRACE_MODULE_ID eTraceId_MainWindow

// Map CLI commands to associated functions for the default command set
CJCLI_CREATE_FCNMAPPER(CliMainWindowCommandSet)
CJCLI_MAP_CMD_TO_FCN(CliMainWindowCommandSet, ln, CliCommand_loadNetwork)

// Define CLI commands for the default command set
CJCLI_COMMAND_DEFINTION_START(CliMainWindowCommandSet)
CJCLI_COMMAND_DESCRIPTOR(ln, eCliAccess_Guest, "Loads a HTM given network file, or the last loaded network file")
CJCLI_COMMAND_DEFINTION_END




/*

   "ln <filename>\n"\
   "   PARAMS:"\
   "     filename - full path of .xml network datafile, or blank to load last file",

*/

MainWindow::MainWindow()
{

   // Create the NetworkManager
   dpConsoleDock = new ConsoleDock(this);  // This creates the CLI Object...TODO: BREAK OUT CLI AND CONSOLE CREATION
   dpHtmNetworkManager = new NetworkManager();
   pControlWidget = new ControlWidget(dpHtmNetworkManager); // Create the tabbed widget for the frames in the right-hand panel.
   dpHtmView1 = new View(pControlWidget);
   dpHtmView2 = new View(pControlWidget);
   dpBasicGraph1 = new BasicGraph(this);
   dpBasicGraph2 = new BasicGraph(this);

   pControlWidget->RegisterHtmView(dpHtmView1, dpHtmView2);
   
   CJTRACE_SET_TRACEID_STRING(eTraceId_MainWindow, "MW")
   CJTRACE_REGISTER_CLI_COMMAND_OBJ(CliMainWindowCommandSet)
   CJCLI_CMDSET_OBJECT(CliMainWindowCommandSet).Initialize(this);
   
   //connect(&CJCLI_CMDSET_OBJECT(CliMainWindowCommandSet), CliMainWindowCommandSet::LoadNetworkFileSignal(QString, CliReturnCode*), this, cliLoadNetworkFile(QString, CliReturnCode*)/*, Qt::BlockingQueuedConnection*/);


   createActions();
   createMenus();
   createStatusBar();
   createDockWindows();
	createMainFrame();
}


void MainWindow::createActions()
{    
    quitAct = new QAction(tr("&Quit"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit the application"));
    connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
   // File menu
   fileMenu = menuBar()->addMenu(tr("&File"));

   loadNetworkAct = new QAction(tr("&Load Network..."), this);
   fileMenu->addAction(loadNetworkAct);
   connect(loadNetworkAct, SIGNAL(triggered()), this, SLOT(menuLoadNetworkFile()));

   loadDataAct = new QAction(tr("&Load Data..."), this);
   fileMenu->addAction(loadDataAct);
   connect(loadDataAct, SIGNAL(triggered()), this, SLOT(loadDataFile()));

   saveDataAct = new QAction(tr("&Save Data..."), this);
   fileMenu->addAction(saveDataAct);
   connect(saveDataAct, SIGNAL(triggered()), this, SLOT(saveDataFile()));

   fileMenu->addSeparator();

   exitAct = new QAction(tr("E&xit"), this);
   fileMenu->addAction(exitAct);
   connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

   // View menu
   viewMenu = menuBar()->addMenu(tr("&View"));

   viewDuringRunAct = new QAction(tr("&Update while running"), this);
   viewMenu->addAction(viewDuringRunAct);
   viewDuringRunAct->setCheckable(true);
   viewDuringRunAct->setChecked(FALSE); 
   connect(viewDuringRunAct, SIGNAL(triggered()), this, SLOT(ViewMode_UpdateWhileRunning()));

   // Mouse menu
   mouseMenu = menuBar()->addMenu(tr("&Mouse"));

   QActionGroup *mouseActionGroup = new QActionGroup(mouseMenu);

   mouseModeSelectAct = new QAction(tr("&Select (s)"), this);
   mouseMenu->addAction(mouseModeSelectAct);
   mouseModeSelectAct->setCheckable(true);
   mouseActionGroup->addAction(mouseModeSelectAct);
   connect(mouseModeSelectAct, SIGNAL(triggered()), this, SLOT(MouseMode_Select()));

   mouseModeDragAct = new QAction(tr("&Drag (d)"), this);
   mouseMenu->addAction(mouseModeDragAct);
   mouseModeDragAct->setCheckable(true);
   mouseActionGroup->addAction(mouseModeDragAct);
   connect(mouseModeDragAct, SIGNAL(triggered()), this, SLOT(MouseMode_Drag()));

   mouseModeSelectAct->setChecked(true);
   mouseModeDragAct->setChecked(false);

   // Help Menu
   helpMenu = menuBar()->addMenu(tr("&Help"));
   helpMenu->addAction(aboutAct);
   helpMenu->addAction(aboutQtAct);
}


void MainWindow::ViewMode_UpdateWhileRunning()
{
   pControlWidget->SetUpdateWhileRunning(viewDuringRunAct->isChecked());
}

void MainWindow::menuLoadNetworkFile()
{
   QString error_msg;
   QString fileName = QFileDialog::getOpenFileName(this, tr("Load the network architecture"), tr(""), tr("XML File (*.xml)"));
   if (!fileName.isEmpty())
   {
      if (!dpHtmNetworkManager->LoadNetwork(fileName, error_msg))
         QMessageBox::critical(this, "Error loading network.", error_msg, QMessageBox::Ok);
      else
         pControlWidget->InitForNewNetworkLoad();
   }
}

void MainWindow::cliLoadNetworkFile(QString fileName, CliReturnCode* pCliRc)
{   
   QString error_msg;
   bool result;

   if (fileName.isEmpty())
      result = dpHtmNetworkManager->LoadLastNetwork(error_msg);
   else
      result = dpHtmNetworkManager->LoadNetwork(fileName, error_msg);

   if (result == false)
   {
      *pCliRc = eCliReturn_Failed;
      return;
   }
   pControlWidget->InitForNewNetworkLoad();
   *pCliRc = eCliReturn_Success;
   return;
}


void MainWindow::loadDataFile()
{
   QString fileName = QFileDialog::getOpenFileName(this, tr("Load the network data"), tr(""), tr("CLA Data (*.clad)"));

   if (!fileName.isEmpty())
   {
      // Load data
      QFile* file = new QFile(fileName);

      // If the file failed to open, display message.
      if (!file->open(QIODevice::ReadOnly))
      {
         QMessageBox::critical(this, "Error loading data.", QString("Couldn't open ") + fileName, QMessageBox::Ok);
         return;
      }

      // Parse the data file
      QString error_msg;
      bool result = dpHtmNetworkManager->LoadData(QFileInfo(*file).fileName(), file, error_msg);

      if (result == false)
      {
         QMessageBox::critical(this, "Error loading data.", error_msg, QMessageBox::Ok);
         return;
      }

      // Update the UI to reflect the network's data that has been loaded.
      pControlWidget->UpdateUIForNetwork();
   }
}


void MainWindow::saveDataFile()
{
   QString fileName = QFileDialog::getSaveFileName(this, tr("Save the network data"), tr(""), tr("CLA Data (*.clad)"));

   if (!fileName.isEmpty())
   {
      // Load data
      QFile* file = new QFile(fileName);

      // If the file failed to open, display message.
      if (!file->open(QIODevice::WriteOnly))
      {
         QMessageBox::critical(this, "Error saving data.", QString("Couldn't open ") + fileName, QMessageBox::Ok);
         return;
      }

      // Save the data file
      QString error_msg;
      bool result = dpHtmNetworkManager->SaveData(QFileInfo(*file).fileName(), file, error_msg);

      if (result == false)
      {
         QMessageBox::critical(this, "Error saving data.", error_msg, QMessageBox::Ok);
         return;
      }
   }
}


void MainWindow::MouseMode_Select()
{
   SetMouseMode(MOUSE_MODE_SELECT);
}


void MainWindow::MouseMode_Drag()
{
   SetMouseMode(MOUSE_MODE_DRAG);
}


void MainWindow::SetMouseMode(MouseMode _mouseMode)
{
   mouseMode = _mouseMode;

   dpHtmView1->SetMouseMode(_mouseMode);
   dpHtmView2->SetMouseMode(_mouseMode);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}


void MainWindow::createDockWindows()
{
    dpConsoleDock->setFeatures(QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::BottomDockWidgetArea, dpConsoleDock);
}


void MainWindow::createMainFrame()
{
   // Create a frame to contain the central layout 
   QFrame *topFrame = new QFrame();
   QHBoxLayout *topLayout = new QHBoxLayout;
   QHBoxLayout *graphLayout = new QHBoxLayout;

   QSplitter *hSplitterGraph = new QSplitter;
   hSplitterGraph->setOrientation(Qt::Vertical);
   hSplitterGraph->addWidget(dpBasicGraph1);
   hSplitterGraph->addWidget(dpBasicGraph2);

   QSplitter *vSplitter1 = new QSplitter;
   vSplitter1->setOrientation(Qt::Horizontal);
   vSplitter1->addWidget(hSplitterGraph);
   vSplitter1->addWidget(dpHtmView1);
   vSplitter1->addWidget(dpHtmView2);
   vSplitter1->addWidget(pControlWidget);

   topLayout->addWidget(vSplitter1);
   topLayout->setContentsMargins(0, 0, 0, 0);
   topLayout->setSpacing(2);
   topFrame->setLayout(topLayout);

    // Make the main frame the central widget of the window.
    setCentralWidget(topFrame);

    // Set the window title.
    setWindowTitle(tr("Control Center"));

    // Set initial window size.
    QDesktopWidget desktop;
    resize(desktop.screenGeometry().width() * 0.75, desktop.screenGeometry().height() * 0.75);
}



CliReturnCode CliMainWindowCommandSet::CliCommand_loadNetwork(CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams)
{
   CliReturnCode rc;
   QString fileName;

   if (pParams->numParams == 1)
   {
      fileName = "";      
   }
   else if (pParams->numParams == 2)
   {
      fileName = pParams->parsedBuffer[1];
   }
   else
      return eCliReturn_InvalidParam;

   // Send signal to main window so HTM & GUI operations can execute on proper thread
   LoadNetworkFileSignal(fileName, &rc);

   return eCliReturn_Success;
}
