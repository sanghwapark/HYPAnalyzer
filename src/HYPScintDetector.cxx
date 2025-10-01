// Class for HYPScintDetector detectors

#include "HYPScintDetector.h"
#include "THaCutList.h"
#include "THaEvData.h"
#include "THaGlobals.h"
#include "THaSpectrometer.h"
#include "THcDetectorMap.h"
#include "THcGlobals.h"
#include "THcHitList.h"
#include "THcParmList.h"
#include "THcSignalHit.h"
#include "VarDef.h"
#include "VarType.h"

#include <iostream>

using namespace std;

//________________________________________________________________
HYPScintDetector::HYPScintDetector( const char* name, const char* description,
		      THaApparatus* apparatus) :
  THaNonTrackingDetector(name, description, apparatus)
{
  fNPlanes = 1;
}

//________________________________________________________________
HYPScintDetector::~HYPScintDetector()
{
  // Destructor
for (int ip = 0; ip < fNPlanes; ip++)
  delete fPlanes[ip];
delete[] fPlanes;

delete[] fNPaddle;
  fNPaddle = nullptr;
delete[] fTdcOffset;
fTdcOffset = nullptr;
delete[] fAdcTdcOffset;
fAdcTdcOffset = nullptr;

delete[] fHodoNegAdcTimeWindowMin;
fHodoNegAdcTimeWindowMin = nullptr;
delete[] fHodoNegAdcTimeWindowMax;
fHodoNegAdcTimeWindowMax = nullptr;
delete[] fHodoPosAdcTimeWindowMin;
fHodoPosAdcTimeWindowMin = nullptr;
delete[] fHodoPosAdcTimeWindowMax;
fHodoPosAdcTimeWindowMax = nullptr;

delete[] fPlaneNames;
fPlaneNames = nullptr;

}

//________________________________________________________________
void HYPScintDetector::Clear( Option_t* opt )
{
  fNhits = 0;

  if (*opt != 'I') {
    for (Int_t ip = 0; ip < fNPlanes; ip++) {
      fPlanes[ip]->Clear();
    }
  }
}
//________________________________________________________________
void HYPScintDetector::Setup( const char *name, const char *description )
{
  if (IsZombie())
  return;

  char prefix[2];

  prefix[0] = tolower(GetApparatus()->GetName()[0]);
  prefix[1] = '\0';

  TString temp(prefix[0]);

  string planenamelist;
  DBRequest listextra[] = {{"scin_num_planes", &fNPlanes, kInt},
                          {"scin_plane_names", &planenamelist, kString},
                          {"scin_tdcrefcut", &fTDC_RefTimeCut, kInt, 0, 1},
                          {"scin_adcrefcut", &fADC_RefTimeCut, kInt, 0, 1},
                          {0}};

  fTDC_RefTimeCut = 0; // Minimum allowed reference times
  fADC_RefTimeCut = 0;
  gHcParms->LoadParmValues((DBRequest *)&listextra, prefix);

  cout << "Plane Name List : " << planenamelist << endl;
  
  vector<string> plane_names = Podd::vsplit(planenamelist);
  // Plane names
  if (plane_names.size() != (UInt_t)fNPlanes) {
    cout << "ERROR: Number of planes " << fNPlanes << " doesn't agree with number of plane names " << plane_names.size()
        << endl;
    // Should quit.  Is there an official way to quit?
  }

  fPlaneNames = new char *[fNPlanes];
  for (Int_t i = 0; i < fNPlanes; i++) {
    fPlaneNames[i] = new char[plane_names[i].length() + 1];
    strcpy(fPlaneNames[i], plane_names[i].c_str());
  }

  // Probably shouldn't assume that description is defined
  char *desc = new char[strlen(description) + 100];
  fPlanes    = new HYPScintillatorPlane *[fNPlanes];
  for (Int_t i = 0; i < fNPlanes; i++) {
    strcpy(desc, description);
    strcat(desc, " Plane ");
    strcat(desc, fPlaneNames[i]);
    fPlanes[i] = new HYPScintillatorPlane(fPlaneNames[i], desc, i + 1, this); // Number planes starting from zero!!
    cout << "Created Scintillator Plane " << fPlaneNames[i] << ", " << desc << endl;
  }

  delete[] desc;

}

