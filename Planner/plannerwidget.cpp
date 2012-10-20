#include <QBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QMessageBox>
#include <QDebug>

#include "plannerwidget.h"
#include <stack>

PlannerWidget::PlannerWidget(QWidget *parent)
    : QDialog(parent)
{
    // Top button layout
    clearButton = new QPushButton(tr("&Clear Fields"));
    refreshButton = new QPushButton(tr("&Refresh Selected"));

    connect(clearButton, SIGNAL(clicked()), this, SLOT(clearFields()));
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(refresh()));

    QHBoxLayout* buttonsLayout_1 = new QHBoxLayout;
    buttonsLayout_1->addWidget(clearButton);
    buttonsLayout_1->addWidget(refreshButton);

    // Name input layout
    QLabel* nameInputLabel = new QLabel(tr("Name:"));
    nameField = new QLineEdit;

    QHBoxLayout* nameLayout = new QHBoxLayout;
    nameLayout->addWidget(nameInputLabel);
    nameLayout->addWidget(nameField);

    // Notes layout
    QLabel* notesLabel = new QLabel(tr("Event Notes:"));
    notesField = new QPlainTextEdit;
    whenAddedDisplay = new QLabel;

    QVBoxLayout* notesLayout = new QVBoxLayout;
    notesLayout->addWidget(notesLabel);
    notesLayout->addWidget(notesField);
    notesLayout->addWidget(whenAddedDisplay);

    // Date/time layout
    QLabel* startingDateTimeLabel = new QLabel(tr("Starting Date & Time: "));
    QLabel* endingDateTimeLabel = new QLabel(tr("Ending Date & Time: "));

    startingDateTime = new QDateTimeEdit();
    endingDateTime   = new QDateTimeEdit();

    QHBoxLayout* dateTime_1 = new QHBoxLayout;
    dateTime_1->addWidget(startingDateTimeLabel);
    dateTime_1->addWidget(startingDateTime);
    QHBoxLayout* dateTime_2 = new QHBoxLayout;
    dateTime_2->addWidget(endingDateTimeLabel);
    dateTime_2->addWidget(endingDateTime);

    connect(startingDateTime, SIGNAL(editingFinished()), this,
            SLOT(synchDT()));

    QVBoxLayout* dateTimeLayout = new QVBoxLayout;
    dateTimeLayout->addLayout(dateTime_1);
    dateTimeLayout->addLayout(dateTime_2);

    // Bottom button layout
    addButton = new QPushButton(tr("&Add"));
    replaceButton = new QPushButton(tr("&Modify"));
    deleteButton = new QPushButton(tr("&Delete"));

    connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));
    connect(replaceButton, SIGNAL(clicked()), this, SLOT(replaceEntry()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteEntry()));

    QHBoxLayout* buttonsLayout_2 = new QHBoxLayout;
    buttonsLayout_2->addWidget(addButton);
    buttonsLayout_2->addWidget(replaceButton);
    buttonsLayout_2->addWidget(deleteButton);

    // Interface layout
    QVBoxLayout* interfaceLayout = new QVBoxLayout;
    interfaceLayout->addLayout(buttonsLayout_1);
    interfaceLayout->addLayout(nameLayout);
    interfaceLayout->addLayout(dateTimeLayout);
    interfaceLayout->addLayout(notesLayout);
    interfaceLayout->addLayout(buttonsLayout_2);

    // Entry list layout
    QLabel *finderLabel = new QLabel("Find: ");
    finder = new QLineEdit;
    entryList = new QListWidget;

    QHBoxLayout *finderLayout = new QHBoxLayout;
    finderLayout->addWidget(finderLabel);
    finderLayout->addWidget(finder);

    QVBoxLayout *entryListLayout = new QVBoxLayout;
    entryListLayout->addLayout(finderLayout);
    entryListLayout->addWidget(entryList);

    // Base layout
    QHBoxLayout* baseLayout = new QHBoxLayout;
    baseLayout->addLayout(entryListLayout);
    baseLayout->addLayout(interfaceLayout);

    setLayout(baseLayout);
    setWindowTitle(tr("Planner[*]"));

    // When a list item is clicked/selected, its data will display on interface
    connect(entryList,
            SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
            this, SLOT(refresh()));
    connect(entryList, SIGNAL(clicked(QModelIndex)), this, SLOT(refresh()));
    connect(finder, SIGNAL(textChanged(QString)), this, SLOT(find()));

    /* Gray out buttons when they shouldn't be used */
    connect(entryList, SIGNAL(currentItemChanged(QListWidgetItem*,
            QListWidgetItem*)), this, SLOT(enableButtons()));

    enableButtons();
}

