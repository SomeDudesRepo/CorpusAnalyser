#-------------------------------------------------
#
# Project created by QtCreator 2014-08-03T16:33:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CorpusAnalyser
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    runner.cpp \
    wordextractror.cpp \
    corpusanalysis.cpp \
    log.cpp \
    analyses.cpp

HEADERS  += mainwindow.h \
    runner.h \
    wordextractor.h \
    corpusanalysis.h \
    types.h \
    log.h \
    analyses.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    corpusanalyser.ico

RC_FILE = CorpusAnalyser.rc

RESOURCES += \
    resources.qrc
