#ifndef HYPDCCluster_h
#define HYPDCCluster_h

#include "TObject.h"
#include "HYPDCHit.h"
#include "HYPDCPlane.h"

class HYPDCCluster : public TObject {
public:
  HYPDCCluster()
  : fNHits(0), fX(0), fY(0) {}
  virtual ~HYPDCCluster() {}

  void Clear( Option_t* opt ) { fNHits = 0; fHits.clear(); }
  void AddHit(HYPDCHit* hit) { fHits.push_back(hit); }
  void SetXY(Double_t x, Double_t y) { fX = x; fY = y; }

  Int_t GetNHits() { return fNHits; }
  HYPDCHit* GetHit(Int_t i) const { return fHits[i]; }
  Double_t GetX() { return fX; }
  Double_t GetY() { return fY; }

protected:
  Int_t fNHits;
  Double_t fX;
  Double_t fY;
  std::vector<HYPDCHit*> fHits;
  //HYPDCPlane* fPlane;

  ClassDef(HYPDCCluster,0)
};

#endif
