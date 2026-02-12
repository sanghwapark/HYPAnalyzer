#include "HYPDC.h"
#include "HYPDCChamber.h"
#include "HYPDCCluster.h"
#include "HYPSpacePoint.h"
#include "HYPDCPlane.h"
#include "THaApparatus.h"
#include "THcGlobals.h"
#include "THcParmList.h"
#include "VarDef.h"
#include "VarType.h"
#include "THaTrack.h"
#include "TClonesArray.h"
#include "TVectorD.h"
#include "TMath.h"
#include <sstream>
#include <string>

#define MAX_SPACE_POINTS 100
#define MAX_HITS_PER_POINT 20
#define MAX_NUMBER_PAIRS 1000
//____________________________________________________
HYPDCChamber::HYPDCChamber( const char* name, const char* description, 
  Int_t chambernum, THaDetectorBase* parent) :
  THaSubDetector(name, description, parent),
  fNPlanes(0), fNHits(0), fChamberNum(chambernum), fNSpacePoints(0)
{
  // constructor
  fSpacePoints = new TClonesArray("HYPSpacePoint",10);
  fUClusters = new TClonesArray("HYPDCCluster", 10);
  fVClusters = new TClonesArray("HYPDCCluster", 10);
  fXClusters = new TClonesArray("HYPDCCluster", 10);
  fUXClusters = new TClonesArray("HYPDCCluster", 10);
  fVXClusters = new TClonesArray("HYPDCCluster", 10);
}

//____________________________________________________
HYPDCChamber::~HYPDCChamber()
{
  // destructor
  for(auto plane : fPlanes )
    delete plane;
  fPlanes.clear();

  delete fUClusters;
  delete fVClusters;
  delete fXClusters;
  delete fUXClusters;
  delete fVXClusters;

  if( fIsSetup )
    RemoveVariables();
  if( fIsInit )
    DeleteArrays();
}

//____________________________________________________
HYPDCChamber::HYPDCChamber() :
  THaSubDetector()
{
  // Constructor
  fTrackProj = nullptr;
  fSpacePoints = nullptr;
  fXClusters = nullptr;
  fUClusters = nullptr;
  fVClusters = nullptr;
  fUXClusters = nullptr;
  fVXClusters = nullptr;
  fIsInit = 0;

}

//____________________________________________________
void HYPDCChamber::DeleteArrays()
{
  // Delete member arrays. Used by destructor.
  delete [] fCosBeta; fCosBeta = nullptr;
  delete [] fSinBeta; fSinBeta = nullptr;
  delete [] fTanBeta; fTanBeta = nullptr;
  delete [] fSigma; fSigma = nullptr;
  delete [] fPsi0; fPsi0 = nullptr;
  delete [] fStubCoefs; fStubCoefs = nullptr;
}

//____________________________________________________
THaAnalysisObject::EStatus HYPDCChamber::Init(const TDatime &date)
{

  // cout << "HYPDCChamber::Init()" << endl;
  EStatus status;
  // calls ReadDatabaseas and DefineVariables
  if( (status = THaSubDetector::Init(date)) )
    return fStatus = status;

  return fStatus = kOK;
}

//____________________________________________________
void HYPDCChamber::AddPlane(HYPDCPlane *plane)
{
  plane->SetPlaneIndex(fNPlanes);
  fPlanes.push_back(plane);

  // Hard code X plane numbers.  
  // FIXME: instead check axis (kU, kX, kV)
  if(fChamberNum == 1) {
    XPlaneNum=3;
    XPlanePNum=4;
    if(plane->GetPlaneNum() == 3) XPlaneInd = fNPlanes;
    if(plane->GetPlaneNum() == 4) XPlanePInd = fNPlanes;
  } else {
    XPlaneNum=9;
    XPlanePNum=10;
    if(plane->GetPlaneNum() == 9) XPlaneInd = fNPlanes;
    if(plane->GetPlaneNum() == 10) XPlanePInd = fNPlanes;
  }
  fNPlanes++;
  return;
}

//____________________________________________________
void HYPDCChamber::Clear( Option_t* opt )
{
  fNHits = 0;
  fHits.clear();
}

//____________________________________________________
Int_t HYPDCChamber::Decode( const THaEvData& evdata )
{
  return 0;
}

//________________________________________________________________
void HYPDCChamber::ProcessHits()
{
  // Make a list of hits for whole chamber
  fNHits = 0;
  fHits.clear();
  fHits.reserve(40);
  fSpHit.clear();
  for(Int_t ip=0;ip<fNPlanes;ip++) {
    TClonesArray* hitsarray = fPlanes[ip]->GetHits();
    for(Int_t ihit=0;ihit<fPlanes[ip]->GetNHits();ihit++) {
      fHits.push_back(static_cast<HYPDCHit*>(hitsarray->At(ihit)));
      fNHits++;
    }
  }
}

