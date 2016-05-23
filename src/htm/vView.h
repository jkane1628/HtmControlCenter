#pragma once
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsView>
#include <map>

class QSlider;
class QToolButton;
class QLabel;
class QComboBox;
class QMenu;
class vDataSpace;
class vView;
class vColumnDisp;
class vRegion;
class vInputSpace;
class vNetworkManager;
class ControlWidget;

enum MouseMode {
	MOUSE_MODE_SELECT = 0,
	MOUSE_MODE_DRAG = 1
};

enum MarkType {
	MARK_ACTIVE,
	MARK_PREDICTED,
	MARK_LEARNING
};

class vGraphicsView : public QGraphicsView
{
	Q_OBJECT
public:
    vGraphicsView(vView *v) : QGraphicsView(), view(v) { }

protected:
    void wheelEvent(QWheelEvent *);
		void mousePressEvent(QMouseEvent *event);
		void mouseDoubleClickEvent(QMouseEvent *event);

private:
    vView *view;
};

class vView : public QFrame
{
	Q_OBJECT
public:
   explicit vView(ControlWidget *_win, QWidget *parent = 0);

    QGraphicsView *view() const;

public slots:
    void zoomIn(int level = 1);
    void zoomOut(int level = 1);
		void ShowDataSpace(const QString &_id);

private slots:
    void setupMatrix();

		void ViewMode_Activity();
		void ViewMode_Reconstruction();
		void ViewMode_Prediction();
		void ViewMode_Boost();
		void ViewMode_ConnectionsIn();
		void ViewMode_ConnectionsOut();
		void ViewMode_MarkedCells();

		void View_MarkActiveCells();
		void View_MarkPredictedCells();
		void View_MarkLearningCells();

private:

	QMenu *CreateOptionsMenu();

public:
	void keyPressEvent(QKeyEvent* e);

	void SetMouseMode(MouseMode _mouseMode);
	MouseMode GetMouseMode() {return mouseMode;}

	void SetViewMode(bool _viewActivity, bool _viewReconstruction, bool _viewPrediction, bool _viewBoost, bool _viewConnectionsIn, bool _viewConnectionsOut, bool _viewMarkedCells);
	bool GetViewActivity() {return viewActivity;}
	bool GetViewReconstruction() {return viewReconstruction;}
	bool GetViewBoost() {return viewBoost;}
	bool GetViewPrediction() {return viewPrediction;}
	bool GetViewConnectionsIn() {return viewConnectionsIn;}
	bool GetViewConnectionsOut() {return viewConnectionsOut;}
	bool GetViewMarkedCells() {return viewMarkedCells;}

	void MarkCells(MarkType _type);

	void MousePressed(QMouseEvent *event, bool doubleClick);
	void SetSelected(vRegion *_region, vInputSpace *_input, int _colX, int _colY, int _cellIndex, int _selSegmentIndex);

	void UpdateForNetwork(vNetworkManager *_networkManager);
	void UpdateForExecution();

	void GenerateDataImage();
	
	vGraphicsView *graphicsView;
	QLabel *label_show;
	QComboBox *showComboBox;
	QToolButton *optionsMenuButton;
	QAction *viewActivityAct, *viewReconstructionAct, *viewPredictionAct, *viewBoostAct, *viewConnectionsInAct, *viewConnectionsOutAct, *viewMarkedCellsAct, *viewDuringRunAct;
	QSlider *zoomSlider;

   ControlWidget *win;
	vNetworkManager *networkManager;
	QGraphicsScene *scene;
	int sceneWidth, sceneHeight;
	vColumnDisp **columnDisps;
	bool addingDataSpaces;
	MouseMode mouseMode;
	bool viewActivity, viewPrediction, viewReconstruction, viewBoost, viewConnectionsIn, viewConnectionsOut, viewMarkedCells;

   vDataSpace*    dpDataSpace;
	vRegion*       dpSelRegion;
	vInputSpace*   dpSelInput;
	int selColIndex, selColX, selColY, selCellIndex, selSegmentIndex;

	std::map<vColumnDisp*,vColumnDisp*> cols_with_sel_synapses;
	std::map<int, int> marked_cells;
};

