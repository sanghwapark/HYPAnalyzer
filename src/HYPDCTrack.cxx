#include "HYPDCHit.h"
#include "HYPDCTrack.h"
#include "HYPSpacePoint.h"

HYPDCTrack::HYPDCTrack(Int_t nplanes) : fnSP(0), fNHits(0)
{
  fHits.clear();
  fCoords.resize(nplanes);
  fResiduals.resize(nplanes);
  fResidualsExclPlane.resize(nplanes);
  fDoubleResiduals.resize(nplanes);
}

void HYPDCTrack::AddHit(HYPDCHit * hit, Double_t dist, Int_t lr)
{
  /**
     Add hit to list of hits associated with the track
  */
  Hit newhit;
  newhit.dchit = hit;
  newhit.distCorr = dist;
  newhit.lr = lr;
  fHits.push_back(newhit);
  fNHits++;
}
void HYPDCTrack::RemoveHit(Int_t RemoveHitIndex)
{
  for (Int_t ih=RemoveHitIndex;ih<fNHits-1;ih++) {
    fHits[ih] = fHits[ih+1];
  }
  fNHits = fNHits -1;
  fHits.resize(fNHits);
}
void HYPDCTrack::AddSpacePoint( HYPSpacePoint* sp )
{
  /**
     Add a space point to the list of space points associated with the track.
     All hits in the SP are added to the tracks hit list.
  */

  if (fnSP <10) {
    fSp[fnSP++] = sp;
    // Copy all the hits from the space point into the track
    // Will need to also copy the corrected distance and lr information
    for(Int_t ihit=0;ihit<sp->GetNHits();ihit++) {
      AddHit(sp->GetHit(ihit),sp->GetHitDist(ihit),sp->GetHitLR(ihit));
    }
  }
}

void HYPDCTrack::Clear( const Option_t* )
{
  /**
     Clear the space point and hit lists
  */
  fnSP = 0;
  fSp1_ID=-1;
  fSp2_ID=-1;
  ClearHits();
  // Need to set default values  (0 or -100)
  //fCoords.clear();
  //fResiduals.clear();
  //fDoubleResiduals.clear();
}
void HYPDCTrack::ClearHits( )
{
  fNHits = 0;
  fHits.clear();
}

ClassImp(HYPDCTrack)

///////////////////////////////////////////////////////////////////////////////
