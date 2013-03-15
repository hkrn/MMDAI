QT += core
QT -= gui

TARGET = cglint
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

mac:LIBS += -framework Cg
!mac:LIBS += -lCg -lCgGL

SOURCES += main.cc
