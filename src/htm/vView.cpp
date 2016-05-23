
#include <QtGui/QWheelEvent>
#include <QtWidgets/QStyle.h>
#include <QtWidgets/QToolButton.h>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLabel>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QMenu>
#include <QtCore/qmath.h>

#include "controlwidget.h"

#include "vNetworkManager.h"
#include "vInputSpace.h"
#include "vRegion.h"
#include "vColumnDisp.h"

#include "vView.h"


void vGraphicsView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        if (e->delta() > 0)
            view->zoomIn(6);
        else
            view->zoomOut(6);
        e->accept();
    } else {
        QGraphicsView::wheelEvent(e);
    }
}

void vGraphicsView::mousePressEvent(QMouseEvent *event)
{
	if (view->GetMouseMode() != MOUSE_MODE_SELECT)
	{
		QGraphicsView::mousePressEvent(event);
		event->ignore();
		return;
	}

	// Pass this event to the View
	view->MousePressed(event, false);
 }

void vGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (view->GetMouseMode() != MOUSE_MODE_SELECT)
	{
		QGraphicsView::mousePressEvent(event);
		event->ignore();
		return;
	}

	// Pass this event to the View
	view->MousePressed(event, true);
}

vView::vView(ControlWidget *_win, QWidget *parent)
    : QFrame(parent)
{
	win = _win;
	mouseMode = MOUSE_MODE_SELECT;
	viewActivity = true;
	viewReconstruction = false;
	viewPrediction = false;
	viewBoost = false;
	viewConnectionsIn = true;
	viewConnectionsOut = false;
	viewMarkedCells = false;
	dpSelRegion = NULL;
	dpSelInput = NULL;
	selColX = -1;
	selColY = -1;
	selColIndex = -1;
	selCellIndex = -1;
	selSegmentIndex = -1;
	columnDisps = NULL;
	sceneWidth = -1;
	sceneHeight = -1;
	dpDataSpace = NULL;
	addingDataSpaces = false;

	setFrameStyle(Sunken | StyledPanel);
	graphicsView = new vGraphicsView(this);
	graphicsView->setRenderHint(QPainter::Antialiasing, true);
	graphicsView->setDragMode(QGraphicsView::NoDrag);
	graphicsView->setOptimizationFlags(QGraphicsView::DontSavePainterState);
	graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
	graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	graphicsView->setInteractive(false);

	int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
	QSize iconSize(size, size);

	QToolButton *optionsMenuButton = new QToolButton;
	optionsMenuButton->setText(tr("Options "));
	optionsMenuButton->setMenu(CreateOptionsMenu());
	optionsMenuButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
	optionsMenuButton->setPopupMode(QToolButton::InstantPopup);

	QToolButton *zoomOutIcon = new QToolButton;
	zoomOutIcon->setAutoRepeat(true);
	zoomOutIcon->setAutoRepeatInterval(33);
	zoomOutIcon->setAutoRepeatDelay(0);
	zoomOutIcon->setIcon(QPixmap(":/zoomout.png"));
	zoomOutIcon->setIconSize(iconSize);

	QToolButton *zoomInIcon = new QToolButton;
	zoomInIcon->setAutoRepeat(true);
	zoomInIcon->setAutoRepeatInterval(33);
	zoomInIcon->setAutoRepeatDelay(0);
	zoomInIcon->setIcon(QPixmap(":/zoomin.png"));
	zoomInIcon->setIconSize(iconSize);

	zoomSlider = new QSlider(Qt::Horizontal);
	zoomSlider->setMinimum(250);
	zoomSlider->setMaximum(675);
	zoomSlider->setValue(470);
	zoomSlider->setTickPosition(QSlider::TicksBelow);

	// Zoom slider layout
	QHBoxLayout *zoomSliderLayout = new QHBoxLayout;
	zoomSliderLayout->setContentsMargins(0,0,0,0);
	zoomSliderLayout->addWidget(zoomOutIcon);
	zoomSliderLayout->addWidget(zoomSlider);
	zoomSliderLayout->addWidget(zoomInIcon);

	// Zoom slider frame
	QFrame *zoomSliderFrame = new QFrame();
	zoomSliderFrame->setLayout(zoomSliderLayout);

	// Top layout
	QHBoxLayout *topLayout = new QHBoxLayout;
	topLayout->setContentsMargins(4,2,4,2);

	label_show = new QLabel(tr("Show: "));
	showComboBox = new QComboBox();
	showComboBox->setMinimumWidth(100);

	topLayout->addWidget(label_show);
	topLayout->addWidget(showComboBox);
	topLayout->addStretch();
	topLayout->addWidget(optionsMenuButton);
	topLayout->addStretch();
	topLayout->addWidget(zoomSliderFrame);

	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->setContentsMargins(1,1,1,1);
	mainLayout->setVerticalSpacing(0);
	mainLayout->setHorizontalSpacing(0);
	mainLayout->addLayout(topLayout, 0, 0);
	mainLayout->addWidget(graphicsView, 1, 0);
	setLayout(mainLayout);

	connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
	connect(graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)),
					this, SLOT(setResetButtonEnabled()));
	connect(graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)),
					this, SLOT(setResetButtonEnabled()));
	connect(zoomInIcon, SIGNAL(clicked()), this, SLOT(zoomIn()));
	connect(zoomOutIcon, SIGNAL(clicked()), this, SLOT(zoomOut()));
	connect(showComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(ShowDataSpace(const QString &)));

	// Create scene.
	scene = new QGraphicsScene();
	graphicsView->setScene(scene);

	setupMatrix();
}

