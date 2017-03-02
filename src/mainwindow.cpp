#include "mainwindow.h"
#include "networkmanager.h"
#include "utils.h"

#include "settings.h"

#include <QPushButton>
#include <QDebug>
#include <QWebView>
#include <QWebFrame>
#include <QWebElement>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStyle>
#include <QSettings>
#include <QCloseEvent>
#include <QToolBar>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QDesktopServices>

namespace tabor
{
static const quint32 MimeTypeId = 0x33445511;
static const QString FileStoragePath = "/mamba.dta";
static const QString RemoteServiceName = "http://wap.mamba.ru";
static const unsigned short int maxUsersPerDay = 45;

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags):QMainWindow(parent, flags)
{
    QWidget* mainWidget = new QWidget();

    state = STATE_LOGIN;

	messageTimerDelayMSec = 2000;
    isLogedIn = false;
    isSpaming = false;

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), SLOT(timerTimeoutSlot()));
	timer->setInterval(messageTimerDelayMSec);
	timer->setSingleShot(true);

    progressBar = new QProgressBar;

    view = new QWebView;
    connect(view, SIGNAL(loadFinished(bool)), SLOT(loadFinished(bool)));
    connect(view, SIGNAL(loadProgress(int)), progressBar, SLOT(setValue(int)));

    view->load(QUrl(RemoteServiceName + "/?area=Login"));

    createActions();
    createMenus();
    createToolBar();

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(view);
    mainLayout->addWidget(progressBar);
    mainWidget->setLayout(mainLayout);

    setCentralWidget(mainWidget);

    setWindowTitle(tr("Mamba client"));

    loadSettings();
}

void MainWindow::timerTimeoutSlot()
{
	timer->stop();
	loadFinished(true);
}

void MainWindow::processNextState(state_t st)
{
	state = st;
	timer->setInterval(messageTimerDelayMSec);
	timer->setSingleShot(true);
	timer->start();
}



