#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QApplication>
#include <QFontDatabase>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 从资源加载字体
    int fontId = QFontDatabase::addApplicationFont(":/fonts/Minecraft.ttf");
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            QFont customFont(fontFamilies.at(0), 10);
            a.setFont(customFont);
        }
    }

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "NbtConverter_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    a.setWindowIcon(QIcon(":/icons/NbtConverter.png"));
    MainWindow w;
    w.show();
    return a.exec();
}