QGraphicsView *vView::view() const
{
    return static_cast<QGraphicsView *>(graphicsView);
}

QMenu *vView::CreateOptionsMenu()
{
	// Create the options menu
	QMenu *menu = new QMenu("Options");

	viewActivityAct = new QAction(tr("View &Activity (a)"), this);
	menu->addAction(viewActivityAct);
  viewActivityAct->setCheckable(true);
	connect(viewActivityAct, SIGNAL(triggered()), this, SLOT(ViewMode_Activity()));

	viewReconstructionAct = new QAction(tr("View &Top-Down Reconstruction (r)"), this);
	menu->addAction(viewReconstructionAct);
  viewReconstructionAct->setCheckable(true);
	connect(viewReconstructionAct, SIGNAL(triggered()), this, SLOT(ViewMode_Reconstruction()));

	viewPredictionAct = new QAction(tr("View &Top-Down Prediction (p)"), this);
	menu->addAction(viewPredictionAct);
  viewPredictionAct->setCheckable(true);
	connect(viewPredictionAct, SIGNAL(triggered()), this, SLOT(ViewMode_Prediction()));

	menu->addSeparator();

	viewBoostAct = new QAction(tr("View &Boost"), this);
	menu->addAction(viewBoostAct);
  viewBoostAct->setCheckable(true);
	connect(viewBoostAct, SIGNAL(triggered()), this, SLOT(ViewMode_Boost()));

	menu->addSeparator();

	viewConnectionsInAct = new QAction(tr("View &Connections In (i)"), this);
	menu->addAction(viewConnectionsInAct);
  viewConnectionsInAct->setCheckable(true);
	connect(viewConnectionsInAct, SIGNAL(triggered()), this, SLOT(ViewMode_ConnectionsIn()));

	viewConnectionsOutAct = new QAction(tr("View &Connections Out (o)"), this);
	menu->addAction(viewConnectionsOutAct);
  viewConnectionsOutAct->setCheckable(true);
	connect(viewConnectionsOutAct, SIGNAL(triggered()), this, SLOT(ViewMode_ConnectionsOut()));

	viewMarkedCellsAct = new QAction(tr("View &Marked Cells (m)"), this);
	menu->addAction(viewMarkedCellsAct);
  viewMarkedCellsAct->setCheckable(true);
	connect(viewMarkedCellsAct, SIGNAL(triggered()), this, SLOT(ViewMode_MarkedCells()));

	viewActivityAct->setChecked(true);
	viewPredictionAct->setChecked(false);
	viewReconstructionAct->setChecked(false);
	viewBoostAct->setChecked(false);
	viewConnectionsInAct->setChecked(true);
	viewConnectionsOutAct->setChecked(false);
	viewMarkedCellsAct->setChecked(false);

	menu->addSeparator();

	QAction *markActiveCellsAct = new QAction(tr("&Mark Active and Predicted Cells"), this);
	menu->addAction(markActiveCellsAct);
	connect(markActiveCellsAct, SIGNAL(triggered()), this, SLOT(View_MarkActiveCells()));

	QAction *markPredictedCellsAct = new QAction(tr("&Mark Predicted Cells"), this);
	menu->addAction(markPredictedCellsAct);
	connect(markPredictedCellsAct, SIGNAL(triggered()), this, SLOT(View_MarkPredictedCells()));

	QAction *markLearningCellsAct = new QAction(tr("&Mark Learning Cells"), this);
	menu->addAction(markLearningCellsAct);
	connect(markLearningCellsAct, SIGNAL(triggered()), this, SLOT(View_MarkLearningCells()));

	return menu;
}


