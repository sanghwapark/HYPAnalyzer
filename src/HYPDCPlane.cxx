#include "HYPDCPlane.h"
#include "HYPDC.h"
#include "HYPDCHit.h"
//#include "vfTDC.h"
#include "THcDCTimeToDistConv.h"
#include "THcDCLookupTTDConv.h"
#include "THcDCWire.h"
#include "THcSignalHit.h"
#include "THcGlobals.h"
#include "THcParmList.h"
#include <cstdlib>
#include <iostream>

using namespace std;

//__________________________________________________________________
HYPDCPlane::HYPDCPlane( const char* name, const char* description, 
  const Int_t planenum, THaDetectorBase *parent) :
  THaSubDetector(name, description, parent), 
  fAxis(0), fPlaneNum(planenum), fTTDConv(0)
{
  // constructor
  fWires = new TClonesArray("THcDCWire", 160);
  fHits = new TClonesArray("HYPDCHit", 100);
  fRawHits = new TClonesArray("HYPDCHit", 100);
  fFirstPassHits = new TClonesArray("HYPDCHit",100);

}

//__________________________________________________________________
HYPDCPlane::HYPDCPlane() :
  THaSubDetector()
{
  // Constructor
  fHits = nullptr;
  fFirstPassHits = nullptr;
  fRawHits = nullptr;
  fWires = nullptr;
  fTTDConv = nullptr;
}

//__________________________________________________________________
HYPDCPlane::~HYPDCPlane()
{
  RemoveVariables();

  delete [] fTzeroWire;
  delete fWires;
  delete fHits;
  delete fFirstPassHits;
  delete fRawHits;
  delete fTTDConv;
}

//__________________________________________________________________
THaAnalysisObject::EStatus HYPDCPlane::Init(const TDatime &date)
{

  if(fName.Contains("u")) SetAxis(DC::kU);
  else if(fName.Contains("v")) SetAxis(DC::kV);
  else if(fName.Contains("x")) SetAxis(DC::kX);

  if( IsZombie() )
    return fStatus = kInitError;

  EStatus status;
  if( (status = THaSubDetector::Init(date)) )
    return fStatus = status;
  
  return fStatus = kOK;

}

