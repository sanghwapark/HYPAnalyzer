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

  delete [] fPosAdcTimeWindowMin;  fPosAdcTimeWindowMin = nullptr;
  delete [] fPosAdcTimeWindowMax;  fPosAdcTimeWindowMax = nullptr;
  delete [] fNegAdcTimeWindowMin;  fNegAdcTimeWindowMin = nullptr;
  delete [] fNegAdcTimeWindowMax;  fNegAdcTimeWindowMax = nullptr;

  for (auto ptr : fHodoHits) delete ptr;
  fHodoHits.clear();

  fPosCalib.clear();
  fNegCalib.clear();
}

//__________________________________________________________________
THaAnalysisObject::EStatus HYPTOFPlane::Init(const TDatime &date)
{
  // cout << "HYPTOFPlane::Init" << endl;

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

  DBRequest list0[] = {
    {Form("tof_%s_nr", GetName()), &fNelem, kInt},
    {nullptr}
  };
  gHcParms->LoadParmValues((DBRequest*)&list0, prefix);

  DBRequest list[] = {
    {"tof_debug_adc", &fDebugADC, kInt, 0, optional},
    {"tof_SampNSA",   &fSampNSA,  kInt, 0, optional},
    {"tof_SampNSB",   &fSampNSAT, kInt, 0, optional},
    {"tof_SampNSAT",  &fSampNSB,  kInt, 0, optional},
    {"tof_SampThreshold", &fSampThreshold, kDouble, 0, optional},
    {"tof_outputSampWaveform", &fOutputSampWaveform, kInt, 0, optional},
    {"tof_UseSampWaveform", &fUseSampWaveform, kInt, 0, optional},
    {Form("tof_PosAdcTimeWindowMin_%s",GetName()), fPosAdcTimeWindowMin, kDouble, (UInt_t) fNelem, optional},
    {Form("tof_PosAdcTimeWindowMax_%s",GetName()), fPosAdcTimeWindowMax, kDouble, (UInt_t) fNelem, optional},
    {Form("tof_NegAdcTimeWindowMin_%s",GetName()), fNegAdcTimeWindowMin, kDouble, (UInt_t) fNelem, optional},
    {Form("tof_NegAdcTimeWindowMax_%s",GetName()), fNegAdcTimeWindowMax, kDouble, (UInt_t) fNelem, optional},
    {nullptr}
  };

  fDebugADC = 1;
  fUseSampWaveform = 0;
  fOutputSampWaveform = 0;
  fSampNSA = 0;
  fSampNSB = 0;
  fSampNSAT = 2;
  fSampThreshold = 5.;

  // AdcTime window cuts
  fPosAdcTimeWindowMin = new Double_t[fNelem];
  fPosAdcTimeWindowMax = new Double_t[fNelem];
  fNegAdcTimeWindowMin = new Double_t[fNelem];
  fNegAdcTimeWindowMax = new Double_t[fNelem];
  for(Int_t i = 0; i < fNelem; i++) {
    fPosAdcTimeWindowMin[i] = -1000.0;
    fPosAdcTimeWindowMax[i] = 1000.0;
    fNegAdcTimeWindowMin[i] = -1000.0;
    fNegAdcTimeWindowMax[i] = 1000.0;
  }

  gHcParms->LoadParmValues((DBRequest*)&list, prefix);

  // Get parameters from parent detector
  HYPTOFDetector* parent = (HYPTOFDetector*)GetParent();
  fTdcOffset= parent->GetTdcOffset(fPlaneNum-1);
  fAdcTdcOffset= parent->GetAdcTdcOffset(fPlaneNum-1);
  fScinTdcMin=parent->GetTdcMin();
  fScinTdcMax=parent->GetTdcMax();
  fScinTdcToTime=parent->GetTdcToTime();

  // Time correction calib parameters
  fPosCalib.resize(fNelem);
  fNegCalib.resize(fNelem);
  for(Int_t i = 0; i < fNelem; i++){
    Int_t scin_index = parent->GetScinIndex(fPlaneNum-1, i);
    Double_t c1_pos = parent->GetCorrPosC1(scin_index);
    Double_t c2_pos = parent->GetCorrPosC2(scin_index);
    Double_t c3_pos = parent->GetCorrPosC3(scin_index);
    Double_t c1_neg = parent->GetCorrNegC1(scin_index);
    Double_t c2_neg = parent->GetCorrNegC2(scin_index);
    Double_t c3_neg = parent->GetCorrNegC3(scin_index);
    fPosCalib[i].SetParams(c1_pos, c2_pos, c3_pos);
    fNegCalib[i].SetParams(c1_neg, c2_neg, c3_neg);
  }

  // Init some vars
  for(int i = 0; i < 2; i++) {
    fTdcRefTime[i] = kBig;
    fTdcRefDiffTime[i] = kBig;
    fAdcRefTime[i] = kBig;
    fAdcRefDiffTime[i] = kBig;
  }

  // Pedestal vectors
  fPosAdcPedRaw = vector<Int_t> (fNelem, 0);
  fNegAdcPedRaw = vector<Int_t> (fNelem, 0);
  fPosAdcPed.assign(fNelem, 0.0);
  fNegAdcPed.assign(fNelem, 0.0);

  // Good Data (passed ADC and TDC cuts)
  fGoodPosData.resize(fNelem);
  fGoodNegData.resize(fNelem);
  
  return kOK;
}