//________________________________________________________________
Int_t HYPDCChamber::ReadDatabase( const TDatime& date )
{
  // Read parameters 
  // Called by ThaDetectorBase::Init()   

  char prefix[2];
  prefix[0]=tolower(GetApparatus()->GetName()[0]);
  prefix[1]='\0';
  DBRequest list[]={
    {"dc_wire_velocity", &fWireVelocity, kDouble},
    {"SmallAngleApprox", &fSmallAngleApprox, kInt,0,1},
    {"stub_max_xpdiff",  &fStubMaxXPDiff, kDouble,0,1},
    {"debugflagpr",      &fhdebugflagpr, kInt},
    {"debugstubchisq",   &fdebugstubchisq, kInt},
    {Form("dc_%d_zpos", fChamberNum), &fZPos, kDouble},
    {0}
  };

  // Default values
  fSmallAngleApprox = 0;
  fRatio_xpfp_to_xfp = 0.0011; // FIXME: use HMS value as default here, check
  fStubMaxXPDiff = 999.;	  

  gHcParms->LoadParmValues((DBRequest*)&list,prefix);
  
  // Get parameters parent knows about
  fParent = GetParent();
  fMinHits = static_cast<HYPDC*>(fParent)->GetMinHits(fChamberNum);
  fMaxHits = static_cast<HYPDC*>(fParent)->GetMaxHits(fChamberNum);
  fMinCombos = static_cast<HYPDC*>(fParent)->GetMinCombos(fChamberNum);
  fSpacePointCriterion = static_cast<HYPDC*>(fParent)->GetSpacePointCriterion(fChamberNum);

  // Generate the HAA3INV matrix for all the acceptable combinations
  // of hit planes.  Try to make it as generic as possible
  // pindex=0 -> Plane 1 missing, pindex5 -> plane 6 missing.  Won't
  // replicate the exact values used in the ENGINE, because the engine
  // had one big list of matrices for both chambers, while here we will
  // have a list just for one chamber.  Also, call pindex, pmindex as
  // we tend to use pindex as a plane index.
  fCosBeta = new Double_t [fNPlanes];
  fSinBeta = new Double_t [fNPlanes];
  fTanBeta = new Double_t [fNPlanes];
  fSigma = new Double_t [fNPlanes];
  fPsi0 = new Double_t [fNPlanes];
  fStubCoefs = new Double_t* [fNPlanes];
  Int_t allplanes=0;
  for(Int_t ip = 0; ip < fNPlanes; ip++) {
    fCosBeta[ip] = TMath::Cos(fPlanes[ip]->GetBeta());
    fSinBeta[ip] = TMath::Sin(fPlanes[ip]->GetBeta());
    fTanBeta[ip] = fSinBeta[ip]/fCosBeta[ip];
    fSigma[ip] = fPlanes[ip]->GetSigma();
    fPsi0[ip] = fPlanes[ip]->GetPsi0();
    fStubCoefs[ip] = fPlanes[ip]->GetStubCoef();
    allplanes |= 1<<ip;
  }

  // Unordered map introduced in C++-11
  // Can use unordered_map if using C++-11
  // May not want to use map at all for performance, but using it now
  // for code clarity
  for(Int_t ipm1=0;ipm1<fNPlanes+1;ipm1++) { // Loop over missing plane1
    for(Int_t ipm2=ipm1;ipm2<fNPlanes+1;ipm2++) {
      if(ipm1==ipm2 && ipm1<fNPlanes) continue;
      TMatrixD AA3(3,3);
      for(Int_t i=0;i<3;i++) {
        for(Int_t j=i;j<3;j++) {
          AA3[i][j] = 0.0;
          for(Int_t ip=0;ip<fNPlanes;ip++) {
            if(ipm1 != ip && ipm2 != ip) {
              AA3[i][j] += fStubCoefs[ip][i]*fStubCoefs[ip][j];
            }
          }
          AA3[j][i] = AA3[i][j];
	      }
      }
      Int_t bitpat = allplanes & ~(1<<ipm1) & ~(1<<ipm2);
      // Should check that it is invertable
      //      if (fhdebugflagpr) cout << bitpat << " Determinant: " << AA3->Determinant() << endl;
      AA3.Invert();
      fAA3Inv[bitpat].ResizeTo(AA3);
      fAA3Inv[bitpat] = AA3;
    }
  }

  fIsInit = true;
  return kOK;

}
//____________________________________________________
Int_t HYPDCChamber::DefineVariables( EMode mode )
{

  if( mode == kDefine && fIsSetup ) return kOK;
  fIsSetup = ( mode == kDefine );
  // Register variables in global list

  RVarDef vars[] = {
    { "maxhits",     "Maximum hits allowed",    "fMaxHits" },
    { "nspacepoints", "Space points of DC",      "fNSpacePoints" },
    { "nhit", "Number of DC hits",  "fNHits" },
    { "sp_nhits", "", "fSpacePoints.HYPSpacePoint.GetNHits()" },
    { "stub_x", "", "fSpacePoints.HYPSpacePoint.GetStubX()" },
    { "stub_xp", "", "fSpacePoints.HYPSpacePoint.GetStubXP()" },
    { "stub_y", "", "fSpacePoints.HYPSpacePoint.GetStubY()" },
    { "stub_yp", "", "fSpacePoints.HYPSpacePoint.GetStubYP()" },
    { "stub_chi2", "", "fSpacePoints.HYPSpacePoint.GetChi2()" },  
    { "ncombos", "", "fSpacePoints.HYPSpacePoint.GetCombos()" },
    { "U_pos", "", "fUClusters.HYPDCCluster.GetX()" },
    { "X_pos", "", "fXClusters.HYPDCCluster.GetX()" },
    { "V_pos", "", "fVClusters.HYPDCCluster.GetX()" },
    { "UX_posx", "", "fUXClusters.HYPDCCluster.GetX()" },
    { "UX_posy", "", "fUXClusters.HYPDCCluster.GetY()" },
    { "VX_posx", "", "fVXClusters.HYPDCCluster.GetX()" },
    { "VX_posy", "", "fVXClusters.HYPDCCluster.GetY()" },
    { nullptr }
  };
  DefineVarsFromList( vars, mode );

  std::vector<RVarDef> ve;
  ve.push_back( { "sphit", "", "fSpHit.SpNHits" });
  ve.push_back(  { "sphit_index", "", "fSpHit.SpHitIndex" });
  ve.push_back({0}); // Needed to specify the end of list
  return DefineVarsFromList( ve.data(), mode );
}