//__________________________________________________________________
Int_t HYPDCPlane::ReadDatabase( const TDatime& date )
{

  char prefix[2];
  prefix[0] = tolower(GetParent()->GetPrefix()[0]);
  prefix[1] = '\0';

  UInt_t NumDriftMapBins;
  Double_t DriftMapFirstBin;
  Double_t DriftMapBinSize;
  Bool_t optional = true;
  DBRequest list[] = {
    {"driftbins",  &NumDriftMapBins, kInt},
    {"drift1stbin", &DriftMapFirstBin, kDouble},
    {"driftbinsz",  &DriftMapBinSize, kDouble},
    {"_using_tzero_per_wire", &fUsingTzeroPerWire, kInt, 0, optional},
    {"_using_sigma_per_wire", &fUsingSigmaPerWire, kInt, 0, optional},
    {"min_drifttime", &fMin_DriftTime, kDouble, 0, optional},
    {"max_drifttime", &fMax_DriftTime, kDouble,0, optional},
    {nullptr}
  };

  // Default values
  fMin_DriftTime = -50.0; // in ns
  fMax_DriftTime = 200.0; // in ns
  fUsingTzeroPerWire = 0;
  fUsingSigmaPerWire = 0;

  gHcParms->LoadParmValues((DBRequest*)&list, prefix);

  Double_t *DriftMap = new Double_t[NumDriftMapBins];
  DBRequest list2[]={
    {Form("wc%sfract",GetName()), DriftMap, kDouble, NumDriftMapBins},
    {nullptr}
  };
  gHcParms->LoadParmValues((DBRequest*)&list2,prefix);

  // Retrieve parameters we need from parent class
  HYPDC* fParent;
  fParent = (HYPDC*) GetParent();

  fSigma = fParent->GetSigma(fPlaneNum);
  fChamberNum = fParent->GetNChamber(fPlaneNum);
  fNWires = fParent->GetNWires(fPlaneNum);
  fWireOrder = fParent->GetWireOrder(fPlaneNum);
  fPitch = fParent->GetPitch(fPlaneNum);
  fCentralWire = fParent->GetCentralWire(fPlaneNum);
  fTdcWinMin = fParent->GetTdcWinMin(fPlaneNum);
  fTdcWinMax = fParent->GetTdcWinMax(fPlaneNum);
  fPlaneTimeZero = fParent->GetPlaneTimeZero(fPlaneNum);
  fCenter = fParent->GetCenter(fPlaneNum);
  fCentralTime = fParent->GetCentralTime(fPlaneNum);
  fDriftTimeSign = fParent->GetDriftTimeSign(fPlaneNum);
  fReadoutLR = fParent->GetReadoutLR(fPlaneNum);
  fReadoutTB = fParent->GetReadoutTB(fPlaneNum);
  fNSperChan = fParent->GetNSperChan();
  fVersion = fParent->GetVersion();


  Double_t fTzeroWire[fNWires];
  // Dobule_t fSigmaWire[fNWires]; // per wire sigma is not really used in HMS/SHMS
  for(Int_t i = 0; i < fNWires; i++) {
    fTzeroWire[i] = 0.0;
  }

  DBRequest list3[] = {
    {Form("tzero%s", GetName()), fTzeroWire, kDouble, static_cast<UInt_t>(fNWires)},
    {nullptr}
  };
  gHcParms->LoadParmValues((DBRequest*)&list3,prefix);

  // Calculate geometry parameters
  Double_t z0 = fParent->GetZPos(fPlaneNum);
  Double_t alpha = fParent->GetAlphaAngle(fPlaneNum);
  Double_t beta = fParent->GetBetaAngle(fPlaneNum);
  fBeta = beta;
  Double_t gamma = fParent->GetGammaAngle(fPlaneNum);

  Double_t cosalpha = TMath::Cos(alpha);
  Double_t sinalpha = TMath::Sin(alpha);
  Double_t cosbeta = TMath::Cos(beta);
  Double_t sinbeta = TMath::Sin(beta);
  Double_t cosgamma = TMath::Cos(gamma);
  Double_t singamma = TMath::Sin(gamma);

  Double_t hzchi = -cosalpha*sinbeta + sinalpha*cosbeta*singamma;
  Double_t hzpsi =  sinalpha*sinbeta + cosalpha*cosbeta*singamma;
  Double_t hxchi = -cosalpha*cosbeta - sinalpha*sinbeta*singamma;
  Double_t hxpsi =  sinalpha*cosbeta - cosalpha*sinbeta*singamma;
  Double_t hychi =  sinalpha*cosgamma;
  Double_t hypsi =  cosalpha*cosgamma;
  Double_t stubxchi = -cosalpha;
  Double_t stubxpsi = sinalpha;
  Double_t stubychi = sinalpha;
  Double_t stubypsi = cosalpha;

  Double_t sumsqupsi = hzpsi*hzpsi+hxpsi*hxpsi+hypsi*hypsi;
  Double_t sumsquchi = hzchi*hzchi+hxchi*hxchi+hychi*hychi;
  Double_t sumcross = hzpsi*hzchi + hxpsi*hxchi + hypsi*hychi;
  Double_t denom1 = sumsqupsi*sumsquchi-sumcross*sumcross;
  Double_t fPsi0 = (-z0*hzpsi*sumsquchi
        +z0*hzchi*sumcross) / denom1;
  Double_t hchi0 = (-z0*hzchi*sumsqupsi
        +z0*hzpsi*sumcross) / denom1;
  Double_t hphi0 = TMath::Sqrt(pow(z0+hzpsi*fPsi0+hzchi*hchi0,2)
            + pow(hxpsi*fPsi0+hxchi*hchi0,2)
            + pow(hypsi*fPsi0+hychi*hchi0,2) );
  if(z0 < 0.0) hphi0 = -hphi0;
  
  Double_t denom2 = stubxpsi*stubychi - stubxchi*stubypsi;

  fXsp = hychi/denom2;		// sin(a)
  fYsp = -hxchi/denom2;		// cos(a)

  fStubCoef[0] = stubychi/(fSigma*denom2);   // sin(a)/sigma
  fStubCoef[1] = -stubxchi/(fSigma*denom2);   // cos(a)/sigma
  fStubCoef[2] = hphi0*fStubCoef[0];     // z0*sin(a)/sig
  fStubCoef[3] = hphi0*fStubCoef[1];     // z0*cos(a)/sig

  if(cosalpha <= 0.707) { // x-like wire, need dist from x=0 line
    fReadoutX = 1;
    fReadoutCorr = 1/sinalpha;
  } else {
    fReadoutX = 0;
    fReadoutCorr = 1/cosalpha;
  }

  // Comput track fitting coefficients
  #define LOCRAYZT 0.0
  fPlaneCoef[0]= hzchi;		      // = 0.
  fPlaneCoef[1]=-hzchi;		      // = 0.
  fPlaneCoef[2]= hychi*(z0-LOCRAYZT);  // sin(a)*(z-hlocrayzt)
  fPlaneCoef[3]= hxchi*(LOCRAYZT-z0);  // cos(a)*(z-hlocrayzt)
  fPlaneCoef[4]= hychi;		       // sin(a)
  fPlaneCoef[5]=-hxchi;		       // cos(a)
  fPlaneCoef[6]= hzchi*hypsi - hychi*hzpsi; // 0.
  fPlaneCoef[7]=-hzchi*hxpsi + hxchi*hzpsi; // 0.
  fPlaneCoef[8]= hychi*hxpsi - hxchi*hypsi; // 1.

  fTTDConv = new THcDCLookupTTDConv(DriftMapFirstBin,fPitch/2,DriftMapBinSize,
				    NumDriftMapBins,DriftMap);
  delete [] DriftMap;

  Int_t nWires = fParent->GetNWires(fPlaneNum);
  // wire numbers start with one, but arrays start with zero.
  for (int i=0; i<nWires; i++) {
    Double_t pos = fPitch*( (fWireOrder==0?(i+1):fNWires-i)
			    - fCentralWire) - fCenter;
    Int_t readoutside = GetReadoutSide(i+1);
    new((*fWires)[i]) THcDCWire( i+1, pos , fTzeroWire[i], fSigma, readoutside, fTTDConv); 
  }
  
  return kOK;
}

