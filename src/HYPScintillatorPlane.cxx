#include "HYPScintillatorPlane.h"
#include "THcGlobals.h"
#include "THcHitList.h"
#include "HYPScintHit.h"
#include "HYPScintDetector.h"
#include "THcParmList.h"
#include "THcRawAdcHit.h"
#include "THcRawTdcHit.h"
#include "THcSignalHit.h"

//_______________________________________________________________________________________
HYPScintillatorPlane::HYPScintillatorPlane(const char *name, const char *description, const Int_t planenum,
                                 THaDetectorBase *parent)
    : THaSubDetector(name, description, parent) {

  fPlaneNum      = planenum;
  
  fPosCenter = 0;
  fPosLeft = 0;
  fPosRight = 0;
  
  frPosAdcErrorFlag = new TClonesArray("THcSignalHit", 16);
  frNegAdcErrorFlag = new TClonesArray("THcSignalHit", 16);

  frPosTdcHits = new TClonesArray("THcSignalHit", 16);
  frNegTdcHits = new TClonesArray("THcSignalHit", 16);
  frPosAdcHits = new TClonesArray("THcSignalHit", 16);
  frNegAdcHits = new TClonesArray("THcSignalHit", 16);
  frPosAdcSums = new TClonesArray("THcSignalHit", 16);
  frNegAdcSums = new TClonesArray("THcSignalHit", 16);
  frPosAdcPeds = new TClonesArray("THcSignalHit", 16);
  frNegAdcPeds = new TClonesArray("THcSignalHit", 16);

  frPosTdcTimeRaw      = new TClonesArray("THcSignalHit", 16);
  frPosAdcPedRaw       = new TClonesArray("THcSignalHit", 16);
  frPosAdcPulseIntRaw  = new TClonesArray("THcSignalHit", 16);
  frPosAdcPulseAmpRaw  = new TClonesArray("THcSignalHit", 16);
  frPosAdcPulseTimeRaw = new TClonesArray("THcSignalHit", 16);

  frPosTdcTime      = new TClonesArray("THcSignalHit", 16);
  frPosAdcPed       = new TClonesArray("THcSignalHit", 16);
  frPosAdcPulseInt  = new TClonesArray("THcSignalHit", 16);
  frPosAdcPulseAmp  = new TClonesArray("THcSignalHit", 16);
  frPosAdcPulseTime = new TClonesArray("THcSignalHit", 16);

  frNegTdcTimeRaw      = new TClonesArray("THcSignalHit", 16);
  frNegAdcPedRaw       = new TClonesArray("THcSignalHit", 16);
  frNegAdcPulseIntRaw  = new TClonesArray("THcSignalHit", 16);
  frNegAdcPulseAmpRaw  = new TClonesArray("THcSignalHit", 16);
  frNegAdcPulseTimeRaw = new TClonesArray("THcSignalHit", 16);

  frNegTdcTime      = new TClonesArray("THcSignalHit", 16);
  frNegAdcPed       = new TClonesArray("THcSignalHit", 16);
  frNegAdcPulseInt  = new TClonesArray("THcSignalHit", 16);
  frNegAdcPulseAmp  = new TClonesArray("THcSignalHit", 16);
  frNegAdcPulseTime = new TClonesArray("THcSignalHit", 16);

  frPosAdcSampPedRaw       = new TClonesArray("THcSignalHit", 16);
  frPosAdcSampPulseIntRaw  = new TClonesArray("THcSignalHit", 16);
  frPosAdcSampPulseAmpRaw  = new TClonesArray("THcSignalHit", 16);
  frPosAdcSampPulseTimeRaw = new TClonesArray("THcSignalHit", 16);
  frPosAdcSampPed          = new TClonesArray("THcSignalHit", 16);
  frPosAdcSampPulseInt     = new TClonesArray("THcSignalHit", 16);
  frPosAdcSampPulseAmp     = new TClonesArray("THcSignalHit", 16);
  frPosAdcSampPulseTime    = new TClonesArray("THcSignalHit", 16);

  frNegAdcSampPedRaw       = new TClonesArray("THcSignalHit", 16);
  frNegAdcSampPulseIntRaw  = new TClonesArray("THcSignalHit", 16);
  frNegAdcSampPulseAmpRaw  = new TClonesArray("THcSignalHit", 16);
  frNegAdcSampPulseTimeRaw = new TClonesArray("THcSignalHit", 16);
  frNegAdcSampPed          = new TClonesArray("THcSignalHit", 16);
  frNegAdcSampPulseInt     = new TClonesArray("THcSignalHit", 16);
  frNegAdcSampPulseAmp     = new TClonesArray("THcSignalHit", 16);
  frNegAdcSampPulseTime    = new TClonesArray("THcSignalHit", 16);
}

