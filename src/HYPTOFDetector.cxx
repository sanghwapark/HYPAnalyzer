#include "HYPTOFDetector.h"
#include "HYPTOFPlane.h"
#include "THaApparatus.h"
#include "VarDef.h"
#include "VarType.h"
#include "THcParmList.h"
#include "THcGlobals.h"
#include "THcDetectorMap.h"

#include <cfloat>
#include <iostream>

using namespace std;

//____________________________________________________________________________________
HYPTOFDetector::HYPTOFDetector( const char* name, const char* description, 
				THaApparatus* apparatus) :
  THaNonTrackingDetector(name, description, apparatus)
{
  fNPlanes = 0;
  fMaxElement = 0;
}

//____________________________________________________________________________________
HYPTOFDetector::~HYPTOFDetector()
{
  RemoveVariables();
  for(auto plane : fPlanes )
    delete plane;
  fPlanes.clear();

  delete [] fTdcOffset;     fTdcOffset = nullptr;
  delete [] fAdcTdcOffset;  fAdcTdcOffset = nullptr;
  delete [] fCorrPosC1;   fCorrPosC1 = nullptr;
  delete [] fCorrNegC1;   fCorrNegC1 = nullptr;
  delete [] fCorrPosC2;   fCorrPosC2 = nullptr;
  delete [] fCorrNegC2;   fCorrNegC2 = nullptr;
}

//____________________________________________________________________________________
void HYPTOFDetector::Clear( Option_t* opt )
{
  fNhits = 0;

}

//____________________________________________________________________________________
THaAnalysisObject::EStatus HYPTOFDetector::Init( const TDatime & date )
{
  //    cout << "HYPTOFDetector::Init" << endl;
  // Init subdetectors
  char prefix[2];
  prefix[0] = tolower(GetApparatus()->GetName()[0]);
  prefix[1] = '\0';
  Bool_t optional = true;

  string planenamelist;
  DBRequest list[] = {
    {"tof_num_planes",  &fNPlanes, kInt},
    {"tof_plane_names", &planenamelist, kString},
    {"tof_max_elem",    &fMaxElement, kInt},
    {"tof_tdcrefcut",   &fTDC_RefTimeCut, kInt, 0, optional},
    {"tof_adcrefcut",   &fADC_RefTimeCut, kInt, 0, optional},
    {nullptr}
  };

  fTDC_RefTimeCut = 0;
  fADC_RefTimeCut = 0;

  gHcParms->LoadParmValues((DBRequest*)&list, prefix);

  vector<string> plane_names = Podd::vsplit(planenamelist);
  if(plane_names.size() != (UInt_t) fNPlanes) {
    cout << "ERROR: Number of planes " << fNPlanes << " doesn't agree with number of plane names " << plane_names.size() << endl;
    return kInitError;
  }

  for(Int_t i = 0; i < fNPlanes; i++) {
    HYPTOFPlane* newplane = new HYPTOFPlane(plane_names[i].c_str(), Form("TOF Plane %s", plane_names[i].c_str()), i+1, this);
    fPlanes.push_back(newplane);
  }

  // In detector map, the detector ID should be defined 
  // e.g. KDC is for HKS DC
  string EngineDID = string(GetApparatus()->GetName()).substr(0, 1) + GetName();
  std::transform(EngineDID.begin(), EngineDID.end(), EngineDID.begin(), ::toupper);
  if(gHcDetectorMap->FillMap(fDetMap, EngineDID.c_str()) < 0) {
    static const char* const here = "Init()";
    Error(Here(here), "Error filling detectormap for %s.", EngineDID.c_str());
    return kInitError;
  }

  InitHitList(fDetMap, "THcRawHodoHit", fDetMap->GetTotNumChan()+1, fTDC_RefTimeCut, fADC_RefTimeCut);

  EStatus status;       
  if ((status = THaNonTrackingDetector::Init(date)))
    return fStatus = status;

  for(Int_t ip=0;ip<fNPlanes;ip++) {
    if((status = fPlanes[ip]->Init( date ))) {
      return fStatus=status;
    }
  }

  fPresentP = 0;
  THaVar* vpresent = gHaVars->Find(Form("%s.present",GetApparatus()->GetName()));
  if(vpresent) {
    fPresentP = (Bool_t *) vpresent->GetValuePointer();
  }

  return fStatus = kOK;
}

