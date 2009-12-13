// Copyright (C) 2009 Jochen Küpper,lmf

#include <iostream>
#include <QtGui/QApplication>

#include "cass.h"
#include "cass_event.h"
#include "analyzer.h"
#include "file_input.h"
#include "ringbuffer.h"
#include "format_converter.h"
#include "ratemeter.h"
#include "dialog.h"
#include "worker.h"
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
    -s: Output one single integrated image to this file\n\
    -a: Output all pnCCD events regardless of signal\n\
    -m: Use the given mask for CCD 0 when calculating signal metrics\n\
    -M: Use the given mask for CCD 1 when calculating signal metrics\n\
    -d: Discard CCD 0\n\
    -D: Discard CCD 1\n\
    -t: Start time for conversion\n\
    -T: End time for conversion\n\
    -h: print this text\n\
";
  static char optstring[] = "x:l:sm:M:t:T:adDh";
  while(1){
    c = getopt(argc,argv,optstring);
    if(c == -1){
      break;
    }
    switch(c){
    case 'a':
	cass::globalOptions.outputAllEvents = true;
	break;
    case 's':
	cass::globalOptions.justIntegrateImages = true;
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
    case 'h':
      printf("%s",help_text);
      exit(0);
      break;
    default:
      printf ("?? getopt returned character code 0%o ??\n", c);
    }
  }
}

int main(int argc, char **argv)
{
  // construct Qt application object
  QApplication app(argc, argv);

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
  cass::Window * window(new cass::Window());


  //when the quit button has been pushed we want to close the application//
  QObject::connect(window, SIGNAL(quit()), input, SLOT(end()));
  
  // when the thread has finished, we want to close this application
  QObject::connect(input, SIGNAL(finished()), worker, SLOT(end()));
  QObject::connect(worker, SIGNAL(finished()), window, SLOT(close()));

  //show dialog//
  window->show();

  // start file input thread
  input->start();
  
  // start the worker thread
  worker->start();

  // start Qt event loop
  int retval(app.exec());
  
  // clean up
  delete window;
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