//__________________________________________________________________
Int_t HYPTOFPlane::DefineVariables( EMode mode )
{

  // cout << "HYPTOFPlane::DefineVariables" << endl;
  
  if( mode == kDefine && fIsSetup ) return kOK;
  fIsSetup = ( mode == kDefine );

  if(fDebugADC) {
    RVarDef vars[] = {
      {"posAdcPadNum",       "Paddle number",                     "fPosAdcDataRaw.paddle"},
      {"posAdcPedRaw",       "Positive Raw ADC pedestals",        "fPosAdcPedRaw"},
      {"posAdcPulseIntRaw",  "Positive Raw ADC pulse integrals",  "fPosAdcDataRaw.PulseInt"},
      {"posAdcPulseAmpRaw",  "Positive Raw ADC pulse amplitudes", "fPosAdcDataRaw.PulseAmp"},
      {"posAdcPulseTimeRaw", "Positive Raw ADC pulse times",      "fPosAdcDataRaw.PulseTime"},
      {"posAdcPed",          "Positive ADC pedestals",            "fPosAdcPed"},
      {"posAdcPulseInt",     "Positive ADC pulse integrals",      "fPosAdcData.PulseInt"},
      {"posAdcPulseAmp",     "Positive ADC pulse amplitudes",     "fPosAdcData.PulseAmp"},
      {"posAdcPulseTime",    "Positive ADC pulse times",          "fPosAdcData.PulseTime"},
      {"negAdcPadNum",       "Paddle number",                     "fNegAdcDataRaw.paddle"},
      {"negAdcPedRaw",       "Negative Raw ADC pedestals",        "fNegAdcPedRaw"},
      {"negAdcPulseIntRaw",  "Negative Raw ADC pulse integrals",  "fNegAdcDataRaw.PulseInt"},
      {"negAdcPulseAmpRaw",  "Negative Raw ADC pulse amplitudes", "fNegAdcDataRaw.PulseAmp"},
      {"negAdcPulseTimeRaw", "Negative Raw ADC pulse times",      "fNegAdcDataRaw.PulseTime"},
      {"negAdcPed",          "Negative ADC pedestals",            "fNegAdcPed"},
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
    {"GoodPosAdcPed",         "List of Positive ADC Pedestals that passed TDC and ADC cuts",  "fGoodPosData.ped"},
    {"GoodPosAdcMult",        "List of Positive ADC Mult that passed TDC and ADC cuts",       "fGoodPosData.mult"},
    {"GoodPosAdcHitUsed",     "List of Positive ADC Hit Used that passed TDC and ADC cuts",   "fGoodPosData.hit_used"},
    {"GoodPosAdcPulseInt",    "List of Positive ADC values that passed TDC and ADC cuts",     "fGoodPosData.adc"},
    {"GoodPosAdcPulseAmp",    "List of Positive ADC peak that passed TDC and ADC cuts",       "fGoodPosData.amp"},
    {"GoodPosAdcPulseTime",   "List of Positive ADC time that passed TDC and ADC cuts",       "fGoodPosData.adctime"},
    {"GoodPosAdcTdcTimeDiff", "List of Positive TDC - ADC time that passed TDC and ADC cuts", "fGoodPosData.adctdctdiff"},
    {"GoodNegAdcPed",         "List of Negative ADC Pedestals that passed TDC and ADC cuts",  "fGoodNegData.ped"},
    {"GoodNegAdcMult",        "List of Negative ADC Mult that passed TDC and ADC cuts",       "fGoodNegData.mult"},
    {"GoodNegAdcHitUsed",     "List of Negative ADC Hit Used that passed TDC and ADC cuts",   "fGoodNegData.hit_used"},
    {"GoodNegAdcPulseInt",    "List of Negative ADC values that passed TDC and ADC cuts",     "fGoodNegData.adc"},
    {"GoodNegAdcPulseAmp",    "List of Negative ADC peak that passed TDC and ADC cuts",       "fGoodNegData.amp"},
    {"GoodNegAdcPulseTime",   "List of Negative ADC time that passed TDC and ADC cuts",       "fGoodNegData.adctime"},
    {"GoodNegAdcTdcTimeDiff", "List of Negative TDC - ADC time that passed TDC and ADC cuts", "fGoodNegData.adctdctdiff"},
    {"GoodPosTdcTimeUncorr",  "List of Postive Uncorrected Time that passed TDC and ADC cuts",  "fGoodPosData.time_uncorr"},
    {"GoodNegTdcTimeUncorr",  "List of Negative Uncorrected Time that passed TDC and ADC cuts", "fGoodNegData.time_uncorr"},
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

  fPosAdcPedRaw.assign(fNelem, 0);
  fNegAdcPedRaw.assign(fNelem, 0);
  fPosAdcPed.assign(fNelem, 0.0);
  fNegAdcPed.assign(fNelem, 0.0);

  // Good data that passed ADC, TDC cuts
  fGoodPosData.assign(fNelem, TOFEvent{});
  fGoodNegData.assign(fNelem, TOFEvent{});

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

  // Loop thorugh all rawhits starting at index of nexthit
  // Assumes the hit list is sorted by plane
  // Save ADC and TDC hit information

  // Time corrections:
  // For hits fScinTdcMin < TDC < fScinTdcMax
  // Creates new fHodoHits list
  // Calculates pulse height correction 
  // Correct times for time traveled in paddle
  // Correct times for time of flight using beta from central spectrometer momentum and particle type
  // Set corrected time (THcHodoHit::SetCorrectedTime)

  Clear(); // probably duplicated call

  Int_t nrawhits = rawhits->GetLast()+1;
  Int_t ihit = nexthit;

  while( ihit < nrawhits )
  {
    THcRawHodoHit* hit = (THcRawHodoHit*) rawhits->At(ihit);
    if(hit->fPlane != fPlaneNum) break;

    Int_t padnum = hit->fCounter;

    // Init variables
    Bool_t found_good_tdc[2] = {kFALSE, kFALSE};
    Double_t good_tdc[2] = {-999, -999};
    Double_t good_adcint[2] = {-kBig, -kBig};
    Double_t good_adcamp[2] = {-kBig, -kBig};
    Double_t good_adcped[2] = {-kBig, -kBig};
    Double_t good_adctime[2] = {-kBig, -kBig};
    Double_t good_adctdctdiff[2] = {-kBig, -kBig};
    Int_t good_adcmult[2] = {0, 0};
    Bool_t found_good_adc[2] = {kFALSE, kFALSE};

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
      for(UInt_t thit = 0; thit < rawTdcHit.GetNHits(); thit++){
        Int_t good_tdc_hit_flag = 0;
        Double_t this_tdc = rawTdcHit.GetTime(thit) + fTdcOffset;
        //cout << "Plane, PMT, TDC: " << fPlaneNum << " " << padnum << " " << rawTdcHit.GetTimeRaw(thit) << endl;
        if( this_tdc >= fScinTdcMin && this_tdc <= fScinTdcMax ) {
          good_tdc_hit_flag = 1;
          // Save first good tdc hit information
          if(!found_good_tdc[signal]) {
            found_good_tdc[signal] = kTRUE;
            good_tdc[signal] = this_tdc;
          }
        }
        TDCData t_data;
        t_data.paddle = padnum;
        t_data.TimeRaw = rawTdcHit.GetTimeRaw(thit);
        t_data.Time = this_tdc; // reference subtracted time
        t_data.Is_good_hit = good_tdc_hit_flag;

        if(signal == 0) fPosTdcData.emplace_back(t_data);
        if(signal == 1) fNegTdcData.emplace_back(t_data);
      }

      // ADC
      THcRawAdcHit& rawAdcHit = (signal == 0) ? hit->GetRawAdcHitPos() : hit->GetRawAdcHitNeg();
      // Ref time
      if( (rawAdcHit.GetNPulses() > 0 || rawAdcHit.GetNSamples() > 0) && rawAdcHit.HasRefTime() ) {
        if( fAdcRefTime[signal] == kBig ) {
          fAdcRefTime[signal] = rawAdcHit.GetRefTime();
          fAdcRefDiffTime[signal] = rawAdcHit.GetRefDiffTime();
        }
      }

      Double_t temp_max_amp = -1000.;
      Double_t temp_min_tdiff = 1000.; // FIXME: hard-coded initial cut
      Int_t good_ipulse = -1;
      Int_t good_ipulse_2 = -1;

      Int_t errorflag = -1;
      for(UInt_t thit = 0; thit < rawAdcHit.GetNPulses(); thit++) {
        //cout << "Plane, PMT, ADC: " << fPlaneNum << " " << padnum << " " << rawAdcHit.GetPulseAmpRaw(thit) << endl;
        FADCHitData fdata_raw;
        FADCHitData fdata;

        fdata_raw.paddle = padnum;
        fdata_raw.Ped = rawAdcHit.GetPedRaw();
        fdata_raw.PulseInt = rawAdcHit.GetPulseIntRaw(thit);
        fdata_raw.PulseAmp = rawAdcHit.GetPulseAmpRaw(thit);
        fdata_raw.PulseTime = rawAdcHit.GetPulseTimeRaw(thit);

        fdata.paddle = padnum;
        fdata.Ped = rawAdcHit.GetPed();
        fdata.PulseInt = rawAdcHit.GetPulseInt(thit);
        fdata.PulseAmp = rawAdcHit.GetPulseAmp(thit);
        fdata.PulseTime = rawAdcHit.GetPulseTime(thit) + fAdcTdcOffset;

        // Calculate TdcAdcTimeDiff
        // There is a good tdc hit, use the first good hit
        if(found_good_tdc[signal]) {
          Double_t TdcAdcTimeDiff = good_tdc[signal] * fScinTdcToTime - fdata.PulseTime;

          Double_t AdcTimeWindowMin = (signal == 0) ? fPosAdcTimeWindowMin[padnum-1] : fNegAdcTimeWindowMin[padnum-1];
          Double_t AdcTimeWindowMax = (signal == 0) ? fPosAdcTimeWindowMax[padnum-1] : fNegAdcTimeWindowMax[padnum-1];
          bool pulseTimeCut = ( TdcAdcTimeDiff > AdcTimeWindowMin ) && ( TdcAdcTimeDiff < AdcTimeWindowMax );
          // idenfiy the pulse with max amp
          if( pulseTimeCut && fdata.PulseAmp > temp_max_amp ) {
            temp_max_amp = fdata.PulseAmp;
            good_ipulse = thit;
          }
          // identify the pulse with min TdcAdcTimeDiff
          if( abs(TdcAdcTimeDiff) < temp_min_tdiff ) {
            temp_min_tdiff = abs(TdcAdcTimeDiff);
            good_ipulse_2 = thit;
          }          
        }

        if(fdata_raw.PulseAmp > 0)  errorflag = 0;
        if(fdata_raw.PulseAmp <= 0) errorflag = 1;
        if(fdata_raw.PulseAmp <= 0 && rawAdcHit.GetNSamples() > 0) errorflag = 2;

        if(signal == 0){
          fPosAdcDataRaw.emplace_back(fdata_raw);
          fPosAdcData.emplace_back(fdata);
          fPosAdcErrorFlag.emplace_back(errorflag);
          fPosAdcPedRaw[padnum-1] = fdata_raw.Ped;
          fPosAdcPed[padnum-1] = fdata.Ped;
        }
        else{
          fNegAdcDataRaw.emplace_back(fdata_raw);
          fNegAdcData.emplace_back(fdata);
          fNegAdcErrorFlag.emplace_back(errorflag);
          fNegAdcPedRaw[padnum-1] = fdata_raw.Ped;
          fNegAdcPed[padnum-1] = fdata.Ped;
        }
      }// Pulse data loop

      // Sample data
      if( rawAdcHit.GetNSamples() > 0 ){
        rawAdcHit.SetSampThreshold(fSampThreshold);
        if( fSampNSA == 0 ) fSampNSA = rawAdcHit.GetF250_NSA();
        if( fSampNSB == 0 ) fSampNSB = rawAdcHit.GetF250_NSB();
        rawAdcHit.SetF250Params(fSampNSA, fSampNSB, 4); // Set NPED = 4
        if( fSampNSAT != 2 ) rawAdcHit.SetSampNSAT(fSampNSAT);
        rawAdcHit.SetSampIntTimePedestalPeak();

        // Save waveform data
        if(fOutputSampWaveform){
          auto& fSampWaveform = (signal == 0) ? fPosSampWaveform : fNegSampWaveform;
          fSampWaveform.push_back(padnum);
          fSampWaveform.push_back(rawAdcHit.GetNSamples());
          for(UInt_t i = 0; i < rawAdcHit.GetNSamples(); i++)
            fSampWaveform.push_back(rawAdcHit.GetSample(i));
        }

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
          fsampdata.PulseTime = rawAdcHit.GetSampPulseTime(thit) + fAdcTdcOffset;

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
              fPosAdcPedRaw[padnum-1] = fsampdata_raw.Ped;
              fPosAdcPed[padnum-1] = fsampdata.Ped;
            }
            else{
              fNegAdcDataRaw.emplace_back(fsampdata_raw);
              fNegAdcData.emplace_back(fsampdata);
              fNegAdcErrorFlag.emplace_back(errorflag);
              fNegAdcPedRaw[padnum-1] = fsampdata_raw.Ped;
              fNegAdcPed[padnum-1] = fsampdata.Ped;
            }

          }
        }// loop over samp data
      }// Sample data

      // Selection of the best pulse: 1) max amplitude + pass pulseTimeCut 2) with min(tdcadctimediff) 3) first pulse
      // Use the pulse with minimum time diff as a good pulse
      if( good_ipulse == -1 && good_ipulse_2 != -1 ) good_ipulse = good_ipulse_2;
      // No pulse that passes the timediff cut, min(timediff) < 1000.. then use the first pulse as a good pulse
      if( good_ipulse == -1 && good_ipulse_2 == -1 && rawAdcHit.GetNPulses() > 0 ) good_ipulse = 0;
      if( good_ipulse != -1 && good_ipulse < (Int_t)rawAdcHit.GetNPulses() ) {
        found_good_adc[signal] = kTRUE;
      }

      // Save good adc information
      if( found_good_tdc[signal] && found_good_adc[signal] ) {
        good_adcint[signal] = rawAdcHit.GetPulseInt(good_ipulse);
        good_adcamp[signal] = rawAdcHit.GetPulseAmp(good_ipulse);
        if(rawAdcHit.GetPulseAmpRaw(good_ipulse) <= 0) good_adcamp[signal] = 200.;        
        good_adcped[signal] = rawAdcHit.GetPed();
        good_adcmult[signal] = rawAdcHit.GetNPulses();
        good_adctime[signal] = rawAdcHit.GetPulseTime(good_ipulse) + fAdcTdcOffset; 
        good_adctdctdiff[signal] = good_tdc[signal] * fScinTdcToTime - good_adctime[signal];

        if(signal == 0) { 
          fGoodPosData[padnum-1].SetPaddle(padnum);
          fGoodPosData[padnum-1].SetADC(good_adcped[signal], good_adcint[signal], good_adcamp[signal],
            good_adctime[signal], good_adctdctdiff[signal], good_adcmult[signal], good_ipulse);
          // set corrected time = uncorr time for now  
          fGoodPosData[padnum-1].SetTDC(good_tdc[signal], good_tdc[signal]);
          DoTimeCorrection(0, padnum-1);
        }
        else{
          fGoodNegData[padnum-1].SetPaddle(padnum);
          fGoodNegData[padnum-1].SetADC(good_adcped[signal], good_adcint[signal], good_adcamp[signal],
            good_adctime[signal], good_adctdctdiff[signal], good_adcmult[signal], good_ipulse);
          fGoodNegData[padnum-1].SetTDC(good_tdc[signal], good_tdc[signal]);
          DoTimeCorrection(1, padnum-1);
        }
      }
    }// for each side pmt

    // if both end have good hits
    /*
    if((found_good_tdc[0] && found_good_tdc[1]) && (found_good_tdc[1] && found_good_adc[1])){
      Double_t TWCorrDiff = fGoodPosData[padnum-1].time_corr - fGoodNegData[padnum-1].time_corr;
    }
    */

    // Add to the good THcHodoHit list
    // FIXME: In hcana, THcHodoHit is defined as
    // THcHodoHit(Int_t postdc, Int_t negtdc, Double_t posadc, Double_t negadc,Int_t ipad, THcScintillatorPlane* sp)
    // THcScintillatorPlane sp is not really used. Just remove it or set to nullptr as a default?
    THcHodoHit* hodohit = new THcHodoHit(good_tdc[0], good_tdc[1], good_adcint[0], good_adcint[1], hit->fCounter, nullptr);
    hodohit->SetPosADCpeak(good_adcamp[0]);
    hodohit->SetNegADCpeak(good_adcamp[1]);
    hodohit->SetPosADCtime(good_adctime[0]);
    hodohit->SetNegADCtime(good_adctime[1]);
    // FIXME: Also need to set corrected times
    fHodoHits.push_back(hodohit);
    
    ihit++;
  }// while loop

  return ihit;
}