void MainWindow::loadFinished(bool ok)
{
    if (!ok) {
        qWarning() << "Processing request is not ok";
        return;
    }

    switch (state)
    {
    case STATE_LOGIN:
    {
        QWebFrame* frame = view->page()->mainFrame();
        QWebElement firstName = frame->findFirstElement("[name='login']");
        QWebElement password = frame->findFirstElement("[name='password']");
		QWebElement remember = frame->findFirstElement("[name='remember']");

        if (firstName.isNull() || password.isNull())
            return;

        firstName.setAttribute("value", userName);
        password.setAttribute("value", userPassword);
		remember.setAttribute("checked", "checked");

        state = STATE_POST_LOGIN;
    }
    break;

    case STATE_POST_LOGIN:
        // TODO: check if login success

        isLogedIn = true;
		statusBar()->showMessage(tr("Create search parameters"));
        state = STATE_NONE;
        break;

    case STATE_LOAD_SEARCH_PAGE:
	{
        QWebFrame *frame = view->page()->currentFrame();
		statusBar()->showMessage(tr("Process search page %0").arg(frame->url().toString()));

        QWebElementCollection links = frame->findAllElements("strong > a");
		QRegExp regexp("userId=([0-9]+)");
		
        foreach (QWebElement link, links) {
            QString lnk = link.attribute("href");
            if (!lnk.isEmpty()) {				

				if (regexp.indexIn(lnk) != -1) {
					qlonglong uuid = regexp.cap(1).toLongLong();
					if (uuid) {
						if (!processedUserIds.contains(uuid)) {
							userPageIds.push_back(uuid);
							statusBar()->showMessage(tr("Add for processing user uuid %0").arg(uuid), 1000);
							if (userPageIds.size() >= maxUsersPerDay) {
								nextUrl.clear();
								processNextState(STATE_LOAD_USER_PROFILE);
								statusBar()->showMessage(tr("Users limit, start sending message"), 1000);
								return;
							}
							
						} else {
							statusBar()->showMessage(tr("Ignore already sended uuid %0").arg(uuid), 1000);
						}
					}
				}               
            }
        }

        nextUrl = frame->findFirstElement("a[class=\"new-pager-next\"]").attribute("href");
		if (!nextUrl.isEmpty()) {
			QRegExp regexp("offset=([0-9]+)");

			if (regexp.indexIn(nextUrl) == -1) {
				nextUrl.clear();
			} else {
				nextUrl = RemoteServiceName + "/" + nextUrl;
				processNextState(STATE_LOAD_NEXT_SEARCH_PAGE);
				statusBar()->showMessage(tr("Start process next search page"), 1000);
				return;
			}
		}
		processNextState(STATE_LOAD_USER_PROFILE);
		statusBar()->showMessage(tr("All search pages processed, start sending message"), 1000);		
	}
    break;

	case STATE_LOAD_NEXT_SEARCH_PAGE: {
		state = STATE_LOAD_SEARCH_PAGE;
		view->load(nextUrl);
		qDebug() << nextUrl;
		statusBar()->showMessage(tr("Load next search page %0").arg(nextUrl), 1000);
	}
    break;

    case STATE_LOAD_USER_PROFILE:
        if (!userPageIds.empty()) {
			QString url = RemoteServiceName + "/?area=Contact&action=chat&userId=" + QString::number(userPageIds.first());
            view->load(url);			
            state = STATE_SEND_MESSAGE;
			statusBar()->showMessage(tr("Load user profile %0").arg(url), 1000);
        } else {
			state = STATE_NONE;
			setIsSpaming(false);
			statusBar()->showMessage(tr("All user profiles processed"), 1000);
        }
        break;

    case STATE_SEND_MESSAGE:
    {
        QWebFrame *frame = view->page()->currentFrame();
        QWebElement textArea = frame->findFirstElement("textarea[name=\"message\"]");
        if (!textArea.isNull()) {
			textArea.appendInside(message);

			QWebElement button = frame->findFirstElement("input[type=\"submit\"]");
			if (!button.isNull()) {
				button.evaluateJavaScript("this.click()");			
				statusBar()->showMessage(tr("Send message to user"), 1000);				
			}
        } else {
			statusBar()->showMessage(tr("Ignore sending to user"), 1000);
			// looks like page doesn't contain message area, process next user
		}
		processedUserIds.push_back(userPageIds.first());
		userPageIds.pop_front();

		// call timer, because javascript need some time to execute inside frame
		processNextState(STATE_LOAD_USER_PROFILE);
    }
    break;

	case STATE_NONE:
		if (view->page()->currentFrame()->url().toString().contains("SearchResult")) {
			startStopAction->setEnabled(true);
		} else {
			if (startStopAction->isEnabled())
				startStopAction->setEnabled(false);
		}
		break;

    default:
        break;
    }
}

