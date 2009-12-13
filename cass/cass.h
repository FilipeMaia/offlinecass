#ifndef CASS_GLOBAL_H
#define CASS_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QtCore/QString>
#include <QtGui/QImage>
#include <QtCore/QDateTime>

#if defined(CASS_LIBRARY)
#  define CASSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CASSSHARED_EXPORT Q_DECL_IMPORT
#endif

namespace cass{
const int RingBufferSize=4;

class CommandLineOptions{
public:
    CommandLineOptions()
	{
	    outputHitsToFile = false;
	    onlyAnalyzeGivenHits = false;
	    justIntegrateImages = false;
	    outputAllEvents = false;
	    useSignalMask[0] = false;
	    useSignalMask[1] = false;
	}
    bool outputHitsToFile;
    QString hitsOutputFile;
    bool onlyAnalyzeGivenHits;
    QString hitsInputFile;
    bool justIntegrateImages;
    bool outputAllEvents;        
    bool useSignalMask[2];
    QString signalMaskFile[2];    
    QImage signalMask[2];
    bool discardCCD[2];
    QDateTime startTime;
    QDateTime endTime;
    QString lastFile;
};

extern cass::CommandLineOptions globalOptions;

}

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
