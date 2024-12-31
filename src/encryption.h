#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <QString>
#include <QByteArray>

class encryption
{
public:

    static bool encryptFile(const QString& inputFilePath, const QString& outputFilePath, const QString& password);
    static bool decryptFile(const QString& inputFilePath, const QString& outputFilePath, const QString& password);


    // AES blok boyutu (16 byte)
    static constexpr int AES_BLOCK_SIZE = 16;

};
#endif // ENCRYPTION_H