//__________________________________________________________________
Int_t HYPDCPlane::DefineVariables( EMode mode )
{

if( mode == kDefine && fIsSetup ) return kOK;
  fIsSetup = ( mode == kDefine );

  // Register variables in global list
  RVarDef vars[] = {
    {"raw.wirenum", "List of TDC wire number of all hits in DC",
     "fRawHits.HYPDCHit.GetWireNum()"},
    {"wirenum", "List of TDC wire number (select first hit in TDc window",
     "fHits.HYPDCHit.GetWireNum()"},
    {"timeraw", "Raw time with trigger time subtracted, in ns",
     "fHits.HYPDCHit.GetRawTime()"},
    {"raw.refcortime", "Raw time with reference time subtracted Values, in ns",
     "fRawHits.HYPDCHit.GetRefCorrTime()"},
    {"refcortime", "Raw time with reference time subtracted Values, in ns",
     "fHits.HYPDCHit.GetRefCorrTime()"},
    {"time","Drift time",
     "fHits.HYPDCHit.GetTime()"},
     {"dist","Drift distancess",
     "fHits.HYPDCHit.GetDist()"},
    {"pos"," Wire pos",
     "fHits.HYPDCHit.GetPos()"},
    {"nhit", "Number of hits", "GetNHits()"},
    {"RefTime", "Reference time in ns", "fTdcRefTime"},
    { nullptr }
  };

  return DefineVarsFromList( vars, mode );
}


//__________________________________________________________________
void HYPDCPlane::Clear( Option_t* opt )
{
  // cout << "HYPDCPlane::Clear" << endl;

  THaSubDetector::Clear(opt);

  fHits->Clear();
  fRawHits->Clear();
  fFirstPassHits->Clear();

}

//__________________________________________________________________
Int_t HYPDCPlane::Decode( const THaEvData& evdata )
{
 return 0;
}

//__________________________________________________________________
Double_t HYPDCPlane::CalcWireFromPos(Double_t pos) {
  Double_t wire_num_calc=-1000;
  if (fWireOrder==0) wire_num_calc = (pos+fCenter)/(fPitch)+fCentralWire;
  if (fWireOrder==1) wire_num_calc = 1-((pos+fCenter)/(fPitch)+fCentralWire-fNWires);
  return(wire_num_calc);
}