//____________________________________________________
Int_t HYPDCChamber::FindSpacePoints()
{

  // 1. Form U, X, V plane clusters
  // 2. Form UX, VX cluster pairs 
  // 3. Loop over UX and VX cluster pair combinations to find space points. 
  //  - Here we check if two clusters have matching x hits
  //  - If the distance between two pairs is within fSpacePointCriterion 
  //  - Total number of hits >= fMinHits

  fNSpacePoints = 0;

  // cluster arrays
  fUClusters->Clear("C");
  fVClusters->Clear("C");
  fXClusters->Clear("C");
  fUXClusters->Clear("C");
  fVXClusters->Clear("C");

  Int_t nuclus = 0;
  Int_t nvclus = 0;
  Int_t nxclus = 0;

  // U, X, V plane clusters
  if(fNHits >= fMinHits && fNHits < fMaxHits ) {
    for(Int_t ihit = 0; ihit < fNHits; ihit++) {
      HYPDCHit* hit1 = fHits[ihit];
      for(Int_t jhit = ihit+1; jhit < fNHits; jhit++) {
        HYPDCHit* hit2 = fHits[jhit];
        if( hit1->GetPlaneNum() != hit2->GetPlaneNum() && abs(hit1->GetPos() - hit2->GetPos()) < 0.6) {

          Double_t x = (hit1->GetPos() + hit2->GetPos())/2.0;
          Double_t y = 0.0;

          HYPDCCluster* clus = nullptr;
          if(hit1->GetAxis() == DC::kU){
            clus = (HYPDCCluster*)fUClusters->ConstructedAt(nuclus++);
	        }
          else if(hit1->GetAxis() == DC::kV){
            clus = (HYPDCCluster*)fVClusters->ConstructedAt(nvclus++);
	        }
          else {
            clus = (HYPDCCluster*)fXClusters->ConstructedAt(nxclus++);
      	  }
          clus->AddHit(hit1);
          clus->AddHit(hit2);
          clus->SetXY(x, y);
          
          hit1->IncrNPlaneClust();
          hit2->IncrNPlaneClust();
	      }
      } // hit2
      
      // single plane cluster
      if(hit1->GetNPlaneClust() == 0) {
          Double_t x = hit1->GetPos();
          Double_t y = 0.0;

          HYPDCCluster* clus;
          if(hit1->GetAxis() == DC::kU){
            clus = (HYPDCCluster*)fUClusters->ConstructedAt(nuclus++);
      	  }
          else if(hit1->GetAxis() == DC::kV){
            clus = (HYPDCCluster*)fVClusters->ConstructedAt(nvclus++);
	        }
          else {
            clus = (HYPDCCluster*)fXClusters->ConstructedAt(nxclus++);
	        }
          clus->AddHit(hit1);
          clus->SetXY(x,y);
          hit1->IncrNPlaneClust();
      }
    } // hit1
  }
  
  // now form UX, VX pairs
  Int_t nuxclus = 0;
  for(Int_t iu = 0; iu < nuclus; iu++) {
    HYPDCCluster *uclus = static_cast<HYPDCCluster*>(fUClusters->At(iu));
    Double_t upos = uclus->GetX();

    HYPDCHit* uhit = uclus->GetHit(0);
    HYPDCPlane* uplane = uhit->GetWirePlane();

    for(Int_t ix = 0; ix < nxclus; ix++) {
      HYPDCCluster *xclus = static_cast<HYPDCCluster*>(fXClusters->At(ix));
      Double_t xpos = xclus->GetX();

      HYPDCHit* xhit = xclus->GetHit(0); // first hit in the cluster
      HYPDCPlane* xplane = xhit->GetWirePlane();

      // calculate pos
  	  Double_t determinate = uplane->GetXsp() * xplane->GetYsp() - uplane->GetYsp() * xplane->GetXsp();
  	  Double_t x = (upos*xplane->GetYsp()- xpos*uplane->GetYsp())/determinate;
	    Double_t y = (xpos*uplane->GetXsp()- upos*xplane->GetXsp())/determinate;

      HYPDCCluster* clus = (HYPDCCluster*)fUXClusters->ConstructedAt(nuxclus++);

      for (Int_t ih=0;ih<uclus->GetNHits();ih++) {
	      HYPDCHit* temphit = uclus->GetHit(ih);
	      clus->AddHit(temphit);
	    }
	    for (Int_t ih=0;ih<xclus->GetNHits();ih++) {
	      HYPDCHit* temphit = xclus->GetHit(ih);
	      clus->AddHit(temphit);
	    }
      clus->SetXY(x,y);
    }
  }

  Int_t nvxclus = 0;
  for(Int_t iv = 0; iv < nvclus; iv++) {
    HYPDCCluster *vclus = static_cast<HYPDCCluster*>(fVClusters->At(iv));
    Double_t vpos = vclus->GetX();
    HYPDCHit* vhit = vclus->GetHit(0);
    HYPDCPlane* vplane = vhit->GetWirePlane();

    for(Int_t ix = 0; ix < nxclus; ix++) {
      HYPDCCluster *xclus = static_cast<HYPDCCluster*>(fXClusters->At(ix));
      Double_t xpos = xclus->GetX();

      HYPDCHit* xhit = xclus->GetHit(0); // first hit in the cluster
      HYPDCPlane* xplane = xhit->GetWirePlane();

      // calculate pos
  	  Double_t determinate = vplane->GetXsp() * xplane->GetYsp() - vplane->GetYsp() * xplane->GetXsp();
  	  Double_t x = (vpos*xplane->GetYsp()- xpos*vplane->GetYsp())/determinate;
	    Double_t y = (xpos*vplane->GetXsp()- vpos*xplane->GetXsp())/determinate;
      
      HYPDCCluster* clus = (HYPDCCluster*)fVXClusters->ConstructedAt(nvxclus++);

      for (Int_t ih=0;ih<vclus->GetNHits();ih++) {
	      HYPDCHit* temphit = vclus->GetHit(ih);
	      clus->AddHit(temphit);
	    }
	    for (Int_t ih=0;ih<xclus->GetNHits();ih++) {
	      HYPDCHit* temphit = xclus->GetHit(ih);
	      clus->AddHit(temphit);
	    }
      clus->SetXY(x,y);
    }
  }

  vector <HYPDCHit*> UX_uHits;
  vector <HYPDCHit*> UX_xHits;
  vector <HYPDCHit*> VX_vHits;
  vector <HYPDCHit*> VX_xHits;

  // Loop over ux and vx cluster pairs
  for(Int_t i = 0; i < nuxclus; i++) {
    HYPDCCluster* uxclus = static_cast<HYPDCCluster*>(fUXClusters->At(i));
    Double_t pos_x1 = uxclus->GetX();
    Double_t pos_y1 = uxclus->GetY();

    // Get U, X hits from the UX clusters
    for (Int_t ih = 0; ih < uxclus->GetNHits(); ih++) {
	     HYPDCHit* hit1 = uxclus->GetHit(ih);
       if( hit1->GetAxis() == DC::kU ) UX_uHits.push_back(hit1);
       if( hit1->GetAxis() == DC::kX ) UX_xHits.push_back(hit1);
    }

    for(Int_t j = 0; j < nvxclus; j++) {
      HYPDCCluster* vxclus = static_cast<HYPDCCluster*>(fVXClusters->At(j));
      Double_t pos_x2 = vxclus->GetX();
      Double_t pos_y2 = vxclus->GetY();

      // Get V, X hits from the VX clusters
      for (Int_t ih = 0; ih < vxclus->GetNHits(); ih++) {
        HYPDCHit* hit1 = vxclus->GetHit(ih);
        if( hit1->GetAxis() == DC::kV ) VX_vHits.push_back(hit1);
        if( hit1->GetAxis() == DC::kX ) VX_xHits.push_back(hit1);
      }

	    // Check ux and vx clusters have common x hits
      Bool_t Xhits_match = kFALSE;
      if (VX_xHits.size() == UX_xHits.size() && UX_xHits.size()==1) {
	      if (VX_xHits[0] == UX_xHits[0]) Xhits_match = kTRUE;
	    } else if (VX_xHits.size() == UX_xHits.size() && UX_xHits.size()==2) {
	      if (VX_xHits[0] == UX_xHits[0] && VX_xHits[1] == UX_xHits[1])  Xhits_match = kTRUE;
	      if (VX_xHits[0] == UX_xHits[1] && VX_xHits[1] == UX_xHits[0])  Xhits_match = kTRUE;
   	  }

      // Calculate distance between ux and vx clusters
      Double_t dist2 = pow(pos_x1-pos_x2,2) + pow(pos_y1-pos_y2,2);

      // Total number of hits for the cluster
  	  Int_t TotHits = UX_uHits.size()+UX_xHits.size()+VX_vHits.size();

      // Save spacepoints if passed cuts
    	if (dist2 <= fSpacePointCriterion && Xhits_match && TotHits >= fMinHits) {
        HYPSpacePoint* sp = (HYPSpacePoint*)fSpacePoints->ConstructedAt(fNSpacePoints++);
        sp->Clear();
        
        Double_t x = (pos_x1 + pos_x2) / 2.0;
        Double_t y = (pos_y1 + pos_y2) / 2.0;
        sp->SetXY(x, y);
        sp->SetCombos(1);
        
        for (UInt_t ih=0;ih<UX_uHits.size();ih++) { sp->AddHit(UX_uHits[ih]);}
        for (UInt_t ih=0;ih<UX_xHits.size();ih++) { sp->AddHit(UX_xHits[ih]);}
        for (UInt_t ih=0;ih<VX_vHits.size();ih++) { sp->AddHit(VX_vHits[ih]);}
        fSpHit.SpNHits.push_back(sp->GetNHits());
        for(Int_t ihit1 = 0; ihit1 < fNHits; ihit1++) {
          HYPDCHit* hit1=fHits[ihit1];
          for(Int_t isph = 0; isph < sp->GetNHits(); isph++) {
            HYPDCHit* hitsp=sp->GetHit(isph);
            if (hitsp==hit1) fSpHit.SpHitIndex.push_back(ihit1);
          }
        }
      }
    }
  }// loop over ux and vx cluster pairs

  return fNSpacePoints;

}

