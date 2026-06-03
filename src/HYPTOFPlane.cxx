#include "HYPTOFPlane.h"
#include "HYPTOFDetector.h"
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
  fPlaneNum = iplane;
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
  cout << "HYPTOFPlane::Init" << endl;

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

  fDebugADC = 1;
  fUseSampWaveform = 0;
  fOutputSampWaveform = 0;
  fSampNSA = 0;
  fSampNSB = 0;
  fSampNSAT = 2;
  fSampThreshold = 5.;

  gHcParms->LoadParmValues((DBRequest*)&list, prefix);

  // Get parameters from parent detector
  /*
  HYPTOFDetector* parent = (HYPTOFDetector*)GetParent();
  fTdcOffset= parent->GetTdcOffset(fPlaneNum-1);
  fAdcTdcOffset= parent->GetAdcTdcOffset(fPlaneNum-1);
  fScinTdcMin=parent->GetTdcMin();
  fScinTdcMax=parent->GetTdcMax();
  fScinTdcToTime=parent->GetTdcToTime();
  */
  fTdcOffset= 0;
  fAdcTdcOffset= 0;
  fScinTdcMin = 0;
  fScinTdcMax= 2000;
  fScinTdcToTime= 0.1;

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

  cout << "HYPTOFPlane::DefineVariables" << endl;
  
