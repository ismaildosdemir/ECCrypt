#ifndef ECCRYPT_H
#define ECCRYPT_H

#include "qmimedata.h"
#include "ui_eccrypt.h"
#include <QMainWindow>
#include <QFrame>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QListWidget>
#include <QFileIconProvider>



namespace Ui {
class eccrypt;
}

class eccrypt : public QMainWindow
{
    Q_OBJECT

public:
    explicit eccrypt(QWidget* parent = nullptr);
    ~eccrypt();
    QString generateRandomPassword();

    QTranslator *previousTranslator = nullptr;  // Önceki çeviri işaretçisi


protected:
    void dragEnterEvent(QDragEnterEvent* event) override {
        // Sürüklenen nesne bir dosya içeriyor mu kontrol et
        if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }


    void dropEvent(QDropEvent *event) override {
        // Sadece dosya URL'lerini kabul et
        if (event->mimeData()->hasUrls()) {
            const QList<QUrl> urls = event->mimeData()->urls();
            for (const QUrl &url : urls) {
                QString filePath = url.toLocalFile();
                QFileInfo fileInfo(filePath);

                if (fileInfo.exists() && fileInfo.isFile()) {
                    // Dosya zaten eklenmiş mi kontrol et
                    if (!isFileAlreadyAdded(filePath)) {
                        // Dosya simgesini ve adını al
                        QFileIconProvider iconProvider;
                        QIcon fileIcon = iconProvider.icon(fileInfo);
                        QString fileName = fileInfo.fileName();

                        // QListWidgetItem oluştur ve ekle
                        QListWidgetItem *item = new QListWidgetItem(fileIcon, fileName);
                        item->setData(Qt::UserRole, filePath); // Dosya yolunu sakla
                        ui->listWidget->addItem(item);

                        qDebug() << "Dosya başarıyla eklendi: " << filePath;
                    } else {
                        qDebug() << "Dosya zaten eklenmiş: " << filePath;
                    }
                } else {
                    qDebug() << "Geçersiz dosya veya klasör: " << filePath;
                }
            }
            event->acceptProposedAction();
        } else {
            qDebug() << "Sürüklenen nesne bir dosya değil.";
            event->ignore();
        }
    }


private slots:



    void on_pushButton_add_clicked();

    void on_pushButton_remove_clicked();

    void on_pushButton_encryption_clicked();

    void on_pushButton_decryption_clicked();

    void on_pushButton_random_clicked();

    void on_comboBox_languages_currentIndexChanged(int index);

private:

    bool isFileAlreadyAdded(const QString &filePath) {
        // Tüm mevcut öğeleri kontrol et
        for (int i = 0; i < ui->listWidget->count(); ++i) {
            QListWidgetItem *item = ui->listWidget->item(i);
            if (item->data(Qt::UserRole).toString() == filePath) {
                return true; // Dosya zaten eklenmiş
            }
        }
        return false; // Dosya henüz eklenmemiş
    }


    Ui::eccrypt *ui;
};

#endif // ECCRYPT_H
