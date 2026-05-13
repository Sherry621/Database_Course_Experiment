#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QLocale>
#include <QProcessEnvironment>

#include "ui/LoginWindow.h"

int main(int argc, char* argv[]) {
    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
#ifdef Q_OS_LINUX
    if (env.contains("WSL_DISTRO_NAME") && env.value("QT_IM_MODULE").trimmed().isEmpty()) {
        qputenv("QT_IM_MODULE", "fcitx");
    }
    if (env.contains("WSL_DISTRO_NAME") && env.value("XMODIFIERS").trimmed().isEmpty()) {
        qputenv("XMODIFIERS", "@im=fcitx");
    }
    if (env.contains("WSL_DISTRO_NAME") && env.value("GTK_IM_MODULE").trimmed().isEmpty()) {
        qputenv("GTK_IM_MODULE", "fcitx");
    }
    if (env.contains("WSL_DISTRO_NAME") && env.value("QT_QPA_PLATFORM").trimmed().isEmpty()) {
        qputenv("QT_QPA_PLATFORM", "xcb");
    }
#endif
    QLocale::setDefault(QLocale(QLocale::Chinese, QLocale::China));

    QApplication app(argc, argv);

    const QStringList preferredFonts = {
        "Noto Sans CJK SC",
        "Microsoft YaHei",
        "WenQuanYi Micro Hei",
        "SimSun"
    };
    const QStringList availableFonts = QFontDatabase::families();
    for (const QString& fontName : preferredFonts) {
        if (availableFonts.contains(fontName)) {
            app.setFont(QFont(fontName, 10));
            break;
        }
    }

    LoginWindow loginWindow;
    loginWindow.show();

    return app.exec();
}