//__________________________________________________________________
void HYPTOFPlane::DoTimeCorrection(Int_t signal, Int_t pad_index)
{
  // pulse hieght corrections from Toshi's prelim analysis
  // assume three parameter fits
  // To-dos: Add other corrections, propagation time, time-of-flight, ..

  if(signal == 0) {
    Double_t t_uncorr = fGoodPosData[pad_index].time_uncorr;
    Double_t pulse_amp = fGoodPosData[pad_index].amp;

    // calib parameters
    Double_t c1 = fPosCalib[pad_index].GetPar(0);
    Double_t c2 = fPosCalib[pad_index].GetPar(1);
    Double_t c3 = fPosCalib[pad_index].GetPar(2);

    Double_t t_corr = t_uncorr - (c1 + c2/sqrt(pulse_amp) + c3/pulse_amp);
    fGoodPosData[pad_index].SetTimeCorr(t_corr);

  } else{
    Double_t t_uncorr = fGoodNegData[pad_index].time_uncorr;
    Double_t pulse_amp = fGoodNegData[pad_index].amp;

    // calib parameters
    Double_t c1 = fNegCalib[pad_index].GetPar(0);
    Double_t c2 = fNegCalib[pad_index].GetPar(1);
    Double_t c3 = fNegCalib[pad_index].GetPar(2);

    Double_t t_corr = t_uncorr - (c1 + c2/sqrt(pulse_amp) + c3/pulse_amp);
    fGoodNegData[pad_index].SetTimeCorr(t_corr);    
  }

  return;
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