PlannerWidget::~PlannerWidget()
{
    clearVector();
}




/******************************************************************************
    SLOTS
******************************************************************************/

void PlannerWidget::addEntry()
{
    nameField->setText(nameField->text().trimmed());
    QString name = nameField->text();

    if(invalidName(name)) return;

    QDateTime start = startingDateTime->dateTime();
    QDateTime end = endingDateTime->dateTime();

    /* Ensure start date/time is before end date/time */
    if(start.toMSecsSinceEpoch() > end.toMSecsSinceEpoch()) {
        QMessageBox::warning(this, tr("Invalid Date/Time"),
                             tr("You must enter an ending date/time\n"
                                "that occurs later than the starting\n"
                                "date/time."),
                             QMessageBox::Ok);
        return;
    }

    QString notes = notesField->toPlainText();

    // Ensure that there are no datetime conflicts with other entries
    AbstractEntry* e = DT_conflict_in_list();
    if(e != NULL) {
        QString boxBody = tr("The supplied date/time interval conflicts\n"
                             "with the following entry:\n\n \"");
        boxBody.append(e->name());
        boxBody.append(tr("\"\nStart: "));
        boxBody.append(e->startDateTime().toString("MM/dd/yyyy h:mm:ss AP"));
        boxBody.append(tr("\nEnd:  "));
        boxBody.append(e->endDateTime().toString("MM/dd/yyyy h:mm:ss AP"));
        boxBody.append(tr("\n\nAdd the entry anyway?"));

        int x = QMessageBox::warning(this, tr("Date/Time Conflict"),
                             boxBody,
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::No);

        if(x == QMessageBox::No) return;
    }

    PlannerEntry *entry = new PlannerEntry(name, start, end, notes,
                                           QDateTime::currentDateTime());
    addEntry(entry);

    // Select the new item in the list (it's at the end)
    entryList->setCurrentRow(entryList->count() - 1);

    setWindowModified(true);
}

// Delete both the contents and indeces of entryVector
void PlannerWidget::clearVector()
{
    std::vector<AbstractEntry*>::iterator it = entryVector.begin();
    while(!entryVector.empty()) {
        delete *(it);
        it = entryVector.erase(it); // Returns it++
    }
}

void PlannerWidget::clearFields()
{
    QDateTime DT = QDateTime::currentDateTime();

    // Set the default time to noon, keeping today's date
    QTime t;
    t.setHMS(12, 0, 0);
    DT.setTime(t);

    nameField->clear();
    startingDateTime->setDateTime(DT);
    endingDateTime->setDateTime(DT);
    notesField->clear();
    whenAddedDisplay->clear();
}

void PlannerWidget::clearList()
{
    entryList->clear();
    clearVector();
}

void PlannerWidget::deleteEntry()
{
    if (entryList->currentRow() == -1) return;

    QListWidgetItem* lwi = entryList->currentItem();
    int row = entryList->row(lwi);

    int x = QMessageBox::warning(this, tr("Delete Entry"),
                         tr("Are you sure you want to\n"
                            "delete the selected entry?"),
                         QMessageBox::Yes | QMessageBox::No,
                         QMessageBox::No);

    if (x == QMessageBox::No) return;
    deleteEntry(row);
    setWindowModified(true);
}

