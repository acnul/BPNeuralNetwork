QT += core gui qml quick widgets

CONFIG += c++17

TARGET = MiteClassification

TEMPLATE = app

SOURCES += \
    main.cpp \
    mitenetworkmodel.cpp \
    bpnn.cpp \
    mnist_classifier.cpp \
    mnist_reader.cpp \
    mnistmodel.cpp

HEADERS += \
    mitenetworkmodel.h \
    bpnn.h \
    mnist_classifier.h \
    mnist_reader.h \
    mnistmodel.h

RESOURCES += qml.qrc

# 确保Qt模块正确链接
win32 {
    CONFIG -= console
    CONFIG += windows
    QMAKE_LFLAGS += -Wl,-subsystem,windows
}

# 设置输出目录
DESTDIR = $$PWD/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui

# 图标资源（Windows）
win32: RC_ICONS = logo.ico
