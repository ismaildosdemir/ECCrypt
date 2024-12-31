#include "eccrypt.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>

QTranslator *previousTranslator = nullptr;  // previousTranslator'ı işaretçi olarak tanımlayın

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ECCrypt_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            // Önceki çeviriyi kaldır
            if (previousTranslator != nullptr) {
                a.removeTranslator(previousTranslator);  // Önceki çeviriyi kaldır
                delete previousTranslator;  // Belleği serbest bırak
            }

            // Yeni çeviriyi yükle
            previousTranslator = new QTranslator();  // Yeni çeviri nesnesi oluştur
            a.installTranslator(previousTranslator);

            if (!previousTranslator->load(":/i18n/" + baseName)) {
                qWarning() << "Translation file could not be loaded:" << baseName;
            }
            break;
        }
    }

    eccrypt w;
    w.show();
    return a.exec();
}