void vView::keyPressEvent(QKeyEvent* e)
{
	if (e->key() == Qt::Key_A) 
	{
		viewActivityAct->setChecked(viewActivityAct->isChecked() == false);

		if (viewActivityAct->isChecked()) {
			viewReconstructionAct->setChecked(false);
			viewPredictionAct->setChecked(false);
		}

		SetViewMode(viewActivityAct->isChecked(), viewReconstructionAct->isChecked(), viewPredictionAct->isChecked(), viewBoostAct->isChecked(), viewConnectionsInAct->isChecked(), viewConnectionsOutAct->isChecked(), viewMarkedCellsAct->isChecked());
	}
	else if (e->key() == Qt::Key_R) 
	{
		viewReconstructionAct->setChecked(viewReconstructionAct->isChecked() == false);

		if (viewReconstructionAct->isChecked()) {
			viewActivityAct->setChecked(false);
			viewPredictionAct->setChecked(false);
		}

		SetViewMode(viewActivityAct->isChecked(), viewReconstructionAct->isChecked(), viewPredictionAct->isChecked(), viewBoostAct->isChecked(), viewConnectionsInAct->isChecked(), viewConnectionsOutAct->isChecked(), viewMarkedCellsAct->isChecked());
	}
	else if (e->key() == Qt::Key_P) 
	{
		viewPredictionAct->setChecked(viewPredictionAct->isChecked() == false);

		if (viewPredictionAct->isChecked()) {
			viewReconstructionAct->setChecked(false);
			viewActivityAct->setChecked(false);
		}

		SetViewMode(viewActivityAct->isChecked(), viewReconstructionAct->isChecked(), viewPredictionAct->isChecked(), viewBoostAct->isChecked(), viewConnectionsInAct->isChecked(), viewConnectionsOutAct->isChecked(), viewMarkedCellsAct->isChecked());
	}
	else if (e->key() == Qt::Key_I) 
	{
		viewConnectionsInAct->setChecked(viewConnectionsInAct->isChecked() == false);
		
		if (viewConnectionsInAct->isChecked()) 
		{
			viewConnectionsOutAct->setChecked(false);
			viewMarkedCellsAct->setChecked(false);
		}

		SetViewMode(viewActivityAct->isChecked(), viewReconstructionAct->isChecked(), viewPredictionAct->isChecked(), viewBoostAct->isChecked(), viewConnectionsInAct->isChecked(), viewConnectionsOutAct->isChecked(), viewMarkedCellsAct->isChecked());
	}
	//else if (e->key() == Qt::Key_O) 
	//{
	//	viewConnectionsOutAct->setChecked(viewConnectionsOutAct->isChecked() == false);
	//  if (viewConnectionsOutAct->isChecked()) 
	//  {
	//    viewConnectionsInAct->setChecked(false);
	//    viewMarkedCellsAct->setChecked(false);
	// 	}
	//	SetViewMode(viewActivityAct->isChecked(), viewReconstructionAct->isChecked(), viewPredictionAct->isChecked(), viewConnectionsInAct->isChecked(), viewConnectionsOutAct->isChecked(), viewMarkedCellsAct->isChecked());
	//}
	else if (e->key() == Qt::Key_M) 
	{
		viewMarkedCellsAct->setChecked(viewMarkedCellsAct->isChecked() == false);
		
		if (viewMarkedCellsAct->isChecked()) 
		{
			viewConnectionsInAct->setChecked(false);
			viewConnectionsOutAct->setChecked(false);
		}

		SetViewMode(viewActivityAct->isChecked(), viewReconstructionAct->isChecked(), viewPredictionAct->isChecked(), viewBoostAct->isChecked(), viewConnectionsInAct->isChecked(), viewConnectionsOutAct->isChecked(), viewMarkedCellsAct->isChecked());
	}
	else 
	{
		e->ignore();
	}
};


void vView::SetMouseMode(MouseMode _mouseMode)
{
	mouseMode = _mouseMode;

	if (mouseMode == MOUSE_MODE_SELECT)
	{
		graphicsView->setDragMode(QGraphicsView::NoDrag);
	}
	else if (mouseMode == MOUSE_MODE_DRAG)
	{
		graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
	}
}

