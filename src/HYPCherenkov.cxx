#include "HYPCherenkov.h"
#include "HYPData.h"
#include "THaEvData.h"
#include "THaDetMap.h"
#include "THcDetectorMap.h"
#include "THcGlobals.h"
#include "THaCutList.h"
#include "THcParmList.h"
#include "THaApparatus.h"
#include "VarDef.h"
#include "VarType.h"
#include "THaTrack.h"
#include "TClonesArray.h"
#include "TMath.h"
#include "THaTrackProj.h"
#include "THcRawAdcHit.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace std;

//_____________________________________________________________________
HYPCherenkov::HYPCherenkov( const char* name, const char* description,
		      THaApparatus* apparatus) :
  THaNonTrackingDetector(name, description, apparatus), 
  fNhits(0), fPresentP(0)
{

}

//_____________________________________________________________________
HYPCherenkov::HYPCherenkov() :
  THaNonTrackingDetector(), fNhits(0), fPresentP(0)
{

}
//_____________________________________________________________________
HYPCherenkov::~HYPCherenkov()
{
  // Destructor
  RemoveVariables();

  DeleteArrays();
}

//__________________________________________________________________
THaAnalysisObject::EStatus HYPCherenkov::Init(const TDatime &date)
{

  //  cout << "HYPCherenkov::Init" << endl;

  string EngineDID = string(GetApparatus()->GetName()).substr(0, 1) + GetName();
  std::transform(EngineDID.begin(), EngineDID.end(), EngineDID.begin(), ::toupper);
  if(gHcDetectorMap->FillMap(fDetMap, EngineDID.c_str()) < 0) {
    static const char* const here = "Init()";
    Error(Here(here), "Error filling detectormap for %s.", EngineDID.c_str());
    return kInitError;
  }

  EStatus status;
  if((status = THaNonTrackingDetector::Init( date )))
    return fStatus=status;

  InitHitList(fDetMap, "THcCherenkovHit", fDetMap->GetTotNumChan()+1, 0, fADC_RefTimeCut);
  
  fPresentP = 0;
  THaVar* vpresent = gHaVars->Find(Form("%s.present",GetApparatus()->GetName()));
  if(vpresent) {
    fPresentP = (Bool_t *) vpresent->GetValuePointer();
  }
  
  return fStatus = kOK;
}

//_____________________________________________________________________
void HYPCherenkov::Clear( Option_t* opt )
{
  fNhits = 0;
  fPosDataRaw.clear();
  fNegDataRaw.clear();
  fPosData.clear();
  fNegData.clear();

  fPosSampDataRaw.clear();
  fNegSampDataRaw.clear();
  fPosSampData.clear();
  fNegSampData.clear();

  fPosErrorFlag.clear();
  fNegErrorFlag.clear();

  fPosDataGood.clear();
  fNegDataGood.clear();

  std::fill(fPosNpe.begin(), fPosNpe.end(), 0);
  std::fill(fNegNpe.begin(), fNegNpe.end(), 0);
  std::fill(fNumGoodPosAdcHits.begin(), fNumGoodPosAdcHits.end(), 0);
  std::fill(fNumGoodNegAdcHits.begin(), fNumGoodNegAdcHits.end(), 0);
  std::fill(fPosAdcPedRaw.begin(), fPosAdcPedRaw.end(), 0);
  std::fill(fNegAdcPedRaw.begin(), fNegAdcPedRaw.end(), 0);
  std::fill(fPosAdcPed.begin(), fPosAdcPed.end(), 0);
  std::fill(fNegAdcPed.begin(), fNegAdcPed.end(), 0);
  std::fill(fGoodPosAdcPed.begin(), fGoodPosAdcPed.end(), 0);
  std::fill(fGoodNegAdcPed.begin(), fGoodNegAdcPed.end(), 0);

  fPosNpeSum = 0.;
  fNegNpeSum = 0.;
  fNpeSum = 0.;
}

//_____________________________________________________________________
void HYPCherenkov::DeleteArrays()
{
  delete [] fPosGain; fPosGain = nullptr;
  delete [] fNegGain; fNegGain = nullptr;
  delete [] fAdcPosTimeWindowMin; fAdcPosTimeWindowMin = nullptr; 
  delete [] fAdcPosTimeWindowMax; fAdcPosTimeWindowMax = nullptr; 
  delete [] fAdcNegTimeWindowMin; fAdcNegTimeWindowMin = nullptr; 
  delete [] fAdcNegTimeWindowMax; fAdcNegTimeWindowMax = nullptr; 
}