//________________________________________________________________
THaAnalysisObject::EStatus HYPScintDetector::Init(const TDatime &date) 
{
  cout << "HYPScinDetector::Init" << endl;
  Setup(GetName(), GetTitle());

  cout << "Fill Detector map" << endl;
  char EngineDID[] = "xSCIN"; 
  EngineDID[0]     = toupper(GetApparatus()->GetName()[0]);
  if (gHcDetectorMap->FillMap(fDetMap, EngineDID) < 0) {
    static const char *const here = "Init()";
    Error(Here(here), "Error filling detectormap for %s.", EngineDID);
    return kInitError;
  }

  // Should probably put this in ReadDatabase as we will know the
  // maximum number of hits after setting up the detector map
  // But it needs to happen before the sub detectors are initialized
  // so that they can get the pointer to the hitlist.
  cout << " Hodo tdc ref time cut = " << fTDC_RefTimeCut << " " << fADC_RefTimeCut << endl;

  InitHitList(fDetMap, "THcRawHodoHit", fDetMap->GetTotNumChan() + 1, fTDC_RefTimeCut, fADC_RefTimeCut);

  EStatus status;
  if ((status = THaNonTrackingDetector::Init(date)))
    return fStatus = status;

  for (Int_t ip = 0; ip < fNPlanes; ip++) {
    if ((status = fPlanes[ip]->Init(date))) {
      return fStatus = status;
    }
  }

  fPresentP        = 0;
  THaVar *vpresent = gHaVars->Find(Form("%s.present", GetApparatus()->GetName()));
  if (vpresent) {
    fPresentP = (Bool_t *)vpresent->GetValuePointer();
  }

  return kOK;

}

//________________________________________________________________
Int_t HYPScintDetector::Decode( const THaEvData& evdata )
{

  Bool_t present = kTRUE;
  if(fPresentP) {
    present = *fPresentP;
  }

  // MC flag
  if (fIsMC) {
    fNSA          = 1;
    fNSB          = 0;
    fNPED         = 1;
    fHaveFADCInfo = true;
  }

  fNhits = DecodeToHitList(evdata, !present);

  // To analyze pedestal events -- Must define "Pedestal_event" cut in the cuts .def file
  if (gHaCuts->Result("Pedestal_event")) {
    Int_t nexthit = 0;
    for (Int_t ip = 0; ip < fNPlanes; ip++) {
      nexthit = fPlanes[ip]->AccumulatePedestals(fRawHitList, nexthit);
    }
    fAnalyzePedestals = 1; // Analyze pedestals first normal events
    return (0);
  }
  if (fAnalyzePedestals) {
    for (Int_t ip = 0; ip < fNPlanes; ip++) {
      fPlanes[ip]->CalculatePedestals();
    }
    fAnalyzePedestals = 0; // Don't analyze pedestals next event
  }

  Int_t nexthit = 0;
  for (Int_t iplane = 0; iplane < fNPlanes; iplane++) {

    nexthit = fPlanes[iplane]->ProcessHits(fRawHitList, nexthit);
  }

  return fNhits;

  return 0;
}

