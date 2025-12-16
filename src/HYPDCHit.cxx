#include "HYPDCHit.h"
#include <iostream>

using namespace std;

//_______________________________________________________
void HYPDCHit::Print( Option_t* opt ) const
{
  
  cout << "Hit: wire=" << GetWireNum()
       << " wpos="     << GetPos()
       << " time="     << GetTime()
       << " drift="    << GetDist();
  if( *opt != 'C' )
    cout << endl;

}

//_______________________________________________________
Int_t HYPDCHit::Compare( const TObject* obj ) const
{
  // Used to sort hits
  // A hit is "less than" another hit if it occurred on a lower wire number.
  // Also, for hits on the same wire, the first hit on the wire (the one with
  // the smallest time) is "less than" one with a higher time.  If the hits
  // are sorted according to this scheme, they will be in order of increasing
  // wire number and, for each wire, will be in the order in which they hit
  // the wire

  if( !obj || IsA() != obj->IsA() || !fWire )
    return -1;

  const HYPDCHit* hit = static_cast<const HYPDCHit*>( obj );

  Int_t myWireNum = fWire->GetWireNum();
  Int_t hitWireNum = hit->GetWire()->GetWireNum();
//  Int_t myPlaneNum = GetPlaneNum();
//  Int_t hitPlaneNum = hit->GetPlaneNum();
//  if (myPlaneNum < hitPlaneNum) return -1;
//  if (myPlaneNum > hitPlaneNum) return 1;

  // If planes are the same, compare wire numbers
  if (myWireNum < hitWireNum) return -1;
  if (myWireNum > hitWireNum) return  1;

  // If wire numbers are the same, compare times
  Double_t hitTime = hit->GetTime();
  if (fTime < hitTime) return -1;
  if (fTime > hitTime) return  1;
  return 0;
  
}

ClassImp(HYPDCHit)
