#pragma once

#include <QString>

struct Member {
    int memberId = 0;
    int genealogyId = 0;
    QString name;
    QChar gender = 'M';
    int birthYear = 0;
    int deathYear = 0;
    int generation = 0;
    QString biography;
};