//____________________________________________________
void HYPDCChamber::CorrectHitTimes()
{
  /**
   Use the rough hit positions in the chambers to correct the drift time
   for hits in the space points.

   Assume all wires for a plane are read out on the same side (l/r or t/b).
   If the wire is closer to horizontal, read out left/right.  If nearer
   vertical, assume top/bottom.  (Note, this is not always true for the
   SOS u and v planes.  They have 1 card each on the side, but the overall
   time offset per card will cancel much of the error caused by this.  The
   alternative is to check by card, rather than by plane and this is harder.
  */

  for(Int_t isp = 0; isp < fNSpacePoints; isp++) {
    HYPSpacePoint* sp = (HYPSpacePoint*)(*fSpacePoints)[isp];
    Double_t x = sp->GetX();
    Double_t y = sp->GetY();
    for(Int_t ihit=0;ihit<sp->GetNHits();ihit++) {
      HYPDCHit* hit = sp->GetHit(ihit);
      HYPDCPlane* plane=hit->GetWirePlane();
      
      // This applies the wire velocity correction for new SHMS chambers --hszumila, SEP17
      Int_t pln = hit->GetPlaneNum();
      Int_t readoutSide = hit->GetReadoutSide();

      Double_t posn = hit->GetPos();
      //The following values are determined from param file as permutations on planes 5 and 10
      Int_t readhoriz = plane->GetReadoutLR();
      Int_t readvert = plane->GetReadoutTB();

      //+x is up and +y is beam right!
      double alpha = static_cast<HYPDC*>(fParent)->GetAlphaAngle(pln);
      double xc = posn*TMath::Sin(alpha);
      double yc = posn*TMath::Cos(alpha);

      Double_t wireDistance = plane->GetReadoutX() ?
        (abs(y-yc))*abs(plane->GetReadoutCorr()) :
        (abs(x-xc))*abs(plane->GetReadoutCorr());

      //Readout side is based off wiring diagrams
      switch (readoutSide){
      case 1: //readout from top of chamber
        if (x>xc){wireDistance = -readvert*wireDistance;}
        else{wireDistance = readvert*wireDistance;}
        
        break;
      case 2://readout from right of chamber
        if (y>yc){wireDistance = -readhoriz*wireDistance;}
        else{wireDistance = readhoriz*wireDistance;}
      
        break;
      case 3: //readout from bottom of chamber
        if (xc>x){wireDistance= -readvert*wireDistance;}
        else{wireDistance = readvert*wireDistance;}

        break;
      case 4: //readout from left of chamber
        if(yc>y){wireDistance = -readhoriz*wireDistance;}
        else{wireDistance = readhoriz*wireDistance;}

        break;
      default:
        wireDistance = 0.0;
      }

	    Double_t timeCorrection = wireDistance/fWireVelocity;

	    // New behavior: Save corrected distance with the hit in the space point
      // so that the same hit can have a different correction depending on
      // which space point it is in.
      //
      // This is a hack now because the converttimetodist method is connected to the hit
      // so I compute the corrected time and distance, and then restore the original
      // time and distance.  Can probably add a method to hit that does a conversion on a time
      // but does not modify the hit data.
      // FIXME -- Implement as it is, but yeah gotta fix this

        Double_t time=hit->GetTime();
        Double_t dist=hit->GetDist();

        hit->SetTime(time - timeCorrection);
        hit->ConvertTimeToDist();
        sp->SetHitDist(ihit, hit->GetDist());

        hit->SetTime(time);	// Restore time
        hit->SetDist(dist);	// Restore distance

    }// Loop over hits for the given space point
  }// spacepoint loop

}

