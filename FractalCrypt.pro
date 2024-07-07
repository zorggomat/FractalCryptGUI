QT += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QUAZIP_STATIC

win32 {
    INCLUDEPATH += C:/OpenSSL-Win64/include
    LIBS += -LC:/OpenSSL-Win64/lib -llibcrypto
    RC_ICONS = logo.ico
    QMAKE_TARGET_DESCRIPTION = FractalCrypt
}

unix {
    LIBS += -largon2 -ldl -lz -lcrypto
}

SOURCES += \
    main.cpp \
    core/aes.cpp \
    core/directorysizecalculator.cpp \
    core/fractalcryptcore.cpp \
    core/quazipfunctions.cpp \
    widgets/archivewindow.cpp \
    widgets/createcontainerwindow.cpp \
    widgets/mainwindow.cpp \
    widgets/progressdialog.cpp \
    widgets/resizecontainerwindow.cpp \
    widgets/startwindow.cpp \
    quazip/JlCompress.cpp \
    quazip/qioapi.cpp \
    quazip/quaadler32.cpp \
    quazip/quacrc32.cpp \
    quazip/quagzipfile.cpp \
    quazip/quaziodevice.cpp \
    quazip/quazip.cpp \
    quazip/quazipdir.cpp \
    quazip/quazipfile.cpp \
    quazip/quazipfileinfo.cpp \
    quazip/quazipnewinfo.cpp \
    quazip/unzip.c \
    quazip/zip.c

HEADERS += \
    core/aes.h \
    core/argon2.h \
    core/directorysizecalculator.h \
    core/fractalcryptcore.h \
    core/noizecreator.hpp \
    core/quazipfunctions.h \
    widgets/archivewindow.h \
    widgets/createcontainerwindow.h \
    widgets/resizecontainerwindow.h \
    widgets/startwindow.h \
    widgets/mainwindow.h \
    widgets/progressdialog.h \
    quazip/JlCompress.h \
    quazip/crypt.h \
    quazip/ioapi.h \
    quazip/quaadler32.h \
    quazip/quachecksum32.h \
    quazip/quacrc32.h \
    quazip/quagzipfile.h \
    quazip/quaziodevice.h \
    quazip/quazip.h \
    quazip/quazip_global.h \
    quazip/quazipdir.h \
    quazip/quazipfile.h \
    quazip/quazipfileinfo.h \
    quazip/quazipnewinfo.h \
    quazip/unzip.h \
    quazip/zip.h

FORMS += \
    ui/archivewindow.ui \
    ui/createcontainerwindow.ui \
    ui/mainwindow.ui \
    ui/progressdialog.ui \
    ui/resizecontainerwindow.ui \
    ui/startwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