//  if( mode == kDefine && fIsSetup ) return kOK;
//  fIsSetup = ( mode == kDefine );

  if(fDebugADC) {
    RVarDef vars[] = {
      {"posAdcPadNum",       "Paddle number",                     "fPosAdcDataRaw.paddle"},
      {"posAdcPedRaw",       "Positive Raw ADC pedestals",        "fPosAdcDataRaw.Ped"},
      {"posAdcPulseIntRaw",  "Positive Raw ADC pulse integrals",  "fPosAdcDataRaw.PulseInt"},
      {"posAdcPulseAmpRaw",  "Positive Raw ADC pulse amplitudes", "fPosAdcDataRaw.PulseAmp"},
      {"posAdcPulseTimeRaw", "Positive Raw ADC pulse times",      "fPosAdcDataRaw.PulseTime"},
      {"posAdcPed",          "Positive ADC pedestals",            "fPosAdcData.Ped"},
      {"posAdcPulseInt",     "Positive ADC pulse integrals",      "fPosAdcData.PulseInt"},
      {"posAdcPulseAmp",     "Positive ADC pulse amplitudes",     "fPosAdcData.PulseAmp"},
      {"posAdcPulseTime",    "Positive ADC pulse times",          "fPosAdcData.PulseTime"},
      {"negAdcPadNum",       "Paddle number",                     "fNegAdcDataRaw.paddle"},
      {"negAdcPedRaw",       "Negative Raw ADC pedestals",        "fNegAdcDataRaw.Ped"},
      {"negAdcPulseIntRaw",  "Negative Raw ADC pulse integrals",  "fNegAdcDataRaw.PulseInt"},
      {"negAdcPulseAmpRaw",  "Negative Raw ADC pulse amplitudes", "fNegAdcDataRaw.PulseAmp"},
      {"negAdcPulseTimeRaw", "Negative Raw ADC pulse times",      "fNegAdcDataRaw.PulseTime"},
      {"negAdcPed",          "Negative ADC pedestals",            "fNegAdcData.Ped"},
      {"negAdcPulseInt",     "Negative ADC pulse integrals",      "fNegAdcData.PulseInt"},
      {"negAdcPulseAmp",     "Negative ADC pulse amplitudes",     "fNegAdcData.PulseAmp"},
      {"negAdcPulseTime",    "Negative ADC pulse times",          "fNegAdcData.PulseTime"},
      {"posErrorFlag",       "Error Flag for When FPGA Fails",    "fPosAdcErrorFlag"},
      {"negErrorFlag",       "Error Flag for When FPGA Fails",    "fNegAdcErrorFlag"},
      {"posTdcPadNum",       "Paddle number",                     "fPosTdcData.paddle"},            
      {"posTdcTimeRaw",      "Positive Raw TDC Time",             "fPosTdcData.TimeRaw"},
      {"posTdcTime",         "Positive Ref time subtracted TDC",  "fPosTdcData.Time"},
      {"posTdcGoodHitFlag",  "Good hit flag for Pos TDC",         "fPosTdcData.Is_good_hit"},
      {"negTdcPadNum",       "Paddle number",                     "fNegTdcData.paddle"},
      {"negTdcTimeRaw",      "Negative Raw TDC Time",             "fNegTdcData.TimeRaw"},
      {"negTdcTime",         "Negative Ref time subtracted TDC",  "fNegTdcData.Time"},
      {"negTdcGoodHitFlag",  "Good hit flag for Neg TDC",         "fNegTdcData.Is_good_hit"},
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

  Clear();

  Int_t nrawhits = rawhits->GetLast()+1;
  Int_t ihit = nexthit;

  // cout << "HYPTOFPlane::ProcessHits " << nrawhits << endl;

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
          //cout << "RefTime: " << rawTdcHit.GetRefTime() << endl;
        }
      }
      // TDC
      Int_t iFirstGoodHit[2] = {-1, -1};
      Double_t good_tdc[2] = {-999, -999};
      for(UInt_t thit = 0; thit < rawTdcHit.GetNHits(); thit++){
        Int_t good_tdc_hit_flag = 0;
        Double_t this_tdc = rawTdcHit.GetTime(thit) + fTdcOffset;
        // cout << "Plane, PMT, TDC: " << fPlaneNum << " " << padnum << " " << rawTdcHit.GetTimeRaw(thit) << endl;
        if( this_tdc >= fScinTdcMin && this_tdc < fScinTdcMax ) {
          good_tdc_hit_flag = 1;
          if(iFirstGoodHit[signal] == -1) {
            iFirstGoodHit[signal] = thit;
            good_tdc[signal] = this_tdc;
          }
        }
        TDCData t_data;
        t_data.paddle = padnum;
        t_data.TimeRaw = rawTdcHit.GetTimeRaw(thit);
        t_data.Time = rawTdcHit.GetTime(thit);
        t_data.Is_good_hit = good_tdc_hit_flag;

        if(signal == 0) fPosTdcData.emplace_back(t_data);
        if(signal == 1) fNegTdcData.emplace_back(t_data);
        /*
        if(signal == 0) {
          fPosTdcData.emplace_back(padnum, rawTdcHit.GetTimeRaw(thit), rawTdcHit.GetTime(thit), good_tdc_hit_flag );
        }
        else {
          fNegTdcData.emplace_back(padnum, rawTdcHit.GetTimeRaw(thit), rawTdcHit.GetTime(thit), good_tdc_hit_flag );
        } 
        */       
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
      }// Sample data

      /*
      Int_t iMaxAmpIndex = -1;
      Int_t iMinAdcTdcDiff = -1;
      Double_t max_amp_temp = -1000;
      Double_t min_adctdcdiff = 1000;

      auto& fAdcData = (signal == 0) ? fPosAdcData : fNegAdcData;
      for( auto& f_data : fAdcData ) {
        Data_t this_amp = f_data.PulseAmp;
        Data_t this_time = f_data.PulseTime + fAdcTdcOffset;
        if( this_amp > max_amp_temp ) {
          iMaxAmpIndex = 
        }

      }

	Double_t pulseTime    = rawNegAdcHit.GetPulseTime(ielem)+fAdcTdcOffset;
        Double_t TdcAdcTimeDiff = tdc_neg*fScinTdcToTime-pulseTime;
        if (rawNegAdcHit.GetPulseAmpRaw(ielem) <= 0)pulseAmp= 200.;
	Bool_t   pulseTimeCut =( TdcAdcTimeDiff > fHodoNegAdcTimeWindowMin[index]) &&  (TdcAdcTimeDiff < fHodoNegAdcTimeWindowMax[index]);
	if (pulseTimeCut &&  pulseAmp>max_adcamp_test) {
	  good_ielem_negadc = ielem;
	  max_adcamp_test=pulseAmp;
	}
	if (abs(TdcAdcTimeDiff) < max_adctdcdiff_test) {
	  good_ielem_negadc_test2 = ielem;
	  max_adctdcdiff_test=abs(TdcAdcTimeDiff);
	}

*/

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
