#include <QDebug>
#include "plannerentry.h"

PlannerEntry::PlannerEntry(QString name, QDateTime startDateTime, QDateTime endDateTime,
             QString notes, QDateTime whenAdded)
    : AbstractEntry(name, notes), _startDateTime(startDateTime), _endDateTime(endDateTime),
      _whenAdded(whenAdded) {}

PlannerEntry::~PlannerEntry()
{
    qDebug() << "in PlannerEntry dtor";
}

QDateTime PlannerEntry::startDateTime() const  { return _startDateTime; }
QDateTime PlannerEntry::endDateTime() const    { return _endDateTime; }
QDateTime PlannerEntry::whenAdded() const      { return _whenAdded; }



void PlannerEntry::setStartDateTime(QDateTime newDT)
{
    _startDateTime = newDT;
}
void PlannerEntry::setEndDateTime(QDateTime newDT)
{
    _endDateTime = newDT;
}