//____________________________________________________
void HYPDCChamber::LeftRight()
{
  /**
     For each space point,
     Fit stubs to all possible left-right combinations of drift distances
     and choose the set with the minimum chi**2.
  */
  for(Int_t isp=0; isp<fNSpacePoints; isp++) {
    // Build a bit pattern of which planes are hit
    HYPSpacePoint* sp = (HYPSpacePoint*)(*fSpacePoints)[isp];
    Int_t nhits = sp->GetNHits();
    UInt_t bitpat  = 0;		// Bit pattern of which planes are hit
    Double_t maxchi2= 1.0e10;
    Double_t minchi2 = maxchi2;
    Double_t tmp_minchi2=maxchi2;
    // Double_t minxp = 0.25;

    Int_t plusminusknown[nhits];
    Int_t plusminusbest[nhits];
    Int_t plusminus[nhits];	
    Int_t tmp_plusminus[nhits];
    Int_t plane_list[nhits];
    Double_t stub[4];
    Double_t tmp_stub[4];
    Int_t nplusminus;

    if(nhits < 0) {
      if (fhdebugflagpr) cout << "HYPDCChamber::LeftRight() nhits < 0" << endl;
    } else if (nhits==0) {
      if (fhdebugflagpr) cout << "HYPDCChamber::LeftRight() nhits = 0" << endl;
    }
    for(Int_t ihit=0;ihit < nhits;ihit++) {
      HYPDCHit* hit = sp->GetHit(ihit);
      Int_t pindex = hit->GetPlaneIndex();
      plane_list[ihit] = pindex;

      bitpat |= 1<<pindex;

      plusminusknown[ihit] = 0;
    }
    nplusminus = 1<<nhits;

    if(fSmallAngleApprox !=0) {
    	Int_t npaired = 0;
      for(Int_t ihit1 = 0; ihit1 < nhits; ihit1++) {
        HYPDCHit* hit1 = sp->GetHit(ihit1);
        Int_t pindex1 = hit1->GetPlaneIndex();
        if((pindex1%2)==0) { // Odd plane (or even index)
          for(Int_t ihit2 = 0; ihit2 < nhits; ihit2++) {
            HYPDCHit* hit2 = sp->GetHit(ihit2);
            if(hit2->GetPlaneIndex()-pindex1 == 1 && TMath::Abs(hit2->GetPos()-hit1->GetPos())<0.51) { // Adjacent plane
            if(hit2->GetPos() <= hit1->GetPos() ) {
              plusminusknown[ihit1] = -1;
              plusminusknown[ihit2] = 1;
            } else {
              plusminusknown[ihit1] = 1;
              plusminusknown[ihit2] = -1;
            }
            npaired+=2;
            }
          }// hit2 loop
	      }
      }// hit1 loop
	    nplusminus = 1 << (nhits-npaired);
    }// no small angle approx

    Int_t nplaneshit = Count1Bits(bitpat);
    if (fhdebugflagpr) cout << " num of pm = " << nplusminus << " num of hits =" << nhits << endl;
    // Use bit value of integer word to set + or -
    // Loop over all combinations of left right.
    for(Int_t pmloop = 0; pmloop < nplusminus; pmloop++) {
      Int_t iswhit = 1;
      for(Int_t ihit = 0; ihit < nhits; ihit++) {
      	if(plusminusknown[ihit]!=0) {
	        plusminus[ihit] = plusminusknown[ihit];
	      } else {
	        // Max hits per point has to be less than 32.
	        if(pmloop & iswhit) {
	          plusminus[ihit] = 1;
	        } else {
	          plusminus[ihit] = -1;
      	  }
	        iswhit <<= 1;
	      }
      }
      if ( nplaneshit >= fNPlanes-2 ) {
      	Double_t chi2;
	      chi2 = FindStub(nhits, sp,plane_list, bitpat, plusminus, stub);
	      if (fdebugstubchisq) cout << " pmloop = " << pmloop << " chi2 = " << chi2 << endl;
	      if(chi2 < minchi2) {
	        if (fStubMaxXPDiff < 100. ) {
            // Take best chi2 IF x' of the stub agrees with x' as expected from x.
            // Sometimes an incorrect x' gives a good chi2 for the stub, even though it is
            // not the correct left/right combination for the real track.
            // Rotate x'(=stub(3)) to hut coordinates and compare to x' expected from x.
            // THIS ASSUMES STANDARD HMS TUNE!!!!, for which x' is approx. x/875.
            if(stub[2]*fTanBeta[plane_list[0]]==-1.0) {
	            if (fhdebugflagpr) cout << "HYPDCChamber::LeftRight() Error 3" << endl;
      	    }
	          Double_t xp_fit = stub[2]-fTanBeta[plane_list[0]] / (1+stub[2]*fTanBeta[plane_list[0]]);
      	    Double_t xp_expect = sp->GetX() * fRatio_xpfp_to_xfp;
	          if(TMath::Abs(xp_fit-xp_expect)<fStubMaxXPDiff) {
	            minchi2 = chi2;
	            for(Int_t ihit=0;ihit<nhits;ihit++) {
		            plusminusbest[ihit] = plusminus[ihit];
	            }
              sp->SetStub(stub);
	          } else {		// Record best stub failing angle cut
              if (chi2 < tmp_minchi2) {
		            tmp_minchi2 = chi2;
		            for(Int_t ihit = 0; ihit < nhits; ihit++) {
		              tmp_plusminus[ihit] = plusminus[ihit];
		            }
		            for(Int_t i = 0; i < 4; i++) {
            		  tmp_stub[i] = stub[i];
		            }
	            }
      	    }
	        } else { // Not HMS specific
	          minchi2 = chi2;
	          for(Int_t ihit=0;ihit<nhits;ihit++) {
	            plusminusbest[ihit] = plusminus[ihit];
	          }
            sp->SetStub(stub);
	        }
      	} 


      } else {
	      if (fhdebugflagpr) cout << "Insufficient planes hit in HYPDCChamber::LeftRight()" << bitpat <<endl;
        // do nothing
      }
    } // End loop of pm combinations

    if (minchi2 == maxchi2 && tmp_minchi2 == maxchi2) {
      // do nothing
    } else {
      if(minchi2 == maxchi2 ) {	// No track passed angle cut
	      minchi2 = tmp_minchi2;
	      for(Int_t ihit=0;ihit<nhits;ihit++) {
	        plusminusbest[ihit] = tmp_plusminus[ihit];
	      }
	      sp->SetStub(tmp_stub);
      }
      Double_t *spstub = sp->GetStubP();

      // Calculate final coordinate based on plusminusbest
      // Update the hit positions in the space points
      for(Int_t ihit = 0; ihit < nhits; ihit++) {
        // Save left/right status with the hit and in the spaleftce point
        // In THcDC will decide which to used based on fix_lr flag
        sp->GetHit(ihit)->SetLeftRight(plusminusbest[ihit]);
        sp->SetHitLR(ihit, plusminusbest[ihit]);
      }

      // Stubs are calculated in rotated coordinate system
      // (I think this rotates in case chambers not perpendicular to central ray)
      Int_t pindex=plane_list[0];
      if(spstub[2] - fTanBeta[pindex] == -1.0) {
	      if (fhdebugflagpr) cout << "HYPDCChamber::LeftRight(): stub3 error" << endl;
      }
      stub[2] = (spstub[2] - fTanBeta[pindex]) / (1.0 + spstub[2]*fTanBeta[pindex]);
      if(spstub[2]*fSinBeta[pindex] ==  -fCosBeta[pindex]) {
	      if (fhdebugflagpr) cout << "HYPDCChamber::LeftRight(): stub4 error" << endl;
      }
      stub[3] = spstub[3] / (spstub[2]*fSinBeta[pindex]+fCosBeta[pindex]);
      stub[0] = spstub[0]*fCosBeta[pindex] - spstub[0]*stub[2]*fSinBeta[pindex];
      stub[1] = spstub[1]	- spstub[1]*stub[3]*fSinBeta[pindex];
      sp->SetStub(stub);
      if (fhdebugflagpr) cout << " Left/Right space pt " << isp+1 << " " << stub[0]<< " " << stub[1] << " " << stub[2]<< " " << stub[3] << endl;
    }
    sp->SetChi2(minchi2);
  }// space point loop

}

