# Copyright (C) 2009 jk, ncoppola, lmf
qt += core gui
CONFIG += static debug
TEMPLATE = app
TARGET = cass
DEFINES += CASS_LIBRARY
VERSION = 0.0.1
CODECFORTR = UTF-8

SOURCES +=  cass.cpp \
            analyzer.cpp \
            file_input.cpp \
            format_converter.cpp \
            cass_event.cpp \
            xtciterator.cpp \
            ratemeter.cpp \
            dialog.cpp \
            worker.cpp \
            post_processor.cpp

HEADERS +=  analysis_backend.h \
            analyzer.h \
            conversion_backend.h \
            file_input.h \
            format_converter.h \
            cass.h \
            cass_event.h \
            xtciterator.h \
            parameter_backend.h \
            ratemeter.h \
            dialog.h \
            ringbuffer.h \
            worker.h \
            post_processor.h

INCLUDEPATH +=  ./ \
                $$(LCLSSYSINCLUDE) \
                ../cass_remi \
                ../cass_remi/classes/event \
                ../cass_remi/classes/event/channel \
                ../cass_remi/classes/event/peak \
                ../cass_remi/classes/event/detector \
                ../cass_remi/classes/detektorhitsorter \
                ../cass_remi/classes/waveformanalyzer \
                ../cass_vmi \
                ../cass_vmi/classes/event \
                ../cass_pnccd \
                ../cass_pnccd/classes/event \
                ../cass_pnccd/classes/event/pnccd_detector \
                ../cass_pnccd/pnccd_lib \
                ../cass_machinedata \
                ../cass_machinedata/classes/event \
                ../../../barty/hdf5/include



unix{
QMAKE_LFLAGS += -Wl,-rpath,$$(LCLSSYSLIB)
LIBS += -L../cass_remi -lcass_remi \
        -L../cass_pnccd -lcass_pnccd \
        -L../cass_vmi -lcass_vmi \
        -L../cass_machinedata -lcass_machinedata \
        -L../../../barty/hdf5/lib -lhdf5 \
        -L$$(LCLSSYSLIB) -lacqdata -lxtcdata -lpulnixdata -lcamdata -lpnccddata \

TARGETDEPS +=   ../cass_remi/libcass_remi.a \
                ../cass_pnccd/libcass_pnccd.a \
                ../cass_vmi/libcass_vmi.a \
               ../cass_machinedata/libcass_machinedata.a \
}


INSTALLBASE = /usr/local/cass
bin.path = $$INSTALLBASE/bin
header.path = $$INSTALLBASE/include
libs.path = $$INSTALLBASE/libs
bin.files = cass.app
header.files = $$HEADERS
libs.files = libcass.a

# INSTALLS += bin
INSTALLS += header \
    libs \
    bin
