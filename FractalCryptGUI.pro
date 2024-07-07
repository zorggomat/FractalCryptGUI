######################################################################
# Automatically generated by qmake (3.1) Sun Jul 7 16:46:33 2024
######################################################################

TEMPLATE = app
TARGET = FractalCryptGUI
INCLUDEPATH += .

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += ui_archivewindow.h \
           ui_createcontainerwindow.h \
           ui_mainwindow.h \
           ui_progressdialog.h \
           ui_resizecontainerwindow.h \
           ui_startwindow.h \
           core/aes.h \
           core/argon2.h \
           core/directorysizecalculator.h \
           core/fractalcryptcore.h \
           core/noizecreator.hpp \
           core/quazipfunctions.h \
           quazip/crypt.h \
           quazip/ioapi.h \
           quazip/JlCompress.h \
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
           widgets/archivewindow.h \
           widgets/createcontainerwindow.h \
           widgets/mainwindow.h \
           widgets/progressdialog.h \
           widgets/resizecontainerwindow.h \
           widgets/startwindow.h
FORMS += ui/archivewindow.ui \
         ui/createcontainerwindow.ui \
         ui/mainwindow.ui \
         ui/progressdialog.ui \
         ui/resizecontainerwindow.ui \
         ui/startwindow.ui
SOURCES += main.cpp \
           core/aes.cpp \
           core/directorysizecalculator.cpp \
           core/fractalcryptcore.cpp \
           core/quazipfunctions.cpp \
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
           widgets/archivewindow.cpp \
           widgets/createcontainerwindow.cpp \
           widgets/mainwindow.cpp \
           widgets/progressdialog.cpp \
           widgets/resizecontainerwindow.cpp \
           widgets/startwindow.cpp
RESOURCES += resources.qrc