void PlannerWidget::find()
{
    QString text, name;
    text = finder->text();

    if (text.isEmpty()) {
        entryList->setCurrentRow(-1);
        clearFields();
        return;
    }

    int i = 0;
    while(i < entryList->count()) {
        name = itemEntry(entryList->item(i))->name();
        if (text == name.left(text.size())) {
            entryList->setCurrentRow(i);
            return;
        }
        i++;
    }

    /* No candidate item was found, so let there be no selected list item */
    entryList->setCurrentRow(-1);
    clearFields();
}

/* Disable certain buttons when no list item is selected */
void PlannerWidget::enableButtons()
{
    bool b = entryList->currentRow() != -1;
    replaceButton->setEnabled(b);
    deleteButton->setEnabled(b);
    refreshButton->setEnabled(b);
}

// Load the data from the currently-selected list item into the input fields
void PlannerWidget::refresh()
{
        if (entryList->currentItem() == NULL) return;

        AbstractEntry* e = currentEntry();
        nameField->setText(e->name());
        startingDateTime->setDateTime(e->startDateTime());
        endingDateTime->setDateTime(e->endDateTime());
        notesField->setPlainText(e->notes());
        whenAddedDisplay->setText("Entry created: " + e->whenAdded().toString(
                                      "MM/dd/yyyy h:mm:ss AP"));
}

/* Replaces the selected entry's data with what's in the input fields. It's
just a little data, so it's not worth checking what's changed. */
void PlannerWidget::replaceEntry()
{
    if (entryList->count() == 0) return;

    nameField->setText(nameField->text().trimmed());
    QString name = nameField->text();
    AbstractEntry* e = currentEntry();

    /* If a new name was entered, make sure it's a valid one. */
    if(name != e->name())
        if(invalidName(name)) return;

    e->setName(name);
    e->setStartDateTime(startingDateTime->dateTime());
    e->setEndDateTime(endingDateTime->dateTime());
    e->setNotes(notesField->toPlainText());

    entryList->currentItem()->setText(nameField->text());
    setWindowModified(true);
}

/* If the datetime fields indicate a datetime interval that conflicts with the
   interval of another entry, return a pointer to the first such entry found.
   byStartDT: if true, sort by start DT, else by added DT. */
void PlannerWidget::sortByDate(bool byStartDT)
{
    /* Will go through entryList and compare DT's, then continuously
       put the entry with the lowest DT at the end (selection sort) */
    QDateTime minDT, curDT;
    int row, i, unsorted;
    unsorted = entryList->count();

    /* SELECTION SORT: for entryList->count() iterations, put the earliest
       unsorted item at the end of the list. The first to be sorted will
       become the first item in the list after sorting, the second will be the
       second, and so on. */
    for (int j=0; j < entryList->count(); j++) {

        /* Reset everything */
        row = 0;
        if (byStartDT)
            curDT = itemEntry(entryList->item(0))->startDateTime();
        else curDT = itemEntry(entryList->item(0))->whenAdded();
        minDT = curDT;

        /* Compare each unsorted item to the known min, and update min */
        for (i=0; i < unsorted; i++) {
            if (byStartDT)
                curDT = itemEntry(entryList->item(i))->startDateTime();
            else curDT = itemEntry(entryList->item(i))->whenAdded();
            if(curDT.operator<=(minDT)) {
                minDT = curDT;
                row = i;
            }
        }

        /* Move minLWI to the end of the list, and decrement the number of
           unsorted items */
        entryList->addItem(entryList->takeItem(row));
        unsorted--;
    }
    setWindowModified(true);
}

void PlannerWidget::sortInReverse()
{
    std::stack<QListWidgetItem*> itemStack;

    /* "Move" all the entries to a stack */
    while(entryList->count() > 0) {
        itemStack.push(entryList->takeItem(0));
    }

    /* Put them back on the list widget in reverse order */
    while(!itemStack.empty()) {
        entryList->addItem(itemStack.top());
        itemStack.pop();
    }
    setWindowModified(true);
}

void PlannerWidget::sortByName()
{
    entryList->sortItems();
    setWindowModified(true);
}

void PlannerWidget::synchDT()
{
    endingDateTime->setDateTime(startingDateTime->dateTime());
}




/******************************************************************************
    REGULAR FUNCTIONS
******************************************************************************/

