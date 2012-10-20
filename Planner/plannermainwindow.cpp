#include <QtGui>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QFile>
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>

#include "plannermainwindow.h"
#include "plannerwidget.h"
#include "prefsdialog.h"

PlannerMainWindow::PlannerMainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    appName = tr("Planner");
    fileExt = tr(".pla");

    createActions();
    createMenus();
    readSettings();

    pw = new PlannerWidget();
    setCentralWidget(pw);

    /* Instantiate prefsDialog and load its associated saved settings */
    prefsDialog = new PrefsDialog(this);
    if (prefsDialog->autoLoadChecked())
        readFile(prefsDialog->autoFileNameString());
    else setCurrentFile("");

    if (prefsDialog->autoClearOldChecked()) clearOld();

    pw->clearFields();
}

PlannerMainWindow::~PlannerMainWindow() {}

void PlannerMainWindow::closeEvent(QCloseEvent *event)
{
    if (okToContinue()) {
        writeSettings();
        event->accept();
    }
    else event->ignore();
}

// Return whether this window or a relevant child has been modified
bool PlannerMainWindow::modified() const
{
    if (pw->isWindowModified() || isWindowModified() )
        return true;
    return false;
}

void PlannerMainWindow::setModifiedFalse()
{
    setWindowModified(false);
    pw->setWindowModified(false);
}

void PlannerMainWindow::createActions()
{
    newAction = new QAction(tr("&New"), this);
    newAction->setShortcut(tr("Ctrl+N"));
    connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));

    openAction = new QAction(tr("&Open..."), this);
    openAction->setShortcut(tr("Ctrl+O"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

    saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcut(tr("Ctrl+S"));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAction = new QAction(tr("Save As..."), this);
    connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

    quitAction = new QAction(tr("Quit"), this);
    quitAction->setShortcut(tr("Ctrl+Q"));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    sortByDateAction = new QAction(tr("Date"), this);
    connect(sortByDateAction, SIGNAL(triggered()), this, SLOT(sortByDate()));

    sortByDateAddedAction = new QAction(tr("Date created"), this);
    connect(sortByDateAddedAction, SIGNAL(triggered()), this,
            SLOT(sortByDateAdded()));

    sortByNameAction = new QAction(tr("Name"), this);
    connect(sortByNameAction, SIGNAL(triggered()), this, SLOT(sortByName()));

    sortInReverseAction = new QAction(tr("Reverse"), this);
    connect(sortInReverseAction, SIGNAL(triggered()), this,
            SLOT(sortInReverse()));

    clearOldAction = new QAction(tr("Clear past events"), this);
    clearOldAction->setShortcut(tr("Ctrl+R"));
    connect(clearOldAction, SIGNAL(triggered()), this, SLOT(clearOld()));

    prefsAction = new QAction(tr("Preferences"), this);
    prefsAction->setShortcut(tr("Ctrl+P"));
    connect(prefsAction, SIGNAL(triggered()), this, SLOT(prefs()));

    aboutAction = new QAction(tr("About"), this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }
}

void PlannerMainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);

    separatorAction = fileMenu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i)
        fileMenu->addAction(recentFileActions[i]);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    sortSubmenu = editMenu->addMenu(tr("Sort by..."));
    sortSubmenu->addAction(sortByNameAction);
    sortSubmenu->addAction(sortInReverseAction);
    sortSubmenu->addAction(sortByDateAction);
    sortSubmenu->addAction(sortByDateAddedAction);
    editMenu->addAction(clearOldAction);
    editMenu->addAction(prefsAction);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAction);
}

void PlannerMainWindow::clearOld()
{
    pw->clearOldEntries(QDateTime::currentDateTime());
}

void PlannerMainWindow::newFile()
{
    if (okToContinue()) {
        pw->clearList();
        pw->clearFields();
        setCurrentFile("");
    }
}

void PlannerMainWindow::open()
{
    if (okToContinue()) {
        QString fileName = QFileDialog::getOpenFileName(this,
                                   tr("Open %1 File").arg(appName), ".",
                                   tr("%1 files (*%2)").arg(appName)
                                                       .arg(fileExt));
        if (!fileName.isEmpty())
            readFile(fileName);
    }
    pw->clearFields();
}

bool PlannerMainWindow::save()
{
    if (currentFile == "") return saveAs();
    else {
        writeFile(currentFile);
        updateRecentFileActions();
        return true;
    }
}

bool PlannerMainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                               tr("Save %1 File").arg(appName), ".",
                               tr("%1 files (*%2)").arg(appName)
                                                    .arg(fileExt));
    if (fileName.isEmpty())
        return false;

    updateRecentFileActions();
    return writeFile(fileName);
}

