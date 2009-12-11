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
	  currentXtcFile = 0;
	}
		
		~PostProcessor()  
      	{
      		printf("Post_processor destructor called here\n");
		}

    public:
      void postProcess(CASSEvent&);
      void integrateByQ(CASSEvent&);
  private:
      void appendIntegratedByQ(CASSEvent &cassevent,float * x, float * y,int n,int frame);
      const char * currentXtcFile;
      void openOutputFiles(CASSEvent &cassevent);
      void extractEnergy(CASSEvent &cassevent);
      long long integrateImage(cass::CASSEvent &cassevent);
      double stdDevImage(cass::CASSEvent &cassevent,long long integral);
      bool isGoodImage(cass::CASSEvent &cassevent);
  };
}


#endif
