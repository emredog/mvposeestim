#-------------------------------------------------
#
# Project created by QtCreator 2015-10-06T16:10:41
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = Calc2DMeanError
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

#DEFINES += HUMANEVA
#DEFINES += UMPM


HEADERS += structs.h

#HUMANEVA
#SOURCES += main.cpp \
#           he_pose.cpp \
#           groundtruth.cpp \
#           errorcalculation.cpp
#HEADERS += he_pose.h \
#           groundtruth.h \
#           errorcalculation.h


#UMPM
SOURCES += umpm_pose.cpp \
           groundtruth_umpm.cpp\
           errorcalculation_umpm.cpp \
           main_umpm.cpp
HEADERS += umpm_pose.h \
           groundtruth_umpm.h \
    errorcalculation_umpm.h

