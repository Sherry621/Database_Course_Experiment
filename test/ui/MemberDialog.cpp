#include "MemberDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>

MemberDialog::MemberDialog(int genealogyId, QWidget* parent)
    : QDialog(parent), genealogyId_(genealogyId) {
    setWindowTitle("新增成员");

    nameEdit_ = new QLineEdit(this);
    genderCombo_ = new QComboBox(this);
    genderCombo_->addItem("男", "M");
    genderCombo_->addItem("女", "F");
    birthYearEdit_ = new QLineEdit(this);
    deathYearEdit_ = new QLineEdit(this);
    generationEdit_ = new QLineEdit(this);
    biographyEdit_ = new QTextEdit(this);

    auto* form = new QFormLayout();
    form->addRow("姓名", nameEdit_);
    form->addRow("性别", genderCombo_);
    form->addRow("出生年", birthYearEdit_);
    form->addRow("死亡年", deathYearEdit_);
    form->addRow("代数", generationEdit_);
    form->addRow("生平简介", biographyEdit_);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

Member MemberDialog::member() const {
    Member member;
    member.genealogyId = genealogyId_;
    member.name = nameEdit_->text().trimmed();
    member.gender = genderCombo_->currentData().toString().at(0);
    member.birthYear = birthYearEdit_->text().toInt();
    member.deathYear = deathYearEdit_->text().toInt();
    member.generation = generationEdit_->text().toInt();
    member.biography = biographyEdit_->toPlainText();
    return member;
}
