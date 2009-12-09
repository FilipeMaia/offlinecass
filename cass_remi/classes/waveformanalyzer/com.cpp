#include "com.h"

#include "helperfunctionsforstdc.h"
#include "channel.h"
#include "peak.h"


//______________________Implementation of simple Version__________________________________________________________
template <typename T>
void com(cass::REMI::Channel& c, const double SampleInterval)
{
    //extract info from the event//
    const cass::REMI::Channel::waveform_t Data = c.waveform();
    const int32_t vOffset   = static_cast<int32_t>(c.offset() / c.gain());    //mV -> ADC Bytes
    const size_t wLength    = c.waveform().size();
    const double threshold  = c.threshold() / c.gain();    //mV -> ADC Bytes
//    std::cout << "in waveformanalyzer threshold of channel "<<c.channelNbr()<<" is "<<c.threshold()<<std::endl;

    //initialize values for finding peaks//
    bool risingEdge         = false;
    bool firsttime          = true;
    int32_t startpos        = -1;

    //--go through the waveform--//
    for (size_t i=3; i<wLength;++i)
    {
        //check wether we have an indication of a peak in the signal//
        if (   (abs(Data[i] - vOffset) <= threshold)                    //checking for inside noise
            || ( i == wLength-1)                                        //check if we are at the second to last index
            || ( ( (Data[i]-vOffset)*(Data[i-1]-vOffset) ) < 0.))       //check wether we go through the zero
            {
            if (risingEdge)            //if we had a rising edge before we know that it was a real peak
            {
                //--create a new peak--//
                cass::REMI::Peak p;
                //--set all known settings--//
                p.startpos() = startpos;
                p.stoppos()  = i-1;

                //--height stuff--//
                cass::REMI::maximum<T>(c,p);

                //--fwhm stuff--//
                cass::REMI::fwhm<T>(c,p);

                //--center of mass stuff--//
                cass::REMI::CoM<T>(c,p,SampleInterval);

                //--Time is the Center of Mass--//
                p.time()= p.com();

                //--check the polarity--//
                if (Data[p.maxpos()]-vOffset == p.maximum())       //positive
                    p.polarity() = cass::REMI::Peak::Positive;
                else if (Data[p.maxpos()]-vOffset == -p.maximum()) //negative
                    p.polarity() = cass::REMI::Peak::Negative;
                else                                               //error: polarity not found
                    p.polarity() = cass::REMI::Peak::Bad;

                //--add peak to the channel--//
                c.peaks().push_back(p);
            }
            risingEdge = false;
            firsttime=true;
        }
        //if the above is not true then we are outside the noise
        else
        {
            if(firsttime)    //if it is the firsttime we are outside the noise
            {
                firsttime = false;
                startpos= i;            //remember the position
            }

            //--if haven't found a risingEdge (3 consecutive higher points) before check if we have--//
            if (!risingEdge)
            {
                if ((abs(Data[i-3]-vOffset) < abs(Data[i-2]-vOffset)) &&
                    (abs(Data[i-2]-vOffset) < abs(Data[i-1]-vOffset)) &&
                    (abs(Data[i-1]-vOffset) < abs(Data[i  ]-vOffset)) )
                {
                    risingEdge=true;
                }
            }
        }
    }
}



//########################## 8 Bit Version ###########################################################################
//______________________________________________________________________________________________________________________
void cass::REMI::CoM8Bit::analyze(cass::REMI::Channel& c, const double SampleInterval)
{
    com<char>(c,SampleInterval);
}
//########################## 16 Bit Version ###########################################################################
//______________________________________________________________________________________________________________________
void cass::REMI::CoM16Bit::analyze(cass::REMI::Channel& c, const double SampleInterval)
{
    com<short>(c,SampleInterval);
}
