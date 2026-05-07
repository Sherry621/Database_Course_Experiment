#include <QApplication>
#include <QFont>
#include <QFontDatabase>

#include "ui/LoginWindow.h"

int main(int argc, char* argv[]) {
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
