#ifndef HYPCherenkovHit_h
#define HYPCherenkovHit_h

#include "TObject.h"

class HYPCherenkovHit : public TObject {
  public:
    HYPCherenkovHit(Int_t pmt)
      : fPMT(pmt), fPulseInt(0), fNpe(0.0) {}
    HYPCherenkovHit(Int_t pmt, Int_t pulseint, Double_t npe)
      : fPMT(pmt), fPulseInt(pulseint), fNpe(npe){}

    virtual ~HYPCherenkovHit() {}

    void SetNpe(Double_t npe) { fNpe = npe; }
    void SetData(Int_t pulseint, Double_t npe) { fPulseInt = pulseint, fNpe = npe; }

    Int_t    GetPMTNum()   const { return fPMT; }
    Int_t    GetPulseInt() const { return fPulseInt; }
    Double_t GetNpe()      const { return fNpe; }

    protected:
    
    Int_t    fPMT;
    Int_t    fPulseInt;
    Double_t fNpe;

  ClassDef(HYPCherenkovHit,0)
};

#endif