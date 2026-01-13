#ifndef HYPDCCluster_h
#define HYPDCCluster_h

#include "TObject.h"
#include "HYPDCHit.h"
#include "HYPDCPlane.h"

class HYPDCCluster : public TObject {
public:
  HYPDCCluster(Double_t x, Double_t y)
  : fNHits(0), fX(x), fY(y) {}
  virtual ~HYPDCCluster() {}

  void Clear( Option_t* opt ) { fNHits = 0; fHits.clear(); }
  void AddHit(HYPDCHit* hit) { fHits.push_back(hit); fNHits++; }
  void SetXY(Double_t x, Double_t y) { fX = x; fY = y; }
  void SetCoord(Int_t coord) { fCoord = coord; }

  Int_t     GetNHits() { return fNHits; }
  HYPDCHit* GetHit(Int_t i) const { return fHits[i]; }
  Double_t  GetX() { return fX; }
  Double_t  GetY() { return fY; }
  Int_t     GetCoord() { return fCoord; }

protected:
  Int_t fNHits;
  Double_t fX;
  Double_t fY;
  Int_t fCoord;
  std::vector<HYPDCHit*> fHits;
  //HYPDCPlane* fPlane;

  ClassDef(HYPDCCluster,0)
};

#endif