//_______________________________________________________________________________________
HYPScintillatorPlane::~HYPScintillatorPlane() 
{
  // destructor
  if (fIsSetup)
    RemoveVariables();

  delete frPosAdcErrorFlag;
  frPosAdcErrorFlag = nullptr;
  delete frNegAdcErrorFlag;
  frNegAdcErrorFlag = nullptr;

  delete frPosTdcHits;
  delete frNegTdcHits;
  delete frPosAdcHits;
  delete frNegAdcHits;
  delete frPosAdcSums;
  delete frNegAdcSums;
  delete frPosAdcPeds;
  delete frNegAdcPeds;

  delete frPosTdcTimeRaw;
  delete frPosAdcPedRaw;
  delete frPosAdcPulseIntRaw;
  delete frPosAdcPulseAmpRaw;
  delete frPosAdcPulseTimeRaw;

  delete frPosTdcTime;
  delete frPosAdcPed;
  delete frPosAdcPulseInt;
  delete frPosAdcPulseAmp;
  delete frPosAdcPulseTime;

  delete frNegTdcTimeRaw;
  delete frNegAdcPedRaw;
  delete frNegAdcPulseIntRaw;
  delete frNegAdcPulseAmpRaw;
  delete frNegAdcPulseTimeRaw;

  delete frNegTdcTime;
  delete frNegAdcPed;
  delete frNegAdcPulseInt;
  delete frNegAdcPulseAmp;
  delete frNegAdcPulseTime;

  delete frPosAdcSampPedRaw;  frPosAdcSampPedRaw = nullptr;
  delete frPosAdcSampPulseIntRaw;  frPosAdcSampPulseIntRaw = nullptr;
  delete frPosAdcSampPulseAmpRaw;  frPosAdcSampPulseAmpRaw = nullptr;
  delete frPosAdcSampPulseTimeRaw;  frPosAdcSampPulseTimeRaw = nullptr;
  delete frPosAdcSampPed;  frPosAdcSampPed = nullptr;
  delete frPosAdcSampPulseInt;  frPosAdcSampPulseInt = nullptr;
  delete frPosAdcSampPulseAmp;  frPosAdcSampPulseAmp = nullptr;
  delete frPosAdcSampPulseTime;  frPosAdcSampPulseTime = nullptr;

  delete frNegAdcSampPedRaw;  frNegAdcSampPedRaw = nullptr;
  delete frNegAdcSampPulseIntRaw;  frNegAdcSampPulseIntRaw = nullptr;
  delete frNegAdcSampPulseAmpRaw;  frNegAdcSampPulseAmpRaw = nullptr;
  delete frNegAdcSampPulseTimeRaw;  frNegAdcSampPulseTimeRaw = nullptr;
  delete frNegAdcSampPed;  frNegAdcSampPed = nullptr;
  delete frNegAdcSampPulseInt;  frNegAdcSampPulseInt = nullptr;
  delete frNegAdcSampPulseAmp;  frNegAdcSampPulseAmp = nullptr;
  delete frNegAdcSampPulseTime;  frNegAdcSampPulseTime = nullptr;

  delete[] fPosPedSum;
  fPosPedSum = 0;
  delete[] fPosPedSum2;
  fPosPedSum2 = 0;
  delete[] fPosPedLimit;
  fPosPedLimit = 0;
  delete[] fPosPedCount;
  fPosPedCount = 0;
  delete[] fNegPedSum;
  fNegPedSum = 0;
  delete[] fNegPedSum2;
  fNegPedSum2 = 0;
  delete[] fNegPedLimit;
  fNegPedLimit = 0;
  delete[] fNegPedCount;
  fNegPedCount = 0;
  delete[] fPosPed;
  fPosPed = 0;
  delete[] fNegPed;
  fNegPed = 0;
  delete[] fPosThresh;
  fPosThresh = 0;
  delete[] fNegThresh;
  fNegThresh = 0;

  delete[] fPosCenter;
  fPosCenter = 0;

}

//_______________________________________________________________________________________
THaAnalysisObject::EStatus HYPScintillatorPlane::Init(const TDatime &date) {

  if (IsZombie())
    return fStatus = kInitError;

  EStatus status;
  if ((status = THaSubDetector::Init(date)))
    return fStatus = status;
  return fStatus = kOK;
}

//_______________________________________________________________________________________
void HYPScintillatorPlane::Clear(Option_t *opt) {

  frPosAdcErrorFlag->Clear();
  frNegAdcErrorFlag->Clear();

  frPosTdcHits->Clear();
  frNegTdcHits->Clear();
  frPosAdcHits->Clear();
  frNegAdcHits->Clear();

  frPosTdcTimeRaw->Clear();
  frPosAdcPedRaw->Clear();
  frPosAdcPulseIntRaw->Clear();
  frPosAdcPulseAmpRaw->Clear();
  frPosAdcPulseTimeRaw->Clear();

  frPosTdcTime->Clear();
  frPosAdcPed->Clear();
  frPosAdcPulseInt->Clear();
  frPosAdcPulseAmp->Clear();
  frPosAdcPulseTime->Clear();

  frPosAdcSampPedRaw->Clear();
  frPosAdcSampPulseIntRaw->Clear();
  frPosAdcSampPulseAmpRaw->Clear();
  frPosAdcSampPulseTimeRaw->Clear();
  frPosAdcSampPed->Clear();
  frPosAdcSampPulseInt->Clear();
  frPosAdcSampPulseAmp->Clear();
  frPosAdcSampPulseTime->Clear();

  frNegTdcTimeRaw->Clear();
  frNegAdcPedRaw->Clear();
  frNegAdcPulseIntRaw->Clear();
  frNegAdcPulseAmpRaw->Clear();
  frNegAdcPulseTimeRaw->Clear();

  frNegTdcTime->Clear();
  frNegAdcPed->Clear();
  frNegAdcPulseInt->Clear();
  frNegAdcPulseAmp->Clear();
  frNegAdcPulseTime->Clear();

  frNegAdcSampPedRaw->Clear();
  frNegAdcSampPulseIntRaw->Clear();
  frNegAdcSampPulseAmpRaw->Clear();
  frNegAdcSampPulseTimeRaw->Clear();
  frNegAdcSampPed->Clear();
  frNegAdcSampPulseInt->Clear();
  frNegAdcSampPulseAmp->Clear();
  frNegAdcSampPulseTime->Clear();

  // Clear occupancies
  fNumPosAdcHits.clear();
  fNumNegAdcHits.clear();
  fNumPosTdcHits.clear();
  fNumNegTdcHits.clear();

  fHitDistance    = kBig;
  fScinYPos       = kBig;
  fScinXPos       = kBig;

  // Ref time
  fPosTdcRefTime     = kBig;
  fPosAdcRefTime     = kBig;
  fNegTdcRefTime     = kBig;
  fNegAdcRefTime     = kBig;
  fPosTdcRefDiffTime = kBig;
  fPosAdcRefDiffTime = kBig;
  fNegTdcRefDiffTime = kBig;
  fNegAdcRefDiffTime = kBig;

  // Waveform vectors
  fPosAdcSampWaveform.clear();
  fNegAdcSampWaveform.clear();

  // Counters
  fTotNumPosAdcHits = 0;
  fTotNumNegAdcHits = 0;
  fTotNumAdcHits    = 0;

  fTotNumPosTdcHits = 0;
  fTotNumNegTdcHits = 0;
  fTotNumTdcHits    = 0;
}

//_______________________________________________________________________________________
Int_t HYPScintillatorPlane::Decode(const THaEvData &evdata) {
  // Leave it to do nothing

  return 0;
}

