

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QBasicTimer>
#include <QTextStream>
#include <QComboBox>
#include <QTime>
#include <QTimerEvent>

#include "htm/Segment.h"
#include "htm/Cell.h"
#include "htm/NetworkManager.h"
#include "htm/View.h"
#include "controlwidget.h"

ControlWidget::ControlWidget(NetworkManager *pNetworkManager)
{
   view1 = NULL;
   view2 = NULL;
   dpNetworkManager = pNetworkManager;

   running = false;
   updateWhileRunning = false;
   networkFrameRequiresUpdate = false;
   selectedFrameRequiresUpdate = false;
   stopTimeVal = 0;

   selRegion = NULL;
   selInput = NULL;
   selColX = -1;
   selColY = -1;
   selCellIndex = -1;
   selSegmentIndex = -1;

   createHTMNetworkFrame();
   createHTMSelectedFrame();

   addTab(dpHTMNetworkFrame, tr("Network"));
   addTab(dpHTMSelectedFrame, tr("Selected"));
   connect(this, SIGNAL(currentChanged(int)), this, SLOT(CurrentTabChanged(int)));
}

ControlWidget::~ControlWidget()
{

}

void ControlWidget::RegisterHtmView(View* pView1, View* pView2)
{
   view1 = pView1;
   view2 = pView2;
}

void ControlWidget::InitForNewNetworkLoad()
{
   // Initialize stop time.
   stopTimeVal = 0;
   stopTime->setText("0");

   // Update the network controls
   pauseButton->setEnabled(false);
   stepButton->setEnabled(true);
   runButton->setEnabled(true);

   // Update the UI to reflect the network that has been loaded.
   UpdateUIForNetwork();
}



void ControlWidget::createHTMNetworkFrame()
{
   // Create the network frame.
   dpHTMNetworkFrame = new QFrame();
   QVBoxLayout *networkFrameLayout = new QVBoxLayout();
   networkFrameLayout->setAlignment(Qt::AlignTop);
   dpHTMNetworkFrame->setLayout(networkFrameLayout);

   // Create the network name label
   networkName = new QLabel();
   networkFrameLayout->addWidget(networkName);

   // Create the controls frame.
   QFrame *controlsFrame = new QFrame();
   controlsFrame->setFrameShadow(QFrame::Sunken);
   controlsFrame->setFrameShape(QFrame::WinPanel);
   //controlsFrame->setFixedHeight(50);
   networkFrameLayout->addWidget(controlsFrame);
   QVBoxLayout *controlsFrameLayout = new QVBoxLayout();
   controlsFrame->setLayout(controlsFrameLayout);

   // Create the control buttons frame
   QFrame *controlButtonsFrame = new QFrame();
   controlsFrameLayout->addWidget(controlButtonsFrame);
   QHBoxLayout *controlsLayout = new QHBoxLayout();
   controlsLayout->setMargin(0);
   controlButtonsFrame->setLayout(controlsLayout);

   // Pause simulation button
   pauseButton = new QPushButton("||");
   controlsLayout->addWidget(pauseButton);
   connect(pauseButton, SIGNAL(clicked()), this, SLOT(Pause()));

   // Step simulation button
   stepButton = new QPushButton(">");
   controlsLayout->addWidget(stepButton);
   connect(stepButton, SIGNAL(clicked()), this, SLOT(Step()));

   // Run simulation button
   runButton = new QPushButton(">>");
   controlsLayout->addWidget(runButton);
   connect(runButton, SIGNAL(clicked()), this, SLOT(Run()));

   // Create the network stop time frame
   QFrame *timeFrame = new QFrame();
   controlsFrameLayout->addWidget(timeFrame);
   QHBoxLayout *timeLayout = new QHBoxLayout();
   timeLayout->setMargin(0);
   timeFrame->setLayout(timeLayout);

   // Create the network time label
   networkTime = new QLabel();
   timeLayout->addWidget(networkTime);

   // Add stretch to layout
   timeLayout->addStretch();

   // Create the network stop time label
   QLabel *stopTimeLabel = new QLabel("Stop at:");
   timeLayout->addWidget(stopTimeLabel);

   // Create the stop time QLineEdit
   stopTime = new QLineEdit();
   stopTime->setMaximumWidth(50);
   timeLayout->addWidget(stopTime);

   // Create the network info label
   networkInfo = new QLabel();
   networkFrameLayout->addWidget(networkInfo);

}

