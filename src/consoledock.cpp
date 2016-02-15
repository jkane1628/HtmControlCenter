
#include "CJSem.h"
#include "CJConsole.h"
#include "CJCli.h"

#include <QScrollbar>
#include <QUrl>
#include <QDropEvent>
#include <QMimeData>
#include <QKeyEvent>
#include <QMutex>

#include "consoledock.h"


ConsoleDock::ConsoleDock(QWidget *parent)
	: QDockWidget(parent)
{
   ui.setupUi(this);
   
   dpConsoleWidget = new QConsoleWidget(ui.ConsoleCLI);
   dpConsoleWidget->setObjectName(QStringLiteral("consoleTextEdit"));
   setWidget(dpConsoleWidget);
   dpConsoleWidget->setReadOnly(TRUE);
   dpConsoleWidget->StartCLI();
}

ConsoleDock::~ConsoleDock()
{
   delete dpConsoleWidget;
}

CJCli* ConsoleDock::GetCliObject() { return dpConsoleWidget->GetCliObject(); }
void ConsoleDock::dropEvent(QDropEvent *ev) { dpConsoleWidget->dropEvent(ev); }
void ConsoleDock::dragEnterEvent(QDragEnterEvent *ev) { dpConsoleWidget->dragEnterEvent(ev); }


QConsoleWidget::QConsoleWidget(QWidget *parent) : QTextEdit(parent)
{
    setAcceptDrops(true);
    setUndoRedoEnabled(false);
    fixedPosition = 0;

    connect(this, SIGNAL(ExternalThreadInsertStringSignal(QString)), this, SLOT(OnChildStdOutWrite(QString)));

    dExternalThreadStringCount = 0;

    // Create the semaphores for the input buffer
    mpCJSemInputBufferNotEmpty = new CJSem();
    mpCJSemInputBufferLock = new CJSem(1);
    mCJConsoleInputBufferCount = 0;

    dpConsole = new CJTerminalConsole(this, "QTConsole");
    dpCli = new CJCli("QTCLI", dpConsole);

    setFont(QFont("Courier", 9));
}

QConsoleWidget::~QConsoleWidget()
{
   delete mpCJSemInputBufferNotEmpty;
   delete mpCJSemInputBufferLock;
   delete dpCli;
   delete dpConsole;
}


void QConsoleWidget::dropEvent(QDropEvent *ev)
{
   QString filepath = ev->mimeData()->text();

   mpCJSemInputBufferLock->Wait();
   for (int i = 0; i < filepath.length(); i++)
   {      
      if (mCJConsoleInputBufferCount >= CJCONSOLE_INPUT_BUFFER_SIZE)
      {
         insertPlainText(" INPUT BUF FULL ");
      }
      else
      {
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = filepath.at(i).toLatin1();
         mCJConsoleInputBufferCount++;
      }
   } 
   mpCJSemInputBufferLock->Post();
   if (mCJConsoleInputBufferCount > 0)
      mpCJSemInputBufferNotEmpty->Post();

}

void QConsoleWidget::dragEnterEvent(QDragEnterEvent *ev)
{
   ev->accept();
}

void QConsoleWidget::StartCLI()
{
   dpCli->StartCliSession();
}

void QConsoleWidget::OnChildStarted()
{
}

void QConsoleWidget::ExternalThreadInsertString(QString outputString)
{
   int SleepTrigger = FALSE;


   dExternalThreadStringCountMutex.lock();

   if (dExternalThreadStringCount > 2000)
      SleepTrigger = TRUE;

   while (SleepTrigger == TRUE)
   {
      dExternalThreadStringCountMutex.unlock();
      Sleep(1000);
      dExternalThreadStringCountMutex.lock();
      if (dExternalThreadStringCount == 0)
         SleepTrigger = FALSE;
   }
   dExternalThreadStringCount++;
   dExternalThreadStringCountMutex.unlock();

   

  /* if (StringCountSnap > 2000)
      SleepTrigger = TRUE;
   while (SleepTrigger == TRUE)
   {
      dExternalThreadStringCountMutex.lock();
      StringCountSnap = dExternalThreadStringCount;
      dExternalThreadStringCountMutex.unlock();
      if (StringCountSnap == 0)
         SleepTrigger = FALSE;
      Sleep(100);
   }*/

   ExternalThreadInsertStringSignal(outputString);
}

