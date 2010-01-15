#tgtnames = xtcreader xtcmonserver xtcmonclient xtcmonclientexample acqconfig agatfile
tgtnames = xtcreader xtcmonserver xtcmonclient xtcmonclientexample xtcEpicsReaderTest dmgreader bldreader

#CXXFLAGS += -pthread -m32 -I/reg/g/pcds/package/root/include

#LXFLAGS += -L/reg/g/pcds/package/root/lib -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -pthread -lm -ldl -rdynamic

ifneq ($(findstring x86_64-linux,$(tgt_arch)),)
syslibdir := /usr/lib64
else
syslibdir := /usr/lib
endif

tgtsrcs_xtcreader := xtcreader.cc
tgtlibs_xtcreader := pdsdata/xtcdata pdsdata/acqdata pdsdata/epics pdsdata/bld
tgtslib_xtcreader := $(syslibdir)/rt

tgtsrcs_dmgreader := dmgreader.cc
tgtlibs_dmgreader := pdsdata/xtcdata pdsdata/acqdata pdsdata/epics pdsdata/bld
tgtslib_dmgreader := $(syslibdir)/rt

tgtsrcs_bldreader := bldreader.cc
tgtlibs_bldreader := pdsdata/xtcdata pdsdata/bld
tgtslib_bldreader := $(syslibdir)/rt

tgtsrcs_agatfile := agatfile.cc
tgtlibs_agatfile := pdsdata/xtcdata pdsdata/acqdata 
tgtslib_agatfile := $(syslibdir)/rt

tgtsrcs_acqconfig := acqconfig.cc
tgtlibs_acqconfig := pdsdata/xtcdata pdsdata/acqdata

tgtsrcs_xtcmonserver := xtcmonserver.cc
tgtlibs_xtcmonserver := pdsdata/xtcdata pdsdata/acqdata
tgtslib_xtcmonserver := $(syslibdir)/usr/lib/rt

tgtsrcs_xtcmonclient := xtcmonclient.cc XtcMonitorClient.cc
tgtlibs_xtcmonclient := pdsdata/xtcdata pdsdata/acqdata
tgtslib_xtcmonclient := $(syslibdir)/usr/lib/rt

tgtsrcs_xtcmonclientexample := xtcMonClientExample.cc XtcMonitorClient.cc
tgtlibs_xtcmonclientexample := pdsdata/pnccddata pdsdata/xtcdata pdsdata/acqdata
tgtslib_xtcmonclientexample := $(syslibdir)/usr/lib/rt

tgtsrcs_agatfile := agatfile.cc
tgtlibs_agatfile := pdsdata/xtcdata pdsdata/acqdata
tgtslib_agatfile := $(syslibdir)/rt

tgtsrcs_xtcEpicsReaderTest := xtcEpicsReaderTest.cc XtcEpicsFileReader.cc XtcEpicsFileReader.hh XtcEpicsIterator.cc XtcEpicsIterator.hh
tgtincs_xtcEpicsReaderTest := 
tgtlibs_xtcEpicsReaderTest := pdsdata/xtcdata pdsdata/epics
tgtslib_xtcEpicsReaderTest := 
