#pragma once

#include <QDialog>

#include "model/Member.h"

class QComboBox;
class QLineEdit;
class QTextEdit;

class MemberDialog : public QDialog {
    Q_OBJECT

public:
    explicit MemberDialog(int genealogyId, QWidget* parent = nullptr);
    explicit MemberDialog(const Member& member, QWidget* parent = nullptr);

    Member member() const;

private:
    void buildUi();
    void fillMember(const Member& member);

    int memberId_ = 0;
    int genealogyId_ = 0;
    QLineEdit* nameEdit_ = nullptr;
    QComboBox* genderCombo_ = nullptr;
    QLineEdit* birthYearEdit_ = nullptr;
    QLineEdit* deathYearEdit_ = nullptr;
    QLineEdit* generationEdit_ = nullptr;
    QTextEdit* biographyEdit_ = nullptr;
};
