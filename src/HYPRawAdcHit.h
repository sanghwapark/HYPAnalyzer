#ifndef HYPRawAdcHit_h
#define HYPRawAdcHit_h

#include "TObject.h"

class HYPRawAdcHit : public TObject {
  public:
    HYPRawAdcHit() {}
    HYPRawAdcHit(UInt_t ielem, Int_t data) {}
    virtual ~HYPRawAdcHit() {}

    virtual void SetData(UInt_t ielem, UInt_t data) { fChan = ielem, fPulseIntRaw = data; }
    virtual void SetData(UInt_t ielem, UInt_t data, UInt_t amp, UInt_t time, UInt_t ped, UInt_t ctime, UInt_t ftime)
    { fChan = ielem, fPulseIntRaw = data; fPulsePeak = amp;
      fPulseTime = time; fPulsePed = ped; fCoarseTime = ctime, fFineTime = ftime; }    
    virtual void SetSampData(UInt_t data) { fSampleData.push_back(data); }

    UInt_t GetNum()     const { return fChan; }
    UInt_t GetPulseIntRaw() const { return fPulseIntRaw; }
    UInt_t GetPulseInt()    const { return fPulseInt; }
    UInt_t GetPulseAmp()    const { return fPulsePeak; }
    UInt_t GetPulseTime()   const { return fPulseTime; }
    UInt_t GetPulsePed()   const { return fPulsePed; }

    std::vector<uint32_t>& GetSampleData() { return fSampleData; }

  protected:

    // pulse data
    UInt_t fChan;
    UInt_t fPulseIntRaw;
    UInt_t fPulseInt;
    UInt_t fPulsePeak;
    UInt_t fPulseTime;
    UInt_t fPulsePed;
    UInt_t fCoarseTime;
    UInt_t fFineTime;

    // sample data
    std::vector<uint32_t> fSampleData;
  
  ClassDef(HYPRawAdcHit,0)
};

#endif