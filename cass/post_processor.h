#ifndef __POSTPROCESSOR_H__
#define __POSTPROCESSOR_H__

#include "cass.h"
#include "cass_event.h"
#include <stdio.h>
#include <QList>

namespace cass
{
  class CASSEvent;


  class HDRImage
  {
  public:
    HDRImage();
    /* create a high dynamic range image with the same dimensions as
     the cass image */
    HDRImage(CASSEvent &cassevent);
    ~HDRImage();
    void addToImage(CASSEvent &cassevent); 
      void outputImage(const char * filename);

  private:
    int m_nframes;
    QList<int> m_rows;
    QList<int> m_columns;
    QList<long long *> m_data;
  };

  class PostProcessor
  {
    public:
    	PostProcessor()   
      	{
	  printf("Post_processor creator called here\n");
	  currentXtcFile = 0;
	  firstIntegratedImage = true;
	}
		
	~PostProcessor(){
	  printf("Post_processor destructor called here\n");
	  integratedImage.outputImage(globalOptions.integratedImageOutput.toAscii().constData());
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
      void addToIntegratedImage(cass::CASSEvent &cassevent);
      bool firstIntegratedImage;
      HDRImage integratedImage;
  };
}


#endif
