#pragma once

#include <QDialog>

#include "model/Genealogy.h"

class QDateEdit;
class QLineEdit;
class QTextEdit;

class GenealogyDialog : public QDialog {
    Q_OBJECT

public:
    explicit GenealogyDialog(int creatorUserId, QWidget* parent = nullptr);
    explicit GenealogyDialog(const Genealogy& genealogy, QWidget* parent = nullptr);

    Genealogy genealogy() const;

private:
    void buildUi();
    void fillGenealogy(const Genealogy& genealogy);

    int genealogyId_ = 0;
    int creatorUserId_ = 0;
    QLineEdit* titleEdit_ = nullptr;
    QLineEdit* surnameEdit_ = nullptr;
    QDateEdit* revisionDateEdit_ = nullptr;
    QTextEdit* descriptionEdit_ = nullptr;
};
