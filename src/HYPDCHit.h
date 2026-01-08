#ifndef HYPDCHit_h
#define HYPDCHit_h

#include "TObject.h"
#include "DataType.h"
#include "HYPDCWire.h"

class HYPDCHit : public TObject {

public:
  explicit HYPDCHit(HYPDCWire* wire = nullptr, UInt_t tdc = 0, Double_t time = 0.0)
    : fWire(wire), fTDC(tdc), fTime(time), fDist(kBig) {}
  virtual ~HYPDCHit() {}

  Double_t   GetTime()    const { return fTime; }
  Double_t   GetDist()    const { return fDist; }
  Int_t      GetWireNum() const { return fWire->GetNum(); }
  Double_t   GetPos()     const { return fWire->GetPos(); }
  Int_t      GetAxis()    const { return fWire->GetAxis(); }
  Int_t      Compare( const TObject* obj ) const;  // to sort hits
  HYPDCWire* GetWire()    const { return fWire; }
  
  void Print( Option_t* opt ) const;
  void SetWire(HYPDCWire* wire) { fWire = wire; }
  void SetRawTime(Double_t rawtime) { fTDC = rawtime; }
  void SetTime(Double_t time) { fTime = time; }
  void SetDist(Double_t dist) { fDist = dist; }

protected:

  HYPDCWire* fWire;
  UInt_t   fTDC;  // raw TDC value
  Double_t fTime; // Time corrected for time offset
  Double_t fDist; // Drift distance

private:
  HYPDCHit( const HYPDCHit& );
  HYPDCHit& operator=(const HYPDCHit& );

  ClassDef(HYPDCHit,2)
};

#endif