//_____________________________________________________________________
Int_t HYPCherenkov::ReadDatabase( const TDatime& date )
{
  //  cout << "HYPCherenkov::ReadDatabase" << endl;

  string prefix = string(GetApparatus()->GetName()).substr(0, 1) + GetName();
  std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);

  CreateMissReportParms(prefix.c_str());

  DBRequest list_1[] = {
    {"_nelem", &fNelem, kInt},
    {nullptr}
  };
  gHcParms->LoadParmValues(list_1, prefix.c_str());

  fPosGain = new Double_t[fNelem];
  fNegGain = new Double_t[fNelem];

  fAdcPosTimeWindowMin = new Double_t[fNelem];
  fAdcPosTimeWindowMax = new Double_t[fNelem];
  fAdcNegTimeWindowMin = new Double_t[fNelem];
  fAdcNegTimeWindowMax = new Double_t[fNelem];

  // Initialize variables
  for(Int_t i = 0; i < fNelem; i++) {
    fPosGain[i] = 1.0;    
    fNegGain[i] = 1.0;    
    fAdcPosTimeWindowMin[i] = -1000.;
    fAdcPosTimeWindowMax[i] = 1000.;
    fAdcNegTimeWindowMin[i] = -1000.;
    fAdcNegTimeWindowMax[i] = 1000.;
  }

  DBRequest list[]={
    {"_SampThreshold",     &fSampThreshold,   kDouble,0,1},
    {"_SampNSA",           &fSampNSA,         kInt,0,1},
    {"_SampNSAT",          &fSampNSAT,        kInt,0,1},
    {"_SampNSB",           &fSampNSB,         kInt,0,1},
    {"_UseSampWaveform",   &fUseSampWaveform, kInt,0,1},
    {"_adcrefcut",         &fADC_RefTimeCut,  kInt,0,1},
    {"_debug_adc",         &fDebugAdc,        kInt,0,1},
    {"_adc_tdc_offset",    &fAdcTdcOffset,    kDouble, 0, 1},
    {"_adcPosTimeWindowMin", fAdcPosTimeWindowMin, kDouble, static_cast<UInt_t>(fNelem), 1},
    {"_adcPosTimeWindowMax", fAdcPosTimeWindowMax, kDouble, static_cast<UInt_t>(fNelem), 1},
    {"_adcNegTimeWindowMin", fAdcNegTimeWindowMin, kDouble, static_cast<UInt_t>(fNelem), 1},
    {"_adcNegTimeWindowMax", fAdcNegTimeWindowMax, kDouble, static_cast<UInt_t>(fNelem), 1},
    {"_pos_gain",            fPosGain, kDouble, static_cast<UInt_t>(fNelem), 1},
    {"_neg_gain",            fNegGain, kDouble, static_cast<UInt_t>(fNelem), 1},
    {nullptr}
  };

  // Default values
  fADC_RefTimeCut = 0;
  fAdcTdcOffset = 0.;
  fSampThreshold = 5.;
  fSampNSA = 0;   // use value stored in event 125 info
  fSampNSB = 0;   // use value stored in event 125 info
  fSampNSAT = 2;  // default value in THcRawHit::SetF250Params
  fUseSampWaveform = 0; // 0= do not use , 1 = use Sample Waveform
  fDebugAdc = 0;  // Save raw adc variables, default = 0

  gHcParms->LoadParmValues((DBRequest*)&list, prefix.c_str());

  fPosDataRaw.reserve(fNelem);
  fNegDataRaw.reserve(fNelem);
  fPosData.reserve(fNelem);
  fNegData.reserve(fNelem);
  fPosSampDataRaw.reserve(fNelem);
  fNegSampDataRaw.reserve(fNelem);
  fPosSampData.reserve(fNelem);
  fNegSampData.reserve(fNelem);

  fPosNpe.assign(fNelem, 0.0);
  fNegNpe.assign(fNelem, 0.0);

  fPosAdcPedRaw.assign(fNelem, 0.0);
  fNegAdcPedRaw.assign(fNelem, 0.0);
  fPosAdcPed.assign(fNelem, 0.0);
  fNegAdcPed.assign(fNelem, 0.0);
  fGoodPosAdcPed.assign(fNelem, 0.0);
  fGoodNegAdcPed.assign(fNelem, 0.0);
  fNumGoodPosAdcHits.assign(fNelem, 0);
  fNumGoodNegAdcHits.assign(fNelem, 0);

  fIsInit = true;

  return kOK;
}