void ControlWidget::createHTMSelectedFrame()
{
   // Create the selected frame.
   dpHTMSelectedFrame = new QFrame();
   QVBoxLayout *selectedFrameLayout = new QVBoxLayout();
   selectedFrameLayout->setAlignment(Qt::AlignTop);
   dpHTMSelectedFrame->setLayout(selectedFrameLayout);

   // Create the selected info label
   selectedInfo = new QLabel();
   selectedInfo->setMinimumHeight(300);
   selectedInfo->setAlignment(Qt::AlignTop);
   selectedFrameLayout->addWidget(selectedInfo);

   // Create the segmentsTable
   segmentsTable = new QTableWidget(0, 6);
   segmentsTable->setHorizontalHeaderLabels(QStringList() << "Seq" << "Steps" << "Syns" << "Actv" << "Prev" << "Created");
   segmentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
   segmentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
   segmentsTable->setSelectionMode(QAbstractItemView::SingleSelection);
   connect(segmentsTable->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(SelectSegment(QModelIndex, QModelIndex)));
   selectedFrameLayout->addWidget(segmentsTable);
   segmentsTable->setColumnWidth(0, 30);
   segmentsTable->setColumnWidth(1, 40);
   segmentsTable->setColumnWidth(2, 35);
   segmentsTable->setColumnWidth(3, 50);
   segmentsTable->setColumnWidth(4, 50);
   segmentsTable->setColumnWidth(5, 50);

   // Deselect segment
   deselectSegButton = new QPushButton("Display All Segments");
   selectedFrameLayout->addWidget(deselectSegButton);
   connect(deselectSegButton, SIGNAL(clicked()), this, SLOT(DeselectSegment()));
}

void ControlWidget::UpdateUIForNetworkExecution()
{
   // Update the network UI
   UpdateNetworkInfo();

   // Do not update the rest of the UI if currently running, and the UI isn't to be updated while running.
   if (running && !updateWhileRunning) {
      return;
   }

   // Update the selected item info.
   UpdateSelectedInfo();

   // Update the views for execution.
   if (view1 != NULL)
      view1->UpdateForExecution();
   if (view2 != NULL)
      view2->UpdateForExecution();
}

void ControlWidget::UpdateUIForNetwork()
{
   // Update the network UI
   UpdateNetworkInfo();

   if (view1 != NULL)
   {
      view1->UpdateForNetwork(dpNetworkManager);
      // Show the first DataSpace in view1.
      if (view1->showComboBox->count() > 0) {
         view1->showComboBox->setCurrentIndex(0);
      }
   }

   if (view2 != NULL)
   {
      view2->UpdateForNetwork(dpNetworkManager);
      // Show the first DataSpace in view2.
      if (view2->showComboBox->count() > 0) {
         view2->showComboBox->setCurrentIndex(0);
      }
   }

   // Update information frames.
   UpdateSelectedInfo();
}


void ControlWidget::Pause()
{
   if (!running) {
      return;
   }

   running = false;

   pauseButton->setDisabled(true);
   stepButton->setDisabled(false);
   runButton->setDisabled(false);

   // Stop timer.
   timer.stop();

   // If the UI hasn't been updated fully while running, update fully now.
   if (!updateWhileRunning) {
      UpdateUIForNetworkExecution();
   }
}

void ControlWidget::Step()
{
   if (running) {
      return;
   }

   // Have the network manager take one step.
   dpNetworkManager->Step();

   // Update UI
   UpdateUIForNetworkExecution();
}

void ControlWidget::Run()
{
   if (running) {
      return;
   }

   running = true;

   pauseButton->setDisabled(false);
   stepButton->setDisabled(true);
   runButton->setDisabled(true);

   // Start timer.
   timer.start(0, this);
}

void ControlWidget::SelectSegment(const QModelIndex & current, const QModelIndex & previous)
{
   selSegmentIndex = current.row();

   // Let the Views know what has been selected.

   if (view1 != NULL) view1->SetSelected(selRegion, selInput, selColX, selColY, selCellIndex, selSegmentIndex);
   if (view2 != NULL) view2->SetSelected(selRegion, selInput, selColX, selColY, selCellIndex, selSegmentIndex);
}

void ControlWidget::DeselectSegment()
{
   segmentsTable->clearSelection();
   segmentsTable->selectionModel()->clearCurrentIndex();
}