//_______________________________________________________________________________________
Int_t HYPScintillatorPlane::ReadDatabase(const TDatime &date) {

  cout << "HYPScintillatorPlane::ReadDatabase()" << endl;
  
  // Read database files as needed here
  char prefix[2];
  prefix[0] = tolower(GetParent()->GetPrefix()[0]);
  prefix[1] = '\0';

  // Get # of element for each hodo detector
  string parname     = "scin_" + string(GetName()) + "_nr";
  DBRequest list_1[] = {{parname.c_str(), &fNelem, kInt}, {0}};
  gHcParms->LoadParmValues(list_1, prefix);

  delete[] fPosCenter;
  fPosCenter = new Double_t[fNelem];

  DBRequest list[] = {{Form("scin_%s_zpos", GetName()), &fZpos, kDouble},
                      {Form("scin_%s_dzpos", GetName()), &fDzpos, kDouble},
                      {Form("scin_%s_size", GetName()), &fSize, kDouble},
                      {Form("scin_%s_spacing", GetName()), &fSpacing, kDouble},
                      {Form("scin_%s_%s", GetName(), "left"), &fPosLeft, kDouble},
                      {Form("scin_%s_%s", GetName(), "right"), &fPosRight, kDouble},
                      {Form("scin_%s_offset", GetName()), &fPosOffset, kDouble},
                      {Form("scin_%s_center", GetName()), fPosCenter, kDouble, fNelem},
                      {"scin_debug_adc", &fDebugAdc, kInt, 0, 1},
                      {"scin_SampThreshold", &fSampThreshold, kDouble, 0, 1},
                      {"scin_SampNSA", &fSampNSA, kInt, 0, 1},
                      {"scin_SampNSAT", &fSampNSAT, kInt, 0, 1},
                      {"scin_SampNSB", &fSampNSB, kInt, 0, 1},
                      {"scin_OutputSampWaveform", &fOutputSampWaveform, kInt, 0, 1},
                      {"scin_UseSampWaveform", &fUseSampWaveform, kInt, 0, 1},
                      {0}};

  // Set Default values
  fDebugAdc           = 0; // Set ADC debug parameter to false unless set in parameter file
  fSampThreshold      = 5.;
  fSampNSA            = 0;  // use value stored in event 125 info
  fSampNSB            = 0;  // use value stored in event 125 info
  fSampNSAT           = 2;  // default value in THcRawHit::SetF250Params
  fOutputSampWaveform = 0;  // 0= no output , 1 = output Sample Waveform
  fUseSampWaveform    = 0;  // 0= do not use , 1 = use Sample Waveform

  gHcParms->LoadParmValues((DBRequest *)&list, prefix);

  DBRequest list5[] = {{"is_mc", &fIsMC, kInt, 0, 1}, {0}};
  fIsMC             = 0;
  gHcParms->LoadParmValues((DBRequest *)&list5, "");

  // Retrieve parameters we need from parent class
  // Common for all planes
  HYPScintDetector *parent = (HYPScintDetector *)GetParent();

  fTdcOffset     = parent->GetTdcOffset(fPlaneNum - 1);
  fAdcTdcOffset  = parent->GetAdcTdcOffset(fPlaneNum - 1);

  // Some of these parameters not being used right now.
  // Post raw hit process to be added/moved out for clean up
  fScinTdcMin    = parent->GetTdcMin();
  fScinTdcMax    = parent->GetTdcMax();
  fScinTdcToTime = parent->GetTdcToTime();

  // Create arrays to hold results here
  InitializePedestals();

  return 0;
}

