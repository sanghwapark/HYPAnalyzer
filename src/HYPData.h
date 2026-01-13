#ifndef HYPDATA_h
#define HYPDATA_h

#include "THaDetMap.h"
#include "DataType.h"
#include <vector>

// HYP raw data obj: FADC250 and vfTDC

namespace HYPData {

  struct FADCHit {
    UInt_t fChan;
    UInt_t fPulseIntegral;
    UInt_t fPulsePeak;
    UInt_t fPulseTime;
    UInt_t fPedestal;    
    UInt_t fCoarseTime;
    UInt_t fFineTime;
  };

  struct TDCHit {
    UInt_t fChan; // channel number
    UInt_t fTime; // Raw TDC data
    UInt_t fOpt;  // TDC mode  0: LE 1: TE

    TDCHit() : fChan(0), fTime(0), fOpt(0) {}
    TDCHit(UInt_t chan, UInt_t tdc, UInt_t tdc_opt)
      : fChan(chan), fTime(tdc), fOpt(tdc_opt) {}
  };

  // FADC config parameters
  struct FADCConfig {
    UInt_t NSA;
    UInt_t NSB;
    UInt_t NPED;
  };
  
  class FADCData {
  public:  
    FADCData() : fNHits(0) {}
    
    virtual ~FADCData() {}

    Int_t      AddHit( const DigitizerHitInfo_t& hitinfo );
    Int_t      Decode( const THaEvData& evdata, THaDetMap::Module *d );
    Int_t      GetNHits()          { return fNHits; }
    size_t     GetSize()     const { return fPulseData.size(); }
    FADCHit&   GetData( size_t i ) { return fPulseData[i]; }
    std::vector<uint32_t>& GetSampleData() { return fSampleData; }
    std::vector<FADCHit>&  GetPulseData() { return fPulseData; }
    void       Clear();

  protected:  
    Int_t fNHits;

    std::vector<FADCHit>  fPulseData;
    std::vector<uint32_t> fSampleData;  
  };

}

#endif
