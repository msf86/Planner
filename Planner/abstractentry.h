#ifndef ABSTRACTENTRY_H
#define ABSTRACTENTRY_H

#include <QDateTime>

class AbstractEntry {

protected:
    AbstractEntry(QString name, QString notes);
    QString _name;
    QString _notes;

public:
    virtual ~AbstractEntry() {}

    virtual QString name() const;
    virtual QString notes() const;
    virtual QString email() const {}
    virtual QDateTime startDateTime() const {}
    virtual QDateTime endDateTime() const {}
    virtual QDateTime whenAdded() const {}

    virtual void setName(QString newName);
    virtual void setNotes(QString newNotes);
    virtual void setEmail(QString newEmail) {}
    virtual void setStartDateTime(QDateTime newDT) {}
    virtual void setEndDateTime(QDateTime newDT) {}
};

#endif // ABSTRACTENTRY_H
