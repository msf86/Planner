#ifndef PLANNERENTRY_H
#define PLANNERENTRY_H

#include "abstractentry.h"

class PlannerEntry : public AbstractEntry
{

public:
    PlannerEntry(QString name, QDateTime startDateTime, QDateTime endDateTime,
          QString notes, QDateTime whenAdded);
    virtual ~PlannerEntry();

    virtual QDateTime startDateTime() const;
    virtual QDateTime endDateTime() const;
    virtual QDateTime whenAdded() const;

    virtual void setStartDateTime(QDateTime newDT);
    virtual void setEndDateTime(QDateTime newDT);

private:
    QDateTime       _startDateTime;
    QDateTime       _endDateTime;
    const QDateTime _whenAdded;
};

#endif // PLANNERENTRY_H