//__________________________________________________________________
Int_t HYPDCPlane::ProcessHits(TClonesArray* rawhits, Int_t nexthit)
{

  fHits->Clear();
  fFirstPassHits->Clear();
  fRawHits->Clear();
  fTdcRefTime = kBig;
  Int_t nrawhits = rawhits->GetLast()+1;
  fNRawhits=0;
  Int_t ihit = nexthit;
  Int_t nextHit = 0;
  Int_t nextRawHit = 0;
  while(ihit < nrawhits) {
    THcRawDCHit* hit = (THcRawDCHit *) rawhits->At(ihit);
    if(hit->fPlane > fPlaneNum) {
      break;
    }
    Int_t wireNum = hit->fCounter;
    THcDCWire* wire = GetWire(wireNum);
    Bool_t First_Hit_In_Window = kTRUE;
    if (hit->GetRawTdcHit().HasRefTime()) fTdcRefTime = hit->GetRawTdcHit().GetRefTime();
    for(UInt_t mhit=0; mhit<hit->GetRawTdcHit().GetNHits(); mhit++) {
      fNRawhits++;
      /* Sort into early, late and ontime */
      Int_t rawtime = hit->GetRawTdcHit().GetTimeRaw(mhit); // rawtime in ns
      Int_t refcortime = hit->GetRawTdcHit().GetTime(mhit); // ref time subtracted time
      Double_t time = refcortime; // FIXME: add time offset and other corr as needed
      //Double_t time = - rawtdc*fNSperChan + fPlaneTimeZero - wire->GetTOffset(); // fNSperChan > 0 for 1877
      new( (*fRawHits)[nextRawHit++] ) HYPDCHit(wire, rawtime, refcortime, time, this);	
      if(rawtime >= fTdcWinMin && rawtime <= fTdcWinMax) {
      	if (First_Hit_In_Window) {
	        new( (*fFirstPassHits)[nextHit++] ) HYPDCHit(wire, rawtime, refcortime, time, this);
	        First_Hit_In_Window = kFALSE;
	      }
      }
    }
    ihit++;
  }
  return(ihit);
}

//__________________________________________________________________
Int_t HYPDCPlane::SubtractStartTime()
{
  Double_t StartTime = 0.0;
  Int_t nextHit=0; 
  // FIXME: Gotta decide how to subtract start time 
  // if( fglHod ) StartTime = fglHod->GetStartTime();
  if (StartTime == -1000) StartTime = 0.0;
  for(Int_t ihit=0;ihit<(fFirstPassHits->GetLast()+1);ihit++) { 
    HYPDCHit *thishit = (HYPDCHit*) fFirstPassHits->At(ihit);
    Double_t temptime= thishit->GetTime()-StartTime;
    Int_t tempRawtime= thishit->GetRawTime();
    Int_t tempRefCorrtime= thishit->GetRefCorrTime();
    THcDCWire *tempWire = (THcDCWire*) thishit->GetWire();
    thishit->SetTime(temptime);
    thishit->ConvertTimeToDist();
    if (temptime > fMin_DriftTime && temptime <fMax_DriftTime) {
	    new( (*fHits)[nextHit++] ) HYPDCHit(tempWire, tempRawtime, tempRefCorrtime, temptime, this);
    }

  }
  return 0;
}

//__________________________________________________________________
Int_t HYPDCPlane::GetReadoutSide(Int_t wirenum)
{
  Int_t readoutside;
  //if new HMS
  if (fVersion == 1) {
    if ((fPlaneNum>=3 && fPlaneNum<=4) || (fPlaneNum>=9 && fPlaneNum<=10)) {
      if (fReadoutTB>0) {
	if (wirenum < 60) {
	  readoutside = 2;
	} else {
	  readoutside = 4;
	}
      } else {
	if (wirenum < 44) {
	  readoutside = 4;
	} else {
	  readoutside = 2;
	}
      }
    } else {
      if (fReadoutTB>0) {
	if (wirenum < 51) {
	  readoutside = 2;
	} else if (wirenum >= 51 && wirenum <= 64) {
	  readoutside = 1;
	} else {
	  readoutside =4;
	}
      } else {
	if (wirenum < 33) {
	  readoutside = 4;
	} else if (wirenum >=33 && wirenum<=46) {
	  readoutside = 1;
	} else {
	  readoutside = 2;
	}
      }
    }
  } else {//appplies SHMS DC configuration
    //check if x board
    if ((fPlaneNum>=3 && fPlaneNum<=4) || (fPlaneNum>=9 && fPlaneNum<=10)) {
      if (fReadoutTB>0) {
	if (wirenum < 49) {
	  readoutside = 4;
	} else {
	  readoutside = 2;
	}
      } else {
	if (wirenum < 33) {
	  readoutside = 2;
	} else {
	  readoutside = 4;
	}
      }
    } else { //else is u board
      if (fReadoutTB>0) {
	if (wirenum < 41) {
	  readoutside = 4;
	} else if (wirenum >= 41 && wirenum <= 63) {
	  readoutside = 3;
	} else if (wirenum >=64 && wirenum <=69) {
	  readoutside = 1;
	} else {
	  readoutside = 2;
	}
      } else {
	if (wirenum < 39) {
	  readoutside = 2;
	} else if (wirenum >=39 && wirenum<=44) {
	  readoutside = 1;
	} else if (wirenum>=45 && wirenum<=67) {
	  readoutside = 3;
	} else {
	  readoutside = 4;
	}
      }
    }
  }
  return(readoutside);
}

ClassImp(HYPDCPlane)