
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

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
#include "pdsdata/epics/EpicsPvData.hh"
#include "pdsdata/epics/EpicsXtcSettings.hh"
#include "pdsdata/bld/bldData.hh"

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
  void process(const DetInfo&, const EpicsPvHeader& epicsPv)
  {    
    printf("*** Processing Epics object\n");
    epicsPv.printPv();
    printf( "\n" );
  }    
  void process(const DetInfo&, const BldDataFEEGasDetEnergy& bldData) {
    printf("*** Processing FEEGasDetEnergy object\n");
    bldData.print();
    printf( "\n" );    
  }  
  void process(const DetInfo&, const BldDataEBeamV0& bldData) {
    printf("*** Processing EBeamV0 object\n");
    bldData.print();
    printf( "\n" );    
  }  
  void process(const DetInfo&, const BldDataEBeam& bldData) {
    printf("*** Processing EBeam object\n");
    bldData.print();
    printf( "\n" );    
  }  
  void process(const DetInfo&, const BldDataPhaseCavity& bldData) {
    printf("*** Processing PhaseCavity object\n");
    bldData.print();
    printf( "\n" );    
  }  
  int process(Xtc* xtc) {
    unsigned i=_depth; while (i--) printf("  ");
    Level::Type level = xtc->src.level();
    printf("%s level contains: %s: ",Level::name(level), TypeId::name(xtc->contains.id()));
    const DetInfo& info = *(DetInfo*)(&xtc->src);
    if (level==Level::Source) {
      printf("%s,%d  %s,%d\n",
             DetInfo::name(info.detector()),info.detId(),
             DetInfo::name(info.device()),info.devId());
    } else {
      ProcInfo& info = *(ProcInfo*)(&xtc->src);
      printf("IpAddress 0x%x ProcessId 0x%x\n",info.ipAddr(),info.processId());
    }
    if (level < 0 || level >= Level::NumberOfLevels )
    {
        printf("Unsupported Level %d\n", (int) level);
        return Continue;
    }    
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
      break;      
    }
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
    case (TypeId::Id_Epics) :      
    {
      int iVersion = xtc->contains.version();
      if ( iVersion != EpicsXtcSettings::iXtcVersion ) 
      {
          printf( "Xtc Epics version (%d) is not compatible with reader supported version (%d)", iVersion, EpicsXtcSettings::iXtcVersion );
          break;
      }
      process(info, *(const EpicsPvHeader*)(xtc->payload()));
      break;
    }            
    case (TypeId::Id_FEEGasDetEnergy) :
    {
      process(info, *(const BldDataFEEGasDetEnergy*) xtc->payload() );
      break;        
    }
    case (TypeId::Id_EBeam) :
    {
      switch(xtc->contains.version()) {
      case 0:
	process(info, *(const BldDataEBeamV0*) xtc->payload() );
	break; 
      case 1:
	process(info, *(const BldDataEBeam*) xtc->payload() );
	break; 
      default:
	break;
      }       
    }    
    case (TypeId::Id_PhaseCavity) :
    {
      process(info, *(const BldDataPhaseCavity*) xtc->payload() );
      break;        
    }
    default :
      printf("Unsupported TypeId %d\n", (int) xtc->contains.id());
      break;
    }
    return Continue;
  }
private:
  unsigned _depth;
};

void usage(char* progname) {
  fprintf(stderr,"Usage: %s -f <filename> [-h]\n", progname);
}

int main(int argc, char* argv[]) {
  int c;
  char* xtcname=0;
  int parseErr = 0;

  while ((c = getopt(argc, argv, "hf:")) != -1) {
    switch (c) {
    case 'h':
      usage(argv[0]);
      exit(0);
    case 'f':
      xtcname = optarg;
      break;
    default:
      parseErr++;
    }
  }
  
  if (!xtcname) {
    usage(argv[0]);
    exit(2);
  }

  FILE* file = fopen(xtcname,"r");
  if (!file) {
    perror("Unable to open file %s\n");
    exit(2);
  }

  XtcFileIterator iter(file,0x900000);
  Dgram* dg;
  while ((dg = iter.next())) {
    printf("%s transition: time 0x%x/0x%x, payloadSize 0x%x\n",TransitionId::name(dg->seq.service()),
           dg->seq.stamp().fiducials(),dg->seq.stamp().ticks(),dg->xtc.sizeofPayload());
    myLevelIter iter(&(dg->xtc),0);
    iter.iterate();
  }

  fclose(file);
  return 0;
}
