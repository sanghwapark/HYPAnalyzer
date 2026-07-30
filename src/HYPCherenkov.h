#ifndef HYPCherenkov_h
#define HYPCherenkov_h

// Cherenkov detector class
#include "TClonesArray.h"
#include "THaNonTrackingDetector.h"
#include "THcHitList.h"
#include "THcCherenkovHit.h"
#include "HYPData.h"
#include <cmath>
#include <vector>

using namespace std;
using namespace HYPData;

// base class for AC and WC Cherenkov

class HYPCherenkov : public THaNonTrackingDetector, THcHitList {
 public:
  HYPCherenkov(const char* name, const char* description="",
	    THaApparatus* apparatus = NULL);
  virtual ~HYPCherenkov();

  virtual void    Clear( Option_t* opt="" );
  virtual Int_t   Decode( const THaEvData& );
  virtual EStatus Init( const TDatime& date );
  virtual Int_t   CoarseProcess( TClonesArray& tracks );
  virtual Int_t   FineProcess( TClonesArray& tracks );

  HYPCherenkov();

 protected:

  Int_t fNhits;
  Bool_t* fPresentP;

  Int_t     fADC_RefTimeCut;
  Double_t  fAdcTdcOffset;
  Int_t     fUseSampWaveform;
  Int_t     fOutSampWaveform;
  Double_t  fSampThreshold;
  Int_t     fSampNSA;
  Int_t     fSampNSAT;
  Int_t     fSampNSB;
  Int_t     fSampNPED;
  Int_t     fDebugAdc;

  Double_t  *fAdcPosTimeWindowMin;
  Double_t  *fAdcPosTimeWindowMax;
  Double_t  *fAdcNegTimeWindowMin;
  Double_t  *fAdcNegTimeWindowMax;

  Double_t  *fPosGain;
  Double_t  *fNegGain;

  // Raw data containers
  vector<FADCHitData> fPosDataRaw;
  vector<FADCHitData> fNegDataRaw;
  vector<FADCHitData> fPosData;
  vector<FADCHitData> fNegData;

  vector<FADCHitData> fPosSampDataRaw;
  vector<FADCHitData> fNegSampDataRaw;
  vector<FADCHitData> fPosSampData;
  vector<FADCHitData> fNegSampData;

  vector<Double_t> fPosSampWaveform; //waveform data
  vector<Double_t> fNegSampWaveform; //waveform data

  vector<Double_t> fGoodPosAdcPed;
  vector<Double_t> fGoodNegAdcPed;

  vector<Int_t> fPosErrorFlag;
  vector<Int_t> fNegErrorFlag;

  // Npe variables
  Double_t fPosNpeSum;
  Double_t fNegNpeSum;
  Double_t fNpeSum;
  vector<Double_t> fPosNpe; // npe per pmt
  vector<Double_t> fNegNpe;

  // Occupancy
  vector<Int_t> fNumGoodPosAdcHits;
  vector<Int_t> fNumGoodNegAdcHits;

  // Save pulse data that passed the cuts
  vector<FADCHitData> fPosDataGood;
  vector<FADCHitData> fNegDataGood;

  
  virtual Int_t ReadDatabase( const TDatime& date );
  virtual Int_t DefineVariables( EMode mode = kDefine );

  void DeleteArrays();

  ClassDef(HYPCherenkov,0)

};

#endif
