#ifndef PLANNERMAINWINDOW_H
#define PLANNERMAINWINDOW_H

#include <QtGui/QMainWindow>

// Arbitrary fixed 32-bit integer
#define MagicNumber 0x37406D6B

class QMenu;
class QAction;
class PlannerWidget;
class PrefsDialog;
class QSettings;
class AbstractEntry;

class PlannerMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit PlannerMainWindow(QWidget *parent = 0);
    ~PlannerMainWindow();
    bool modified() const;
    void setModifiedFalse();

private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void openRecentFile();
    void sortByDate();
    void sortByDateAdded();
    void sortByName();
    void sortInReverse();
    void clearOld();
    void prefs();
    void about();

protected:
    void closeEvent(QCloseEvent *event);

private:
    PlannerWidget *pw;
    PrefsDialog *prefsDialog;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *sortSubmenu;
    QMenu *helpMenu;
    QString appName;
    QString fileExt;


    QStringList recentFiles;
    QString currentFile;
    enum { MaxRecentFiles = 6 };
    QAction *recentFileActions[MaxRecentFiles];
    QAction *separatorAction;

    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *quitAction;
    QAction *sortByDateAction;
    QAction *sortByDateAddedAction;
    QAction *sortByNameAction;
    QAction *sortInReverseAction;
    QAction *clearOldAction;
    QAction *prefsAction;
    QAction *aboutAction;

    bool okToContinue();
    void createActions();
    void createMenus();
    bool writeFile(const QString& fileName);
    bool readFile(const QString& fileName);
    QString strippedName(const QString &fullFileName);
    void setCurrentFile(const QString& fileName);
    void readSettings();
    void writeSettings();
    void updateRecentFileActions();

    void sendEntryToStream(std::vector<AbstractEntry*>::iterator it,
                           QDataStream &outStream);
    AbstractEntry *getEntryFromStream(QDataStream& inStream);
};

#endif // PLANNERMAINWINDOW_H
