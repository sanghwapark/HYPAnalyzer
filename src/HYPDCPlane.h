#ifndef HYPDCPlane_h
#define HYPDCPlane_h

#include "THaSubDetector.h"
#include "TClonesArray.h"
#include <cassert>
#include <cmath>

class THaEvData;
class THcDCWire;
class HYPDCHit;
class THcDCTimeToDistConv;

using namespace std;

namespace DC {
  enum Axis{ kX = 0, kU, kV, kUX, kVX};
}
 
class HYPDCPlane : public THaSubDetector {
  public:
    HYPDCPlane( const char *name, const char* description, Int_t planenum, THaDetectorBase *parent = nullptr );
    virtual ~HYPDCPlane();

    virtual void    Clear( Option_t *opt = "" );
    virtual EStatus Init( const TDatime &date );
    virtual Int_t   Decode( const THaEvData& );
    virtual Int_t   ProcessHits(TClonesArray* rawhits, Int_t nexthit);

    virtual Int_t SubtractStartTime();
    virtual Int_t GetReadoutSide(Int_t wirenum);

    //    virtual Int_t   Print( Option_t* opt ="" ) const;
    void  SetPlaneNum(Int_t i) { fPlaneNum = i; }
    void  SetAxis(Int_t axis) { fAxis = axis; }
    
    Int_t         GetAxis() { return fAxis; }
    Int_t         GetNHits() const { return fHits->GetLast()+1; }
    Int_t         GetNRawhits() const {return fNRawhits; }
    TClonesArray* GetHits()  const { return fHits; }

    Int_t         GetNWires()      const { return fWires->GetLast()+1; }
    THcDCWire*    GetWire(Int_t i) const 
    { assert( i>=0 && i<=GetNWires() );
      return (THcDCWire*)fWires->UncheckedAt(i); }

    Int_t        GetPlaneNum() const { return fPlaneNum; }
    Int_t        GetChamberNum() const { return fChamberNum; }
    void         SetPlaneIndex(Int_t index) { fPlaneIndex = index; }
    Int_t        GetPlaneIndex() { return fPlaneIndex; }
    Double_t     GetXsp() const { return fXsp; }
    Double_t     GetYsp() const { return fYsp; }
    Int_t        GetReadoutX() { return fReadoutX; }
    Double_t     GetReadoutCorr() { return fReadoutCorr; }
    Double_t     GetCentralTime() { return fCentralTime; }
    Int_t        GetDriftTimeSign() { return fDriftTimeSign; }
    Double_t     GetBeta() { return fBeta; }
    Double_t     GetSigma() { return fSigma; }
    Double_t     GetPsi0() { return fPsi0; }
    Double_t*    GetStubCoef() { return fStubCoef; }
    Double_t*    GetPlaneCoef() { return fPlaneCoef; }
    Double_t     CalcWireFromPos(Double_t pos);
    Int_t        GetReadoutLR() const { return fReadoutLR;}
    Int_t        GetReadoutTB() const { return fReadoutTB;}
    Int_t        GetVersion() const {return fVersion;}

    HYPDCPlane(); // for ROOT I/O

  protected:

  Int_t fAxis;

  TClonesArray* fHits;
  TClonesArray* fRawHits;
  TClonesArray* fWires;
  TClonesArray* fFirstPassHits;

  Double_t fTdcRefTime;

  Int_t fVersion;
  Int_t fWireOrder;
  Int_t fPlaneNum;
  Int_t fPlaneIndex;		/* Index of this plane within it's chamber */
  Int_t fChamberNum;
  Int_t fUsingTzeroPerWire;
  Int_t fUsingSigmaPerWire;
  Int_t fNRawhits;
  Int_t fNWires;
  Int_t fTdcWinMin;
  Int_t fTdcWinMax;
  Double_t fPitch;
  Double_t fCentralWire;
  Double_t fPlaneTimeZero;
  Double_t fXsp;
  Double_t fYsp;
  Double_t fSigma;
  Double_t fPsi0;
  Double_t fStubCoef[4];
  Double_t fBeta;
  Double_t fPlaneCoef[9];

  Double_t fMin_DriftTime;
  Double_t fMax_DriftTime;
  
  Int_t    fReadoutX;
  Double_t fReadoutCorr;
  Double_t fCentralTime;
  Int_t    fDriftTimeSign;
  Int_t    fReadoutLR;
  Int_t    fReadoutTB;

  Double_t  fCenter;
  Double_t  fNSperChan;		/* TDC bin size */
  Double_t* fTzeroWire;

  virtual Int_t  ReadDatabase( const TDatime& date );
  virtual Int_t  DefineVariables( EMode mode = kDefine );

  THcDCTimeToDistConv* fTTDConv;  // Time-to-distance converter for this plane's wires

  ClassDef(HYPDCPlane, 0);
};
#endif
