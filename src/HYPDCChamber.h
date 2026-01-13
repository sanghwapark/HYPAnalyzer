#ifndef HYPDCChamber_h
#define HYPDCChamber_h

#include "HYPDCPlane.h"
#include "HYPDCHit.h"
#include "HYPDCCluster.h"

struct DCSpacePoint {
  Double_t x;
  Double_t y;
};

class HYPDCChamber : public THaSubDetector {

public:
  HYPDCChamber( const char* name, const char* description,
		   THaDetectorBase* parent = nullptr );
  virtual ~HYPDCChamber();

  virtual void    Clear( Option_t* opt );
  virtual Int_t   Decode( const THaEvData& );
  virtual EStatus Init( const TDatime& run_time );
  virtual Int_t   CoarseTrack( TClonesArray& tracks );
  virtual Int_t   FineTrack( TClonesArray& tracks );
  virtual Int_t   FindSpacePoints() ;
  
  Int_t GetNPlanes() { return fNPlanes; }

protected:

  Int_t fNPlanes;
  Int_t fNHits;
  std::vector<HYPDCPlane*> fPlanes;
  std::vector<HYPDCHit*> fHits;

  Int_t fMinHitCut;
  Int_t fMaxHitCut;
  Double_t* fXsp;
  Double_t* fYsp;

  TClonesArray* fUClusters;
  TClonesArray* fVClusters;
  TClonesArray* fXClusters;
  TClonesArray* fUXClusters;
  TClonesArray* fVXClusters;

  //  virtual Int_t  ReadDatabase( const TDatime& date );
  virtual void   MakePrefix();
  virtual Int_t  ReadDatabase( const TDatime &date );
  virtual Int_t  DefineVariables( EMode mode = kDefine );
  ClassDef(HYPDCChamber,0)

};

#endif
