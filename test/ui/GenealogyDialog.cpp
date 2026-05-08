#include "GenealogyDialog.h"

#include <QDate>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>

GenealogyDialog::GenealogyDialog(int creatorUserId, QWidget* parent)
    : QDialog(parent), creatorUserId_(creatorUserId) {
    setWindowTitle("新增族谱");
    buildUi();
}

GenealogyDialog::GenealogyDialog(const Genealogy& genealogy, QWidget* parent)
    : QDialog(parent), genealogyId_(genealogy.genealogyId), creatorUserId_(genealogy.creatorUserId) {
    setWindowTitle("编辑族谱");
    buildUi();
    fillGenealogy(genealogy);
}

void GenealogyDialog::buildUi() {
    titleEdit_ = new QLineEdit(this);
    surnameEdit_ = new QLineEdit(this);
    revisionDateEdit_ = new QDateEdit(QDate::currentDate(), this);
    revisionDateEdit_->setCalendarPopup(true);
    descriptionEdit_ = new QTextEdit(this);

    auto* form = new QFormLayout();
    form->addRow("谱名", titleEdit_);
    form->addRow("姓氏", surnameEdit_);
    form->addRow("修谱时间", revisionDateEdit_);
    form->addRow("备注", descriptionEdit_);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

void GenealogyDialog::fillGenealogy(const Genealogy& genealogy) {
    titleEdit_->setText(genealogy.title);
    surnameEdit_->setText(genealogy.familySurname);
    revisionDateEdit_->setDate(genealogy.revisionTime.isValid() ? genealogy.revisionTime : QDate::currentDate());
    descriptionEdit_->setPlainText(genealogy.description);
}

Genealogy GenealogyDialog::genealogy() const {
    Genealogy genealogy;
    genealogy.genealogyId = genealogyId_;
    genealogy.creatorUserId = creatorUserId_;
    genealogy.title = titleEdit_->text().trimmed();
    genealogy.familySurname = surnameEdit_->text().trimmed();
    genealogy.revisionTime = revisionDateEdit_->date();
    genealogy.description = descriptionEdit_->toPlainText();
    return genealogy;
}
