# Copyright (C) 2009 Jochen Küpper,lmf

CONFIG += static 
QT -= core gui
TEMPLATE = lib
TARGET = cass_machinedata
DEFINES += CASS_MACHINEDATA_LIBRARY
VERSION = 0.0.1


SOURCES += machine_analysis.cpp \
           machine_converter.cpp \
           ./classes/event/machine_event.cpp \


HEADERS += ../cass/analysis_backend.h \
           ../cass/parameter_backend.h \
           ../cass/conversion_backend.h \
           machine_analysis.h \
           machine_converter.h \
           cass_machine.h \
           ./classes/event/machine_event.h \

INCLUDEPATH += ../LCLS \
               ../cass \
               ./classes/event \




INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_machinedata.a
INSTALLS      += header libs
