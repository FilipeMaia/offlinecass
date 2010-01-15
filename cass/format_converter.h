// Copyright (C) 2009 Jochen Küpper

#ifndef CASS_FORMATCONVERTER_H
#define CASS_FORMATCONVERTER_H

#include <map>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include "cass.h"
#include "cass_event.h"

namespace Pds
{
  class Dgram;
}

namespace cass
{
  class ConversionBackend;

  class CASSSHARED_EXPORT FormatConverter
  {
    public:
      FormatConverter();
      ~FormatConverter();

    public:
      enum Converters {pnCCD, REMI, Pulnix,MachineData};
      bool processDatagram(cass::CASSEvent*);

    private:
      std::map<Converters, ConversionBackend*>    _converters;
  };

}//end namespace cass


#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
