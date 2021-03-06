#ifndef REMICONVERTER_H
#define REMICONVERTER_H

#include "cass_remi.h"
#include "conversion_backend.h"
#include "remi_event.h"
#include <iostream>
#include "pdsdata/acqiris/ConfigV1.hh"

namespace cass
{
    namespace REMI
    {
        class CASS_REMISHARED_EXPORT Converter : public cass::ConversionBackend
        {
        public:
            Converter();
           //called for LCLS event//
            void operator()(const Pds::Xtc*, cass::CASSEvent*);
        private:
            //store the config internally since its only send once for each run//
            REMIEvent               _storedEvent;
	    Pds::Acqiris::ConfigV1 config;
        };
    }//end namespace remi
}//end namespace cass

#endif