//____________________________________________________________________________________
Int_t HYPTOFDetector::Decode( const THaEvData& evdata )
{        
  //cout << "HYPTOFDetector::Decode" << endl;
  Bool_t present = kTRUE;  // suppress reference time warnings
  if(fPresentP) {          // if this spectrometer not part of trigger
    present = *fPresentP;
  }
    
  fNhits = DecodeToHitList(evdata, !present);

  Int_t nexthit = 0;
  for(Int_t ip = 0; ip < fNPlanes; ip++) {
    nexthit = fPlanes[ip]->ProcessHits(fRawHitList, nexthit);
  }

  return fNhits;
}

//____________________________________________________________________________________
Int_t HYPTOFDetector::ReadDatabase( const TDatime & date )
{
  // cout << "HYPTOFDetector::ReadDatabase" << endl;
  char prefix[2];
  prefix[0] = tolower(GetApparatus()->GetPrefix()[0]);
  prefix[1] = '\0';

  Int_t fNArrays = fNPlanes * fMaxElement;
  // Tdc offset
  fTdcOffset = new Int_t[fNPlanes];
  fAdcTdcOffset = new Double_t[fNPlanes];
  // Time correction calibration parameters
  fCorrPosC1 = new Double_t[fNArrays];
  fCorrNegC1 = new Double_t[fNArrays];
  fCorrPosC2 = new Double_t[fNArrays];
  fCorrNegC2 = new Double_t[fNArrays];

  Bool_t optional = true;
  DBRequest list[] = {
    {"tof_tdc_to_time",   &fScinTdcToTime, kDouble, 0},
    {"tof_tdc_min",       &fScinTdcMin,    kDouble, 0},
    {"tof_tdc_max",       &fScinTdcMax,    kDouble, 0},
    {"tof_tdc_offset",    fTdcOffset, kInt,    (UInt_t) fNPlanes, optional},
    {"tof_adctdc_offset", fTdcOffset, kDouble, (UInt_t) fNPlanes, optional},
    {"tof_c1_pos",        fCorrPosC1,  kDouble, (UInt_t) fNArrays, optional},
    {"tof_c1_neg",        fCorrNegC1,  kDouble, (UInt_t) fNArrays, optional},
    {"tof_c2_pos",        fCorrPosC2,  kDouble, (UInt_t) fNArrays, optional},
    {"tof_c2_neg",        fCorrNegC2,  kDouble, (UInt_t) fNArrays, optional},
    {"tof_TDC_threshold", &fTdcThrs,   kDouble, 0, optional},  
    {nullptr}
  };

  // Default values
  fTdcThrs = 1.0;

  for(Int_t i = 0; i < fNPlanes; i++) {
    fTdcOffset[i] = 0.0;
    fAdcTdcOffset[i] = 0.0;
  }
    
  for(Int_t i = 0; i < fNArrays; i++){
    fCorrPosC1[i] = 0.0;
    fCorrNegC1[i] = 0.0;
    fCorrPosC2[i] = 0.0;
    fCorrNegC2[i] = 0.0;
  }

  gHcParms->LoadParmValues((DBRequest*)&list, prefix);

  return kOK;
}
//____________________________________________________________________________________
Int_t HYPTOFDetector::DefineVariables( EMode mode)
{
  return 0;
}

//____________________________________________________________________________________
Int_t HYPTOFDetector::GetScinIndex(Int_t iplane, Int_t ipaddle)
{
  // both plane and paddle index counts from 0
  return fNPlanes*ipaddle + iplane;
}

//____________________________________________________________________________________
Int_t HYPTOFDetector::CoarseProcess( TClonesArray& tracks )
{

  return 0;
}

//____________________________________________________________________________________
Int_t HYPTOFDetector::FineProcess( TClonesArray& tracks )
{
  return 0;
}

//____________________________________________________________________________________

ClassImp(HYPTOFDetector)
