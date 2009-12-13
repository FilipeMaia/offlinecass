#ifndef __POSTPROCESSOR_H__
#define __POSTPROCESSOR_H__

#include "cass.h"
#include "cass_event.h"
#include <stdio.h>
#include <QList>
#include <QFileInfo>

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
    QList<double*> m_data;
  };

  class PostProcessor
  {
    public:
    	PostProcessor()   
      	{
	  printf("Post_processor creator called here\n");
	  firstIntegratedImage = true;
	}
		
	~PostProcessor(){
	  printf("Post_processor destructor called here\n");
	}

    public:
      void postProcess(CASSEvent&);
      void integrateByQ(CASSEvent&);
      void finishProcessing(){
	  char outfile[1024];
	  sprintf(outfile,"%s_integrated.h5",
		  QFileInfo(cass::globalOptions.lastFile).baseName().toAscii().constData());
	  printf("outfile - %s\n",outfile);
	  integratedImage.outputImage(outfile);
      }
  private:
      void appendIntegratedByQ(CASSEvent &cassevent,float * x, float * y,int n,int frame);
      void extractEnergy(CASSEvent &cassevent);
      long long  integrateImage(cass::CASSEvent &cassevent);
      double stdDevImage(cass::CASSEvent &cassevent,long long integral);
      bool isGoodImage(cass::CASSEvent &cassevent);
      void addToIntegratedImage(cass::CASSEvent &cassevent);
      bool firstIntegratedImage;
      HDRImage integratedImage;
  };
}


#endif