//_______________________________________________________________________________________
Int_t HYPScintillatorPlane::DefineVariables(EMode mode) {

  cout << "HYPScintillatorPlane::DefineVariables" << endl;

  if (mode == kDefine && fIsSetup)
    return kOK;
  fIsSetup = (mode == kDefine);

  // Register variables in global list

  if (fDebugAdc) {
    RVarDef vars[] = {
        {"PosAdcErrorFlag", "Error Flag for When FPGA Fails", "frPosAdcErrorFlag.THcSignalHit.GetData()"},
        {"NegAdcErrorFlag", "Error Flag for When FPGA Fails", "frNegAdcErrorFlag.THcSignalHit.GetData()"},

        {"PosTdcTimeRaw", "List of Pos raw TDC values.", "frPosTdcTimeRaw.THcSignalHit.GetData()"},
        {"PosAdcPedRaw", "List of Pos raw ADC pedestals", "frPosAdcPedRaw.THcSignalHit.GetData()"},
        {"PosAdcPulseIntRaw", "List of Pos raw ADC pulse integrals.", "frPosAdcPulseIntRaw.THcSignalHit.GetData()"},
        {"PosAdcPulseAmpRaw", "List of Pos raw ADC pulse amplitudes.", "frPosAdcPulseAmpRaw.THcSignalHit.GetData()"},
        {"PosAdcPulseTimeRaw", "List of Pos raw ADC pulse times.", "frPosAdcPulseTimeRaw.THcSignalHit.GetData()"},

        {"PosTdcTime", "List of Pos TDC values.", "frPosTdcTime.THcSignalHit.GetData()"},
        {"PosAdcPed", "List of Pos ADC pedestals", "frPosAdcPed.THcSignalHit.GetData()"},
        {"PosAdcPulseInt", "List of Pos ADC pulse integrals.", "frPosAdcPulseInt.THcSignalHit.GetData()"},
        {"PosAdcPulseAmp", "List of Pos ADC pulse amplitudes.", "frPosAdcPulseAmp.THcSignalHit.GetData()"},
        {"PosAdcPulseTime", "List of Pos ADC pulse times.", "frPosAdcPulseTime.THcSignalHit.GetData()"},

        {"PosAdcSampPedRaw", "Pos Raw Samp ADC pedestals", "frPosAdcSampPedRaw.THcSignalHit.GetData()"},
        {"PosAdcSampPulseIntRaw", "Pos Raw Samp ADC pulse integrals", "frPosAdcSampPulseIntRaw.THcSignalHit.GetData()"},
        {"PosAdcSampPulseAmpRaw", "Pos Raw Samp ADC pulse amplitudes",
         "frPosAdcSampPulseAmpRaw.THcSignalHit.GetData()"},
        {"PosAdcSampPulseTimeRaw", "Pos Raw Samp ADC pulse times", "frPosAdcSampPulseTimeRaw.THcSignalHit.GetData()"},
        {"PosAdcSampPed", "Pos Samp ADC pedestals", "frPosAdcSampPed.THcSignalHit.GetData()"},
        {"PosAdcSampPulseInt", "Pos Samp ADC pulse integrals", "frPosAdcSampPulseInt.THcSignalHit.GetData()"},
        {"PosAdcSampPulseAmp", "Pos Samp ADC pulse amplitudes", "frPosAdcSampPulseAmp.THcSignalHit.GetData()"},
        {"PosAdcSampPulseTime", "Pos Samp ADC pulse times", "frPosAdcSampPulseTime.THcSignalHit.GetData()"},

        {"NegTdcTimeRaw", "List of neg raw TDC values.", "frNegTdcTimeRaw.THcSignalHit.GetData()"},
        {"NegAdcPedRaw", "List of neg raw ADC pedestals", "frNegAdcPedRaw.THcSignalHit.GetData()"},
        {"NegAdcPulseIntRaw", "List of neg raw ADC pulse integrals.", "frNegAdcPulseIntRaw.THcSignalHit.GetData()"},
        {"NegAdcPulseAmpRaw", "List of neg raw ADC pulse amplitudes.", "frNegAdcPulseAmpRaw.THcSignalHit.GetData()"},
        {"NegAdcPulseTimeRaw", "List of neg raw ADC pulse times.", "frNegAdcPulseTimeRaw.THcSignalHit.GetData()"},

        {"NegTdcTime", "List of neg TDC values.", "frNegTdcTime.THcSignalHit.GetData()"},
        {"NegAdcPed", "List of neg ADC pedestals", "frNegAdcPed.THcSignalHit.GetData()"},
        {"NegAdcPulseInt", "List of neg ADC pulse integrals.", "frNegAdcPulseInt.THcSignalHit.GetData()"},
        {"NegAdcPulseAmp", "List of neg ADC pulse amplitudes.", "frNegAdcPulseAmp.THcSignalHit.GetData()"},
        {"NegAdcPulseTime", "List of neg ADC pulse times.", "frNegAdcPulseTime.THcSignalHit.GetData()"},

        {"NegAdcSampPedRaw", "Neg Raw Samp ADC pedestals", "frNegAdcSampPedRaw.THcSignalHit.GetData()"},
        {"NegAdcSampPulseIntRaw", "Neg Raw Samp ADC pulse integrals",
         "frNegAdcSampPulseIntRaw.THcSignalHit.GetData()"},
        {"NegAdcSampPulseAmpRaw", "Neg Raw Samp ADC pulse amplitudes",
         "frNegAdcSampPulseAmpRaw.THcSignalHit.GetData()"},
        {"NegAdcSampPulseTimeRaw", "Neg Raw Samp ADC pulse times",
         "frNegAdcSampPulseTimeRaw.THcSignalHit.GetData()"},
        {"NegAdcSampPed", "Neg Samp ADC pedestals", "frNegAdcSampPed.THcSignalHit.GetData()"},
        {"NegAdcSampPulseInt", "Neg Samp ADC pulse integrals", "frNegAdcSampPulseInt.THcSignalHit.GetData()"},
        {"NegAdcSampPulseAmp", "Neg Samp ADC pulse amplitudes", "frNegAdcSampPulseAmp.THcSignalHit.GetData()"},
        {"NegAdcSampPulseTime", "Neg Samp ADC pulse times", "frNegAdcSampPulseTime.THcSignalHit.GetData()"},

        {"numPosAdcHits", "Number of Pos ADC Hits Per PMT", "fNumPosAdcHits"}, // Hodo+ ADC occupancy - vector<Int_t>
        {"numNegAdcHits", "Number of Neg ADC Hits Per PMT", "fNumNegAdcHits"}, // Hodo- ADC occupancy - vector <Int_t>

        {"numPosTdcHits", "Number of Pos TDC Hits Per PMT", "fNumPosTdcHits"}, // Hodo+ TDC occupancy - vector<Int_t>
        {"numNegTdcHits", "Number of Neg TDC Hits Per PMT", "fNumNegTdcHits"}, // Hodo- TDC occupancy - vector <Int_t>

        {"totNumPosAdcHits", "Total Number of Pos ADC Hits", "fTotNumPosAdcHits"}, // Hodo+ raw ADC multiplicity Int_t
        {"totNumNegAdcHits", "Total Number of Neg ADC Hits", "fTotNumNegAdcHits"}, // Hodo- raw ADC multiplicity ""
        {"totNumAdcHits", "Total Number of PMTs Hit (as measured by ADCs)", "fTotNumAdcHits"}, // Hodo raw ADC multiplicity  ""

        {"totNumPosTdcHits", "Total Number of Pos TDC Hits", "fTotNumPosTdcHits"},    // Hodo+ raw TDC multiplicity ""
        {"totNumNegTdcHits", "Total Number of Neg TDC Hits", "fTotNumNegTdcHits"}, // Hodo- raw TDC multiplicity ""
        {"totNumTdcHits", "Total Number of PMTs Hits (as measured by TDCs)", "fTotNumTdcHits"}, // Hodo raw TDC multiplicity  ""
        {0}};
    DefineVarsFromList(vars, mode);
  } // end debug statement

  if (fOutputSampWaveform == 1) {
    RVarDef vars[] = {{"adcNegSampWaveform", "FADC Neg ADCSample Waveform", "fNegAdcSampWaveform"},
                      {"adcPosSampWaveform", "FADC Pos ADCSample Waveform", "fPosAdcSampWaveform"},
                      {0}};
    DefineVarsFromList(vars, mode);
  }

  return kOK;
}