void QConsoleWidget::OnChildStdOutWrite(QString szOutput)
{
   if (szOutput[0] == QChar(0x08))
   {
      textCursor().deletePreviousChar();
   }
   else
   {
      dExternalThreadStringCountMutex.lock();
      dExternalThreadStringCount--;
      dExternalThreadStringCountMutex.unlock();
      insertPlainText(szOutput);
   }
   this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
}

void QConsoleWidget::OnChildStdErrWrite(QString szOutput)
{
    append(szOutput);
    fixedPosition = textCursor().position();
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
}

void QConsoleWidget::OnChildTerminate()
{
    //exit(1);
}

void QConsoleWidget::keyPressEvent(QKeyEvent *event)
{
   int key = event->key();
   QChar input = event->text()[0];
   char input_char = input.cell();

   if (key == Qt::Key_Shift || key == Qt::Key_Control)
   {
      // Do nothing
      return;
   }
   keyProcess(key, input_char);
}

void QConsoleWidget::keyProcess(int key, char input_char)
{
   mpCJSemInputBufferLock->Wait();
   if (mCJConsoleInputBufferCount >= CJCONSOLE_INPUT_BUFFER_SIZE)
   {
      insertPlainText(" INPUT BUF FULL ");

      // Just drop the char if the input buffer is full
      mpCJSemInputBufferLock->Post();
      return;
   }

   if (key >= 32 && key < 126)
   {
      mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = input_char;
   }
   else if (key == Qt::Key_Return || key == Qt::Key_Enter)
   {
      mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = 10;
      this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
   }
   else if (key == Qt::Key_Backspace)
   {
      mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = 0x7F;
   }
   else
   {
      mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = CLI_SPECIAL_KEY_IDENTIFIER;
      mCJConsoleInputBufferCount++;

      switch (key)
      {
      case Qt::Key_Escape:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_ESC;
         break;
      case Qt::Key_Insert:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_INSERT;
         break;
      case Qt::Key_Delete:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_DELETE;
         break;
      case Qt::Key_Home:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_HOME;
         break;
      case Qt::Key_End:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_END;
         break;
      case Qt::Key_Left:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_LEFT_ARROW;
         break;
      case Qt::Key_Up:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_UP_ARROW;
         break;
      case Qt::Key_Right:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_RIGHT_ARROW;
         break;
      case Qt::Key_Down:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_DOWN_ARROW;
         break;
      case Qt::Key_PageUp:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_PAGE_UP;
         break;
      case Qt::Key_PageDown:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_PAGE_DOWN;
         break;
      case Qt::Key_F1:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_F1;
         break;
      case Qt::Key_F2:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_F2;
         break;
      case Qt::Key_F3:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_F3;
         break;
      case Qt::Key_F4:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_F4;
         break;
      case Qt::Key_F5:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_F5;
         break;
      case Qt::Key_F6:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_F6;
         break;
      case Qt::Key_F7:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_F7;
         break;
      case Qt::Key_F8:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_F8;
         break;
      case Qt::Key_F9:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_F9;
         break;
      case Qt::Key_F10:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_F10;
         break;
      case Qt::Key_F11:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_F11;
         break;
      case Qt::Key_F12:
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_F12;
         break;
      case Qt::Key_Pause:
      case Qt::Key_Print:
      case Qt::Key_SysReq:
      case Qt::Key_Clear:
      case Qt::Key_Meta:
      case Qt::Key_Alt:
      case Qt::Key_CapsLock:
      case Qt::Key_NumLock:
      case Qt::Key_ScrollLock:
      case Qt::Key_Tab:
      case Qt::Key_Backtab:
      
      default: 
         mCJConsoleInputBuffer[mCJConsoleInputBufferCount] = eCLI_KEY_UNKNOWN;
         break;
      }
      
   }
   mCJConsoleInputBufferCount++;

   if (mCJConsoleInputBufferCount > 0)
      mpCJSemInputBufferNotEmpty->Post();

   // Unlock Input Buffer
   mpCJSemInputBufferLock->Post();
}

void QConsoleWidget::cursorPositionChanged()
{
    if (textCursor().position() < fixedPosition) {
        textCursor().setPosition(fixedPosition);
    }
}
