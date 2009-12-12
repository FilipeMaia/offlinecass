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



int main(int argc, char **argv)
{
  // construct Qt application object
  QApplication app(argc, argv);

  //
  const char *filelistname = "filesToProcess.txt";

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