void vView::SetViewMode(bool _viewActivity, bool _viewReconstruction, bool _viewPrediction, bool _viewBoost, bool _viewConnectionsIn, bool _viewConnectionsOut, bool _viewMarkedCells)
{
	// If changing view mode, redraw this entire View.
	if ((_viewActivity != viewActivity) || (_viewReconstruction != viewReconstruction) || (_viewPrediction != viewPrediction) || (_viewBoost != viewBoost) || (_viewConnectionsIn != viewConnectionsIn) || (_viewConnectionsOut != viewConnectionsOut) || (_viewMarkedCells != viewMarkedCells)) {
		graphicsView->scene()->update(graphicsView->sceneRect());
	}

	// Record new view mode.
	viewActivity = _viewActivity;
	viewReconstruction = _viewReconstruction;
	viewPrediction = _viewPrediction;
	viewBoost = _viewBoost;
	viewConnectionsIn = _viewConnectionsIn;
	viewConnectionsOut = _viewConnectionsOut;
	viewMarkedCells = _viewMarkedCells;

	if (_viewReconstruction || _viewPrediction || _viewBoost)
	{
		// Generate the data image.
		GenerateDataImage();

		// Redraw the entire view.
		graphicsView->scene()->update(graphicsView->sceneRect());
	}

	// Clear all existing records of selected synapses.
	for (std::map<vColumnDisp*,vColumnDisp*>::const_iterator col_iter = cols_with_sel_synapses.begin(), end = cols_with_sel_synapses.end(); col_iter != end; ++col_iter) {
		col_iter->first->ClearSelectedSynapses();
	}

	// Clear the hash table of ColumnDisps with records of selected synapses.
	cols_with_sel_synapses.clear();

	if (_viewConnectionsIn)
	{
		if (dpSelRegion != NULL)
		{
         vColumn selCol(dpSelRegion, selColX, selColY);
         int colX, colY;
         int colIndex = selCol.GetColumnIndex();

			// Display selected proximal synapses.

			// If this View is displaying a DataSpace that is NOT the Region containing the selection...
			if (dpSelRegion != (vRegion*)dpDataSpace)
			{
            // If this region has a proximal segment that attaches to the displayed dataspace, then...
            if (dpSelRegion->GetInputSpace() == dpDataSpace)
            {
               // Walk each proximal synaspe 
               int8_t* pProximalSynaspeArray = selCol.GetProximalSegmentPermanenceArray();
               for (int i = 0; i < dpSelRegion->GetInputSpace()->GetNumCells(); i++)
               {
                  // Record the information for the current synapse in its column's ColumnDisp.
                  columnDisps[i]->SelectSynapse(0 /* This is the cell index into the col, for 2D inputs, this is zero */, pProximalSynaspeArray[i], VHTM_CONNECTED_PERMANENCE);

                  // Record that the current column has one or more selected synapses.
                  cols_with_sel_synapses[columnDisps[colIndex]] = columnDisps[colIndex];
               }
            }
			}

			// Display selected distal synapses.

			// If there is a selected cell and this View is displaying the Region containing the selected cell...
			if ((selCellIndex != -1) && ((vDataSpace*)dpSelRegion == dpDataSpace))
			{
				// Get a pointer to the selected cell.
				vCell selCell = selCol.GetCellByIndex(selCellIndex);

				// Iterate through all of the selected cell's distal segments.
            for (int segIndex = 0; segIndex < selCell.GetNumDistalSegments(); segIndex++)
            {
               // If there is a selected segment and this isn't it, skip this segment.
               if ((selSegmentIndex != -1) && (selSegmentIndex != segIndex)) {
                  continue;
               }

               // Iterate through all synapses on the current distal segment.
               int8_t* pDistalSegmentPermanenceArray = selCell.GetDistalSegmentPermanenceArray(segIndex);

               for (int synIndex = 0; synIndex < selCell.GetGetDistalSegmentReceptiveRange(); synIndex++)
               {
                  // Get information about this distal synapse's input cell.
                  int connectedCellIndex = selCell.GetDistalSynaspeRemoteConnectionIndex(synIndex);

                  // Record the information for the current synapse in its column's ColumnDisp.
                  columnDisps[colIndex]->SelectSynapse(connectedCellIndex, pDistalSegmentPermanenceArray[synIndex], VHTM_CONNECTED_PERMANENCE);

                  // Record that the current column has one or more selected synapses.
                  cols_with_sel_synapses[columnDisps[colIndex]] = columnDisps[colIndex];
               }
            }
			}
		}
	}
}

void vView::MarkCells(MarkType _type)
{
	// Clear the list of marked cells.
	marked_cells.clear();

	if (dpDataSpace == NULL) {
		return;
	}

	int index;
	int numCells = dpDataSpace->GetSizeZ();

   for (int i = 0; i < dpDataSpace->GetNumCells(); i++)
   {
      if (dpDataSpace->GetDataSpaceType() == DATASPACE_TYPE_REGION)
      {
         vCell cell(dpSelRegion, i);
         if (((_type == MARK_ACTIVE) && cell.IsCellActive()) ||
            ((_type == MARK_PREDICTED) && cell.IsCellPredicted()) ||
            ((_type == MARK_LEARNING) && cell.IsCellLearning()))
         {
            marked_cells[index] = 1;
         }
      }
   }

	if (viewMarkedCells)
	{
		// Redraw the entire view.
		graphicsView->scene()->update(graphicsView->sceneRect());
	}
}