//_______________________________________________________________________________________
Int_t HYPScintillatorPlane::ProcessHits(TClonesArray *rawhits, Int_t nexthit) {

  //  cout << "HYPScintillatorPlane::ProcessHits" << endl;

  Clear();

  // counters for Tdc/Adc hits
  UInt_t nrPosTdcHits     = 0;
  UInt_t nrPosAdcHits     = 0;
  UInt_t nrNegTdcHits     = 0;
  UInt_t nrNegAdcHits     = 0;
  UInt_t nrSampPosAdcHits = 0;
  UInt_t nrSampNegAdcHits = 0;

  Int_t nrawhits = rawhits->GetLast() + 1;
  Int_t ihit     = nexthit;

  // A THcRawHodoHit contains all the information (tdc and adc for both
  // pmts) for a single paddle for a single trigger.  The tdc information
  // might include multiple hits if it uses a multihit tdc.
  // Use "ihit" as the index over THcRawHodoHit objects.  Use
  // "thit" to index over multiple tdc hits within an "ihit".

  Bool_t problem_flag = kFALSE; // check if fTdcRefTime is filled correctly or left initialized (kBig)

  while (ihit < nrawhits) {
    THcRawHodoHit *hit = (THcRawHodoHit *)rawhits->At(ihit);

    if (hit->fPlane > fPlaneNum) {
      break;
    }

    Int_t padnum = hit->fCounter;

    // Pos Tdc hits
    THcRawTdcHit &rawPosTdcHit = hit->GetRawTdcHitPos(); // Pos=Pos
    if (rawPosTdcHit.GetNHits() > 0 && rawPosTdcHit.HasRefTime()) {

      // Assume fPosTdcRefTime is initialized
      if (fPosTdcRefTime == kBig) {
        fPosTdcRefTime     = rawPosTdcHit.GetRefTime();
        fPosTdcRefDiffTime = rawPosTdcHit.GetRefDiffTime();
      }

      // Set problem_flag if the it's not set correctly
      if (fPosTdcRefTime != rawPosTdcHit.GetRefTime()) {
        problem_flag = kTRUE;
      }
    }

    // Loop over multiple tdc hits within ihit
    for (UInt_t thit = 0; thit < rawPosTdcHit.GetNHits(); thit++) {
      ((THcSignalHit *)frPosTdcTimeRaw->ConstructedAt(nrPosTdcHits))->Set(padnum, rawPosTdcHit.GetTimeRaw(thit));
      ((THcSignalHit *)frPosTdcTime->ConstructedAt(nrPosTdcHits))->Set(padnum, rawPosTdcHit.GetTime(thit));

      nrPosTdcHits++; 
      fTotNumTdcHits++;
      fTotNumPosTdcHits++;
      fNumPosTdcHits.emplace_back(padnum);
    }

    // Now, repeat for the Neg end
    THcRawTdcHit &rawNegTdcHit = hit->GetRawTdcHitNeg(); // Neg=Neg
    //    cout << "RawNegTdc: " << padnum << " "  << rawNegTdcHit.GetNHits() << endl;
    if (rawNegTdcHit.GetNHits() > 0 && rawNegTdcHit.HasRefTime()) {
      // if (rawNegTdcHit.GetNHits() > 0) {

      if (fNegTdcRefTime == kBig) {
        fNegTdcRefTime     = rawNegTdcHit.GetRefTime();
        fNegTdcRefDiffTime = rawNegTdcHit.GetRefDiffTime();
      }

      if (fNegTdcRefTime != rawNegTdcHit.GetRefTime()) {
        problem_flag = kTRUE;
      }
    }

    for (UInt_t thit = 0; thit < rawNegTdcHit.GetNHits(); thit++) {
      ((THcSignalHit *)frNegTdcTimeRaw->ConstructedAt(nrNegTdcHits))->Set(padnum, rawNegTdcHit.GetTimeRaw(thit));
      ((THcSignalHit *)frNegTdcTime->ConstructedAt(nrNegTdcHits))->Set(padnum, rawNegTdcHit.GetTime(thit));

      nrNegTdcHits++; 
      fTotNumTdcHits++;
      fTotNumNegTdcHits++;
      fNumNegTdcHits.emplace_back(padnum);
    } // thit loop

    // Pos ADC hits
    THcRawAdcHit &rawPosAdcHit = hit->GetRawAdcHitPos(); // Pos=Pos

    if ((rawPosAdcHit.GetNPulses() > 0 || rawPosAdcHit.GetNSamples() > 0) && rawPosAdcHit.HasRefTime()) {

      if (fPosAdcRefTime == kBig) {
        fPosAdcRefTime     = rawPosAdcHit.GetRefTime();
        fPosAdcRefDiffTime = rawPosAdcHit.GetRefDiffTime();
      }

      if (fPosAdcRefTime != rawPosAdcHit.GetRefTime()) {
        problem_flag = kTRUE;
      }
    }

    if (fUseSampWaveform == 0) {

      for (UInt_t thit = 0; thit < rawPosAdcHit.GetNPulses(); thit++) {

        ((THcSignalHit *)frPosAdcPedRaw->ConstructedAt(nrPosAdcHits))->Set(padnum, rawPosAdcHit.GetPedRaw());
        ((THcSignalHit *)frPosAdcPed->ConstructedAt(nrPosAdcHits))->Set(padnum, rawPosAdcHit.GetPed());

        ((THcSignalHit *)frPosAdcPulseIntRaw->ConstructedAt(nrPosAdcHits))
            ->Set(padnum, rawPosAdcHit.GetPulseIntRaw(thit));
        ((THcSignalHit *)frPosAdcPulseInt->ConstructedAt(nrPosAdcHits))->Set(padnum, rawPosAdcHit.GetPulseInt(thit));

        ((THcSignalHit *)frPosAdcPulseAmpRaw->ConstructedAt(nrPosAdcHits))
            ->Set(padnum, rawPosAdcHit.GetPulseAmpRaw(thit));
        ((THcSignalHit *)frPosAdcPulseAmp->ConstructedAt(nrPosAdcHits))->Set(padnum, rawPosAdcHit.GetPulseAmp(thit));

        ((THcSignalHit *)frPosAdcPulseTimeRaw->ConstructedAt(nrPosAdcHits))
            ->Set(padnum, rawPosAdcHit.GetPulseTimeRaw(thit));
        ((THcSignalHit *)frPosAdcPulseTime->ConstructedAt(nrPosAdcHits))
            ->Set(padnum, rawPosAdcHit.GetPulseTime(thit) + fAdcTdcOffset);

        // Error flags 0-2
        if (rawPosAdcHit.GetPulseAmpRaw(thit) > 0)
          ((THcSignalHit *)frPosAdcErrorFlag->ConstructedAt(nrPosAdcHits))->Set(padnum, 0);
        if (rawPosAdcHit.GetPulseAmpRaw(thit) <= 0)
          ((THcSignalHit *)frPosAdcErrorFlag->ConstructedAt(nrPosAdcHits))->Set(padnum, 1);
        if (rawPosAdcHit.GetPulseAmpRaw(thit) <= 0 && rawPosAdcHit.GetNSamples() > 0)
          ((THcSignalHit *)frPosAdcErrorFlag->ConstructedAt(nrPosAdcHits))->Set(padnum, 2);

        nrPosAdcHits++;
        fTotNumAdcHits++;
        fTotNumPosAdcHits++;
        fNumPosAdcHits.emplace_back(padnum);
      }
    }

    // Use Waveform
    if (rawPosAdcHit.GetNSamples() > 0) {
      rawPosAdcHit.SetSampThreshold(fSampThreshold);
      if (fSampNSA == 0)
        fSampNSA = rawPosAdcHit.GetF250_NSA();
      if (fSampNSB == 0)
        fSampNSB = rawPosAdcHit.GetF250_NSB();

      if (!fIsMC)
        rawPosAdcHit.SetF250Params(fSampNSA, fSampNSB, 4); // Set NPED =4

      if (fSampNSAT != 2)
        rawPosAdcHit.SetSampNSAT(fSampNSAT);
      rawPosAdcHit.SetSampIntTimePedestalPeak();
      fPosAdcSampWaveform.push_back(float(padnum));
      fPosAdcSampWaveform.push_back(float(rawPosAdcHit.GetNSamples()));

      for (UInt_t thit = 0; thit < rawPosAdcHit.GetNSamples(); thit++) {
        fPosAdcSampWaveform.push_back(rawPosAdcHit.GetSample(thit)); // ped subtracted sample (mV)
      }
      for (UInt_t thit = 0; thit < rawPosAdcHit.GetNSampPulses(); thit++) {
        ((THcSignalHit *)frPosAdcSampPedRaw->ConstructedAt(nrSampPosAdcHits))
            ->Set(padnum, rawPosAdcHit.GetSampPedRaw());
        ((THcSignalHit *)frPosAdcSampPed->ConstructedAt(nrSampPosAdcHits))->Set(padnum, rawPosAdcHit.GetSampPed());

        ((THcSignalHit *)frPosAdcSampPulseIntRaw->ConstructedAt(nrSampPosAdcHits))
            ->Set(padnum, rawPosAdcHit.GetSampPulseIntRaw(thit));
        ((THcSignalHit *)frPosAdcSampPulseInt->ConstructedAt(nrSampPosAdcHits))
            ->Set(padnum, rawPosAdcHit.GetSampPulseInt(thit));

        ((THcSignalHit *)frPosAdcSampPulseAmpRaw->ConstructedAt(nrSampPosAdcHits))
            ->Set(padnum, rawPosAdcHit.GetSampPulseAmpRaw(thit));
        ((THcSignalHit *)frPosAdcSampPulseAmp->ConstructedAt(nrSampPosAdcHits))
            ->Set(padnum, rawPosAdcHit.GetSampPulseAmp(thit));

        ((THcSignalHit *)frPosAdcSampPulseTimeRaw->ConstructedAt(nrSampPosAdcHits))
            ->Set(padnum, rawPosAdcHit.GetSampPulseTimeRaw(thit));
        ((THcSignalHit *)frPosAdcSampPulseTime->ConstructedAt(nrSampPosAdcHits))
            ->Set(padnum, rawPosAdcHit.GetSampPulseTime(thit) + fAdcTdcOffset);

        if (rawPosAdcHit.GetNPulses() == 0 || fUseSampWaveform == 1) {
          ((THcSignalHit *)frPosAdcPedRaw->ConstructedAt(nrPosAdcHits))->Set(padnum, rawPosAdcHit.GetSampPedRaw());
          ((THcSignalHit *)frPosAdcPed->ConstructedAt(nrPosAdcHits))->Set(padnum, rawPosAdcHit.GetSampPed());

          ((THcSignalHit *)frPosAdcPulseIntRaw->ConstructedAt(nrPosAdcHits))
              ->Set(padnum, rawPosAdcHit.GetSampPulseIntRaw(thit));
          ((THcSignalHit *)frPosAdcPulseInt->ConstructedAt(nrPosAdcHits))
              ->Set(padnum, rawPosAdcHit.GetSampPulseInt(thit));

          ((THcSignalHit *)frPosAdcPulseAmpRaw->ConstructedAt(nrPosAdcHits))
              ->Set(padnum, rawPosAdcHit.GetSampPulseAmpRaw(thit));
          ((THcSignalHit *)frPosAdcPulseAmp->ConstructedAt(nrPosAdcHits))
              ->Set(padnum, rawPosAdcHit.GetSampPulseAmp(thit));

          ((THcSignalHit *)frPosAdcPulseTimeRaw->ConstructedAt(nrPosAdcHits))
              ->Set(padnum, rawPosAdcHit.GetSampPulseTimeRaw(thit));
          ((THcSignalHit *)frPosAdcPulseTime->ConstructedAt(nrPosAdcHits))
              ->Set(padnum, rawPosAdcHit.GetSampPulseTime(thit) + fAdcTdcOffset);
          ((THcSignalHit *)frPosAdcErrorFlag->ConstructedAt(nrPosAdcHits))->Set(padnum, 3);
          if (fUseSampWaveform == 1)
            ((THcSignalHit *)frPosAdcErrorFlag->ConstructedAt(nrPosAdcHits))->Set(padnum, 0);

          ++nrPosAdcHits;
          fTotNumPosAdcHits++;
          fTotNumAdcHits++;
        }
        ++nrSampPosAdcHits;
      }
    }

    // Neg ADC hits
    THcRawAdcHit &rawNegAdcHit = hit->GetRawAdcHitNeg(); // Neg=Neg
    //    cout << "RawAdcNeg: " << padnum << " "  << rawNegAdcHit.GetNPulses() << endl;
    if ((rawNegAdcHit.GetNPulses() > 0 || rawNegAdcHit.GetNSamples() > 0) && rawNegAdcHit.HasRefTime()) {
      // if ((rawNegAdcHit.GetNPulses() > 0 || rawNegAdcHit.GetNSamples() > 0)) { // Remove RefTime Requirement

      if (fNegAdcRefTime == kBig) {
        fNegAdcRefTime     = rawNegAdcHit.GetRefTime();
        fNegAdcRefDiffTime = rawNegAdcHit.GetRefDiffTime();
      }

      if (fNegAdcRefTime != rawNegAdcHit.GetRefTime()) {
        problem_flag = kTRUE;
      }
    }

    if (fUseSampWaveform == 0) {

      for (UInt_t thit = 0; thit < rawNegAdcHit.GetNPulses(); thit++) {

        ((THcSignalHit *)frNegAdcPedRaw->ConstructedAt(nrNegAdcHits))->Set(padnum, rawNegAdcHit.GetPedRaw());
        ((THcSignalHit *)frNegAdcPed->ConstructedAt(nrNegAdcHits))->Set(padnum, rawNegAdcHit.GetPed());

        ((THcSignalHit *)frNegAdcPulseIntRaw->ConstructedAt(nrNegAdcHits))
            ->Set(padnum, rawNegAdcHit.GetPulseIntRaw(thit));
        ((THcSignalHit *)frNegAdcPulseInt->ConstructedAt(nrNegAdcHits))->Set(padnum, rawNegAdcHit.GetPulseInt(thit));

        ((THcSignalHit *)frNegAdcPulseAmpRaw->ConstructedAt(nrNegAdcHits))
            ->Set(padnum, rawNegAdcHit.GetPulseAmpRaw(thit));
        ((THcSignalHit *)frNegAdcPulseAmp->ConstructedAt(nrNegAdcHits))->Set(padnum, rawNegAdcHit.GetPulseAmp(thit));

        ((THcSignalHit *)frNegAdcPulseTimeRaw->ConstructedAt(nrNegAdcHits))
            ->Set(padnum, rawNegAdcHit.GetPulseTimeRaw(thit));
        ((THcSignalHit *)frNegAdcPulseTime->ConstructedAt(nrNegAdcHits))
            ->Set(padnum, rawNegAdcHit.GetPulseTime(thit) + fAdcTdcOffset);

        // Error flags 0-2
        if (rawNegAdcHit.GetPulseAmpRaw(thit) > 0)
          ((THcSignalHit *)frNegAdcErrorFlag->ConstructedAt(nrNegAdcHits))->Set(padnum, 0);
        if (rawNegAdcHit.GetPulseAmpRaw(thit) <= 0)
          ((THcSignalHit *)frNegAdcErrorFlag->ConstructedAt(nrNegAdcHits))->Set(padnum, 1);
        if (rawNegAdcHit.GetPulseAmpRaw(thit) <= 0 && rawNegAdcHit.GetNSamples() > 0)
          ((THcSignalHit *)frNegAdcErrorFlag->ConstructedAt(nrNegAdcHits))->Set(padnum, 2);

        nrNegAdcHits++;
        fTotNumAdcHits++;
        fTotNumNegAdcHits++;
        fNumNegAdcHits.emplace_back(padnum);
      }
    }

    // Use Waveform 
    if (rawNegAdcHit.GetNSamples() > 0) {
      rawNegAdcHit.SetSampThreshold(fSampThreshold);
      if (fSampNSA == 0)
        fSampNSA = rawNegAdcHit.GetF250_NSA();
      if (fSampNSB == 0)
        fSampNSB = rawNegAdcHit.GetF250_NSB();

      if (!fIsMC)
        rawNegAdcHit.SetF250Params(fSampNSA, fSampNSB, 4); // Set NPED =4

      if (fSampNSAT != 2)
        rawNegAdcHit.SetSampNSAT(fSampNSAT);
      rawNegAdcHit.SetSampIntTimePedestalPeak();
      fNegAdcSampWaveform.push_back(float(padnum));
      fNegAdcSampWaveform.push_back(float(rawNegAdcHit.GetNSamples()));

      for (UInt_t thit = 0; thit < rawNegAdcHit.GetNSamples(); thit++) {
        fNegAdcSampWaveform.push_back(rawNegAdcHit.GetSample(thit)); // ped subtracted sample (mV)
      }
      for (UInt_t thit = 0; thit < rawNegAdcHit.GetNSampPulses(); thit++) {
        ((THcSignalHit *)frNegAdcSampPedRaw->ConstructedAt(nrSampNegAdcHits))
            ->Set(padnum, rawNegAdcHit.GetSampPedRaw());
        ((THcSignalHit *)frNegAdcSampPed->ConstructedAt(nrSampNegAdcHits))->Set(padnum, rawNegAdcHit.GetSampPed());

        ((THcSignalHit *)frNegAdcSampPulseIntRaw->ConstructedAt(nrSampNegAdcHits))
            ->Set(padnum, rawNegAdcHit.GetSampPulseIntRaw(thit));
        ((THcSignalHit *)frNegAdcSampPulseInt->ConstructedAt(nrSampNegAdcHits))
            ->Set(padnum, rawNegAdcHit.GetSampPulseInt(thit));

        ((THcSignalHit *)frNegAdcSampPulseAmpRaw->ConstructedAt(nrSampNegAdcHits))
            ->Set(padnum, rawNegAdcHit.GetSampPulseAmpRaw(thit));
        ((THcSignalHit *)frNegAdcSampPulseAmp->ConstructedAt(nrSampNegAdcHits))
            ->Set(padnum, rawNegAdcHit.GetSampPulseAmp(thit));

        ((THcSignalHit *)frNegAdcSampPulseTimeRaw->ConstructedAt(nrSampNegAdcHits))
            ->Set(padnum, rawNegAdcHit.GetSampPulseTimeRaw(thit));
        ((THcSignalHit *)frNegAdcSampPulseTime->ConstructedAt(nrSampNegAdcHits))
            ->Set(padnum, rawNegAdcHit.GetSampPulseTime(thit) + fAdcTdcOffset);

        if (rawNegAdcHit.GetNPulses() == 0 || fUseSampWaveform == 1) {
          ((THcSignalHit *)frNegAdcPedRaw->ConstructedAt(nrNegAdcHits))->Set(padnum, rawNegAdcHit.GetSampPedRaw());
          ((THcSignalHit *)frNegAdcPed->ConstructedAt(nrNegAdcHits))->Set(padnum, rawNegAdcHit.GetSampPed());

          ((THcSignalHit *)frNegAdcPulseIntRaw->ConstructedAt(nrNegAdcHits))
              ->Set(padnum, rawNegAdcHit.GetSampPulseIntRaw(thit));
          ((THcSignalHit *)frNegAdcPulseInt->ConstructedAt(nrNegAdcHits))
              ->Set(padnum, rawNegAdcHit.GetSampPulseInt(thit));

          ((THcSignalHit *)frNegAdcPulseAmpRaw->ConstructedAt(nrNegAdcHits))
              ->Set(padnum, rawNegAdcHit.GetSampPulseAmpRaw(thit));
          ((THcSignalHit *)frNegAdcPulseAmp->ConstructedAt(nrNegAdcHits))
              ->Set(padnum, rawNegAdcHit.GetSampPulseAmp(thit));

          ((THcSignalHit *)frNegAdcPulseTimeRaw->ConstructedAt(nrNegAdcHits))
              ->Set(padnum, rawNegAdcHit.GetSampPulseTimeRaw(thit));
          ((THcSignalHit *)frNegAdcPulseTime->ConstructedAt(nrNegAdcHits))
              ->Set(padnum, rawNegAdcHit.GetSampPulseTime(thit) + fAdcTdcOffset);
          ((THcSignalHit *)frNegAdcErrorFlag->ConstructedAt(nrNegAdcHits))->Set(padnum, 3);
          if (fUseSampWaveform == 1)
            ((THcSignalHit *)frNegAdcErrorFlag->ConstructedAt(nrNegAdcHits))->Set(padnum, 0);

          ++nrNegAdcHits;
          fTotNumNegAdcHits++;
          fTotNumAdcHits++;
        }
        ++nrSampNegAdcHits;
      }
    }

    ihit++;
  } // while loop

  if (problem_flag) {
    cout << "HYPScintillatorPlane::ProcessHits " << fPlaneNum << " " << nexthit << "/" << nrawhits << endl;
    cout << " Ref problem end *******" << endl;
  }
  return (ihit);
}

