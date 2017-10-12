QT += core
QT -= gui

TARGET = ReformatGrountTruth
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../Calc2DError/

SOURCES += main.cpp \
    ../Calc2DError/groundtruth.cpp \
    ../Calc2DError/he_pose.cpp

HEADERS += \
    ../Calc2DError/groundtruth.h \
    ../Calc2DError/he_pose.h

