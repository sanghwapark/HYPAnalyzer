#ifndef HYPScintillatorPlane_h
#define HYPScintillatorPlane_h

#include "TClonesArray.h"
#include "THaSubDetector.h"
#include <vector>

using namespace std;

class THaEvData;
class THaSignalHit;

// Scintillator plane class
// contains xx number of paddles per plane

class HYPScintillatorPlane : public THaSubDetector {

public:
  HYPScintillatorPlane(const char *name, const char *description, const Int_t planenum, THaDetectorBase *parent = nullptr);
  virtual ~HYPScintillatorPlane();

  virtual void Clear(Option_t *opt = "");
  virtual Int_t Decode(const THaEvData &);
  virtual EStatus Init(const TDatime &date);

  virtual Int_t CoarseProcess(TClonesArray &tracks);
  virtual Int_t FineProcess(TClonesArray &tracks);
  virtual Int_t ProcessHits(TClonesArray *rawhits, Int_t nexthit);

  virtual Int_t AccumulatePedestals(TClonesArray *rawhits, Int_t nexthit);
  virtual void  CalculatePedestals();

  // Some getter/setter functions here
  Int_t GetNelem() { return fNelem; };          // return number of paddles in this plane
  Double_t GetSpacing() { return fSpacing; };   // spacing of paddles
  Double_t GetSize() { return fSize; };         // paddle size
  Double_t GetZpos() { return fZpos; };         // return the z position
  Double_t GetDzpos() { return fDzpos; }
  Double_t GetPosOffset() {return fPosOffset;};
  Double_t GetPosCenter(Int_t PaddleNo) { return fPosCenter[PaddleNo]; }; // counting from zero!
  Double_t GetPosLeft() { return fPosLeft; }
  Double_t GetPosRight() { return fPosRight; }
  
protected:

  TClonesArray *frPosAdcErrorFlag;
  TClonesArray *frNegAdcErrorFlag;
  
  TClonesArray *frPosTdcHits;
  TClonesArray *frNegTdcHits;
  TClonesArray *frPosAdcHits;
  TClonesArray *frNegAdcHits;
  TClonesArray *frPosAdcSums;
  TClonesArray *frNegAdcSums;
  TClonesArray *frPosAdcPeds;
  TClonesArray *frNegAdcPeds;

  // Raw data
  TClonesArray *frPosTdcTimeRaw;
  TClonesArray *frPosAdcPedRaw;
  TClonesArray *frPosAdcPulseIntRaw;
  TClonesArray *frPosAdcPulseAmpRaw;
  TClonesArray *frPosAdcPulseTimeRaw;

  TClonesArray *frPosTdcTime;
  TClonesArray *frPosAdcPed;
  TClonesArray *frPosAdcPulseInt;
  TClonesArray *frPosAdcPulseAmp;
  TClonesArray *frPosAdcPulseTime;

  TClonesArray *frNegTdcTimeRaw;
  TClonesArray *frNegAdcPedRaw;
  TClonesArray *frNegAdcPulseIntRaw;
  TClonesArray *frNegAdcPulseAmpRaw;
  TClonesArray *frNegAdcPulseTimeRaw;

  TClonesArray *frNegTdcTime;
  TClonesArray *frNegAdcPed;
  TClonesArray *frNegAdcPulseInt;
  TClonesArray *frNegAdcPulseAmp;
  TClonesArray *frNegAdcPulseTime;

  TClonesArray *frPosAdcSampPedRaw;
  TClonesArray *frPosAdcSampPulseIntRaw;
  TClonesArray *frPosAdcSampPulseAmpRaw;
  TClonesArray *frPosAdcSampPulseTimeRaw;
  TClonesArray *frPosAdcSampPed;
  TClonesArray *frPosAdcSampPulseInt;
  TClonesArray *frPosAdcSampPulseAmp;
  TClonesArray *frPosAdcSampPulseTime;