//_______________________________________________________________________________________
Int_t HYPScintillatorPlane::CoarseProcess(TClonesArray &tracks) {
  // Do nothing
  return 0;
}

//_______________________________________________________________________________________
Int_t HYPScintillatorPlane::FineProcess(TClonesArray &tracks) {
  // Do nothing
  return 0;
}

//_____________________________________________________________________________
Int_t HYPScintillatorPlane::AccumulatePedestals(TClonesArray *rawhits, Int_t nexthit) {
  /*! \brief Extract the data for this plane from raw hit list THcRawHodoHit, accumulating into arrays for calculating
   * pedestals.
   *
   * - Loop through raw data for scintillator plane
   */
  Int_t nrawhits = rawhits->GetLast() + 1;
  // cout << "THcHYPScintillatorPlane::AcculatePedestals " << fPlaneNum << " " << nexthit << "/" << nrawhits << endl;

  Int_t ihit = nexthit;
  while (ihit < nrawhits) {
    THcRawHodoHit *hit = (THcRawHodoHit *)rawhits->At(ihit);
    if (hit->fPlane > fPlaneNum) {
      break;
    }
    Int_t element = hit->fCounter - 1;                       // Should check if in range
    Int_t adcPos  = hit->GetRawAdcHitPos().GetPulseIntRaw(); // Pos=Pos
    Int_t adcNeg  = hit->GetRawAdcHitNeg().GetPulseIntRaw(); // Neg=Neg

    if (adcPos <= fPosPedLimit[element]) {
      fPosPedSum[element] += adcPos;
      fPosPedSum2[element] += adcPos * adcPos;
      fPosPedCount[element]++;
      if (fPosPedCount[element] == fMinPeds / 5) {
        fPosPedLimit[element] = 100 + fPosPedSum[element] / fPosPedCount[element];
      }
    }
    if (adcNeg <= fNegPedLimit[element]) {
      fNegPedSum[element] += adcNeg;
      fNegPedSum2[element] += adcNeg * adcNeg;
      fNegPedCount[element]++;
      if (fNegPedCount[element] == fMinPeds / 5) {
        fNegPedLimit[element] = 100 + fNegPedSum[element] / fNegPedCount[element];
      }
    }
    ihit++;
  }

  fNPedestalEvents++;

  return (ihit);
}