//_____________________________________________________________________
Int_t HYPCherenkov::DefineVariables( EMode mode )
{

  if (fDebugAdc) {
    RVarDef vars[] = {
      // FIXME; put the counters back?
      // {"numPosAdcHits",        "Number of Positive ADC Hits Per PMT",      "fNumPosAdcHits"},        // Aerogel occupancy
      // {"totNumPosAdcHits",     "Total Number of Positive ADC Hits",        "fTotNumPosAdcHits"},     // Aerogel multiplicity
      // {"numNegAdcHits",        "Number of Negative ADC Hits Per PMT",      "fNumNegAdcHits"},        // Aerogel occupancy
      // {"totNumNegAdcHits",     "Total Number of Negative ADC Hits",        "fTotNumNegAdcHits"},     // Aerogel multiplicity
      // {"totnumAdcHits",       "Total Number of ADC Hits Per PMT",          "fTotNumAdcHits"},        // Aerogel multiplicity
      {"posPadNumRaw"        "Paddle number",                     "fPosDataRaw.paddle"},
      {"posAdcPedRaw",       "Positive Raw ADC pedestals",        "fPosAdcPedRaw"},
      {"posAdcPulseIntRaw",  "Positive Raw ADC pulse integrals",  "fPosDataRaw.PulseInt"},
      {"posAdcPulseAmpRaw",  "Positive Raw ADC pulse amplitudes", "fPosDataRaw.PulseAmp"},
      {"posAdcPulseTimeRaw", "Positive Raw ADC pulse times",      "fPosDataRaw.PulseTime"},
      {"posAdcPed",          "Positive ADC pedestals",            "fPosAdcPed"},
      {"posAdcPulseInt",     "Positive ADC pulse integrals",      "fPosData.PulseInt"},
      {"posAdcPulseAmp",     "Positive ADC pulse amplitudes",     "fPosData.PulseAmp"},
      {"posAdcPulseTime",    "Positive ADC pulse times",          "fPosData.PulseTime"},
      {"negPadNumRaw",       "Paddle number",                     "fNegDataRaw.paddle"},
      {"negAdcPedRaw",       "Negative Raw ADC pedestals",        "fNegAdcPedRaw"},
      {"negAdcPulseIntRaw",  "Negative Raw ADC pulse integrals",  "fNegDataRaw.PulseInt"},
      {"negAdcPulseAmpRaw",  "Negative Raw ADC pulse amplitudes", "fNegDataRaw.PulseAmp"},
      {"negAdcPulseTimeRaw", "Negative Raw ADC pulse times",      "fNegDataRaw.PulseTime"},
      {"negAdcPed",          "Negative ADC pedestals",            "fNegAdcPed"},
      {"negAdcPulseInt",     "Negative ADC pulse integrals",      "fNegData.PulseInt"},
      {"negAdcPulseAmp",     "Negative ADC pulse amplitudes",     "fNegData.PulseAmp"},
      {"negAdcPulseTime",    "Negative ADC pulse times",          "fNegData.PulseTime"},
      {"posErrorFlag",       "Error Flag for When FPGA Fails",    "fPosErrorFlag"},
      {"negErrorFlag",       "Error Flag for When FPGA Fails",    "fNegErrorFlag"},
      { nullptr }
    };
    DefineVarsFromList( vars, mode);
  }

  RVarDef vars[] = {
    {"posNpe",      "Number of Positive PEs per PMT", "fPosNpe"},
    {"negNpe",      "Number of Negative PEs per PMT", "fNegNpe"},
    {"posNpeSum",   "Total Number of Positive PEs", "fPosNpeSum"},
    {"negNpeSum",   "Total Number of Negative PEs", "fNegNpeSum"},
    {"npeSum",      "Total Number of PEs",          "fNpeSum"},
    {"posPadNum",              "Paddle number for Pos ADC",              "fPosDataGood.paddle"},
    {"goodPosAdcPed",          "Good Positive ADC pedestals",            "fGoodPosAdcPed"},
    {"goodPosAdcPulseInt",     "Good Positive ADC pulse integrals",      "fPosDataGood.PulseInt"},
    {"goodPosAdcPulseAmp",     "Good Positive ADC pulse amplitudes",     "fPosDataGood.PulseAmp"},
    {"goodPosAdcPulseTime",    "Good Positive ADC pulse times",          "fPosDataGood.PulseTime"},
    {"negPadNum",              "Paddle number for Neg ADC",              "fNegDataGood.paddle"},
    {"goodNegAdcPed",          "Good Negative ADC pedestals",            "fGoodNegAdcPed"},
    {"goodNegAdcPulseInt",     "Good Negative ADC pulse integrals",      "fNegDataGood.PulseInt"},
    {"goodNegAdcPulseAmp",     "Good Negative ADC pulse amplitudes",     "fNegDataGood.PulseAmp"},
    {"goodNegAdcPulseTime",    "Good Negative ADC pulse times",          "fNegDataGood.PulseTime"},
    {"numGoodPosAdcHits",      "Number of Good Pos ADC Hits per PMT",    "fNumGoodPosAdcHits"}, // occupancy
    {"numGoodNegAdcHits",      "Number of Good Neg ADC Hits per PMT",    "fNumGoodNegAdcHits"}, // occupancy
    {nullptr}
  };

  return DefineVarsFromList(vars, mode);
}

