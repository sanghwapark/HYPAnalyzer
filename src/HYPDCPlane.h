#ifndef HYPDCPlane_h
#define HYPDCPlane_h

#include "THaSubDetector.h"
#include "TClonesArray.h"
#include "HYPData.h"
#include "HYPDCWire.h"
#include "HYPDCCluster.h"

using namespace std;
using namespace HYPData;

namespace DC {
  enum Axis{ kX = 0, kU, kV, kUX, kVX};
}

class HYPDC;

class HYPDCPlane : public THaSubDetector {
  public:
    HYPDCPlane( const char *name, const char* description, THaDetectorBase *parent = nullptr );
    virtual ~HYPDCPlane();

    virtual void    Clear( Option_t *opt = "" );
    virtual EStatus Init( const TDatime &date );
    virtual Int_t   Decode( const THaEvData& );
  
    void  SetPlaneNum(Int_t i) { fPlaneNum = i; }
    void  SetAxis(Int_t axis) { fAxis = axis; }
    void  SetXsp(Double_t x) { fXsp = x; }
    void  SetYsp(Double_t y) { fYsp = y; }
    

    Int_t GetAxis() { return fAxis; }
//    virtual Int_t   Print( Option_t* opt ="" ) const;
    Double_t      GetZ()           const { return fCenter[2]; }
    Double_t      GetXsp()         const { return fXsp; }
    Double_t      GetYsp()         const { return fYsp; }
    Int_t         GetPlaneNum()    const { return fPlaneNum; }
    Int_t         GetNHits()       const { return fHits->GetLast()+1; }
    TClonesArray* GetHits()        const { return fHits; }
    HYPDCHit*     GetHit(Int_t i)  const 
    { assert( i>=0 && i<GetNHits() );
      return (HYPDCHit*)fHits->UncheckedAt(i); }
    HYPDCWire*    GetWire(Int_t i) const 
    { assert( i>=0 && i<=GetNWires() );
      return (HYPDCWire*)fWires->UncheckedAt(i); }
          
  protected:

    Int_t fNHits; // Total number of hits decoded
    Int_t fAxis;
    Int_t fPlaneNum;

    TClonesArray* fHits;
    TClonesArray* fWires;

    // temporary output containers
    vector<UInt_t> v_RawHitTDC;
    vector<UInt_t> v_RawHitOpt;
    vector<UInt_t> v_Chan;
    
    // Geometry parameters
    TVector3 fCenter;
    Double_t fXsp;
    Double_t fYsp;

    virtual Int_t ReadDatabase( const TDatime &date );
    virtual Int_t DefineVariables( EMode mode = kDefine );
    virtual Int_t ReadGeometry( FILE* file, const TDatime& date, Bool_t required = false);
    
    HYPDC* fDC; // parent detector

  ClassDef(HYPDCPlane, 0);

};

#endif
