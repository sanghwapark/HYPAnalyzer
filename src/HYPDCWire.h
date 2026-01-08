#ifndef HYPDCWire_h
#define HYPDCWire_h

#include "TObject.h"

class HYPDCWire : public TObject {

public:
  HYPDCWire( Int_t wire_num, Int_t axis, Double_t pos, Double_t offset) :
    fWireNum(wire_num), fPos(pos), fTOffset(offset) {}
  virtual ~HYPDCWire() {}

  Int_t    GetNum() const { return fWireNum; }
  Int_t    GetFlag()    const { return fFlag; }
  Double_t GetPos()     const { return fPos; }
  Double_t GetTOffset() const { return fTOffset; }
  Int_t    GetAxis() const { return fAxis; }

  void SetWireNum(Int_t num) { fWireNum = num; }
  void SetFlag(Int_t flag) { fFlag = flag; }
  void SetPos(Double_t pos) { fPos = pos; }
  void SetTOffset(Double_t offset) { fTOffset = offset; }
  void SetAxis(Int_t axis ) { fAxis = axis; }
  
protected:
  Int_t    fWireNum;     // wire number
  Int_t    fFlag;        // bad wire flag
  Double_t fPos;         // wire position
  Double_t fTOffset;     // Time offset
  Int_t    fAxis;        // axis (u, x, y)

private:
  HYPDCWire( const HYPDCWire& );
  HYPDCWire& operator=( const HYPDCWire& );

  ClassDef(HYPDCWire,0);
};

#endif