void PlannerMainWindow::openRecentFile()
{
    if (okToContinue()) {
        QAction *action = qobject_cast<QAction *>(sender());
        if (action)
            readFile(action->data().toString());
    }
}

void PlannerMainWindow::prefs()
{
    prefsDialog->show();
}

void PlannerMainWindow::about()
{
    QMessageBox::about(this, tr("About Planner"),
    tr("<h2>Planner</h2>"
    "<p>Planner is a small application that "
    "demonstrates QAction, QMainWindow, QMenuBar, "
    "QStatusBar, QTableWidget, QToolBar, and other "
    "Qt classes."));
}

void PlannerMainWindow::sendEntryToStream(std::vector<AbstractEntry*>::iterator it,
                                          QDataStream &outStream)
{
    outStream << (*it)->name()
              << (*it)->startDateTime()
              << (*it)->endDateTime()
              << (*it)->notes()
              << (*it)->whenAdded();
}

bool PlannerMainWindow::writeFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, appName,
        tr("Cannot write file %1:\n%2.")
        .arg(file.fileName())
        .arg(file.errorString()));
        return false;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_4_1);
    out << quint32(MagicNumber);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    std::vector<AbstractEntry*> vec = pw->vector();
    std::vector<AbstractEntry*>::iterator i, end;
    i = vec.begin();
    end = vec.end();

    while (i != end)
    {
        sendEntryToStream(i, out);
        i++;
    }

    QApplication::restoreOverrideCursor();
    setCurrentFile(fileName);
    return true;
}

AbstractEntry* PlannerMainWindow::getEntryFromStream(QDataStream &inStream)
{
    QString name, notes;
    QDateTime start, end, whenAdded;

    inStream >> name >> start >> end >> notes >> whenAdded;
    PlannerEntry *entry = new PlannerEntry(name, start, end, notes, whenAdded);
    return entry;
}

bool PlannerMainWindow::readFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, appName,
        tr("Cannot read file %1:\n%2.")
        .arg(file.fileName())
        .arg(file.errorString()));
        return false;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_4_1);
    quint32 magic;
    in >> magic;
    if (magic != MagicNumber) {
        QMessageBox::warning(this, appName,
        tr("The file is not a %1 file.").arg(appName));
        return false;
    }

    // Clear out any current data, otherwise loaded data will appear atop it
    pw->clearList();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    while (!in.atEnd()) {
        pw->addEntry(getEntryFromStream(in));
    }

    QApplication::restoreOverrideCursor();
    setCurrentFile(fileName);
    return true;
}

QString PlannerMainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void PlannerMainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    setModifiedFalse();
    QString shownName;
    if (!currentFile.isEmpty()) {
        shownName = strippedName(currentFile);
        recentFiles.removeAll(currentFile);
        recentFiles.prepend(currentFile);
        updateRecentFileActions();
    }
    else shownName = "Untitled";

    setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(appName));
}

bool PlannerMainWindow::okToContinue()
{
    if (modified()) {
        int r = QMessageBox::warning(this, appName,
                                    tr("The document has been modified.\n"
                                    "Would you like to save your changes?"),
                                    QMessageBox::Yes | QMessageBox::Default,
                                    QMessageBox::No,
                                    QMessageBox::Cancel | QMessageBox::Escape);
        if (r == QMessageBox::Yes) return save();
        else if (r == QMessageBox::Cancel) return false;
    }
    return true;
}

void PlannerMainWindow::sortByDate()
{
    pw->sortByDate(true);
}

void PlannerMainWindow::sortByDateAdded()
{
    pw->sortByDate(false);
}

void PlannerMainWindow::sortByName()
{
    pw->sortByName();
}

void PlannerMainWindow::sortInReverse()
{
    pw->sortInReverse();
}

void PlannerMainWindow::updateRecentFileActions()
{
    QMutableStringListIterator i(recentFiles);
    while (i.hasNext()) {
        if (!QFile::exists(i.next()))
            i.remove();
    }
    for (int j = 0; j < MaxRecentFiles; j++) {
        if (j < recentFiles.count()) {
            QString text = tr("&%1 %2")
            .arg(j + 1)
            .arg(strippedName(recentFiles[j]));
            recentFileActions[j]->setText(text);
            recentFileActions[j]->setData(recentFiles[j]);
            recentFileActions[j]->setVisible(true);
        }
        else {
            recentFileActions[j]->setVisible(false);
        }
    }
    separatorAction->setVisible(!recentFiles.isEmpty());
}

void PlannerMainWindow::readSettings()
{
    QSettings settings("MSF091886", appName);

    recentFiles = settings.value("recentFiles").toStringList();
    updateRecentFileActions();
}

void PlannerMainWindow::writeSettings()
{
    QSettings settings("MSF091886", appName);
    settings.setValue("recentFiles", recentFiles);
}
