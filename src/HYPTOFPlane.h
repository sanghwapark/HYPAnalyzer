#ifndef HYPTOFPlane_h
#define HYPTOFPlane_h

#include "THaSubDetector.h"
#include "TClonesArray.h"
#include "HYPData.h"

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

  protected:

    Int_t fDebugADC;
    Int_t fUseSampWaveform;
    Int_t fOutputSampWaveform;
    Int_t fSampNSA;
    Int_t fSampNSB;
    Int_t fSampNSAT;
    Double_t fSampThreshold;

    // Container for FADC data
    vector<FADCHitData> fPosAdcDataRaw;
    vector<FADCHitData> fNegAdcDataRaw;
    vector<FADCHitData> fPosAdcData;
    vector<FADCHitData> fNegAdcData;

    vector<FADCHitData> fPosAdcSampDataRaw;
    vector<FADCHitData> fNegAdcSampDataRaw;
    vector<FADCHitData> fPosAdcSampData;
    vector<FADCHitData> fNegAdcSampData;

    vector<Int_t> fPosAdcErrorFlag;
    vector<Int_t> fNegAdcErrorFlag;

    // TDC data
    vector<TDCData> fPosTdcData;
    vector<TDCData> fNegTdcData;
    
    // Waveform data
    vector<UInt_t> fPosSampWaveform;
    vector<UInt_t> fNegSampWaveform;

    Double_t fTdcRefTime[2];
    Double_t fTdcRefDiffTime[2];
    Double_t fAdcRefTime[2];
    Double_t fAdcRefDiffTime[2];

    Int_t ReadDatabase( const TDatime &date );
    Int_t DefineVariables( EMode mode = kDefine );

    Int_t GetNumPosAdcHits() { return static_cast<Int_t>(fPosAdcData.size()); }
    Int_t GetNumNegAdcHits() { return static_cast<Int_t>(fNegAdcData.size()); }


  ClassDef(HYPTOFPlane, 0);

};

#endif