void vView::MousePressed(QMouseEvent *event, bool doubleClick)
{
	// Do nothing if there is no Region or InputSpace being shown.
	if (dpDataSpace == NULL) {
		return;
	}

	if ((dpDataSpace->GetDataSpaceType() == DATASPACE_TYPE_INPUTSPACE) || (dpDataSpace->GetDataSpaceType() == DATASPACE_TYPE_REGION))
	{
		QPointF coords = graphicsView->mapToScene(event->pos());

		coords.setX(coords.x() + ((float)sceneWidth) / 2.0f);
		coords.setY(coords.y() + ((float)sceneHeight) / 2.0f);

		// If the mouse was pressed outside of the DataSpace, select nothing.
		if ((dpDataSpace == NULL) || (coords.x() < 0.0f) || (coords.x() > sceneWidth) || (coords.y() < 0.0f) || (coords.y() > sceneHeight))
		{
			win->SetSelected(NULL, NULL, -1, -1, -1, -1);
			return;
		}

		int wholeX = floor(coords.x());
		int wholeY = floor(coords.y());

		float fractionalX = coords.x() - (float)wholeX;
		float fractionalY = coords.y() - (float)wholeY;

		// If the mouse was pressed within the DataSpace but at the edge's of a ColumnDisp, select nothing.
		if ((fractionalX < 0.1f) || (fractionalX > 0.9f) || (fractionalY < 0.1f) || (fractionalY > 0.9f))
		{
			win->SetSelected(NULL, NULL, -1, -1, -1, -1);
			return;
		}

		// Determine which specific cell (if any) is being selected. Select the whole column if doubleClick is true.
		int cellIndex = doubleClick ? -1 : columnDisps[wholeX + (wholeY * sceneWidth)]->DetermineCellIndex(fractionalX, fractionalY);

		// Select the determined cell (if there is one) in the determined column in the DataSpace.
		
      // TODO: NEED TO FIX - JJK
      
      //win->SetSelected((dpDataSpace->GetDataSpaceType() == DATASPACE_TYPE_REGION) ? dpSelRegion : NULL,
		//				     (dpDataSpace->GetDataSpaceType() == DATASPACE_TYPE_INPUTSPACE) ? dpSelInput : NULL,
		//								 wholeX, wholeY, cellIndex, -1);
      SetSelected(dpSelRegion, dpSelInput, wholeX, wholeY, cellIndex, -1);
	}
}

void vView::SetSelected(vRegion *_region, vInputSpace *_input, int _colX, int _colY, int _cellIndex, int _selSegmentIndex)
{
	if (selColIndex != -1) {
		columnDisps[selColIndex]->SetSelection(false, -1);
	}

	// Record selection information.
   dpSelRegion = _region;
   dpSelInput = _input;
   selColX = _colX;
	selColY = _colY;
	selCellIndex = _cellIndex;	
	selSegmentIndex = _selSegmentIndex;

	if ((dpDataSpace == _region) || (dpDataSpace == _input))
	{
		selColIndex = _colX + (_colY * sceneWidth);
		columnDisps[selColIndex]->SetSelection((_cellIndex == -1), _cellIndex);
	}
	else
	{
		selColIndex = -1;
	}

	// Re-apply the current view mode.
	SetViewMode(viewActivity, viewReconstruction, viewPrediction, viewBoost, viewConnectionsIn, viewConnectionsOut, viewMarkedCells);
}

void vView::UpdateForNetwork(vNetworkManager *_networkManager)
{
	// Reset selection information.
	dpSelRegion = NULL;
	dpSelInput = NULL;
	selColX = -1;
	selColY = -1;
	selColIndex = -1;
	selCellIndex = -1;
	selSegmentIndex = -1;

	// Record pointer to NetworkManager.
	networkManager = _networkManager;

	showComboBox->clear();

	// Record that DataSpaces are being added, so they will not be shown immediately.
	addingDataSpaces = true;

	// Add the ID of each InputSpace to the showComboBox.
	for (std::vector<vInputSpace*>::const_iterator input_iter = _networkManager->inputSpaces.begin(), end = _networkManager->inputSpaces.end(); input_iter != end; ++input_iter) {
		showComboBox->addItem((*input_iter)->GetID());
	}

	// Add the ID of each Region to the showComboBox.
	for (std::vector<vRegion*>::const_iterator region_iter = _networkManager->regions.begin(), end = _networkManager->regions.end(); region_iter != end; ++region_iter) {
		showComboBox->addItem((*region_iter)->GetID());
	}

	// Add the ID of each Classifier to the showComboBox.
	/*for (std::vector<Classifier*>::const_iterator classifier_iter = _networkManager->classifiers.begin(), end = _networkManager->classifiers.end(); classifier_iter != end; ++classifier_iter) {
		showComboBox->addItem((*classifier_iter)->GetID());
	}*/

	// Start with no DataSpace selected, so the selection of the any will trigger a signal.
	showComboBox->setCurrentIndex(-1);

	// Done adding DataSpaces.
	addingDataSpaces = false;
}

