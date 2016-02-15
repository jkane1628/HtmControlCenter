#ifndef MOUSECONTROLS_H
#define MOUSECONTROLS_H

#include <QtWidgets/QMainWindow>
#include "ui_MouseControls.h"

class MouseControls : public QDockWidget
{
	Q_OBJECT

public:
	MouseControls(QWidget *parent = 0);
	~MouseControls();

private:
	Ui::MouseControlsDock ui;
};

#endif // MOUSECONTROLS_H
