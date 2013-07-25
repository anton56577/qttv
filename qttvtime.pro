QT          += opengl network core gui multimedia widgets
TARGET = qttvtime
TEMPLATE = app
HEADERS = glwidget.h \
    v4l2capiture.h \
    propertiesdialog.h \
    channels.h \
    audiocontrol.h \
    controlwidget.h \
    tvwindow.h \
    tcontroller.h
SOURCES = glwidget.cpp \
    main.cpp \
    v4l2capiture.cpp \
    propertiesdialog.cpp \
    channels.cpp \
    audiocontrol.cpp \
    controlwidget.cpp \
    tvwindow.cpp \
    tcontroller.cpp
FORMS += \
    propertiesdialog.ui \
    controlwidget.ui
INCLUDEPATH += \
    /usr/include \
    /usr/include/i386-linux-gnu \
    /usr/lib/gcc/i686-linux-gnu/4.8/include
LIBS += /usr/lib/i386-linux-gnu/libasound.so \
        /usr/lib/i386-linux-gnu/libX11.so
