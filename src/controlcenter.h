#ifndef CONTROLCENTER_H
#define CONTROLCENTER_H

#include <QtWidgets/QMainWindow>
#include "ui_controlcenter.h"

class ControlCenter : public QMainWindow
{
	Q_OBJECT

public:
	ControlCenter(QWidget *parent = 0);
	~ControlCenter();

private:
	Ui::ControlCenterClass ui;
};

#endif // CONTROLCENTER_H
