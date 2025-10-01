#ifndef HYPScintDetector_h
#define HYPScintDetector_h

// Scint detector class
// Shall be used for EHodo and TOF detectors

#include "TClonesArray.h"
#include "THaNonTrackingDetector.h"
#include "THaSpectrometer.h"
#include "THcHitList.h"
#include "HYPScintHit.h"
#include "THcRawHodoHit.h"
#include "HYPScintillatorPlane.h"

class HYPScintDetector : public THaNonTrackingDetector, public THcHitList {
 public:
  HYPScintDetector(const char* name, const char* description="",
	    THaApparatus* apparatus = NULL);
  virtual ~HYPScintDetector();

  virtual void    Clear( Option_t* opt="" );
  virtual Int_t   Decode( const THaEvData& );
  virtual EStatus Init( const TDatime& date );
  virtual Int_t   CoarseProcess( TClonesArray& tracks );
  virtual Int_t   FineProcess( TClonesArray& tracks );

  Double_t DetermineTimePeak(Int_t FillFlag) { return 0.; } // func not defined yet

  Int_t GetScinIndex(Int_t nPlane, Int_t nPaddle) { return fNPlanes * nPaddle + nPlane; }
  Int_t GetScinIndex(Int_t nSide, Int_t nPlane, Int_t nPaddle) {
    return nSide * fMaxHodoScin + fNPlanes * nPaddle + nPlane - 1;
  }

  HYPScintillatorPlane* GetPlane(Int_t ip) { return fPlanes[ip];}
  Int_t GetNPlanes() { return fNPlanes;}
  Int_t GetNElement(Int_t ip) { return fNPaddle[ip]; }
  Int_t GetTdcOffset(Int_t ip) const { return fTdcOffset[ip];}
  Double_t GetAdcTdcOffset(Int_t ip) const { return fAdcTdcOffset[ip];}
  Double_t GetTdcMin() const {return fScinTdcMin;}
  Double_t GetTdcMax() const {return fScinTdcMax;}
  Double_t GetTdcToTime() const {return fScinTdcToTime;}

  Double_t GetHodoPosAdcTimeWindowMax(Int_t iii) const {return fHodoPosAdcTimeWindowMax[iii];}
  Double_t GetHodoPosAdcTimeWindowMin(Int_t iii) const {return fHodoPosAdcTimeWindowMin[iii];}
  Double_t GetHodoNegAdcTimeWindowMax(Int_t iii) const {return fHodoNegAdcTimeWindowMax[iii];}
  Double_t GetHodoNegAdcTimeWindowMin(Int_t iii) const {return fHodoNegAdcTimeWindowMin[iii];}

 protected:

  Int_t  fNhits;
  Bool_t *fPresentP;

  Int_t fNPlanes;
  Int_t *fNPaddle;
  Int_t fMaxHodoScin;
  Int_t fCosmicFlag;
  Double_t fScinTdcMin, fScinTdcMax;
  Double_t fScinTdcToTime;
  Int_t *fTdcOffset;
  Double_t *fAdcTdcOffset;
  Int_t fIsMC;

  Int_t *fNScinHits;       // [fNPlanes]

  Int_t fAnalyzePedestals;

  Double_t *fHodoNegAdcTimeWindowMin;
  Double_t *fHodoNegAdcTimeWindowMax;
  Double_t *fHodoPosAdcTimeWindowMin;
  Double_t *fHodoPosAdcTimeWindowMax;

  HYPScintillatorPlane **fPlanes;
  char **fPlaneNames;

  virtual Int_t ReadDatabase( const TDatime& date );
  virtual Int_t DefineVariables( EMode mode = kDefine );
  void Setup(const char *name, const char *description);

  ClassDef(HYPScintDetector,0)
};

#endif
