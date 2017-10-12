#-------------------------------------------------
#
# Project created by QtCreator 2015-09-23T11:25:18
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = yrmv_automator
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += WITH_PT

# Multiview or MV_PTC on HumanEva:
#SOURCES += main_he.cpp

# SOLO use for HumanEva(original Y&R)
#SOURCES += main_soloHE.cpp

# MV or MVPTC for UMPM
SOURCES += main_umpm.cpp

# SOLO use for UMPM
#SOURCES += main_soloUMPM.cpp



HEADERS +=
