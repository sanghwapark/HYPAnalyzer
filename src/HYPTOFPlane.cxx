#include "HYPTOFPlane.h"
#include "HYPDC.h"
#include "HYPData.h"
#include "THaDetMap.h"
#include "THaEvData.h"
#include "VarDef.h"
#include "VarType.h"
#include "THcGlobals.h"
#include "THcParmList.h"
#include "THcRawAdcHit.h"
#include "THcRawTdcHit.h"
#include "THcRawHodoHit.h"
#include <cstdlib>

using namespace std;

//__________________________________________________________________
HYPTOFPlane::HYPTOFPlane( const char* name, const char* description, 
			  const Int_t iplane, THaDetectorBase *parent) :
  THaSubDetector(name, description, parent)
{
  // constructor
}

//__________________________________________________________________
HYPTOFPlane::~HYPTOFPlane()
{
  // Destructor
  RemoveVariables();
}

//__________________________________________________________________
THaAnalysisObject::EStatus HYPTOFPlane::Init(const TDatime &date)
{

  EStatus status;
  if( (status = THaSubDetector::Init(date)) )
    return fStatus = status;

  return fStatus = kOK;
}

//__________________________________________________________________
Int_t HYPTOFPlane::ReadDatabase( const TDatime& date )
{

  char prefix[2];
  prefix[0]=tolower(GetParent()->GetPrefix()[0]);
  prefix[1]='\0';

  Bool_t optional = true;

  DBRequest list[] = {
    {"tof_debug_adc", &fDebugADC, kInt, 0, optional},
    {"tof_SampNSA",   &fSampNSA,  kInt, 0, optional},
    {"tof_SampNSB",   &fSampNSAT, kInt, 0, optional},
    {"tof_SampNSAT",  &fSampNSB,  kInt, 0, optional},
    {"tof_SampThreshold", &fSampThreshold, kDouble, 0, optional},
    {"tof_outputSampWaveform", &fOutputSampWaveform, kInt, 0, optional},
    {"tof_UseSampWaveform", &fUseSampWaveform, kInt, 0, optional},
    {nullptr}
  };

  fDebugADC = 0;
  fUseSampWaveform = 0;
  fOutputSampWaveform = 0;
  fSampNSA = 0;
  fSampNSB = 0;
  fSampNSAT = 2;
  fSampThreshold = 5.;

  gHcParms->LoadParmValues((DBRequest*)&list, prefix);

  // Init some vars
  for(int i = 0; i < 2; i++) {
    fTdcRefTime[i] = kBig;
    fTdcRefDiffTime[i] = kBig;
    fAdcRefTime[i] = kBig;
    fAdcRefDiffTime[i] = kBig;
  }

  return kOK;
}


//__________________________________________________________________
Int_t HYPTOFPlane::DefineVariables( EMode mode )
{

  if( mode == kDefine && fIsSetup ) return kOK;
  fIsSetup = ( mode == kDefine );

  if(fDebugADC) {
    RVarDef vars[] = {
      {"posAdcPadNum",       "Paddle number",                     "fPosDataRaw.paddle"},
      {"posAdcPedRaw",       "Positive Raw ADC pedestals",        "fPosDataRaw.Ped"},
      {"posAdcPulseIntRaw",  "Positive Raw ADC pulse integrals",  "fPosDataRaw.PulseInt"},
      {"posAdcPulseAmpRaw",  "Positive Raw ADC pulse amplitudes", "fPosDataRaw.PulseAmp"},
      {"posAdcPulseTimeRaw", "Positive Raw ADC pulse times",      "fPosDataRaw.PulseTime"},
      {"posAdcPed",          "Positive ADC pedestals",            "fPosData.Ped"},
      {"posAdcPulseInt",     "Positive ADC pulse integrals",      "fPosData.PulseInt"},
      {"posAdcPulseAmp",     "Positive ADC pulse amplitudes",     "fPosData.PulseAmp"},
      {"posAdcPulseTime",    "Positive ADC pulse times",          "fPosData.PulseTime"},
      {"negAdcPadNum",       "Paddle number",                     "fNegDataRaw.paddle"},
      {"negAdcPedRaw",       "Negative Raw ADC pedestals",        "fNegDataRaw.Ped"},
      {"negAdcPulseIntRaw",  "Negative Raw ADC pulse integrals",  "fNegDataRaw.PulseInt"},
      {"negAdcPulseAmpRaw",  "Negative Raw ADC pulse amplitudes", "fNegDataRaw.PulseAmp"},
      {"negAdcPulseTimeRaw", "Negative Raw ADC pulse times",      "fNegDataRaw.PulseTime"},
      {"negAdcPed",          "Negative ADC pedestals",            "fNegData.Ped"},
      {"negAdcPulseInt",     "Negative ADC pulse integrals",      "fNegData.PulseInt"},
      {"negAdcPulseAmp",     "Negative ADC pulse amplitudes",     "fNegData.PulseAmp"},
      {"negAdcPulseTime",    "Negative ADC pulse times",          "fNegData.PulseTime"},
      {"posErrorFlag",       "Error Flag for When FPGA Fails",    "fPosErrorFlag"},
      {"negErrorFlag",       "Error Flag for When FPGA Fails",    "fNegErrorFlag"},      
      {"posTdcTimeRaw",      "Positive Raw TDC Time",             "fPosTdcData.TimeRaw"},
      {"posTdcTime",         "Positive Ref time subtracted TDC",  "fPosTdcData.Time"},
      {"negTdcTimeRaw",      "Negative Raw TDC Time",             "fNegTdcData.TimeRaw"},
      {"negTdcTime",         "Negative Ref time subtracted TDC",  "fNegTdcData.Time"},
      {nullptr}
    };
    DefineVarsFromList(vars, mode);
  }

  if(fOutputSampWaveform) {
    RVarDef vars[] = {
      {"adcNegSampWaveform", "FADC Neg ADC Sample Waveform", "fNegSampWaveform"},
      {"adcPosSampWaveform", "FADC Pos ADC Sample Waveform", "fPosSampWaveform"},
      {nullptr}
    };
    DefineVarsFromList(vars, mode);
  }

  RVarDef vars[] = {
    {"AdcRefTime",         "Reference time for ADC",       "fAdcRefTime"},
    {"AdcRefDiffTime",     "Reference Diff time for ADC",  "fAdcRefDiffTime"},
    {"TdcRefTime",         "Reference time for TDC",       "fTdcRefTime"},
    {"TdcRefDiffTime",     "Reference Diff time for TDC",  "fTdcRefDiffTime"},
    {nullptr}
  };

  return DefineVarsFromList(vars, mode);
}