//_____________________________________________________________________
Int_t HYPCherenkov::Decode( const THaEvData& evdata )
{
  Bool_t present = kTRUE;	// Suppress reference time warnings
  if(fPresentP) {		      // if this spectrometer not part of trigger
    present = *fPresentP;
  }

  fNhits = DecodeToHitList(evdata, !present);
  
  // FIXME: Define a function to store data and call it for pos/neg
  Int_t ihit = 0;
  while(ihit < fNhits) {
    THcCherenkovHit* hit         = (THcCherenkovHit*) fRawHitList->At(ihit);
    Int_t            npmt        = hit->fCounter;
    THcRawAdcHit&    rawPosAdcHit = hit->GetRawAdcHitPos();
    THcRawAdcHit&    rawNegAdcHit = hit->GetRawAdcHitNeg();

    Int_t errorflag = -1;
    if ( fUseSampWaveform == 0 ) {
      for (UInt_t thit = 0; thit < rawPosAdcHit.GetNPulses(); thit++) {
	FADCHitData posdata_raw;
	FADCHitData posdata;

	rawPosAdcHit.SetF250Params(70, 5, 4);

	posdata_raw.paddle = npmt;
	posdata_raw.Ped = rawPosAdcHit.GetPedRaw();
	posdata_raw.PulseInt = rawPosAdcHit.GetPulseIntRaw(thit);
	posdata_raw.PulseAmp = rawPosAdcHit.GetPulseAmpRaw(thit);
	posdata_raw.PulseTime = rawPosAdcHit.GetPulseTimeRaw(thit);
       
	posdata.paddle = npmt;
	posdata.Ped = rawPosAdcHit.GetPed();
	posdata.PulseInt = rawPosAdcHit.GetPulseInt(thit);
	posdata.PulseAmp = rawPosAdcHit.GetPulseAmp(thit);
	posdata.PulseTime = rawPosAdcHit.GetPulseTime(thit) + fAdcTdcOffset;

	if(posdata_raw.PulseAmp > 0)  errorflag = 0;
	if(posdata_raw.PulseAmp <= 0) errorflag = 1;
	if(posdata_raw.PulseAmp <= 0 && rawPosAdcHit.GetNSamples() > 0) errorflag = 2;

	fPosDataRaw.emplace_back(posdata_raw);
	fPosData.emplace_back(posdata);
	fPosErrorFlag.emplace_back(errorflag);
	fPosAdcPedRaw[npmt-1] = posdata_raw.Ped;
	fPosAdcPed[npmt-1] = posdata.Ped;

	// FIXME: Do we want to add an option to use pedestal from DB
      }
    }// Using Pulse Data

    // Sample Data
    if (rawPosAdcHit.GetNSamples() >0 ) {
      rawPosAdcHit.SetSampThreshold(fSampThreshold);
      if (fSampNSA == 0) fSampNSA=rawPosAdcHit.GetF250_NSA();
      if (fSampNSB == 0) fSampNSB=rawPosAdcHit.GetF250_NSB();
      rawPosAdcHit.SetF250Params(fSampNSA,fSampNSB,4); // Set NPED =4
      if (fSampNSAT != 2) rawPosAdcHit.SetSampNSAT(fSampNSAT);
      rawPosAdcHit.SetSampIntTimePedestalPeak();
      
      // FIXME: Do we want to save waveform data?
      for (UInt_t thit = 0; thit < rawPosAdcHit.GetNSampPulses(); thit++) {

        FADCHitData possampdata_raw;
        FADCHitData possampdata;

        possampdata_raw.paddle = npmt;
        possampdata_raw.Ped = rawPosAdcHit.GetSampPedRaw();
        possampdata_raw.PulseInt = rawPosAdcHit.GetSampPulseIntRaw(thit);
        possampdata_raw.PulseAmp = rawPosAdcHit.GetSampPulseAmpRaw(thit);
        possampdata_raw.PulseTime = rawPosAdcHit.GetSampPulseTimeRaw(thit);
        
        possampdata.paddle = npmt;
        possampdata.Ped = rawPosAdcHit.GetSampPed();
        possampdata.PulseInt = rawPosAdcHit.GetSampPulseInt(thit);
        possampdata.PulseAmp = rawPosAdcHit.GetSampPulseAmp(thit);
        possampdata.PulseTime = rawPosAdcHit.GetSampPulseTime(thit) + fAdcTdcOffset;

        fPosSampDataRaw.emplace_back(possampdata_raw);
        fPosSampData.emplace_back(possampdata);

        if ( rawPosAdcHit.GetNPulses() ==0 || fUseSampWaveform ==1 ) {

          errorflag = 3;
          if(fUseSampWaveform) errorflag = 0;

          fPosDataRaw.emplace_back(possampdata_raw);
          fPosData.emplace_back(possampdata);
      	  fPosErrorFlag.emplace_back(errorflag);
          fPosAdcPedRaw[npmt-1] = possampdata_raw.Ped;
          fPosAdcPed[npmt-1] = possampdata.Ped;

        }
      }// samp pulse loop
    }

    // Negative PMT
    errorflag = -1; // reset
    if ( fUseSampWaveform == 0 ) {
      for (UInt_t thit = 0; thit < rawNegAdcHit.GetNPulses(); thit++) {

      FADCHitData negdata_raw;
      FADCHitData negdata;

      negdata_raw.paddle = npmt;
      negdata_raw.Ped = rawNegAdcHit.GetPedRaw();
      negdata_raw.PulseInt = rawNegAdcHit.GetPulseIntRaw(thit);
      negdata_raw.PulseAmp = rawNegAdcHit.GetPulseAmpRaw(thit);
      negdata_raw.PulseTime = rawNegAdcHit.GetPulseTimeRaw(thit);
       
      negdata.paddle = npmt;
      negdata.Ped = rawNegAdcHit.GetPed();
      negdata.PulseInt = rawNegAdcHit.GetPulseInt(thit);
      negdata.PulseAmp = rawNegAdcHit.GetPulseAmp(thit);
      negdata.PulseTime = rawNegAdcHit.GetPulseTime(thit) + fAdcTdcOffset;

      if(negdata_raw.PulseAmp> 0) errorflag = 0;
      if(negdata_raw.PulseAmp <= 0) errorflag = 1;
      if(negdata_raw.PulseAmp <= 0 && rawNegAdcHit.GetNSamples() > 0) errorflag = 2;

      fNegDataRaw.emplace_back(negdata_raw);
      fNegData.emplace_back(negdata);
      fNegErrorFlag.emplace_back(errorflag);
      fNegAdcPedRaw[npmt-1] = negdata_raw.Ped;
      fNegAdcPed[npmt-1] = negdata.Ped;
      }
    }
    if (rawNegAdcHit.GetNSamples() >0 ) {
      rawNegAdcHit.SetSampThreshold(fSampThreshold);
      if (fSampNSA == 0) fSampNSA=rawNegAdcHit.GetF250_NSA();
      if (fSampNSB == 0) fSampNSB=rawNegAdcHit.GetF250_NSB();
      rawNegAdcHit.SetF250Params(fSampNSA,fSampNSB,4); // Set NPED =4
      if (fSampNSAT != 2) rawNegAdcHit.SetSampNSAT(fSampNSAT);
      rawNegAdcHit.SetSampIntTimePedestalPeak();
      
      for (UInt_t thit = 0; thit < rawNegAdcHit.GetNSampPulses(); thit++) {

        FADCHitData negsampdata_raw;
        FADCHitData negsampdata;

        negsampdata_raw.paddle = npmt;
        negsampdata_raw.Ped = rawNegAdcHit.GetSampPedRaw();
        negsampdata_raw.PulseInt = rawNegAdcHit.GetSampPulseIntRaw(thit);
        negsampdata_raw.PulseAmp = rawNegAdcHit.GetSampPulseAmpRaw(thit);
        negsampdata_raw.PulseTime = rawNegAdcHit.GetSampPulseTimeRaw(thit);
        
        negsampdata.paddle = npmt;
        negsampdata.Ped = rawNegAdcHit.GetSampPed();
        negsampdata.PulseInt = rawNegAdcHit.GetSampPulseInt(thit);
        negsampdata.PulseAmp = rawNegAdcHit.GetSampPulseAmp(thit);
        negsampdata.PulseTime = rawNegAdcHit.GetSampPulseTime(thit) + fAdcTdcOffset;

        fNegSampDataRaw.emplace_back(negsampdata_raw);
        fNegSampData.emplace_back(negsampdata);

        if ( rawNegAdcHit.GetNPulses() ==0 || fUseSampWaveform ==1 ) {

          errorflag = 3;
          if(fUseSampWaveform) errorflag = 0;

          fNegDataRaw.emplace_back(negsampdata_raw);
          fNegData.emplace_back(negsampdata);
      	  fNegErrorFlag.emplace_back(errorflag);
          fNegAdcPedRaw[npmt-1] = negsampdata_raw.Ped;
          fNegAdcPed[npmt-1] = negsampdata.Ped;

        }
      }// samp pulse loop
    }// Negative PMT


    ihit++;
  }// while
  return ihit;
}

