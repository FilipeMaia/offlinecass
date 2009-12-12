// Copyright (C) 2009 lmf

#ifndef CASS_EVENTQUEUE_H
#define CASS_EVENTQUEUE_H

#include <QtCore/QObject>
#include <QThread>
#include <QMutex>

#include "cass.h"
#include "ringbuffer.h"
#include "cass_event.h"

namespace cass
{
  class CASSSHARED_EXPORT FileInput : public QThread
  {
    Q_OBJECT;
  public:
    enum {_maxbufsize=1, _maxdatagramsize=0x1000000};
      FileInput(const char *filelistname, lmf::RingBuffer<cass::CASSEvent,cass::RingBufferSize>&,  QObject *parent=0);
    ~FileInput();

    void run();

  public slots:
    void end();

  private:
      lmf::RingBuffer<cass::CASSEvent,cass::RingBufferSize>  &_ringbuffer;
    bool                                 _quit;
    const char                          *_filelistname;
  };

}//end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