void ControlWidget::CurrentTabChanged(int _currentTabIndex)
{
   if (networkFrameRequiresUpdate)
   {
      UpdateNetworkInfo();
      networkFrameRequiresUpdate = false;
   }

   if (selectedFrameRequiresUpdate)
   {
      UpdateSelectedInfo();
      selectedFrameRequiresUpdate = false;
   }
}

void ControlWidget::UpdateNetworkInfo()
{
   // Do not update the network frame if it is not currently visible.
   if (currentWidget() != dpHTMNetworkFrame)
   {
      networkFrameRequiresUpdate = true;
      return;
   }

   networkTime->setText(QString("Time: %1").arg(dpNetworkManager->GetTime()));

   // If currently running, don't update the rest of the network UI.
   if (running) {
      return;
   }

   networkName->setText(QString("File: ") + dpNetworkManager->GetFilename());
}


void ControlWidget::UpdateSelectedInfo()
{
   QWidget* pCurrentWidget = currentWidget();
   
   // Do not update the selected frame if it is not currently visible.
   if (pCurrentWidget != dpHTMSelectedFrame)
   {
      selectedFrameRequiresUpdate = true;
      return;
   }

   Cell *selCell = NULL;
   QString infoString;
   QTextStream info(&infoString);
   int numSegments = 0;

   if (selInput != NULL)
   {
      info << "<b><u>InputSpace:</b> " << selInput->GetID() << "</u><br>";
      info << "Dimensions: " << selInput->GetSizeX() << " x " << selInput->GetSizeY() << "<br>";
      info << "<br>";
      info << "<b><u>Column:</b> " << selColX << "," << selColY << "</u><br>";
      info << "<br>";

      if (selCellIndex != -1)
      {
         info << "<b><u>Cell:</b> " << selCellIndex << "</u><br>";
         info << (selInput->GetIsActive(selColX, selColY, selCellIndex) ? "Active" : "Inactive") << "<br>";
      }
   }
   else if (selRegion != NULL)
   {
      Column *selCol = selRegion->GetColumn(selColX, selColY);

      info << "<b><u>Region:</b> " << selRegion->GetID() << "</u><br>";
      info << "Dimensions: " << selRegion->GetSizeX() << " x " << selRegion->GetSizeY() << "<br>";
      info << "Time: " << dpNetworkManager->GetTime() << "<br>";
      info << "<br>";
      info << "<b><u>Column:</b> " << selColX << "," << selColY << "</u><br>";
      info << "Overlap: " << selCol->GetOverlap() << "<br>";
      info << "Active: " << (selCol->GetIsActive() ? "Yes" : "No") << "<br>";
      info << "Inhibited: " << (selCol->GetIsInhibited() ? "Yes" : "No") << "<br>";
      info << "Active Duty Cycle: " << selCol->GetActiveDutyCycle() << "<br>";
      info << "Fast Active Duty Cycle: " << selCol->GetFastActiveDutyCycle() << "<br>";
      info << "Overlap Duty Cycle: " << selCol->GetOverlapDutyCycle() << "<br>";
      info << "Max Duty Cycle: " << selCol->GetMaxDutyCycle() << "<br>";
      info << "Min Duty Cycle: " << (selCol->GetMaxDutyCycle() * 0.01f) << "<br>";
      info << "Min Overlap: " << selCol->GetMinOverlap() << "<br>";
      info << "Boost: " << selCol->GetBoost() << "<br>";
      info << "Prev Boost Time: " << selCol->prevBoostTime << "<br>";
      info << "DesiredLocalActivity: " << selCol->DesiredLocalActivity << "<br>";
      info << "<br>";

      if (selCellIndex != -1)
      {
         selCell = selCol->GetCellByIndex(selCellIndex);

         info << "<b><u>Cell:</b> " << selCellIndex << "</u><br>";
         info << "Active: " << (selCell->GetIsActive() ? "Yes" : "No") << "<br>";
         info << "Predicted: " << (selCell->GetIsPredicting() ? "Yes" : "No") << "<br>";
         info << "Num Prediction Steps: " << selCell->GetNumPredictionSteps() << "<br>";
         info << "Learning: " << (selCell->GetIsLearning() ? "Yes" : "No") << "<br>";
         info << "Prev Active: " << (selCell->GetWasActive() ? "Yes" : "No") << "<br>";
         info << "Prev Predicted: " << (selCell->GetWasPredicted() ? "Yes" : "No") << "<br>";
         info << "Prev Learning: " << (selCell->GetWasLearning() ? "Yes" : "No") << "<br>";
         info << "Prev Active Time: " << selCell->GetPrevActiveTme() << "<br>\n";
         info << "Num Distal Segments: " << selCell->Segments.Count() << "<br>\n";

         segmentsTable->setVisible(false);
         segmentsTable->setUpdatesEnabled(false);

         QTableWidgetItem *item;
         FastListIter segments_iter(selCell->Segments);
         for (Segment *seg = (Segment*)(segments_iter.Reset()); seg != NULL; seg = (Segment*)(segments_iter.Advance()))
         {
            //Seq   Steps   Syns    Actv   Prev Actv  Created

            // Add a new row to the segmentsTable if necessary.
            if (segmentsTable->rowCount() <= numSegments) {
               segmentsTable->insertRow(numSegments);
            }

            item = new QTableWidgetItem(seg->GetIsSequence() ? tr("Yes") : tr("No"));
            segmentsTable->setItem(numSegments, 0, item);

            item = new QTableWidgetItem(tr("%1").arg(seg->GetNumPredictionSteps()));
            segmentsTable->setItem(numSegments, 1, item);

            item = new QTableWidgetItem(tr("%1").arg(seg->Synapses.Count()));
            segmentsTable->setItem(numSegments, 2, item);

            item = new QTableWidgetItem(tr("%1 (%2)").arg(seg->GetIsActive() ? "Y" : "N").arg(seg->ActiveSynapses.Count()));
            segmentsTable->setItem(numSegments, 3, item);

            item = new QTableWidgetItem(tr("%1 (%2)").arg(seg->GetWasActive() ? "Y" : "N").arg(seg->PrevActiveSynapses.Count()));
            segmentsTable->setItem(numSegments, 4, item);

            item = new QTableWidgetItem(tr("%1").arg(seg->GetCreationTime()));
            segmentsTable->setItem(numSegments, 5, item);

            numSegments++;
         }

         segmentsTable->setUpdatesEnabled(true);
         segmentsTable->setVisible(true);
      }
   }
   else
   {
      info << "<b>No selection.</b><br>";
   }

   int i;

   for (i = 0; i < numSegments; i++) {
      segmentsTable->showRow(i);
   }

   for (i = numSegments; i < segmentsTable->rowCount(); i++) {
      segmentsTable->hideRow(i);
   }

   // Only make the segments table visible if there is a selected cell.
   segmentsTable->setVisible(selCell != NULL);
   deselectSegButton->setVisible(selCell != NULL);

   selectedInfo->setText(infoString);
}

