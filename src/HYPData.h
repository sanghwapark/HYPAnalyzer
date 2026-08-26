#ifndef HYPDATA_h
#define HYPDATA_h

#include "DataType.h"

// HYP raw data obj: FADC250 and vfTDC/VECTOR TDC

namespace HYPData {

  class FADCHitData {
  public:
    FADCHitData() : paddle(-1), Ped(0), PulseInt(0), PulseAmp(0),
		    PulseTime(0), Is_good_hit(0) {}

    void clear() {
      paddle = -1;
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

  // TOF detector output 
  class TOFEvent {
    public:
      TOFEvent() : paddle(0), ped(0), adc(0), amp(0), adctime(-999), adctdctdiff(-999),
        mult(-1), hit_used(-1), time_uncorr(-999), time_corr(-999) {}

      void clear() {
        paddle = 0;
        ped = adc = amp = 0;
        adctime = adctdctdiff = -999;
        mult = hit_used = -1;
        time_uncorr = -999;
        time_corr = -999;
      }
      void SetPaddle(Int_t padnum) { paddle = padnum; }
      void SetTimeCorr(Double_t t_c) { time_corr = t_c; }
      void SetADC(Double_t a_ped, Double_t a_int, Double_t a_amp, Double_t a_time,
        Double_t a_tdiff, Int_t a_mult, Int_t a_hit) {
          ped = a_ped;
          adc = a_int;
          amp = a_amp;
          adctime = a_time;
          adctdctdiff = a_tdiff;
          mult = a_mult;
          hit_used = a_hit;
        }
      void SetTDC(Double_t t_uc, Double_t t_c) { time_uncorr = t_uc; time_corr = t_c; }

      Int_t  paddle;
      // ADC
      Double_t ped;
      Double_t adc; // pulse integral    
      Double_t amp;
      Double_t adctime;
      Double_t adctdctdiff;
      Int_t  mult;
      Int_t  hit_used;
      // TDC
      Double_t time_uncorr;
      Double_t time_corr;
  };

}// namespace HYPData

#endif
