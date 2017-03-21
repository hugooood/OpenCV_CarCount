#-------------------------------------------------
#
# Project created by QtCreator 2016-07-11T21:14:18
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = car_count
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp \
    block.cpp



INCLUDEPATH+=D:\opencv\opencv\build\include\opencv\
             D:\opencv\opencv\build\include\opencv2\
             D:\opencv\opencv\build\include\
             D:\opencv\opencv\build\x86\mingw\bin


LIBS+=D:\opencv\opencv\build\x86\mingw\lib\libopencv_calib3d248.dll.a\
  D:\opencv\opencv\build\x86\mingw\lib\libopencv_contrib248.dll.a\
  D:\opencv\opencv\build\x86\mingw\lib\libopencv_core248.dll.a\
  D:\opencv\opencv\build\x86\mingw\lib\libopencv_features2d248.dll.a\
  D:\opencv\opencv\build\x86\mingw\lib\libopencv_flann248.dll.a\
  D:\opencv\opencv\build\x86\mingw\lib\libopencv_gpu248.dll.a\
  D:\opencv\opencv\build\x86\mingw\lib\libopencv_highgui248.dll.a\
  D:\opencv\opencv\build\x86\mingw\lib\libopencv_imgproc248.dll.a\
  D:\opencv\opencv\build\x86\mingw\lib\libopencv_legacy248.dll.a\
  D:\opencv\opencv\build\x86\mingw\lib\libopencv_ml248.dll.a\
  D:\opencv\opencv\build\x86\mingw\lib\libopencv_objdetect248.dll.a\
  D:\opencv\opencv\build\x86\mingw\lib\libopencv_video248.dll.a\


HEADERS += \
    block.h
