QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17



INCLUDEPATH += C:/msys64/mingw64/include
LIBS += -LC:/msys64/mingw64/lib -lssl -lcrypto


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/encryption.cpp \
    src/main.cpp \
    src/eccrypt.cpp

HEADERS += \
    src/eccrypt.h \
    src/encryption.h

FORMS += \
    src/eccrypt.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


TRANSLATIONS += resources/languages/ECCrypt_de_DE.ts \
                resources/languages/ECCrypt_en_US.ts \
                resources/languages/ECCrypt_tr_TR.ts \
                resources/languages/ECCrypt_zh_CN.ts \


RESOURCES += \
    resources/images.qrc \
    resources/languages.qrc
