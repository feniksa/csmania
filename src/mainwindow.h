#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QWebView;
class QTimer;
class QAction;
class QMenu;
class QToolBar;
class QVBoxLayout;
class QProgressBar;
class QWebFrame;

namespace tabor
{

class MainWindow : public QMainWindow
{
	Q_OBJECT

    enum state_t {
        STATE_LOGIN,
        STATE_POST_LOGIN,
		STATE_POST_SEARCH,
		STATE_LOAD_NEXT_SEARCH_PAGE,
        STATE_LOAD_SEARCH_PAGE,
		STATE_SEND_MESSAGE,
		STATE_LOAD_USER_PROFILE,
        STATE_NONE
    };
public:
    explicit MainWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void loadFinished(bool ok);
    void aboutSlot();
    void settingsSlot();
    void startStopSpaming();
	void timerTimeoutSlot();

private:
	void processNextState(state_t st);
    bool userReallyWantsToQuit();
    void setIsSpaming(bool);

    void createActions();
    void createMenus();
    void createToolBar();

    void saveSettings();
    void loadSettings();

	void storeProcessedUserIds();
	void loadProcessedUserIds();

	QWebView *view;
	
	QTimer* timer;
	unsigned int messageTimerDelayMSec;
		
    state_t state;
	QString nextUrl;
	QList<qlonglong> userPageIds;

    bool isLogedIn;
    bool isSpaming;

    QString userName;
    QString userPassword;
    QString message;

	QList<qlonglong> processedUserIds;

    QAction* exitAction;
    QAction* settingsAction;
    QAction* aboutAction;
    QAction* startStopAction;

    QMenu* fileMenu;
    QMenu* settingsMenu;
    QMenu* aboutMenu;

    QToolBar* startStopToolbar;
    QVBoxLayout* mainLayout;
    QProgressBar* progressBar;


};

}

#endif
