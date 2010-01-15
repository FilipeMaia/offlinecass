// Copyright (C) 2009 Jochen Küpper,lmf

#include <iostream>
#include <QCoreApplication>

#include "cass.h"
#include "cass_event.h"
#include "analyzer.h"
#include "file_input.h"
#include "ringbuffer.h"
#include "format_converter.h"
#include "ratemeter.h"
#include "dialog.h"
#include "worker.h"
#include "post_processor.h"
#include <unistd.h>

namespace cass{
cass::CommandLineOptions globalOptions;
}

void parseOptions(int argc, char ** argv){
  int c;
  static char help_text[] = 
    "    Options description:\n\
    \n\
    -x: Hits output file\n\
    -l: Only analyze hits given on this file\n\
    -s: Only output one single integrated image to this file\n\
    -I: Output integrated image to this file\n\
    -c: Threshold above which to integrate signal\n\
    -C: Max threshold below which the signal is integrate\n\
    -F: Integration floor\n\
    -g: Only output good events\n\
    -m: Use the given mask for CCD 0 when calculating signal metrics\n\
    -M: Use the given mask for CCD 1 when calculating signal metrics\n\
    -d: Discard CCD 0\n\
    -D: Discard CCD 1\n\
    -t: Start time for conversion\n\
    -T: End time for conversion\n\
    -S: Only look at every nth frame\n\
    -G: Integrate images and display as we go\n\
    -w: Only append wavelength information to already existing files\n\
    -h: print this text\n\
";
  static char optstring[] = "x:l:sc:m:M:t:T:S:GgdDIwh";
  while(1){
    c = getopt(argc,argv,optstring);
    if(c == -1){
      break;
    }
    switch(c){
    case 'g':
	cass::globalOptions.outputAllEvents = false;
	break;
    case 's':
	cass::globalOptions.justIntegrateImages = true;
	break;
    case 'I':
	cass::globalOptions.alsoIntegrateImages = true;
	break;
    case 'c':
	cass::globalOptions.justIntegrateImagesThreshold = atof(optarg);
	cass::globalOptions.useIntegrationThreshold = true;	
	break;
    case 'C':
	cass::globalOptions.integrationCeiling = atof(optarg);
	cass::globalOptions.useIntegrationCeiling = true;	
	break;
    case 'F':
	cass::globalOptions.integrationFloor = atof(optarg);
	cass::globalOptions.useIntegrationFloor= true;	
	break;
    case 'x':
	cass::globalOptions.outputHitsToFile = true;
	cass::globalOptions.hitsOutputFile = QString(optarg);
	break;
    case 'l':
	cass::globalOptions.onlyAnalyzeGivenHits = true;
	cass::globalOptions.hitsInputFile = QString(optarg);
	break;
    case 'm':
	cass::globalOptions.useSignalMask[0] = true;
	cass::globalOptions.signalMaskFile[0] = QString(optarg);
	/* Try to open the mask file */
	cass::globalOptions.signalMask[0] = QImage(cass::globalOptions.signalMaskFile[0]);
      break;
    case 'M':
	cass::globalOptions.useSignalMask[1] = true;
	cass::globalOptions.signalMaskFile[1] = QString(optarg);
	/* Try to open the mask file */
	cass::globalOptions.signalMask[1] = QImage(cass::globalOptions.signalMaskFile[1]);
      break;
    case 'd':
	cass::globalOptions.discardCCD[0] = true;
      break;
    case 'D':
	cass::globalOptions.discardCCD[1] = true;
      break;
    case 't':
	cass::globalOptions.startTime =
	    QDateTime::fromString(QString(optarg),QString("hh:mm:ss"));
	break;
    case 'T':
	cass::globalOptions.endTime =
	    QDateTime::fromString(QString(optarg),QString("hh:mm:ss"));
      break;
    case 'S':
	cass::globalOptions.skipPeriod = atoi(optarg);
      break;
    case 'G':
	cass::globalOptions.displayIntegration = true;
      break;
    case 'w':
	cass::globalOptions.onlyAppendWavelength = true;
      break;
    case 'h':
      printf("%s",help_text);
      exit(0);
      break;
    default:
      printf ("?? getopt returned character code 0%o ??\n", c);
    }
  }
}

