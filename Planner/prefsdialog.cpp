#include <QLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QStackedLayout>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <QSettings>
#include <QDebug>

#include "prefsdialog.h"

PrefsDialog::PrefsDialog(QWidget *parent) :
    QDialog(parent)
{
    appName = tr("Planner");
    fileExt = tr(".pla");

    createGeneralPage();

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    listWidget = new QListWidget;
    listWidget->addItem(tr("General"));

    stackedLayout = new QStackedLayout;

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(1, 3);
    mainLayout->addWidget(listWidget, 0, 0);
    mainLayout->addLayout(stackedLayout, 0, 1);
    mainLayout->addWidget(buttonBox, 1, 0, 1, 2);
    setLayout(mainLayout);

    stackedLayout->addWidget(generalPage);
    connect(listWidget, SIGNAL(currentRowChanged(int)),
            stackedLayout, SLOT(setCurrentIndex(int)));
    setWindowTitle(tr("Preferences"));
    listWidget->setCurrentRow(0);
}

PrefsDialog::~PrefsDialog() {}

void PrefsDialog::createGeneralPage()
{
    generalPage = new QWidget;

    QVBoxLayout *pageLayout = new QVBoxLayout;

    /* Start-up group box */
    startupGroupBox = new QGroupBox(tr("Start-up options"));

    autoLoad_chkBx = new QCheckBox(tr("Automatically load a file on run"));
    autoClearOld_chkBx = new QCheckBox(tr("Automatically prompt to clear its old entries"));

    chkBxVector.push_back(autoLoad_chkBx);
    chkBxVector.push_back(autoClearOld_chkBx);

    autoFileName = new QLineEdit();
    QLabel *autoFileNameLabel = new QLabel(tr("File to load (in default directory):"));

    QVBoxLayout *startupGroupBoxLayout = new QVBoxLayout;
    startupGroupBoxLayout->addWidget(autoLoad_chkBx);
    startupGroupBoxLayout->addWidget(autoClearOld_chkBx);
    startupGroupBoxLayout->addWidget(autoFileNameLabel);
    startupGroupBoxLayout->addWidget(autoFileName);

    startupGroupBox->setLayout(startupGroupBoxLayout);
    pageLayout->addWidget(startupGroupBox);
    generalPage->setLayout(pageLayout);

    QSettings settings("MSF091886", appName);

    /* Load saved checkbox settings */
    for (std::vector<QCheckBox*>::iterator it = chkBxVector.begin();
         it != chkBxVector.end(); it++)
    {
        (*it)->setChecked(settings.value((*it)->text(), false).toBool());
    }

    /* Load other settings */
    QString fileName = settings.value("autoFileName").toString();
    autoFileName->setText(fileName);

    /* Gray out fields under autoLoad_chkBx if it's not checked */
    connect(autoLoad_chkBx, SIGNAL(clicked()), this,
            SLOT(enableStartUpOptions()));
}

void PrefsDialog::enableStartUpOptions()
{
    bool b = autoLoad_chkBx->isChecked();
    autoClearOld_chkBx->setEnabled(b);
    autoFileName->setEnabled(b);
}

bool PrefsDialog::isChecked(QCheckBox *chkbx)
{
    QSettings settings("MSF091886", appName);
    return settings.value(chkbx->text(), false).toBool();
}

bool PrefsDialog::autoLoadChecked() { return isChecked(autoLoad_chkBx); }

bool PrefsDialog::autoClearOldChecked() {
    return isChecked(autoClearOld_chkBx);
}

QString PrefsDialog::autoFileNameString() const
{
    QSettings settings("MSF091886", appName);
    return settings.value("autoFileName").toString();
}

void PrefsDialog::saveSettings()
{
    QSettings settings("MSF091886", appName);

    /* Save checkbox settings. Just make sure none of the checkboxes have the
    same text() value, or the settings object will get confused. */
    for (std::vector<QCheckBox*>::iterator it = chkBxVector.begin();
         it != chkBxVector.end(); it++)
    {
        settings.setValue((*it)->text(), (*it)->isChecked());
    }

    /* Save other settings */
    QString fileName = autoFileName->text();
    if (fileName.right(fileExt.length()) != fileExt) fileName.append(fileExt);
    settings.setValue("autoFileName", fileName);

    accept();
}
