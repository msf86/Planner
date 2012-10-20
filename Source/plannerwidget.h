#ifndef PLANNERWIDGET_H
#define PLANNERWIDGET_H

#include <QtGui/QDialog>
#include "plannerentry.h"

class QListWidget;
class QPushButton;
class QLabel;
class QLineEdit;
class QDateTime;
class QDateTimeEdit;
class QPlainTextEdit;
class QDateTimeEdit;
class QListWidgetItem;

class PlannerWidget : public QDialog
{
    Q_OBJECT
    
public:
    PlannerWidget(QWidget *parent = 0);
    ~PlannerWidget();

    void addEntry(AbstractEntry *entry);
    void            clearList();
    void            clearOldEntries(QDateTime dt);
    void            clearVector();
    AbstractEntry  *currentEntry();
    void            deleteEntry(int row);
    AbstractEntry  *DT_conflict_in_list();
    bool            invalidName(QString name);
    AbstractEntry  *itemEntry(QListWidgetItem* lwi);
    std::vector<AbstractEntry*> vector();

protected:
    virtual void keyPressEvent(QKeyEvent *e);

public slots:
    void addEntry();
    void clearFields();
    void deleteEntry();
    void enableButtons();
    void find();
    void refresh();
    void replaceEntry();
    void sortByDate(bool byStartDT);
    void sortInReverse();
    void sortByName();
    void synchDT();

private:
    std::vector<AbstractEntry*> entryVector;
    QListWidget *entryList;
    QLineEdit *finder;

    QPushButton *clearButton;
    QPushButton *refreshButton;

    QLineEdit *nameField;
    QDateTimeEdit *startingDateTime;
    QDateTimeEdit *endingDateTime;
    QPlainTextEdit *notesField;

    QLabel *whenAddedDisplay;

    QPushButton *addButton;
    QPushButton *replaceButton;
    QPushButton *deleteButton;
};

#endif // PLANNERWIDGET_H
