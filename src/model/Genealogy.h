#pragma once

#include <QDate>
#include <QString>

struct Genealogy {
    int genealogyId = 0;
    QString title;
    QString familySurname;
    QDate revisionTime;
    int creatorUserId = 0;
    QString description;
};
