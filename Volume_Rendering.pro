#-------------------------------------------------
#
# Project created by QtCreator 2014-10-29T21:43:36
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Volume_Rendering
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    utility.cpp \
    glwidget_tranfunc.cpp \
    transfer_func1d.cpp \
    volumeRender_kernel.cu

HEADERS  += mainwindow.h \
    utility.h \
    glwidget_tranfunc.h \
    transfer_func1d.h \
    Vector3.h \
    helper_cuda.h \
    helper_math.h \
    helper_string.h \
    helper_cuda_gl.h

FORMS    += mainwindow.ui \
    transfer_func1d.ui
