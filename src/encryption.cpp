// encryption.cpp
#include "encryption.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QDataStream>
#include <QString>
#include <QObject>

// Şifreleme işlevi
bool encryption::encryptFile(const QString& inputFilePath, const QString& outputFilePath, const QString& password) {
    const int keySize = 32;
    const int ivSize = 16;
    const int saltSize = 16;

    // Rastgele salt oluştur
    unsigned char salt[saltSize];
    if (!RAND_bytes(salt, saltSize)) {
        qWarning() << QObject::tr("Salt oluşturulamadı.");
        return false;
    }

    // Key ve IV oluştur
    unsigned char key[keySize];
    unsigned char iv[ivSize];
    if (!PKCS5_PBKDF2_HMAC(password.toUtf8().data(), password.size(), salt, saltSize, 10000, EVP_sha256(), keySize, key)) {
        qWarning() << QObject::tr("Anahtar türetme başarısız oldu.");
        return false;
    }
    if (!RAND_bytes(iv, ivSize)) {
        qWarning() << QObject::tr("IV oluşturulamadı.");
        return false;
    }

    QFile inputFile(inputFilePath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        qWarning() << QObject::tr("Girdi dosyası açılamadı: %1").arg(inputFilePath);
        return false;
    }

    QFile outputFile(outputFilePath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        qWarning() << QObject::tr("Çıktı dosyası açılamadı: %1").arg(outputFilePath);
        return false;
    }

    // Salt ve IV'yi çıktı dosyasına yaz
    outputFile.write(reinterpret_cast<char*>(salt), saltSize);
    outputFile.write(reinterpret_cast<char*>(iv), ivSize);

    // Şifreleme bağlamı
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning() << QObject::tr("Şifreleme bağlamı oluşturulamadı.");
        return false;
    }

    if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv)) {
        qWarning() << QObject::tr("Şifreleme başlatılamadı.");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    const size_t bufferSize = 4096;
    QByteArray buffer(bufferSize, 0);
    QByteArray encryptedBuffer(bufferSize + EVP_CIPHER_block_size(EVP_aes_256_cbc()), 0);

    int encryptedLength = 0;
    while (!inputFile.atEnd()) {
        qint64 bytesRead = inputFile.read(buffer.data(), bufferSize);

        if (!EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(encryptedBuffer.data()), &encryptedLength,
                               reinterpret_cast<const unsigned char*>(buffer.data()), bytesRead)) {
            qWarning() << QObject::tr("Şifreleme sırasında hata oluştu.");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        outputFile.write(encryptedBuffer.left(encryptedLength));
    }

    if (!EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(encryptedBuffer.data()), &encryptedLength)) {
        qWarning() << QObject::tr("Şifreleme tamamlanırken hata oluştu.");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    outputFile.write(encryptedBuffer.left(encryptedLength));

    EVP_CIPHER_CTX_free(ctx);
    inputFile.close();
    outputFile.close();

    qInfo() << QObject::tr("Dosya başarıyla şifrelendi: %1").arg(outputFilePath);
    return true;
}


// Şifre çözme işlevi
bool encryption::decryptFile(const QString& inputFilePath, const QString& outputFilePath, const QString& password) {
    const int keySize = 32;
    const int ivSize = 16;
    const int saltSize = 16;

    QFile inputFile(inputFilePath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        qWarning() << QObject::tr("Girdi dosyası açılamadı: %1").arg(inputFilePath);
        return false;
    }

    QFile outputFile(outputFilePath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        qWarning() << QObject::tr("Çıktı dosyası açılamadı: %1").arg(outputFilePath);
        return false;
    }

    // Salt ve IV'yi dosyadan oku
    unsigned char salt[saltSize];
    unsigned char iv[ivSize];
    inputFile.read(reinterpret_cast<char*>(salt), saltSize);
    inputFile.read(reinterpret_cast<char*>(iv), ivSize);

    // Key oluştur
    unsigned char key[keySize];
    if (!PKCS5_PBKDF2_HMAC(password.toUtf8().data(), password.size(), salt, saltSize, 10000, EVP_sha256(), keySize, key)) {
        qWarning() << QObject::tr("Anahtar türetme başarısız oldu.");
        return false;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning() << QObject::tr("Şifre çözme bağlamı oluşturulamadı.");
        return false;
    }

    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv)) {
        qWarning() << QObject::tr("Şifre çözme başlatılamadı.");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    const size_t bufferSize = 4096;
    QByteArray buffer(bufferSize, 0);
    QByteArray decryptedBuffer(bufferSize + EVP_CIPHER_block_size(EVP_aes_256_cbc()), 0);

    int decryptedLength = 0;
    while (!inputFile.atEnd()) {
        qint64 bytesRead = inputFile.read(buffer.data(), bufferSize);

        if (!EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(decryptedBuffer.data()), &decryptedLength,
                               reinterpret_cast<const unsigned char*>(buffer.data()), bytesRead)) {
            qWarning() << QObject::tr("Şifre çözme sırasında hata oluştu.");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        outputFile.write(decryptedBuffer.left(decryptedLength));
    }

    if (!EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(decryptedBuffer.data()), &decryptedLength)) {
        qWarning() << QObject::tr("Şifre çözme tamamlanırken hata oluştu. Muhtemelen parola yanlış.");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    outputFile.write(decryptedBuffer.left(decryptedLength));

    EVP_CIPHER_CTX_free(ctx);
    inputFile.close();
    outputFile.close();

    qInfo() << QObject::tr("Dosya başarıyla çözüldü: %1").arg(outputFilePath);
    return true;
}