void vView::UpdateForExecution()
{
	// If viewing data that needs to be re-generated for the executed step, re-apply view mode so as to update that data.
	if (viewConnectionsIn || viewConnectionsOut || viewReconstruction || viewPrediction || viewBoost) {
		SetViewMode(viewActivity, viewReconstruction, viewPrediction, viewBoost, viewConnectionsIn, viewConnectionsOut, viewMarkedCells);
	}

	// Mark the entire scene to be redrawn, to display changes in activity and connection strengths.
	graphicsView->scene()->update(graphicsView->sceneRect());
}

void vView::GenerateDataImage()
{
	//int i, colX, colY, cellIndex;
	float maxImageVal = 0.0f;
	bool projectCol;
	vRegion *curRegion;
	//Column *curCol;
	//Cell *curCell;
	//ProximalSynapse *pSyn;

	if (viewBoost && (dpDataSpace->GetDataSpaceType() == DATASPACE_TYPE_REGION))
	{
      curRegion = dpSelRegion;

		// For each column...
		for ( int colIndex = 0; colIndex < curRegion->GetNumColumns(); colIndex++)
		{

			// Get the current column.
         vColumn curCol(dpSelRegion, colIndex);
				
			// Record the curColumn's Boost value in its corresponding ColumnDisp's imageVal.
			columnDisps[colIndex]->imageVal = curCol.GetBoostValue() - 1.0f;

			// Keep track of the maximum Boost value in the curRegion, for normaliation.
			maxImageVal = MAX(maxImageVal, curCol.GetBoostValue() - 1.0f);
				
			/*
			// Record the curColumn's overlap duty cycle value in its corresponding ColumnDisp's imageVal.
			columnDisps[colX + (colY * sceneWidth)]->imageVal = curCol->GetOverlapDutyCycle();

			// Keep track of the maximum overlap duty cycle value in the curRegion, for normaliation.
			maxImageVal = Max(maxImageVal, curCol->GetOverlapDutyCycle());
			*/
			/*
			// Record the curColumn's overlap duty cycle value in its corresponding ColumnDisp's imageVal.
			curVal = ((networkManager->GetTime() - curCol->prevBoostTime) > 10) ? 0.0f : (10.0f - (float)(networkManager->GetTime() - curCol->prevBoostTime)) / 10.0f;
			columnDisps[colX + (colY * sceneWidth)]->imageVal = curVal;

			// Keep track of the maximum overlap duty cycle value in the curRegion, for normaliation.
			maxImageVal = Max(maxImageVal, curVal);
			*/
			
		}

		if (maxImageVal > 0.0f)
		{
			// Normalize the imageVals of all columns, relative to maxImageVal.
			for (int i = 0; i < sceneWidth * sceneHeight; i++) 
			{
				columnDisps[i]->imageVal = columnDisps[i]->imageVal / maxImageVal;
			}
		}
	}

	if (viewReconstruction || viewPrediction)
	{
		int numCells = dpDataSpace->GetSizeZ();

		// For each column...
		for (int i = 0; i < sceneWidth * sceneHeight; i++) 
		{
			// If this ColumnDisp doesn't yet have an imageVals array of the correct number of dimensions...
			if (columnDisps[i]->imageValCount != numCells)
			{
				// Delete the old imageVals array if it exists.
				if (columnDisps[i]->imageVals != NULL) {
					delete [] columnDisps[i]->imageVals;
				}

				// Create the corresponding ColumnDisp's imageVals array of the correct number of dimensions. 
				columnDisps[i]->imageVals = new float[numCells];
			}
		
			// Reset each cell's imageVal to 0.
			for (int cellIndex = 0; cellIndex < numCells; cellIndex++) {
				columnDisps[i]->imageVals[cellIndex] = 0.0f;
			}
		}

		// Loop through each Region in the network...
		for (std::vector<vRegion*>::const_iterator region_iter = networkManager->regions.begin(), end = networkManager->regions.end(); region_iter != end; ++region_iter) 
		{
			// Loop through each input DataSpace to the current Region...TODO: ONLY SUPPORTING ONE DATASPACE PER REGION??
         vDataSpace* input_iter = (*region_iter)->GetInputSpace();

			//for (std::vector<vDataSpace*>::const_iterator input_iter = (*region_iter)->InputList.begin(), end = (*region_iter)->InputList.end(); input_iter != end; ++input_iter) 
			//{
				// If the current input DataSpace is the DataSpace being displayed by this View...
				if (input_iter == dpDataSpace)
				{
					curRegion = (*region_iter);

               for (int colIndex = 0; colIndex < curRegion->GetNumColumns(); colIndex++)
               {

                  // Get the current column.
                  vColumn curCol(dpSelRegion, colIndex);

						projectCol = false;

						if (viewReconstruction)
						{
							// Project this column if it is active.
							projectCol = curCol.GetIsActive();
						}
						else if (viewPrediction)
						{
                     // Set projectCol to true if any cell in this column is predicting for the next time step (a sequence prediction).
                     projectCol = curCol.GetIsAnyCellPredicting(1);
						}

						if (projectCol)
						{


                     // If this region has a proximal segment that attaches to the displayed dataspace, then...
                     if (dpSelRegion->GetInputSpace() == dpDataSpace)
                     {
                        // Walk each proximal synaspe 
                        int8_t* pProximalSynaspeArray = curCol.GetProximalSegmentPermanenceArray();
                        for (int i = 0; i < dpSelRegion->GetInputSpace()->GetNumCells(); i++)
                        {
                           // Add the current synapse's permanence to the imageVal corresponding to the cell in the DataSpace being
                           // displayed by this view, that the Synapse connects from.
                           columnDisps[i]->imageVals[colIndex] += pProximalSynaspeArray[i];

                           // Record the maximum imageVal among all cells, to use later for normalization.
                           maxImageVal = MAX(maxImageVal, columnDisps[i]->imageVals[colIndex]);
                        }
                     }
						}
					}

					// Exit input DataSpace iteration loop.
					break;
				}
			//}
		}

		// Normalize all imageVals...
		if (maxImageVal > 0.0f)
		{
			// For each column...
			for (int i = 0; i < sceneWidth * sceneHeight; i++) 
			{
				// Normalize each cell's imageVal, proportional to maxImageVal.
				for (int cellIndex = 0; cellIndex < numCells; cellIndex++)
				{
					columnDisps[i]->imageVals[cellIndex] = columnDisps[i]->imageVals[cellIndex] / maxImageVal;

					// Square the imageVal to empahasize the higher values.
					columnDisps[i]->imageVals[cellIndex] = columnDisps[i]->imageVals[cellIndex] * columnDisps[i]->imageVals[cellIndex];
				}
			}
		}
	}
}