void MainWindow::createActions()
{
    exitAction = new QAction(tr("&Quit"), this);
    exitAction->setStatusTip(tr("Exit from this program"));
    exitAction->setShortcut(QKeySequence::Quit);
    exitAction->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    connect(exitAction, SIGNAL(triggered()), SLOT(close()));

    settingsAction = new QAction(tr("&Settings"), this);
    settingsAction->setStatusTip(tr("Settings for program"));
    connect(settingsAction, SIGNAL(triggered()), SLOT(settingsSlot()));

    startStopAction = new QAction(tr("&Run"), this);
    startStopAction->setDisabled(true);
    connect(startStopAction, SIGNAL(triggered()), SLOT(startStopSpaming()));
    setIsSpaming(false);

    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip(tr("Show information about this program"));
    aboutAction->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
    connect(aboutAction, SIGNAL(triggered()), SLOT(aboutSlot()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(startStopAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    settingsMenu = menuBar()->addMenu(tr("&Preferences"));
    settingsMenu->addAction(settingsAction);

    aboutMenu = menuBar()->addMenu(tr("&Help"));
    aboutMenu->addAction(aboutAction);
}

void MainWindow::createToolBar()
{
    startStopToolbar = addToolBar(tr("Actions"));
    startStopToolbar->addAction(startStopAction);
}

void MainWindow::aboutSlot()
{
    QMessageBox::information(this, tr("About"), tr("Client for mamba.ru services. <br><br>All rights belong to Feniks Gordon Freman (feniksa@rambler.ru)"
                             "<br><a href=\"http://200volts.com/software/jump4love\">http://200volts.com/software/mamba</a>"));
}

void MainWindow::settingsSlot()
{
    settings settingswindow;

    settingswindow.setMessage(message);
    settingswindow.setUserName(userName);
    settingswindow.setPassword(userPassword);

    if (settingswindow.exec() == QDialog::Accepted) {
        message = settingswindow.getMessage();
        userName = settingswindow.getUserName();
        userPassword = settingswindow.getPassword();

        saveSettings();
    }
}

void MainWindow::loadSettings()
{
    QSettings settings("200volts", "Dating");

    settings.beginGroup("mainwindow");
    resize(settings.value("size", sizeHint()).toSize());
    bool maximazied = settings.value("maximazed", true).toBool();
    if (maximazied)
        showMaximized();
    settings.endGroup();

    settings.beginGroup("settings");
    message = settings.value("message", tr("Hello")).toString();
    userName = settings.value("user").toString();
    userPassword = settings.value("password").toString();	
    settings.endGroup();

	loadProcessedUserIds();
}

void MainWindow::saveSettings()
{
    QSettings settings("200volts", "Dating");

    settings.beginGroup("mainwindow");
    settings.setValue("size", size());
    settings.setValue("maximazed", isMaximized());
    settings.endGroup();

    settings.beginGroup("settings");
    settings.setValue("message", message);
    settings.setValue("user", userName);
    settings.setValue("password", userPassword);
    settings.endGroup();

	storeProcessedUserIds();
}

void MainWindow::storeProcessedUserIds()
{
	QString fileName = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + FileStoragePath;	
	QFile file(fileName);
     if (!file.open(QIODevice::WriteOnly)) {
		 qWarning() << "can't open file to store" << fileName;
         return;
	 }

	 qDebug() << "write" << fileName;

     QDataStream out(&file);
	 out.setVersion(QDataStream::Qt_4_8);
	 
     out << quint32(MimeTypeId) << processedUserIds;
}

void MainWindow::loadProcessedUserIds()
{
	QString fileName = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + FileStoragePath;
	QFile file(fileName);
     if (!file.open(QIODevice::ReadOnly)) {
         return;
	 }
	 qDebug() << "read" << fileName;

     QDataStream in(&file);
	 in.setVersion(QDataStream::Qt_4_8);
	 quint32 mimeTypeId;
     in >> mimeTypeId;

	if (mimeTypeId != MimeTypeId) {
		qWarning() << "Wrong file type";
		return;
	}
	 in >> processedUserIds;

	 qDebug() << "processed page ids" << processedUserIds;
}



void MainWindow::closeEvent(QCloseEvent *event)
{
    if (userReallyWantsToQuit()) {
        saveSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::userReallyWantsToQuit()
{
    if (QMessageBox::question(this, tr("Quit?"), tr("Do you realy want to quit?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Ok) {
        return true;
    }

    return false;
}

void MainWindow::startStopSpaming()
{
    if (!isSpaming) {
		userPageIds.clear();
		setIsSpaming(true);
        processNextState(STATE_LOAD_SEARCH_PAGE);
    } else {
        state = STATE_NONE;
		setIsSpaming(false);
    }
}

void MainWindow::setIsSpaming(bool spaming)
{
    isSpaming = spaming;
    if (isSpaming) {
        startStopAction->setIcon(QIcon(":/rc/Cancel.png"));
        startStopAction->setText(tr("&Stop"));
        startStopAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
        startStopAction->setToolTip(tr("Stop sending messages"));
    } else {
        startStopAction->setIcon(QIcon(":/rc/Forward.png"));
        startStopAction->setText(tr("&Run"));
        startStopAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
        startStopAction->setToolTip(tr("Start sending messages to users"));
    }
}

}