void ControlWidget::SetSelected(Region *_region, InputSpace *_input, int _colX, int _colY, int _cellIndex, int _segmentIndex)
{
   selRegion = _region;
   selInput = _input;
   selColX = _colX;
   selColY = _colY;
   selCellIndex = _cellIndex;
   selSegmentIndex = _segmentIndex;

   // Let the Views know what has been selected.
   if (view1 != NULL) view1->SetSelected(_region, _input, _colX, _colY, _cellIndex, selSegmentIndex);
   if (view2 != NULL) view2->SetSelected(_region, _input, _colX, _colY, _cellIndex, selSegmentIndex);

   // Update info about selected item.
   UpdateSelectedInfo();
}


void ControlWidget::timerEvent(QTimerEvent *event)
{
   if (event->timerId() == timer.timerId())
   {
      QTime start_time;
      start_time.start();

      // Get the latest value for the stop time.
      if (stopTime->isModified())
      {
         stopTimeVal = stopTime->text().toInt();
         stopTime->setModified(false);
      }

      // Run as many time steps as will fit within a small limited time period.
      do
      {
         // Execute one step for the network.
         dpNetworkManager->Step();

         // If the current time is the stop time, pause and exit loop.
         if (dpNetworkManager->GetTime() == stopTimeVal)
         {
            Pause();
            break;
         }

         // If the UI is to be updated while running, run only one step at a time.
         if (updateWhileRunning) {
            break;
         }
      } while ((start_time.elapsed()) < 500);

      // Update the UI
      UpdateUIForNetworkExecution();
   }
   else
   {
      QWidget::timerEvent(event);
   }
}