void vView::setupMatrix()
{
    qreal scale = qPow(qreal(2), (zoomSlider->value() - 250) / qreal(50));

    QMatrix matrix;
    matrix.scale(scale, scale);
    //matrix.rotate(rotateSlider->value());

    graphicsView->setMatrix(matrix);
}


void vView::ViewMode_Activity()
{
	if (viewReconstructionAct->isChecked()) {
		viewReconstructionAct->setChecked(false);
	}

	if (viewPredictionAct->isChecked()) {
		viewPredictionAct->setChecked(false);
	}

	SetViewMode(viewActivityAct->isChecked(), viewReconstructionAct->isChecked(), viewPredictionAct->isChecked(), viewBoostAct->isChecked(), viewConnectionsInAct->isChecked(), viewConnectionsOutAct->isChecked(), viewMarkedCellsAct->isChecked());
}

void vView::ViewMode_Reconstruction()
{
	if (viewActivityAct->isChecked()) {
		viewActivityAct->setChecked(false);
	}

	if (viewPredictionAct->isChecked()) {
		viewPredictionAct->setChecked(false);
	}

	SetViewMode(viewActivityAct->isChecked(), viewReconstructionAct->isChecked(), viewPredictionAct->isChecked(), viewBoostAct->isChecked(), viewConnectionsInAct->isChecked(), viewConnectionsOutAct->isChecked(), viewMarkedCellsAct->isChecked());
}

void vView::ViewMode_Prediction()
{
	if (viewReconstructionAct->isChecked()) {
		viewReconstructionAct->setChecked(false);
	}

	if (viewActivityAct->isChecked()) {
		viewActivityAct->setChecked(false);
	}

	SetViewMode(viewActivityAct->isChecked(), viewReconstructionAct->isChecked(), viewPredictionAct->isChecked(), viewBoostAct->isChecked(), viewConnectionsInAct->isChecked(), viewConnectionsOutAct->isChecked(), viewMarkedCellsAct->isChecked());
}

void vView::ViewMode_Boost()
{
	SetViewMode(viewActivityAct->isChecked(), viewReconstructionAct->isChecked(), viewPredictionAct->isChecked(), viewBoostAct->isChecked(), viewConnectionsInAct->isChecked(), viewConnectionsOutAct->isChecked(), viewMarkedCellsAct->isChecked());
}

void vView::ViewMode_ConnectionsIn()
{
	if (viewConnectionsInAct->isChecked()) 
	{
		viewConnectionsOutAct->setChecked(false);
		viewMarkedCellsAct->setChecked(false);
	}

	SetViewMode(viewActivityAct->isChecked(), viewReconstructionAct->isChecked(), viewPredictionAct->isChecked(), viewBoostAct->isChecked(), viewConnectionsInAct->isChecked(), viewConnectionsOutAct->isChecked(), viewMarkedCellsAct->isChecked());
}

