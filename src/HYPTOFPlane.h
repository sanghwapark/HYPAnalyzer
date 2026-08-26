#ifndef HYPTOFPlane_h
#define HYPTOFPlane_h

#include "THaSubDetector.h"
#include "TClonesArray.h"
#include "HYPData.h"
#include "THcHodoHit.h"

using namespace std;
using namespace HYPData;

class HYPTOFDetector;

class HYPTOFPlane : public THaSubDetector{
  public:
    HYPTOFPlane(const char *name, const char* description, const Int_t iplane, 
      THaDetectorBase *parent = nullptr);
    virtual ~HYPTOFPlane();
    
    virtual void    Clear( Option_t *opt = "" );
    virtual EStatus Init( const TDatime &date );
    virtual Int_t   Decode( const THaEvData& );
    virtual Int_t   ProcessHits(TClonesArray* rawhits, Int_t nexthit);
    virtual Int_t   CoarseProcess( TClonesArray& tracks );
    virtual Int_t   FineProcess( TClonesArray& tracks );
//    virtual Int_t   Print( Option_t* opt ="" ) const;

    vector<THcHodoHit*> GetHits() { return fHodoHits; }

  protected:

    Int_t fPlaneNum;
    
    Int_t fDebugADC;
    Int_t fUseSampWaveform;
    Int_t fOutputSampWaveform;
    Int_t fSampNSA;
    Int_t fSampNSB;
    Int_t fSampNSAT;
    Double_t fSampThreshold;

    // HodoHits
    vector<THcHodoHit*> fHodoHits;

    // Container for FADC data
    vector<FADCHitData> fPosAdcDataRaw;
    vector<FADCHitData> fNegAdcDataRaw;
    vector<FADCHitData> fPosAdcData;
    vector<FADCHitData> fNegAdcData;

    // Good ADC and TDC Data
    vector<TOFEvent> fGoodPosData;
    vector<TOFEvent> fGoodNegData;

    // Sample data
    vector<FADCHitData> fPosAdcSampDataRaw;
    vector<FADCHitData> fNegAdcSampDataRaw;
    vector<FADCHitData> fPosAdcSampData;
    vector<FADCHitData> fNegAdcSampData;

    // Pedestal
    vector<Int_t> fPosAdcPedRaw;
    vector<Int_t> fNegAdcPedRaw;
    vector<Double_t> fPosAdcPed;
    vector<Double_t> fNegAdcPed;

    vector<Int_t> fPosAdcErrorFlag;
    vector<Int_t> fNegAdcErrorFlag;

    // TDC data
    vector<TDCData> fPosTdcData;
    vector<TDCData> fNegTdcData;
    
    // Waveform data
    vector<UInt_t> fPosSampWaveform;
    vector<UInt_t> fNegSampWaveform;

    // Time correction 
    class TOFCalib {
      private:
        // assume two parameters
        Double_t TWCorr[2];
      public:
        void SetParams(Double_t c1, Double_t c2) {
          TWCorr[0] = c1; TWCorr[1] = c2;
        }
        Double_t GetPar(Int_t i) const { return TWCorr[i]; }
    };
    vector<TOFCalib> fPosCalib;
    vector<TOFCalib> fNegCalib;

    Double_t fFineCalib; // calibraiton constant for vfTDC

    // Reference time variables
    Double_t fTdcRefTime[2];
    Double_t fTdcRefDiffTime[2];
    Double_t fAdcRefTime[2];
    Double_t fAdcRefDiffTime[2];

    Double_t *fPosAdcTimeWindowMin;
    Double_t *fPosAdcTimeWindowMax;
    Double_t *fNegAdcTimeWindowMin;
    Double_t *fNegAdcTimeWindowMax;

    // Parameters from parent detector
    Int_t fTdcOffset;
    Double_t fAdcTdcOffset;
    Double_t fScinTdcMin;
    Double_t fScinTdcMax;
    Double_t fScinTdcToTime;
    Double_t fTdcThrs;

    Int_t ReadDatabase( const TDatime &date );
    Int_t DefineVariables( EMode mode = kDefine );

    Int_t GetNumPosAdcHits() { return static_cast<Int_t>(fPosAdcData.size()); }
    Int_t GetNumNegAdcHits() { return static_cast<Int_t>(fNegAdcData.size()); }
    void DoTimeCorrection(Int_t signal, Int_t pad_index);
    Double_t DecodeTDCData(Int_t tdc);

  ClassDef(HYPTOFPlane, 0);

};

#endif
