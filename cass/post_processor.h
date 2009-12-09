#ifndef __POSTPROCESSOR_H__
#define __POSTPROCESSOR_H__

#include "cass.h"
#include <stdio.h>

namespace cass
{
  class CASSEvent;

  class PostProcessor
  {
    public:
    	PostProcessor()   
      	{
      		printf("Post_processor creator called here\n");
		}
		
		~PostProcessor()  
      	{
      		printf("Post_processor destructor called here\n");
		}

    public:
      void postProcess(CASSEvent&);
  };
}


#endif
