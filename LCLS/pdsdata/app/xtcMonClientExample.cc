#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/ProcInfo.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/XtcFileIterator.hh"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include "pdsdata/camera/FrameV1.hh"
#include "pdsdata/camera/FrameFexConfigV1.hh"
#include "pdsdata/camera/TwoDGaussianV1.hh"
#include "pdsdata/evr/ConfigV1.hh"
#include "pdsdata/opal1k/ConfigV1.hh"
#include "pdsdata/pnCCD/FrameV1.hh"
#include "pdsdata/pnCCD/ConfigV1.hh"

#include "XtcMonitorClient.hh"

static PNCCD::ConfigV1 cfg;

class myLevelIter : public XtcIterator {
public:
  enum {Stop, Continue};
  myLevelIter(Xtc* xtc, unsigned depth) : XtcIterator(xtc), _depth(depth) {}

  void process(const DetInfo& d, const Camera::FrameV1& f) {
    printf("*** Processing frame object\n");
  }
  void process(const DetInfo&, const Acqiris::DataDescV1&) {
    printf("*** Processing acqiris data object\n");
  }
  void process(const DetInfo&, const Acqiris::ConfigV1&) {
    printf("*** Processing Acqiris config object\n");
  }
  void process(const DetInfo&, const Opal1k::ConfigV1&) {
    printf("*** Processing Opal1000 config object\n");
  }
  void process(const DetInfo&, const Camera::FrameFexConfigV1&) {
    printf("*** Processing frame feature extraction config object\n");
  }
  void process(const DetInfo&, const EvrData::ConfigV1&) {
    printf("*** Processing EVR config object\n");
  }
  void process(const DetInfo&, const Camera::TwoDGaussianV1& o) {
    printf("*** Processing 2DGauss object\n");
  }
  void process(const DetInfo&, const PNCCD::ConfigV1& config) {
    cfg = config;
    printf("*** Processing pnCCD config.  Number of Links: %d, PayloadSize per Link: %d\n",
           cfg.numLinks(),cfg.payloadSizePerLink());
  }
//   void process(const DetInfo& di, PnccdFrameHeaderType* frh) {
//     enum {numberOfLinks=4, payloadPerLink=(1<<19)+16};
//     uint8_t* pb = reinterpret_cast<uint8_t*>(frh);
//     PnccdFrameHeaderType* fp;
//     for (uint32_t i=0; i<numberOfLinks; i++) {
//       fp = reinterpret_cast<PnccdFrameHeaderType*>(pb);
//       printf("\tpnCCD frame: %08X %08X %08X %08X\n", fp->specialWord, fp->frameNumber,
//           fp->TimeStampHi, fp->TimeStampLo);
//       unsigned* pu = (unsigned*)(fp+1);
//       printf("\tdata begins: %08X %08X %08X %08X %08X\n", pu[0], pu[1], pu[2], pu[3], pu[4]);
//       pb += payloadPerLink;
//     }
//   }
  void process(const DetInfo& d, const PNCCD::FrameV1& f) {
    for (unsigned i=0;i<cfg.numLinks();i++) {
      printf("*** Processing pnCCD frame number %x segment %d\n",f.frameNumber(),i);
      printf("\tpnCCD frameHeader: %08X, %08X, %08X, %08X\n", f.specialWord(), f.frameNumber(),
          f.timeStampHi(), f.timeStampLo());
      const uint16_t* data = f.data();
      unsigned last  = f.sizeofData(cfg); 
      printf("First data words: 0x%4.4x 0x%4.4x\n",data[0],data[1]);
      printf("Last  data words: 0x%4.4x 0x%4.4x\n",data[last-2],data[last-1]);
    }
  }
  int process(Xtc* xtc) {
    unsigned i=_depth; while (i--) printf("  ");
    Level::Type level = xtc->src.level();
    printf("%s level contains: %s: ",Level::name(level), TypeId::name(xtc->contains.id()));
    const DetInfo& info = *(DetInfo*)(&xtc->src);
    if (level==Level::Source) {
      printf("%s %d %s %d",
             DetInfo::name(info.detector()),info.detId(),
             DetInfo::name(info.device()),info.devId());
    } else {
      ProcInfo& info = *(ProcInfo*)(&xtc->src);
      printf("IpAddress 0x%x ProcessId 0x%x",info.ipAddr(),info.processId());
    }
    if (xtc->damage.value()) {
      printf(", damage 0x%x", xtc->damage.value());
    }
    printf("\n");
    switch (xtc->contains.id()) {
    case (TypeId::Id_Xtc) : {
      myLevelIter iter(xtc,_depth+1);
      iter.iterate();
      break;
    }
    case (TypeId::Id_Frame) :
      process(info, *(const Camera::FrameV1*)(xtc->payload()));
      break;
    case (TypeId::Id_AcqWaveform) :
      process(info, *(const Acqiris::DataDescV1*)(xtc->payload()));
      break;
    case (TypeId::Id_AcqConfig) :
      {
        unsigned version = xtc->contains.version();
        switch (version) {
        case 1:
          process(info,*(const Acqiris::ConfigV1*)(xtc->payload()));
          break;
        default:
          printf("Unsupported acqiris configuration version %d\n",version);
          break;
        }
      }
      break;
    case (TypeId::Id_TwoDGaussian) :
      process(info, *(const Camera::TwoDGaussianV1*)(xtc->payload()));
      break;
    case (TypeId::Id_Opal1kConfig) :
      process(info, *(const Opal1k::ConfigV1*)(xtc->payload()));
      break;
    case (TypeId::Id_FrameFexConfig) :
      process(info, *(const Camera::FrameFexConfigV1*)(xtc->payload()));
      break;
    case (TypeId::Id_EvrConfig) :
      process(info, *(const EvrData::ConfigV1*)(xtc->payload()));
      break;
    case (TypeId::Id_pnCCDframe) :
      process(info, *(const PNCCD::FrameV1*)(xtc->payload()));
      break;
    case (TypeId::Id_pnCCDconfig) :
      process(info, *(const PNCCD::ConfigV1*)(xtc->payload()));
      break;
//     case (TypeId::Id_pnCCDconfig) :
//       process(info, (fileHeaderType*) xtc->payload());
//       break;
//     case (TypeId::Id_pnCCDframe) :
//       process(info, (PnccdFrameHeaderType*) xtc->payload());
//       break;
    default :
      break;
    }
    return Continue;
  }
private:
  unsigned _depth;
};