void PlannerWidget::addEntry(AbstractEntry* entry)
{
    /* Create a list item and corresponding entry */
    QListWidgetItem* item = new QListWidgetItem(entry->name());

    /* Add a pointer to the entry to the list item */
    QVariant qvar = qVariantFromValue((void*)entry);
    item->setData(Qt::UserRole, qvar);

    entryVector.push_back(entry);
    entryList->addItem(item);
}

/* Remove entries whose ending datetime is earlier than DT. */
void PlannerWidget::clearOldEntries(QDateTime dt)
{
    /* First, check if any "old" entries are present */
    uint i = 0;
    while (i < entryVector.size()) {
        if(entryVector[i]->endDateTime().operator <=(dt))
            break;
        i++;
    }

    /* If none are found, return */
    if (i == entryVector.size()) return;

    /* Ask whether or not to delete them */
    int x = QMessageBox::question(this, tr("Planner"),
            tr("Would you like to clear\n"
               "the events which have\n"
               "already occurred?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
    if (x == QMessageBox::No) return;

    /* Traverse list widget, deleting old items */
    QDateTime eventEnd;
    for(int j = i; j < entryList->count(); j++) {
        eventEnd = itemEntry(entryList->item(j))->endDateTime();

        if(eventEnd.operator <=(dt)) {
            deleteEntry(j);
            j--; // Otherwise you'll skip the now-j'th (prev. j+1'th) entry
        }
    }
}

/* Return pointer to the entry represented by the current list item */
AbstractEntry *PlannerWidget::currentEntry()
{
    return itemEntry(entryList->currentItem());
}

void PlannerWidget::deleteEntry(int row)
{
    if (entryList->count() == 0) return;

    QListWidgetItem *lwi = entryList->item(row);

    /* Delete the entry associated with lwi, but erase its vector element
    first. Search for it linearly, since the list widget might get resorted,
    which would leave the vector unsorted. */
    AbstractEntry* e = itemEntry(lwi);
    std::vector<AbstractEntry*>::iterator i = entryVector.begin();
    while (*i != e) i++;
    delete e;
    entryVector.erase(i);

    /* Delete the list item */
    delete entryList->takeItem(row);
}

AbstractEntry *PlannerWidget::DT_conflict_in_list()
{
    AbstractEntry *e;
    QDateTime start_1, end_1, start_2, end_2;
    start_1 = startingDateTime->dateTime();
    end_1 = endingDateTime->dateTime();

    for(uint i=0; i < entryVector.size(); i++) {
        e = entryVector[i];
        start_2 = e->startDateTime();
        end_2 = e->endDateTime();

        /* Check if either of the events have a starting point that is within
           the start-end interval of the other, i.e. there is DT overlap. */
        if (start_2.operator>=(start_1) && start_2.operator<=(end_1) ||
            start_1.operator>=(start_2) && start_1.operator<=(end_2))
        {
            return e;
        }
    }
    return NULL;
}

bool PlannerWidget::invalidName(QString name)
{
    // Make sure name is non-empty
    if (name == "") {
        QMessageBox::warning(this, tr("Name Required"),
                             tr("Please supply a name for this entry."),
                             QMessageBox::Ok);
        nameField->setFocus();
        return true;
    }

    // Make sure name isn't taken
    for (uint i=0; i < entryVector.size(); i++) {
        if (name == entryVector[i]->name()) {
            QMessageBox::warning(this, tr("Naming Conflict"),
                                 tr("Another entry already has this\n"
                                 "name. Please choose another."),
                                 QMessageBox::Ok);
            nameField->setFocus();
            return true;
        }
    }
    return false;
}

AbstractEntry* PlannerWidget::itemEntry(QListWidgetItem* lwi)
{
    return (PlannerEntry*)lwi->data(Qt::UserRole).value<void*>();
}

std::vector<AbstractEntry*> PlannerWidget::vector()
{
    return entryVector;
}

/* Override the ESC button's ability to close this widget */
void PlannerWidget::keyPressEvent(QKeyEvent *e){
    if(e->key()!=Qt::Key_Escape) QDialog::keyPressEvent(e);
}
