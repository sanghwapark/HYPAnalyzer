#ifndef HYPDCCluster_h
#define HYPDCCluster_h

#include "TObject.h"
#include "HYPDCHit.h"

class HYPDCCluster : public TObject {

public:

  HYPDCCluster(Int_t nhits=0) :
  fNHits(nhits) {
    fHits.clear();
  }
  virtual ~HYPDCCluster() {}
  void SetXY(Double_t x, Double_t y) {fX = x; fY = y;};
  void Clear(Option_t* opt="") {fNHits=0; fHits.clear();};
  void AddHit(HYPDCHit* hit) {
    fHits.push_back(hit);
    fNHits++;
  }
  HYPDCHit* GetHit(Int_t ihit) {return fHits[ihit];};
  Int_t GetNHits() {return fNHits;};
  Double_t GetX() {return fX;};
  Double_t GetY() {return fY;};

protected:

  Double_t fX;
  Double_t fY;
  Int_t fNHits;
  std::vector<HYPDCHit*> fHits;


  ClassDef(HYPDCCluster,0);   
};

#endif
