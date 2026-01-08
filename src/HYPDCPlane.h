#ifndef HYPDCPlane_h
#define HYPDCPlane_h

#include "THaSubDetector.h"
#include "TClonesArray.h"
#include "HYPData.h"
#include "HYPDCWire.h"

using namespace std;
using namespace HYPData;

namespace DC {
  enum Axis{ kX, kU, kV};
}

class HYPDC;

class HYPDCPlane : public THaSubDetector {
  public:
    HYPDCPlane( const char *name, const char* description, THaDetectorBase *parent = nullptr );
    virtual ~HYPDCPlane();

    virtual void    Clear( Option_t *opt = "" );
    virtual EStatus Init( const TDatime &date );
    virtual Int_t   Decode( const THaEvData& );
  
    Int_t SetAxis(Int_t axis) { return fAxis = axis; }
    Int_t GetAxis() { return fAxis; }
//    virtual Int_t   Print( Option_t* opt ="" ) const;

    TClonesArray* GetHits()        const { return fHits; }
    Int_t         GetNHits()       const { return fHits->GetLast()+1; }
    HYPDCWire*    GetWire(Int_t i) const 
    { assert( i>=0 && i<=GetNWires() );
      return (HYPDCWire*)fWires->UncheckedAt(i); }
          
  protected:

    Int_t fNHits; // Total number of hits decoded
    Int_t fAxis;

    TClonesArray* fHits;
    TClonesArray* fWires;

    // temporary output containers
    vector<UInt_t> v_RawHitTDC;
    vector<UInt_t> v_RawHitOpt;
    vector<UInt_t> v_Chan;
    
    virtual Int_t ReadDatabase( const TDatime &date );
    virtual Int_t DefineVariables( EMode mode = kDefine );
    virtual Int_t ReadGeometry( FILE* file, const TDatime& date, Bool_t required = false);
    
    HYPDC* fDC; // parent detector

  ClassDef(HYPDCPlane, 0);

};

#endif