/*
cass::DisplayWidget::DisplayWidget(cass::PostProcessor * pp){
    QVBoxLayout * layout = new QVBoxLayout(this);
    QHBoxLayout * hbox = new QHBoxLayout(this);
    layout->addLayout(hbox);
    hbox->addWidget(new QLabel("Max"));
    maxSlider = new QSlider(this);
    maxSlider->setOrientation(Qt::Horizontal);
    hbox->addWidget(maxSlider);
    hbox->addWidget(new QLabel("Min"));
    minSlider = new QSlider(this);
    minSlider->setOrientation(Qt::Horizontal);
    minSlider->setValue(99);
    hbox->addWidget(minSlider);
    m_label = new QLabel("little label",this);  
    logScale = new QCheckBox("Log Scale");
    hbox->addWidget(logScale);
    hbox->addWidget(new QLabel("# Images to average:"));
    nImagesToAverage = new QSpinBox(this);
    nImagesToAverage->setRange(0,10000);
    nImagesToAverage->setValue(0);
    nImagesToAverage->setSingleStep(500);

    hbox->addWidget(nImagesToAverage);
    layout->addWidget(m_label);
    logScale->setChecked(true);
    connect(logScale,SIGNAL(setChecked(bool)),this,SLOT(setLogScale(bool)));
    connect(nImagesToAverage,SIGNAL(valueChanged(int)),this,SLOT(changeImagesToAverage(int)));
    connect(maxSlider,SIGNAL(sliderReleased()),this,SLOT(changeMax()));
    connect(minSlider,SIGNAL(sliderReleased()),this,SLOT(changeMin()));
    m_pp = pp;
    minModifier = 0;
    maxModifier = 1;
    
}


void cass::DisplayWidget::update(){
    m_label->setText(QString::number(cass::globalOptions.eventCounter));
    m_label->setPixmap(QPixmap::fromImage(m_pp->integratedImage.toQImage(1,maxModifier,
									 minModifier,logScale->checkState() == Qt::Checked)));
}
*/

int main(int argc, char **argv)
{
  // construct Qt application object
  QCoreApplication app(argc, argv);

  //
  const char *filelistname = "filesToProcess.txt";

  parseOptions(argc,argv);

  // a ringbuffer for the cassevents//
  lmf::RingBuffer<cass::CASSEvent,cass::RingBufferSize> ringbuffer;
  // create file input object //
  cass::FileInput *input(new cass::FileInput(filelistname,ringbuffer));
  // create a worker//
  cass::Worker *worker(new cass::Worker(ringbuffer));
  // create format converter object
  cass::Ratemeter *ratemeter(new cass::Ratemeter());
  // create a dialog object
//  cass::Window * window(new cass::Window());

/*  if(cass::globalOptions.displayIntegration){
      QTimer * timer = new QTimer;
      timer->start(1000);
      cass::DisplayWidget * display = new cass::DisplayWidget(worker->_postprocessor);
      display->show();
      QObject::connect(timer, SIGNAL(timeout()), display, SLOT(update()));    
}*/

  //when the quit button has been pushed we want to close the application//
//  QObject::connect(window, SIGNAL(quit()), input, SLOT(end()));
  
  // when the thread has finished, we want to close this application
  QObject::connect(input, SIGNAL(finished()), worker, SLOT(end()));
  QObject::connect(worker, SIGNAL(finished()), &app, SLOT(quit()));

  //show dialog//
//  window->show();

  // start file input thread
  input->start();
  
  // start the worker thread
  worker->start();

  // start Qt event loop
  int retval(app.exec());
  
  // clean up
//  delete window;
  delete ratemeter;
  delete worker;
  delete input;

  // finish
  return retval;
}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
