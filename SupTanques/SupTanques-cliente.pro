QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../MySocket/mysocket.cpp \
    main.cpp \
    suptanques_img.cpp \
    suptanques_login.cpp \
    suptanques_main.cpp

HEADERS += \
    ../MySocket/mysocket.h \
    suptanques.h \
    suptanques_img.h \
    suptanques_login.h \
    suptanques_main.h \
    tanques-param.h

FORMS += \
    suptanques_login.ui \
    suptanques_main.ui

LIBS   += \
    -lWs2_32

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
