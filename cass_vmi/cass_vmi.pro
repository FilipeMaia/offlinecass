# Copyright (C) 2009 Jochen Küpper
#modified Copyright (C) 2009 N Coppola
CONFIG += static 
QT -= core gui
TEMPLATE = lib
TARGET = cass_vmi

DEFINES += CASS_VMI_LIBRARY

VERSION = 0.0.1

OBJECTS_DIR = ./obj


SOURCES += vmi_analysis.cpp \
           vmi_converter.cpp \
           ./classes/event/vmi_event.cpp \

HEADERS += ../cass/analysis_backend.h \
           ../cass/parameter_backend.h \
           ../cass/conversion_backend.h \
           vmi_analysis.h \
           vmi_converter.h \
           cass_vmi.h \
           ./classes/event/vmi_event.h \

INCLUDEPATH += ../LCLS \
               ../cass \
               ./classes/event \




INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_vmi.a
INSTALLS      += header libs
