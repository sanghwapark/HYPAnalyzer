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

  Int_t    GetTdcOffset(Int_t iplane)    const { return fTdcOffset[iplane]; }
  Double_t GetAdcTdcOffset(Int_t iplane) const { return fAdcTdcOffset[iplane]; }
  Double_t GetTdcToTime()                const { return fScinTdcToTime; }
  Double_t GetTdcMin()                   const { return fScinTdcMin; }
  Double_t GetTdcMax()                   const { return fScinTdcMax; }

  Double_t GetCorrPosC1(Int_t scin_index) const { return fCorrPosC1[scin_index]; }
  Double_t GetCorrPosC2(Int_t scin_index) const { return fCorrPosC2[scin_index]; }
  Double_t GetCorrNegC1(Int_t scin_index) const { return fCorrNegC1[scin_index]; }
  Double_t GetCorrNegC2(Int_t scin_index) const { return fCorrNegC2[scin_index]; }
  Double_t GetTDCThrs()                   const { return fTdcThrs; }
  // Pulse height correction calib parameters
  HYPTOFPlane* GetPlane(Int_t ip) { return fPlanes[ip];}
  Int_t GetScinIndex(Int_t iplane, Int_t ipaddle);
  protected:

  Int_t  fNPlanes;
  Int_t  fNhits;
  Int_t  fMaxElement;
  Bool_t *fPresentP;

  Int_t fADC_RefTimeCut;
  Int_t fTDC_RefTimeCut;

  Int_t    *fTdcOffset;
  Double_t *fAdcTdcOffset;
  Double_t fScinTdcToTime;
  Double_t fScinTdcMin;
  Double_t fScinTdcMax;
  Double_t *fCorrPosC1;
  Double_t *fCorrNegC1;
  Double_t *fCorrPosC2;
  Double_t *fCorrNegC2;
  Double_t fTdcThrs;

  std::vector<HYPTOFPlane*> fPlanes;
 
  virtual Int_t ReadDatabase( const TDatime& date );
  virtual Int_t DefineVariables( EMode mode = kDefine );

  ClassDef(HYPTOFDetector,0)
};

#endif