//_____________________________________________________________________
Int_t HYPCherenkov::CoarseProcess( TClonesArray& tracks )
{
  
  // FIXME: Add start time correction?
  Double_t start_time = 0.0;

  // Loop over pos adc hits
  for(auto adchit : fPosData) {
    Int_t ipmt = adchit.paddle -1;
    Double_t timediff = start_time - adchit.PulseTime;

    Bool_t pass_timecut = (timediff > fAdcPosTimeWindowMin[ipmt] && timediff < fAdcPosTimeWindowMax[ipmt]);
    if(pass_timecut) {
      // Calculate NPE
      Double_t npe = fPosGain[ipmt] * adchit.PulseInt; // use pulse int or pulse amp?
      fPosNpeSum += npe;
      fPosNpe.at(ipmt) = npe;

      adchit.Is_good_hit = 1; // set good hit flag to true
      fPosDataGood.emplace_back(adchit); // add to the good hit list
      fGoodPosAdcPed.at(ipmt-1) = adchit.Ped;
      fNumGoodPosAdcHits.at(ipmt) += 1;
    }
  }  

  // Loop over neg adc hits
  for(auto adchit : fNegData) {
    Int_t ipmt = adchit.paddle -1;
    Double_t timediff = start_time - adchit.PulseTime;

    Bool_t pass_timecut = (timediff > fAdcNegTimeWindowMin[ipmt] && timediff < fAdcNegTimeWindowMax[ipmt]);
    if(pass_timecut) {
      // Calculate NPE
      Double_t npe = fNegGain[ipmt] * adchit.PulseInt; // use pulse int or pulse amp?
      fNegNpeSum += npe;
      fNegNpe.at(ipmt) = npe;

      adchit.Is_good_hit = 1; // set good hit flag to true
      fNegDataGood.emplace_back(adchit); // add to the good hit list
      fGoodNegAdcPed.at(ipmt-1) = adchit.Ped;
      fNumGoodNegAdcHits.at(ipmt) += 1;
    }
  }  

  fNpeSum = fPosNpeSum + fNegNpeSum;

  return 0;

}

//_____________________________________________________________________
Int_t HYPCherenkov::FineProcess( TClonesArray& tracks )
{
  return 0;
}

//_____________________________________________________________________

ClassImp(HYPCherenkov)