//_______________________________________________________________________________________
void HYPScintillatorPlane::CalculatePedestals() {
  /*! \brief   Calculate pedestals from arrays made in THcHYPScintillatorPlane::AccumulatePedestals
   *
   * - Calculate pedestals from arrays made in THcHYPScintillatorPlane::AccumulatePedestals
   * - In old fortran ENGINE code, a comparison was made between calculated pedestals and the pedestals read in by the
   * FASTBUS modules for zero supression. This is not implemented.
   */
  for (UInt_t i = 0; i < fNelem; i++) {

    // Pos tubes
    fPosPed[i]    = ((Double_t)fPosPedSum[i]) / TMath::Max(1, fPosPedCount[i]);
    fPosThresh[i] = fPosPed[i] + 15;

    // Neg tubes
    fNegPed[i]    = ((Double_t)fNegPedSum[i]) / TMath::Max(1, fNegPedCount[i]);
    fNegThresh[i] = fNegPed[i] + 15;
  }
}

//_____________________________________________________________________________
void HYPScintillatorPlane::InitializePedestals() {
  /*! \brief   called by THcHodoPlane::ReadDatabase
   *
   * - Initialize variables used in  THcHYPScintillatorPlane::AccumulatePedestals and
   * THcHYPScintillatorPlane::CalculatePedestals
   * - Minimum number of pedestal events needed for calculation, fMinPeds, hadrcoded to 500
   */
  fNPedestalEvents = 0;
  fMinPeds         = 500; // In engine, this is set in parameter file
  fPosPedSum       = new Int_t[fNelem];
  fPosPedSum2      = new Int_t[fNelem];
  fPosPedLimit     = new Int_t[fNelem];
  fPosPedCount     = new Int_t[fNelem];
  fNegPedSum       = new Int_t[fNelem];
  fNegPedSum2      = new Int_t[fNelem];
  fNegPedLimit     = new Int_t[fNelem];
  fNegPedCount     = new Int_t[fNelem];

  fPosPed    = new Double_t[fNelem];
  fNegPed    = new Double_t[fNelem];
  fPosThresh = new Double_t[fNelem];
  fNegThresh = new Double_t[fNelem];
  for (UInt_t i = 0; i < fNelem; i++) {
    fPosPedSum[i]   = 0;
    fPosPedSum2[i]  = 0;
    fPosPedLimit[i] = 1000; // In engine, this are set in parameter file
    fPosPedCount[i] = 0;
    fNegPedSum[i]   = 0;
    fNegPedSum2[i]  = 0;
    fNegPedLimit[i] = 1000; // In engine, this are set in parameter file
    fNegPedCount[i] = 0;
  }
}
//____________________________________________________________________________

ClassImp(HYPScintillatorPlane)