//________________________________________________________________
Int_t HYPScintDetector::ReadDatabase( const TDatime& date )
{
  cout << "HYPScintDetector::ReadDatabase()" << endl;
  char prefix[2];
  prefix[0] = tolower(GetApparatus()->GetName()[0]);
  prefix[1] = '\0';

  fNPaddle = new Int_t[fNPlanes]; 
  for(Int_t i = 0; i < fNPlanes; i++){
    DBRequest list[]={{Form("scin_%s_nr",fPlaneNames[i]), &fNPaddle[i], kInt},
		      {0}};
    gHcParms->LoadParmValues((DBRequest*)&list,prefix);
  }

  // for all planes
  fTdcOffset    = new Int_t[fNPlanes];
  fAdcTdcOffset = new Double_t[fNPlanes];

  for (int ip = 0; ip < fNPlanes; ip++) {
    fTdcOffset[ip]    = 0;
    fAdcTdcOffset[ip] = 0;

  }

  Int_t fMaxScinPerPlane = fNPaddle[0];
  for (Int_t i=1;i<fNPlanes;i++) {
    fMaxScinPerPlane=(fMaxScinPerPlane > fNPaddle[i])? fMaxScinPerPlane : fNPaddle[i];
  }
  fMaxHodoScin=fMaxScinPerPlane*fNPlanes;

  // for all elements
  fMaxHodoScin             = fMaxScinPerPlane * fNPlanes;
  fHodoPosAdcTimeWindowMin = new Double_t[fMaxHodoScin];
  fHodoPosAdcTimeWindowMax = new Double_t[fMaxHodoScin];
  fHodoNegAdcTimeWindowMin = new Double_t[fMaxHodoScin];
  fHodoNegAdcTimeWindowMax = new Double_t[fMaxHodoScin];

  for (int ii = 0; ii < fMaxHodoScin; ii++) {
    fHodoPosAdcTimeWindowMin[ii] = -1000.;
    fHodoPosAdcTimeWindowMax[ii] = 1000.;
    fHodoNegAdcTimeWindowMin[ii] = -1000.;
    fHodoNegAdcTimeWindowMax[ii] = 1000.;
  }

  DBRequest list3[] = {{"scin_cosmicflag", &fCosmicFlag, kInt, 0, 1},
                       {"scin_tdc_min", &fScinTdcMin, kDouble},
                       {"scin_tdc_max", &fScinTdcMax, kDouble},
                       {"scin_tdc_to_time", &fScinTdcToTime, kDouble},
                       {"scin_PosAdcTimeWindowMin", fHodoPosAdcTimeWindowMin, kDouble, (UInt_t)fMaxHodoScin, 1}, // optional for now
                       {"scin_PosAdcTimeWindowMax", fHodoPosAdcTimeWindowMax, kDouble, (UInt_t)fMaxHodoScin, 1},
                       {"scin_NegAdcTimeWindowMin", fHodoNegAdcTimeWindowMin, kDouble, (UInt_t)fMaxHodoScin, 1},
                       {"scin_NegAdcTimeWindowMax", fHodoNegAdcTimeWindowMax, kDouble, (UInt_t)fMaxHodoScin, 1},
                       {0}};

  fCosmicFlag          = 0;
  fScinTdcMin          = 0;
  fScinTdcMax          = 0;
  fScinTdcToTime       = 0;

  gHcParms->LoadParmValues((DBRequest *)&list3, prefix);

  DBRequest list5[] = {{"is_mc", &fIsMC, kInt, 0, 1}, {0}};
  fIsMC             = 0;
  gHcParms->LoadParmValues((DBRequest *)&list5, "");

  DBRequest list6[] = {{"scin_adc_tdc_offset", fAdcTdcOffset, kDouble, (UInt_t)fNPlanes, 1},
                       {"scin_tdc_offset", fTdcOffset, kInt, (UInt_t)fNPlanes, 1},
                       {0}};
  gHcParms->LoadParmValues((DBRequest *)&list6, prefix);


  return kOK;
}

//________________________________________________________________
Int_t HYPScintDetector::DefineVariables( EMode mode )
{
  // Add output variables 

  return 0;
}

//________________________________________________________________
Int_t HYPScintDetector::CoarseProcess( TClonesArray& tracks )
{

  return 0;
}

//________________________________________________________________
Int_t HYPScintDetector::FineProcess( TClonesArray& tracks )
{

  return 0;
}

//________________________________________________________________

ClassImp(HYPScintDetector)
