#ifndef HYPDCHit_h
#define HYPDCHit_h

#include "TObject.h"
#include "HYPDCPlane.h"
#include "THcDCWire.h"

class HYPDCHit : public TObject {

public:
  HYPDCHit(THcDCWire* wire=NULL, Int_t rawtime=0, Int_t refcorrtime = 0, Double_t time=0.0, 
    HYPDCPlane* wp=0) :
    fWire(wire), fRawTime(rawtime), fRefCorrTime(), fTime(time), fWirePlane(wp),
      fDist(0.0), fLR(0), ftrDist(kBig),fNPlaneCluster(0) {
      if (wire) ConvertTimeToDist();
      fCorrected = 0;
    }
  virtual ~HYPDCHit() {}

  virtual Double_t ConvertTimeToDist();
  Int_t  Compare ( const TObject* obj ) const;
  Bool_t IsSortable () const { return kTRUE; }
  virtual void Print( Option_t* opt="" ) const;

  // Get and Set Functions
  THcDCWire* GetWire() const { return fWire; }
  Double_t GetWireSigma() const { return fWire->GetSigma(); }
  Int_t    GetWireNum() const { return fWire->GetNum(); }
  Int_t    GetRawTime() const { return fRawTime; } // no ref corrected time
  Int_t    GetRefCorrTime() const { return fRefCorrTime; } // reference time subtracted
  Double_t GetTime()    const { return fTime; }
  Double_t GetDist()    const { return fDist; }
  Double_t GetPos()     const { return fWire->GetPos(); } //Position of hit wire
  Double_t GetCoord()   const { return fCoord; }
  Double_t GetdDist()   const { return fdDist; }
  Int_t    GetCorrectedStatus() const { return fCorrected; }
  Int_t    GetAxis()    const { return fWirePlane->GetAxis(); }
  HYPDCPlane* GetWirePlane() const { return fWirePlane; }
  
  void     SetWire(THcDCWire * wire) { fWire = wire; ConvertTimeToDist(); }
  void     SetRawTime(Int_t time)     { fRawTime = time; }
  void     SetTime(Double_t time)     { fTime = time; }
  void     SetDist(Double_t dist)     { fDist = dist; }
  void     SetLeftRight(Int_t lr)   { fCoord = GetPos() + lr*fDist; fLR=lr;}
  void     IncrNPlaneClust()   {fNPlaneCluster++ ;}
  Int_t    GetNPlaneClust() const { return fNPlaneCluster; }
  Int_t    GetLR() { return fLR; }
  void     SetdDist(Double_t ddist)   { fdDist = ddist; }
  void     SetFitDist(Double_t dist)  { ftrDist = dist; }
  Int_t    GetPlaneNum() const { return fWirePlane->GetPlaneNum(); }
  Int_t    GetPlaneIndex() const { return fWirePlane->GetPlaneIndex(); }
  Int_t    GetChamberNum() const { return fWirePlane->GetChamberNum(); }
  void     SetCorrectedStatus(Int_t c) { fCorrected = c; }

  Int_t GetReadoutSide() { return(fWire->GetReadoutSide()); }

protected:
  static const Double_t kBig;  //!

  THcDCWire*  fWire;     // Wire on which the hit occurred
  Int_t       fRawTime;  // Time with trigger time subtracted
  Int_t       fRefCorrTime; // Rerence time corrected time
  Double_t    fTime;       // Time corrected for time offset of wire (s)
  HYPDCPlane* fWirePlane; //! Pointer to parent wire plane
  Double_t    fDist;     // (Perpendicular) Drift Distance
  Int_t       fLR;       // +1/-1 which side of wire
  Double_t    fCoord;    // Actual coordinate of hit
  Double_t    fdDist;    // uncertainty in fDist (for chi2 calc)
  Double_t    ftrDist;   // (Perpendicular) distance from the track
  Int_t       fCorrected; // Has this hit been corrected?
  Int_t       fReadoutSide; // Side where wire is read out. 1-4 is T/R/B/L from beam view for new chambers.
  Int_t       fNPlaneCluster; // Number of plane clusters hit is in.

private:
  HYPDCHit( const HYPDCHit& );
  HYPDCHit& operator=( const HYPDCHit& );

  ClassDef(HYPDCHit,1)             // Drift Chamber Hit
};

#endif
