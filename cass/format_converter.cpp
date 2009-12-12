// Copyright (C) 2009 Jochen Küpper
#include <iostream>
#include <iomanip>
#include <fstream>
#include "format_converter.h"
#include "remi_converter.h"
#include "vmi_converter.h"
#include "machine_converter.h"
#include "pnccd_converter.h"
#include "xtciterator.h"
#include "pdsdata/xtc/Dgram.hh"


cass::FormatConverter::FormatConverter()
{
    // create all the necessary individual format converters
  _converters[REMI]        = new cass::REMI::Converter();
  _converters[Pulnix]      = new cass::VMI::Converter();
  _converters[pnCCD]       = new cass::pnCCD::Converter();
  _converters[MachineData] = new cass::MachineData::Converter();
}

cass::FormatConverter::~FormatConverter()
{
    // destruct all the individual format converters
  for (std::map<Converters, ConversionBackend *>::iterator it=_converters.begin() ; it != _converters.end(); ++it )
    delete (it->second);
}


  //this slot is called once the eventqueue has new data available//
bool cass::FormatConverter::processDatagram(cass::CASSEvent *cassevent)
{
  bool retval = false;
  Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(cassevent->datagrambuffer());
  
  //tell what kind of transition you got
  std::cout << "transition \""<< Pds::TransitionId::name(datagram->seq.service())<< "\" ";
  std::cout << "0x"<< std::hex<< datagram->xtc.sizeofPayload()<<std::dec<<"  ";
  std::cout << "0x"<< std::hex<< datagram->xtc.damage.value()<<std::dec<<" ";
  std::cout << "0x"<< std::hex<< datagram->seq.clock().seconds()<<" ";
  std::cout << "0x"<< std::hex<< static_cast<uint32_t>(datagram->seq.stamp().fiducials())<<" ";
  std::cout <<std::dec<<std::endl;
  
  //check whether datagram is damaged//
  uint32_t damage = datagram->xtc.damage.value();
  if (1 || !damage)
  {
    //if datagram is configuration or an event (L1Accept) then we will iterate through it//
    //otherwise we ignore the datagram//
    if ((datagram->seq.service() == Pds::TransitionId::Configure) ||
         (datagram->seq.service() == Pds::TransitionId::L1Accept))
    {
      //if the datagram is an event than we create a new cass event first//
      if (datagram->seq.service() == Pds::TransitionId::L1Accept)
      {
        //extract the bunchId from the datagram//
        uint64_t bunchId = datagram->seq.clock().seconds();
        bunchId = (bunchId<<32) + static_cast<uint32_t>(datagram->seq.stamp().fiducials()<<8);

        //set the id of the cassevent//
        cassevent->id() = bunchId;
      }
      //iterate through the datagram and find the wanted information//
      XtcIterator iter(&(datagram->xtc),_converters,cassevent,0);
      iter.iterate();

      //when the datagram was an event then emit the new CASSEvent//
      retval = true;
    }
  }
    
  return retval;
}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
