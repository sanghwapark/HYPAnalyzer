#ifndef HYPCherenkov_h
#define HYPCherenkov_h

// Cherenkov detector class
#include <vector>
#include "TClonesArray.h"
#include "THaNonTrackingDetector.h"

using namespace std;

class HYPCherenkov : public THaNonTrackingDetector {
 public:
  HYPCherenkov(const char* name, const char* description="",
	    THaApparatus* apparatus = NULL);
  virtual ~HYPCherenkov();

  virtual void    Clear( Option_t* opt="" );
  virtual Int_t   Decode( const THaEvData& );
  virtual EStatus Init( const TDatime& date );
  virtual Int_t   CoarseProcess( TClonesArray& tracks );
  virtual Int_t   FineProcess( TClonesArray& tracks );
  
 protected:

  Int_t fNHits;
  Double_t fNpeSum;

  // chan map and calibration parameters
  class CerInfo {
    public:
      Int_t fPMT;
      Int_t fSigType;
      Double_t fAdcTimeMinCut;
      Double_t fAdcTimeMaxCut;
      Double_t fGain;
};
  std::vector<CerInfo> fCerInfo;

  virtual Int_t ReadDatabase( const TDatime& date );
  virtual Int_t DefineVariables( EMode mode = kDefine );
  
  Int_t ReadGeometry( FILE* file, const TDatime& date, Bool_t required = false);
  Int_t StoreHit( const DigitizerHitInfo_t& hitinfo, UInt_t data );  
  OptUInt_t LoadData( const THaEvData& evdata, const DigitizerHitInfo_t& hitinfo );

  TClonesArray*  fRawHits;
  vector<UInt_t> fSampleData;

  ClassDef(HYPCherenkov,0)

};

#endif
