#include "LoginWindow.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "db/DatabaseManager.h"
#include "ui/MainWindow.h"

LoginWindow::LoginWindow(QWidget* parent) : QWidget(parent) {
    buildUi();
}

void LoginWindow::buildUi() {
    setWindowTitle("寻根溯源 - 登录");
    resize(360, 180);

    usernameEdit_ = new QLineEdit(this);
    passwordEdit_ = new QLineEdit(this);
    usernameEdit_->setAttribute(Qt::WA_InputMethodEnabled, true);
    passwordEdit_->setAttribute(Qt::WA_InputMethodEnabled, true);
    passwordEdit_->setEchoMode(QLineEdit::Password);

    auto* formLayout = new QFormLayout();
    formLayout->addRow("用户名", usernameEdit_);
    formLayout->addRow("密码", passwordEdit_);

    auto* loginButton = new QPushButton("登录", this);
    auto* registerButton = new QPushButton("注册", this);
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::handleLogin);
    connect(registerButton, &QPushButton::clicked, this, &LoginWindow::handleRegister);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(registerButton);
    buttonLayout->addWidget(loginButton);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->addLayout(formLayout);
    rootLayout->addLayout(buttonLayout);
}

void LoginWindow::handleLogin() {
    if (!ensureDatabaseConnected()) {
        return;
    }

    auto user = authService_.login(usernameEdit_->text().trimmed(), passwordEdit_->text());
    if (!user.has_value()) {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误。");
        return;
    }

    auto* mainWindow = new MainWindow(*user);
    mainWindow->show();
    close();
}

void LoginWindow::handleRegister() {
    if (!ensureDatabaseConnected()) {
        return;
    }

    const QString username = usernameEdit_->text().trimmed();
    const QString password = passwordEdit_->text();
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "注册失败", "用户名和密码不能为空。");
        return;
    }

    if (!authService_.registerUser(username, password, username, QString())) {
        const QString detail = authService_.lastError().isEmpty()
            ? "用户可能已存在，或数据库写入失败。"
            : authService_.lastError();
        QMessageBox::warning(this, "注册失败", detail);
        return;
    }

    QMessageBox::information(this, "注册成功", "用户已创建，可以登录。");
}

bool LoginWindow::ensureDatabaseConnected() {
    auto& db = DatabaseManager::instance();
    if (db.isOpen()) {
        return true;
    }

    if (db.connect("localhost", 5432, "genealogy_lab", "genealogy_user", "genealogy_pass")) {
        return true;
    }

    QMessageBox::critical(this, "数据库连接失败", db.lastError());
    return false;
}
