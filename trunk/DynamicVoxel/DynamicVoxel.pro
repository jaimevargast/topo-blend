load($$[STARLAB])
load($$[SURFACEMESH])
load($$[CHOLMOD])
load($$[EIGEN])

TEMPLATE = lib
CONFIG += staticlib
QT += opengl

# Library name and destination
TARGET = DynamicVoxel
DESTDIR = $$PWD/lib

SOURCES += DynamicVoxel.cpp
HEADERS += Voxel.h DynamicVoxel.h DoubleTupleMap.h