class MyXtcMonitorClient : public XtcMonitorClient {
  public:
    MyXtcMonitorClient() {
    }
    virtual void processDgram(Dgram* dg) {
      printf("%s transition: time 0x%x/0x%x, payloadSize 0x%x, damage 0x%x\n",TransitionId::name(dg->seq.service()),
	     dg->seq.stamp().fiducials(),dg->seq.stamp().ticks(),dg->xtc.sizeofPayload(), dg->xtc.damage.value());
      myLevelIter iter(&(dg->xtc),0);
      iter.iterate();
    };
};

void usage(char* progname) {
  fprintf(stderr,"Usage: %s [-t <partitionTag>] [-h]\n", progname);
}

int main(int argc, char* argv[]) {
  int c=0;
  char partitionTag[128] = "";
  MyXtcMonitorClient myClient;

  while ((c = getopt(argc, argv, "?ht:")) != -1) {
    switch (c) {
    case '?':
    case 'h':
      usage(argv[0]);
      exit(0);
    case 't':
      strcpy(partitionTag, optarg);
      // the run method will only return if it encounters an error
      fprintf(stderr, "myClient returned: %d\n", myClient.run(partitionTag));
      break;
    default:
      usage(argv[0]);
    }
  }
  if (c<1) usage(argv[0]);
  return 1;
}
