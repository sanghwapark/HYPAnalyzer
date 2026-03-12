#ifndef HYPTOFDetector_h
#define HYPTOFDetector_h

#include "TClonesArray.h"
#include "THaNonTrackingDetector.h"
#include "HYPTOFPlane.h"
#include "THcHitList.h"
#include <vector>


class HYPTOFDetector : public THaNonTrackingDetector, THcHitList {
 public:
  HYPTOFDetector(const char* name, const char* description="",
	    THaApparatus* apparatus = NULL);
  virtual ~HYPTOFDetector();

  virtual void      Clear( Option_t* opt="" );
  virtual EStatus   Init( const TDatime& date );
  virtual Int_t     Decode( const THaEvData& evdata );
  virtual Int_t     CoarseProcess( TClonesArray& tracks );
  virtual Int_t     FineProcess( TClonesArray& tracks );

  HYPTOFPlane* GetPlane(Int_t ip) { return fPlanes[ip];}

  protected:

  Int_t  fNPlanes;
  Int_t  fNhits;
  Bool_t *fPresentP;

  Int_t fADC_RefTimeCut;
  Int_t fTDC_RefTimeCut;


  std::vector<HYPTOFPlane*> fPlanes;
 
  virtual Int_t ReadDatabase( const TDatime& date );
  virtual Int_t DefineVariables( EMode mode = kDefine );

  ClassDef(HYPTOFDetector,0)
};

#endif
