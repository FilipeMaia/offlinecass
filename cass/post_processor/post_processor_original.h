#ifndef __POSTPROCESSOR_H__
#define __POSTPROCESSOR_H__

#include "cass.h"

namespace cass
{
  class CASSEvent;

  class PostProcessor
  {
    public:
      PostProcessor()   {}
      ~PostProcessor()  {}

    public:
      void postProcess(CASSEvent&);
  };
}


#endif
