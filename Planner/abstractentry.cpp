#include "abstractentry.h"

AbstractEntry::AbstractEntry(QString name, QString notes)
    : _name(name), _notes(notes) {}

QString AbstractEntry::name() const { return _name; }
QString AbstractEntry::notes() const { return _notes; }

void AbstractEntry::setName(QString newName) { _name = newName; }
void AbstractEntry::setNotes(QString newNotes) { _notes = newNotes; }