void vView::ViewMode_ConnectionsOut()
{
	if (viewConnectionsOutAct->isChecked()) 
	{
		viewConnectionsInAct->setChecked(false);
		viewMarkedCellsAct->setChecked(false);
	}

	SetViewMode(viewActivityAct->isChecked(), viewReconstructionAct->isChecked(), viewPredictionAct->isChecked(), viewBoostAct->isChecked(), viewConnectionsInAct->isChecked(), viewConnectionsOutAct->isChecked(), viewMarkedCellsAct->isChecked());
}

void vView::ViewMode_MarkedCells()
{
	if (viewMarkedCellsAct->isChecked()) 
	{
		viewConnectionsInAct->setChecked(false);
		viewConnectionsOutAct->setChecked(false);
	}

	SetViewMode(viewActivityAct->isChecked(), viewReconstructionAct->isChecked(), viewPredictionAct->isChecked(), viewBoostAct->isChecked(), viewConnectionsInAct->isChecked(), viewConnectionsOutAct->isChecked(), viewMarkedCellsAct->isChecked());
}

void vView::View_MarkActiveCells()
{
	MarkCells(MARK_ACTIVE);
}

void vView::View_MarkPredictedCells()
{
	MarkCells(MARK_PREDICTED);
}

void vView::View_MarkLearningCells()
{
	MarkCells(MARK_LEARNING);
}

void vView::zoomIn(int level)
{
    zoomSlider->setValue(zoomSlider->value() + level);
}

void vView::zoomOut(int level)
{
    zoomSlider->setValue(zoomSlider->value() - level);
}

void vView::ShowDataSpace(const QString &_id)
{
	int x, y;
	vColumnDisp *curColumnDisp;

	// Ignore if currently in the process of adding DataSpaces.
	if (addingDataSpaces) {
		return;
	}

	// Get pointer to DataSpace with given ID.
	dpDataSpace = networkManager->GetDataSpace(_id);

	// Clear the scene.
	scene->clear();

	// Delete array of ColumnDisps.
	if (columnDisps != NULL)
	{
		delete [] columnDisps;
		columnDisps = NULL;
	}

	// Clear data pertaining to previously displayed DataSpace.
	selColIndex = -1;
	cols_with_sel_synapses.clear();

	// If there is no DataSpace, do nothing more.
	if (dpDataSpace == NULL) {
		return;
	}

	if ((dpDataSpace->GetDataSpaceType() == DATASPACE_TYPE_INPUTSPACE) || (dpDataSpace->GetDataSpaceType() == DATASPACE_TYPE_REGION))
	{
		// Record dimensions of new scene
		sceneWidth = dpDataSpace->GetSizeX();
		sceneHeight = dpDataSpace->GetSizeY();

		float xOrigin = 0 - ((float)sceneWidth / 2.0f);
		float yOrigin = 0 - ((float)sceneHeight / 2.0f);

		// Fill the scene with ColumnDisp objects representing each column in the dataSpace.
		columnDisps = new vColumnDisp*[sceneWidth * sceneHeight];
		for (x = 0; x < sceneWidth; x++)
		{
			for (y = 0; y < sceneHeight; y++)
			{
				columnDisps[(y * sceneWidth) + x] = curColumnDisp = new vColumnDisp(this, dpDataSpace, x, y);
				curColumnDisp->setPos(QPointF(x + xOrigin, y + yOrigin));
				scene->addItem(curColumnDisp);
			}
		}

      /*
		// Determine the dataSpace's hypercolumn diameter.
		int hypercolumnDiameter = dpDataSpace->GetHypercolumnDiameter();

		// If HypercolumnDiameter > 1, display hypercolumn divisions.
		if (hypercolumnDiameter > 1)
		{
			QPen boundaryPen(QBrush(QColor(160,160,255)), 0.04);

			for (x = 0; x <= sceneWidth; x += hypercolumnDiameter) {
				scene->addLine(x + xOrigin, 0 + yOrigin, x + xOrigin, sceneHeight + yOrigin, boundaryPen);
			}

			for (y = 0; y <= sceneHeight; y += hypercolumnDiameter) {
				scene->addLine(0 + xOrigin, y + yOrigin, sceneWidth + xOrigin, y + yOrigin, boundaryPen);
			}
		}*/
	}
	else if (dpDataSpace->GetDataSpaceType() == DATASPACE_TYPE_CLASSIFIER)
	{
	}

	// Re-apply selection information.
	SetSelected(dpSelRegion, dpSelInput, selColX, selColY, selCellIndex, selSegmentIndex);

	// Clear the list of marked cells.
	marked_cells.clear();
}

