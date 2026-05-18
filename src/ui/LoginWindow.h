#pragma once

#include <QWidget>

#include "model/User.h"
#include "service/AuthService.h"

class QLineEdit;

class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget* parent = nullptr);

private slots:
    void handleLogin();
    void handleRegister();

private:
    void buildUi();
    bool ensureDatabaseConnected();

    QLineEdit* usernameEdit_ = nullptr;
    QLineEdit* passwordEdit_ = nullptr;
    AuthService authService_;
};