//____________________________________________________
UInt_t HYPDCChamber::Count1Bits(UInt_t x)
// From http://graphics.stanford.edu/~seander/bithacks.html
{
  x = x - ((x >> 1) & 0x55555555);
  x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
  return (((x + (x >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

//____________________________________________________
Double_t HYPDCChamber::FindStub(Int_t nhits, HYPSpacePoint *sp,
				       Int_t* plane_list, UInt_t bitpat,
				       Int_t* plusminus, Double_t* stub)
{
   // For a given combination of L/R, fit a stub to the space point
  // This method does a linear least squares fit of a line to the
  // hits in an individual chamber.  It assumes that the y slope is 0
  // The wire coordinate is calculated by
  //          wire center + plusminus*(drift distance).
  // Method is called in a loop over all combinations of plusminus
  Double_t zeros[] = {0.0,0.0,0.0};
  TVectorD TT; TT.Use(3, zeros); // X, X', Y
  Double_t dpos[nhits];
  for(Int_t ihit=0;ihit<nhits; ihit++) {
    dpos[ihit] = sp->GetHit(ihit)->GetPos()
      + plusminus[ihit]*sp->GetHitDist(ihit)
      - fPsi0[plane_list[ihit]];
    for(Int_t index=0;index<3;index++) {
      TT[index]+= dpos[ihit]*fStubCoefs[plane_list[ihit]][index]
	/fSigma[plane_list[ihit]];
    }
  }

  TT *= fAA3Inv[bitpat];

  // Calculate Chi2.  Remember one power of sigma is in fStubCoefs
  stub[0] = TT[0];
  stub[1] = TT[1];
  stub[2] = TT[2];
  stub[3] = 0.0;
  Double_t chi2=0.0;
  for(Int_t ihit=0;ihit<nhits; ihit++) {
    chi2 += pow( dpos[ihit]/fSigma[plane_list[ihit]]
		 - fStubCoefs[plane_list[ihit]][0]*stub[0]
		 - fStubCoefs[plane_list[ihit]][1]*stub[1]
		 - fStubCoefs[plane_list[ihit]][2]*stub[2]
		 , 2);
  }
  return(chi2);
}

//____________________________________________________
void HYPDCChamber::PrintDecode( void )
{
  cout << " Num of nits = " << fNHits << endl;
  cout << " Num " << " Plane "  << " Wire " <<  "  Wire-Center  " << " RAW TDC " << " Drift time" << endl;
  for(Int_t ihit=0;ihit<fNHits;ihit++) {
    HYPDCHit* thishit = fHits[ihit];
    cout << ihit << "       " <<thishit->GetPlaneNum()  << "     " << thishit->GetWireNum() 
    << "     " <<  thishit->GetPos() << "    " << thishit->GetRawTime() << "    " << thishit->GetTime() << endl;
  }
}

ClassImp(HYPDCChamber)
