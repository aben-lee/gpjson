TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


SOURCES += \
    main.cpp \
    source/gpjson.cpp \
    source/json.cpp

HEADERS += \
    source/gpjson.h \
    source/json.h