//__________________________________________________________________
void HYPTOFPlane::Clear( Option_t* opt )
{
  THaSubDetector::Clear(opt);

  fPosAdcDataRaw.clear();
  fNegAdcDataRaw.clear();
  fPosAdcData.clear();
  fNegAdcData.clear();

  fPosAdcSampDataRaw.clear();
  fNegAdcSampDataRaw.clear();
  fPosAdcSampData.clear();
  fNegAdcSampData.clear();

  fPosAdcErrorFlag.clear();
  fNegAdcErrorFlag.clear();

  fPosTdcData.clear();
  fNegTdcData.clear();

  fPosSampWaveform.clear();
  fNegSampWaveform.clear();

  for(int i = 0; i < 2; i++) {
    fTdcRefTime[i] = kBig;
    fTdcRefDiffTime[i] = kBig;
    fAdcRefTime[i] = kBig;
    fAdcRefDiffTime[i] = kBig;
  }

}

//__________________________________________________________________
Int_t HYPTOFPlane::Decode( const THaEvData& evdata )
{
  // Do nothing. Decoding is done in the parent class
  return 0;
}

//__________________________________________________________________
Int_t HYPTOFPlane::ProcessHits(TClonesArray *rawhits, int nexthit)
{

  Int_t nrawhits = rawhits->GetLast()+1;
  Int_t ihit = nexthit;

  while( ihit < nrawhits )
  {
    THcRawHodoHit* hit = (THcRawHodoHit*) rawhits->At(ihit);
    Int_t padnum = hit->fCounter;

    // for both sides of PMTs
    for(Int_t signal = 0; signal < 2; signal++) {

      THcRawTdcHit& rawTdcHit = (signal == 0) ? hit->GetRawTdcHitPos() : hit->GetRawTdcHitNeg();
      // TDC ref time
      if( rawTdcHit.GetNHits() > 0 && rawTdcHit.HasRefTime()) {
        if( fTdcRefTime[signal] == kBig ) {
          fTdcRefTime[signal] = rawTdcHit.GetRefTime();
          fTdcRefDiffTime[signal] = rawTdcHit.GetRefDiffTime();
        }
      }
      // TDC
      for(UInt_t thit = 0; thit < rawTdcHit.GetNHits(); thit++){
        if(signal == 0) {
          fPosTdcData.emplace_back(padnum, rawTdcHit.GetTimeRaw(thit), rawTdcHit.GetTime(thit), 0 );
        }
        else {
          fNegTdcData.emplace_back(padnum, rawTdcHit.GetTimeRaw(thit), rawTdcHit.GetTime(thit), 0 );

        }        
      }

      // ADC
      THcRawAdcHit& rawAdcHit = (signal == 0) ? hit->GetRawAdcHitPos() : hit->GetRawAdcHitNeg();

      // Ref time
      if( (rawAdcHit.GetNPulses() > 0. || rawAdcHit.GetNSamples() > 0) && rawAdcHit.HasRefTime() ) {
        if( fAdcRefTime[signal] == kBig ) {
          fAdcRefTime[signal] = rawAdcHit.GetRefTime();
          fAdcRefDiffTime[signal] = rawAdcHit.GetRefDiffTime();
        }
      }

      Int_t errorflag = -1;
      for(UInt_t thit = 0; thit < rawAdcHit.GetNPulses(); thit++) {
        FADCHitData fdata_raw;
        FADCHitData fdata;
   
        fdata_raw.paddle = padnum;
        fdata_raw.Ped = rawAdcHit.GetPedRaw();
        fdata_raw.PulseInt = rawAdcHit.GetPulseIntRaw(thit);
        fdata_raw.PulseAmp = rawAdcHit.GetPulseAmpRaw(thit);
        fdata_raw.PulseTime = rawAdcHit.GetPulseTimeRaw(thit);

        fdata.paddle = padnum;
        fdata.Ped = rawAdcHit.GetPedRaw();
        fdata.PulseInt = rawAdcHit.GetPulseInt(thit);
        fdata.PulseAmp = rawAdcHit.GetPulseAmp(thit);
        fdata.PulseTime = rawAdcHit.GetPulseTime(thit);

        if(fdata_raw.PulseAmp > 0)  errorflag = 0;
        if(fdata_raw.PulseAmp <= 0) errorflag = 1;
        if(fdata_raw.PulseAmp <= 0 && rawAdcHit.GetNSamples() > 0) errorflag = 2;

        if(signal == 0){
          fPosAdcDataRaw.emplace_back(fdata_raw);
          fPosAdcData.emplace_back(fdata);
          fPosAdcErrorFlag.emplace_back(errorflag);
        }
        else{
          fNegAdcDataRaw.emplace_back(fdata_raw);
          fNegAdcData.emplace_back(fdata);
          fNegAdcErrorFlag.emplace_back(errorflag);
        }
     }

      // Sample data
      if( rawAdcHit.GetNSamples() > 0 ){
        rawAdcHit.SetSampThreshold(fSampThreshold);
        if( fSampNSA == 0 ) fSampNSA = rawAdcHit.GetF250_NSA();
        if( fSampNSB == 0 ) fSampNSB = rawAdcHit.GetF250_NSB();
        rawAdcHit.SetF250Params(fSampNSA, fSampNSB, 4); // Set NPED = 4
        if( fSampNSAT != 2 ) rawAdcHit.SetSampNSAT(fSampNSAT);
        rawAdcHit.SetSampIntTimePedestalPeak();

        // Save waveform data
        auto& fSampWaveform = (signal == 0) ? fPosSampWaveform : fNegSampWaveform;
        fSampWaveform.push_back(padnum);
        fSampWaveform.push_back(rawAdcHit.GetNSamples());
        for(UInt_t i = 0; i < rawAdcHit.GetNSamples(); i++)
          fSampWaveform.push_back(rawAdcHit.GetSample(i));

        for(UInt_t thit = 0; thit < rawAdcHit.GetNSampPulses(); thit++) {
          FADCHitData fsampdata_raw;
          FADCHitData fsampdata;

          fsampdata_raw.paddle = padnum;
          fsampdata_raw.Ped = rawAdcHit.GetSampPedRaw();
          fsampdata_raw.PulseInt = rawAdcHit.GetSampPulseIntRaw(thit);
          fsampdata_raw.PulseAmp = rawAdcHit.GetSampPulseAmpRaw(thit);
          fsampdata_raw.PulseTime = rawAdcHit.GetSampPulseTimeRaw(thit);

          fsampdata.paddle = padnum;
          fsampdata.Ped = rawAdcHit.GetSampPed();
          fsampdata.PulseInt = rawAdcHit.GetSampPulseInt(thit);
          fsampdata.PulseAmp = rawAdcHit.GetSampPulseAmp(thit);
          fsampdata.PulseTime = rawAdcHit.GetSampPulseTime(thit);

          if(signal == 0){
            fPosAdcSampDataRaw.emplace_back(fsampdata_raw);
            fPosAdcSampData.emplace_back(fsampdata);
          }
          else{
            fNegAdcSampDataRaw.emplace_back(fsampdata_raw);
            fNegAdcSampData.emplace_back(fsampdata);
          }

          if( rawAdcHit.GetNPulses() == 0 || fUseSampWaveform == 1 ) {
            errorflag = 3;
            if(fUseSampWaveform) errorflag = 0;

            if(signal == 0) {
              fPosAdcDataRaw.emplace_back(fsampdata_raw);
              fPosAdcData.emplace_back(fsampdata);
              fPosAdcErrorFlag.emplace_back(errorflag);
            }
            else{
              fNegAdcDataRaw.emplace_back(fsampdata_raw);
              fNegAdcData.emplace_back(fsampdata);
              fNegAdcErrorFlag.emplace_back(errorflag);
            }

          }
        }// loop over samp data
      }
    }// for each side pmt
    ihit++;
  }// while loop

  return ihit;
}


//__________________________________________________________________
Int_t HYPTOFPlane::CoarseProcess( TClonesArray& tracks )
{

  return 0;
}

//__________________________________________________________________
Int_t HYPTOFPlane::FineProcess( TClonesArray& tracks )
{
  return 0;
}

ClassImp(HYPTOFPlane)
