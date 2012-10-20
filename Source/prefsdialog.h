#ifndef PREFSDIALOG_H
#define PREFSDIALOG_H

#include <QDialog>

class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QRadioButton;
class QStackedLayout;
class QDialogButtonBox;
class QSettings;

class PrefsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PrefsDialog(QWidget *parent = 0);
    ~PrefsDialog();
    bool isChecked(QCheckBox *chkbx);
    bool autoLoadChecked();
    bool autoClearOldChecked();
    QString autoFileNameString() const;
    
public slots:
    void saveSettings();
    void enableStartUpOptions();
    
private:
    void createGeneralPage();
    QString appName;
    QString fileExt;
    QStackedLayout *stackedLayout;
    QListWidget *listWidget;
    QWidget *generalPage;

    QDialogButtonBox *buttonBox;

    QGroupBox *startupGroupBox;
    QCheckBox *autoLoad_chkBx;
    QCheckBox *autoClearOld_chkBx;

    std::vector<QCheckBox*> chkBxVector;

    QLineEdit *autoFileName;
};

#endif // PREFSDIALOG_H
