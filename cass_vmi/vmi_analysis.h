/*
 *  VMIAnalysis.h
 *  diode
 *
 *  Created by Jochen Küpper on 20.05.09.
 *  Copyright 2009 Fritz-Haber-Institut der MPG. All rights reserved.
 *lmf
 */

#ifndef VMIANALYSIS_H
#define VMIANALYSIS_H

#include <QtCore/QPoint>
#include <vector>

#include "cass_vmi.h"
#include "analysis_backend.h"
#include "parameter_backend.h"

namespace cass
{
    class CASSEvent;

    namespace VMI
    {
        class CASS_VMISHARED_EXPORT Parameter : public cass::ParameterBackend
        {
        public:
            Parameter()     {beginGroup("VMI");}
            ~Parameter()    {endGroup();}
            void load();
            void save();

        public:
            uint16_t   _threshold;
            QPoint     _centerOfMcp;
            uint16_t   _maxMcpRadius;
            uint32_t   _rebinfactor;
        };


        class CASS_VMISHARED_EXPORT Analysis : public cass::AnalysisBackend
        {
        public:
            Analysis()            {loadSettings();}
            ~Analysis()           {}
            void loadSettings()   {_param.load();}
            void saveSettings()   {_param.save();}

            //called for every event//
            void operator()(CASSEvent*);

        private:
            Parameter  _param;
            //temporary frame for rebinning
            std::vector<uint16_t> _tmp;
        };
    }//end namespace vmi
}//end namespace cass

#endif