  TClonesArray *frNegAdcSampPedRaw;
  TClonesArray *frNegAdcSampPulseIntRaw;
  TClonesArray *frNegAdcSampPulseAmpRaw;
  TClonesArray *frNegAdcSampPulseTimeRaw;
  TClonesArray *frNegAdcSampPed;
  TClonesArray *frNegAdcSampPulseInt;
  TClonesArray *frNegAdcSampPulseAmp;
  TClonesArray *frNegAdcSampPulseTime;

  vector<Double_t> fPosAdcSampWaveform;
  vector<Double_t> fNegAdcSampWaveform;

  // Counters
  Int_t fTotNumTdcHits;
  Int_t fTotNumPosTdcHits;
  Int_t fTotNumNegTdcHits;

  Int_t fTotNumAdcHits;
  Int_t fTotNumPosAdcHits;
  Int_t fTotNumNegAdcHits;

  // Occupancies
  vector<Int_t> fNumPosAdcHits;
  vector<Int_t> fNumNegAdcHits;
  vector<Int_t> fNumPosTdcHits;
  vector<Int_t> fNumNegTdcHits;

  Int_t fDebugAdc;
  Double_t fPosTdcRefTime;
  Double_t fNegTdcRefTime;
  Double_t fPosTdcRefDiffTime;
  Double_t fNegTdcRefDiffTime;
  Double_t fPosAdcRefTime;
  Double_t fNegAdcRefTime;
  Double_t fPosAdcRefDiffTime;
  Double_t fNegAdcRefDiffTime;
  Double_t fHitDistance;
  Double_t fScinXPos;
  Double_t fScinYPos;
  Int_t fCosmicFlag;
  Int_t fPlaneNum;
  UInt_t fNelem;
 
  Int_t fADCMode;       // 0: standard, 1: use FADC ped, 2: integrate sample
                        // 3: integrate sample, subract dynamic pedestal
  
  enum { kADCStandard = 0, kADCDynamicPedestal, kADCSampleIntegral, kADCSampIntDynPed };

  Int_t fADCDiagCut;		  // Cut for ADC in hit maps.  Defaults to 50
  Int_t fTdcOffset;		    /* Overall offset to raw tdc */
  Double_t fAdcTdcOffset;	/* Overall offset to raw adc times */
  
  Int_t fOutputSampWaveform;
  Int_t fUseSampWaveform;
  Double_t fSampThreshold;
  Int_t fSampNSA;
  Int_t fSampNSAT;
  Int_t fSampNSB;
 
  Int_t fIsMC; 

  // Geometry parameters
  Double_t fSpacing; /* paddle spacing */
  Double_t fSize;    /* paddle size */
  Double_t fZpos;    /* z position */
  Double_t fDzpos;
  Double_t fPosLeft;            
  Double_t fPosRight;           
  Double_t fPosOffset;
  Double_t *fPosCenter;         /* array with centers for all scintillators in the plane */

  Double_t fScinTdcMin;
  Double_t fScinTdcMax;
  Double_t fStartTimeCenter;
  Double_t fScinTdcToTime;
  Double_t fTofTolerance;
  Double_t fBetaNominal;
 
  // Pedestal calculations
  Int_t fNPedestalEvents; /* Number of pedestal events */
  Int_t fMinPeds;         /* Only analyze/update if num events > */
  Int_t *fPosPedSum;      /* Accumulators for pedestals */
  Int_t *fPosPedSum2;
  Int_t *fPosPedLimit;
  Int_t *fPosPedCount;
  Int_t *fNegPedSum;
  Int_t *fNegPedSum2;
  Int_t *fNegPedLimit;
  Int_t *fNegPedCount;
  
  Double_t *fPosPed;
  Double_t *fPosThresh;
  Double_t *fNegPed;
  Double_t *fNegSig;
  Double_t *fNegThresh;

  Int_t        fEventType;
  Int_t        fEventNum;

  virtual Int_t ReadDatabase(const TDatime &date);
  virtual Int_t DefineVariables(EMode mode = kDefine);
  virtual void InitializePedestals();

  ClassDef(HYPScintillatorPlane, 0);
};

#endif 
