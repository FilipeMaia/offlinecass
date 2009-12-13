#ifndef __POSTPROCESSOR_H__
#define __POSTPROCESSOR_H__

#include "cass.h"
#include "cass_event.h"
#include <stdio.h>
#include <QList>
#include <QFileInfo>
#include <QtGui/QLabel>

namespace cass
{
  class CASSEvent;


  class HDRImage
  {
  public:
    struct sp_rgb{
      double r;
      double g;
      double b;
    };
  typedef enum{SpColormapFirstColorScheme=1,SpColormapGrayScale=1,SpColormapTraditional=2,
	     SpColormapHot=4,SpColormapRainbow=8,SpColormapJet=16,
	       SpColormapWheel=32,SpColormapLastColorScheme=64,SpColormapLogScale=128,SpColormapPhase=256,
	     SpColormapWeightedPhase=512,SpColormapMask=1024}SpColormap;

    HDRImage();
    /* create a high dynamic range image with the same dimensions as
     the cass image */
    HDRImage(CASSEvent &cassevent);
    ~HDRImage();
    void addToImage(CASSEvent &cassevent); 
      void outputImage(const char * filename);
      QImage toQImage(int frame,double maxModifier,double minModifier,int log);
  private:
      HDRImage::sp_rgb colormap_rgb_from_value(double value, int colormap);
      void hsv_to_rgb(double H,double S,double V,double * R,double *G,double *B);
      unsigned char * sp_image_get_false_color(int frame,int color, double min, double max);
      void colormap_create_table(sp_rgb color_table[256],int colormap);
    int m_nframes;
    QList<int> m_rows;
    QList<int> m_columns;
    QList<double*> m_data;
  };

  class PostProcessor
  {
    public:
    PostProcessor();		
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
	  //	  printf("outfile - %s\n",outfile);
	  integratedImage.outputImage(outfile);
      }
      HDRImage integratedImage;

  private:
      void appendIntegratedByQ(CASSEvent &cassevent,float * x, float * y,int n,int frame);
      void extractEnergy(CASSEvent &cassevent);
      long long  integrateImage(cass::CASSEvent &cassevent);
      double stdDevImage(cass::CASSEvent &cassevent,long long integral);
      bool isGoodImage(cass::CASSEvent &cassevent);
      void addToIntegratedImage(cass::CASSEvent &cassevent);
      bool firstIntegratedImage;
      QWidget * integrationDisplay;
      QLabel * labelDisplay;
  };
}


#endif
