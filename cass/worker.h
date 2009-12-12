#ifndef __WORKER_H__
#define __WORKER_H__

#include <QtCore/QObject>
#include <QThread>
#include <QMutex>


#include "cass.h"
#include "ringbuffer.h"
#include "cass_event.h"


namespace cass
{
  class Analyzer;
  class FormatConverter;
  class PostProcessor;
  class CASSSHARED_EXPORT Worker : public QThread
  {
    Q_OBJECT;
    public:
    Worker(lmf::RingBuffer<cass::CASSEvent,cass::RingBufferSize>&, QObject *parent=0);
      ~Worker();

      void run();

    public slots:
      void end();

    private:
      lmf::RingBuffer<cass::CASSEvent,cass::RingBufferSize>  &_ringbuffer;
      Analyzer                            *_analyzer;
      FormatConverter                     *_converter;
      PostProcessor                       *_postprocessor;
      bool                                 _quit;
  };
}

#endif
