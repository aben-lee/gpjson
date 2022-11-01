
TEMPLATE = lib
CONFIG += shared
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


SOURCES += \
#    GPJsonTest.cpp \
    source/gpjson.cpp \
    source/json.cpp

HEADERS += \
    source/gpjson.h \
    source/json.h
