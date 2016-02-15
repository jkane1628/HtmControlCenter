#ifndef CONSOLEDOCK_H
#define CONSOLEDOCK_H

#include <QtWidgets/QMainWindow>
#include "ui_Console.h"

#define CJCONSOLE_INPUT_BUFFER_SIZE 1024

class CJCli;
class QConsoleWidget;

class ConsoleDock : public QDockWidget
{
	Q_OBJECT

public:
   ConsoleDock(QWidget *parent);
    ~ConsoleDock();

    CJCli* GetCliObject();
protected:
   void dropEvent(QDropEvent *ev);
   void dragEnterEvent(QDragEnterEvent *ev);

private:
    Ui::ConsoleDock ui;
    QConsoleWidget* dpConsoleWidget;
};


#include <QTextEdit>
#include <QDir>



class CJSem;
class CJTerminalConsole;


class QConsoleWidget : public QTextEdit
{
    Q_OBJECT
public:
   QConsoleWidget(QWidget *parent);
    ~QConsoleWidget();

    CJSem* mpCJSemInputBufferNotEmpty;
    CJSem* mpCJSemInputBufferLock;
    int  mCJConsoleInputBuffer[CJCONSOLE_INPUT_BUFFER_SIZE];
    int  mCJConsoleInputBufferCount;

    void StartCLI();

    CJCli* GetCliObject() { return dpCli;}

    void dropEvent(QDropEvent *ev);
    void dragEnterEvent(QDragEnterEvent *ev);

    void ExternalThreadInsertString(QString outputString);

 signals:
    void ExternalThreadInsertStringSignal(QString outputString);

private:
   int fixedPosition;
	CJTerminalConsole* dpConsole;
   CJCli*             dpCli;

   QMutex dExternalThreadStringCountMutex;
   int dExternalThreadStringCount;

protected:
    void keyPressEvent(QKeyEvent * event);
    void keyProcess(int key, char input_char);
    
public slots:
    void OnChildStarted();
    void OnChildStdOutWrite(QString szOutput);
    void OnChildStdErrWrite(QString szOutput);
    void OnChildTerminate();

    void cursorPositionChanged();


};


#endif // CONSOLEDOCK_H
