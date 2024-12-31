#include "eccrypt.h"
#include "qtranslator.h"
#include "ui_eccrypt.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QFrame>
#include <QListWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <QRandomGenerator>
#include "encryption.h"


eccrypt::eccrypt(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::eccrypt)
{
    ui->setupUi(this);
    ui->listWidget->setAcceptDrops(true);  // Sadece bu frame dosya kabul edecek
    ui->lineEdit_password->setPlaceholderText(QObject::tr("Şifre"));
    ui->comboBox_languages->addItem("Türkçe", "tr_TR");
    ui->comboBox_languages->addItem("English", "en_US");
    ui->comboBox_languages->addItem("Deutsch", "de_DE");
    ui->comboBox_languages->addItem("中国人", "zh_CN");

}

eccrypt::~eccrypt()
{
    delete ui;
}

QString eccrypt::generateRandomPassword() {
    // Şifre için kullanılacak karakterler
    const QString possibleCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                       "abcdefghijklmnopqrstuvwxyz"
                                       "abcdefghijklmnopqrstuvwxyz"
                                       "abcdefghijklmnopqrstuvwxyz"
                                       "0123456789"
                                       "0123456789"
                                       "0123456789"
                                       "0123456789"
                                       "0123456789"
                                       "0123456789"
                                       "!@#$%^&*()_+-=[]{}|;:,.<>?";

    QString randomPassword;
    randomPassword.reserve(10);

    for (int i = 0; i < 10; ++i) {
        int index = QRandomGenerator::global()->bounded(possibleCharacters.size());
        randomPassword.append(possibleCharacters.at(index));
    }

    return randomPassword;
}


void eccrypt::on_pushButton_add_clicked()
{
    // Dosya seçim penceresi aç
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        QObject::tr("Dosyaları Seç"),
        "",
        QObject::tr("Tüm Dosyalar (*.*)")); // İzin verilen dosya türleri

    // Kullanıcı seçim yapmadıysa çık
    if (fileNames.isEmpty()) {
        return;
    }

    // Daha önce eklenmiş dosyaları kontrol etmek için
    QStringList existingFiles;
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        existingFiles << item->data(Qt::UserRole).toString(); // Dosya yolunu kontrol et
    }

    // Seçilen dosyaları listeye ekle
    for (const QString &filePath : fileNames) {
        QFileInfo fileInfo(filePath);

        if (!fileInfo.exists() || !fileInfo.isFile()) {
            QMessageBox::warning(this, QObject::tr("Geçersiz Dosya"), QObject::tr("Dosya geçersiz veya mevcut değil: %1").arg(filePath));
            continue;
        }

        if (existingFiles.contains(filePath)) {
            QMessageBox::information(this, QObject::tr("Dosya Zaten Eklenmiş"), QObject::tr("Bu dosya zaten listede: %1").arg(filePath));
            continue;
        }

        // Dosya simgesini ve adını al
        QFileIconProvider iconProvider;
        QIcon fileIcon = iconProvider.icon(fileInfo);
        QString fileName = fileInfo.fileName();

        // QListWidgetItem oluştur ve ekle
        QListWidgetItem *item = new QListWidgetItem(fileIcon, fileName);
        item->setData(Qt::UserRole, filePath); // Dosya yolunu sakla
        ui->listWidget->addItem(item);
    }
}


void eccrypt::on_pushButton_remove_clicked()
{
    // Seçili öğeleri al
    QList<QListWidgetItem *> selectedItems = ui->listWidget->selectedItems();

    // Her seçili öğe üzerinde iterasyon yaparak silme işlemi
    for (QListWidgetItem *item : selectedItems) {
        delete ui->listWidget->takeItem(ui->listWidget->row(item)); // Öğeyi sil
    }
}


void eccrypt::on_pushButton_encryption_clicked()
{
    QString password = ui->lineEdit_password->text();
    if (password.isEmpty()) {
        QMessageBox::warning(this, QObject::tr("Hata"), QObject::tr("Lütfen bir parola girin."));
        return;
    }

    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        QString filePath = item->data(Qt::UserRole).toString();
        QString outputFilePath = filePath + ".enc";  // Çıktı dosyasının adını şifreli olarak belirleyin

        QFile inputFile(filePath);
        if (!inputFile.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, QObject::tr("Hata"), QString(QObject::tr("%1 dosyası açılamadı.")).arg(filePath));
            continue;
        }

        inputFile.close();

        try {
            if (encryption::encryptFile(filePath, outputFilePath, password)) {
                QMessageBox::information(this, QObject::tr("Başarılı"), QString(QObject::tr("%1 dosyası başarıyla şifrelendi.")).arg(filePath));
            } else {
                throw std::runtime_error("Şifreleme işlemi başarısız oldu.");
            }
        } catch (const std::runtime_error &e) {
            QMessageBox::critical(this, QObject::tr("Hata"), e.what());
        }
    }
}


void eccrypt::on_pushButton_decryption_clicked()
{
    QString password = ui->lineEdit_password->text();
    if (password.isEmpty()) {
        QMessageBox::warning(this, QObject::tr("Hata"), QObject::tr("Lütfen bir parola girin."));
        return;
    }

    QStringList filePaths;
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        QString filePath = item->data(Qt::UserRole).toString();
        filePaths << filePath;
    }

    for (const QString &filePath : filePaths) {

        QString modifiedFileName = filePath;
        modifiedFileName.chop(4);  // Son 4 karakteri siler
        QString outputFilePath = modifiedFileName;  // Çıktı dosyasının adını şifreli olarak belirleyin

        QFile inputFile(filePath);
        if (!inputFile.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, QObject::tr("Hata"), QString(QObject::tr("%1 dosyası açılamadı.")).arg(filePath));
            continue;
        }

        inputFile.close();

        try {
            if (encryption::decryptFile(filePath, outputFilePath, password)) {
                QMessageBox::information(this, QObject::tr("Başarılı"), QString(QObject::tr("%1 dosyası başarıyla çözüldü.")).arg(filePath));
            } else {
                throw std::runtime_error("Şifre çözme işlemi başarısız oldu.");
            }
        } catch (const std::runtime_error &e) {
            QMessageBox::critical(this, QObject::tr("Hata"), e.what());
        }
    }
}


void eccrypt::on_pushButton_random_clicked()
{
    QString password = generateRandomPassword();
    ui->lineEdit_password->setText(password);
}


void eccrypt::on_comboBox_languages_currentIndexChanged(int index)
{
    QString language_code = ui->comboBox_languages->currentData().toString();

    // Önceki çeviriyi kaldır
    if (previousTranslator != nullptr) {
        QApplication::instance()->removeTranslator(previousTranslator);
        delete previousTranslator;  // Belleği serbest bırak
    }

    // Yeni çeviriyi yükle
    QTranslator *translator = new QTranslator();
    if (translator->load(":/languages/ECCrypt_" + language_code + ".qm")) {
        QApplication::instance()->installTranslator(translator);
        previousTranslator = translator;  // Yeni çeviriyi previousTranslator olarak kaydet

    }

    ui->retranslateUi(this);  // UI'yi yeniden çevir
}

