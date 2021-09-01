QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QUAZIP_STATIC

win32:INCLUDEPATH += C:/OpenSSL-Win64/include
win32:LIBS += -LC:/OpenSSL-Win64/lib/ -llibcrypto

unix:LIBS += /usr/lib/x86_64-linux-gnu/libz.a
unix:LIBS += /usr/lib/x86_64-linux-gnu/libcrypto.a

SOURCES += \
    archivewindow.cpp \
    createcontainerwindow.cpp \
    directorysizecalculator.cpp \
    fractalcryptcore.cpp \
    main.cpp \
    mainwindow.cpp \
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
    quazip/zip.c \
    quazipfunctions.cpp \
    resizecontainerwindow.cpp \
    startwindow.cpp

HEADERS += \
    archivewindow.h \
    createcontainerwindow.h \
    directorysizecalculator.h \
    fractalcryptcore.h \
    mainwindow.h \
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
    quazip/zip.h \
    quazipfunctions.h \
    resizecontainerwindow.h \
    startwindow.h

FORMS += \
    archivewindow.ui \
    createcontainerwindow.ui \
    mainwindow.ui \
    resizecontainerwindow.ui \
    startwindow.ui
	
unix:LIBS += -ldl

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
