#ifndef HYPDATA_h
#define HYPDATA_h

#include "DataType.h"

// HYP raw data obj: FADC250 and vfTDC/VECTOR TDC

namespace HYPData {

class FADCHitData {
    public:
    FADCHitData() : paddle(0), Ped(0), PulseInt(0), PulseAmp(0),
		    PulseTime(0), Is_good_hit(0) {}

    void clear() {
      paddle = 0;
      Ped = PulseInt = PulseAmp = PulseTime = 0.0;
      Is_good_hit = 0;
    }
    Int_t  paddle;
    Data_t Ped;
    Data_t PulseInt;
    Data_t PulseAmp;
    Data_t PulseTime;
    Int_t  Is_good_hit;
  };

class TDCData {
  public:
    TDCData(Int_t padnum, Int_t time_raw, Int_t time_cor, Int_t good_hit) :
      paddle(padnum), TimeRaw(time_raw), Time(time_cor), Is_good_hit(good_hit) {}
    TDCData() : paddle(0), TimeRaw(0), Time(0), Is_good_hit(0) {}

    Int_t  paddle;
    Int_t  TimeRaw;
    Int_t  Time; // Ref time subtracted  
    Int_t  Is_good_hit;
};

}

#endif