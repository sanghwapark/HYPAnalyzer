#include "HYPCherenkov.h"
#include "Fadc250Module.h"
#include "HYPRawAdcHit.h"

using namespace std;

//_____________________________________________________________________
HYPCherenkov::HYPCherenkov( const char* name, const char* description,
		      THaApparatus* apparatus) :
  THaNonTrackingDetector(name, description, apparatus), 
  fNHits(0), fNpeSum(0.0)
{
  fRawHits = new TClonesArray("HYPRawAdcHit", 20);
}

//_____________________________________________________________________
HYPCherenkov::~HYPCherenkov()
{
  // Destructor
  RemoveVariables();
  delete fRawHits;
}

//__________________________________________________________________
THaAnalysisObject::EStatus HYPCherenkov::Init(const TDatime &date)
{

  EStatus status;
  if( (status = THaNonTrackingDetector::Init(date)) )
    return fStatus = status;
  return fStatus = kOK;

}

//_____________________________________________________________________
void HYPCherenkov::Clear( Option_t* opt )
{
  fNHits = 0;
  fRawHits->Clear("C");
  fSampleData.clear();

  fNpeSum = 0.0;
}

//_____________________________________________________________________
Int_t HYPCherenkov::ReadDatabase( const TDatime& date )
{
 const char* const here = "ReadDatabase";
 
  FILE* file = OpenFile(date);
  if ( !file ) return kFileError;

  // Read Geometry 
  Int_t err = ReadGeometry(file, date, true);
  if( err ) {
    fclose(file);
    return err;
  }

  vector<Int_t> detmap;
  UInt_t nelem;
  DBRequest request_map[] = {
    {"detmap",  &detmap,  kIntV},
    {"nelem",   &nelem,   kInt},
    {nullptr}
  };

  err = LoadDB(file, date, request_map, GetPrefix());
  if( err ) {
    fclose(file);
    return err;
  }

  if( FillDetMap(detmap, THaDetMap::kFillLogicalChannel| THaDetMap::kFillRefChan, here) <= 0 )
    return kInitError;

  cout << "Now read calib parameters" << endl;
  const UInt_t nval = nelem;
  vector<Int_t> pmt_num(nval), sig_type(nval); // channel and pmt mapping, signal type for Top/Bottom PMTs
  vector<Double_t> time_min(nval), time_max(nval), gain(nval);
  DBRequest request_par[] = {  
    {"pmt_num",  &pmt_num,   kIntV, nval, false},
    {"sig_type", &sig_type,  kIntV, nval, false}, 
    {"time_window_min", &time_min, kDoubleV, nval, true}, // optional
    {"time_window_max", &time_max, kDoubleV, nval, true},
    {"gain",            &gain,     kDoubleV, nval, false},
    {nullptr}
  };

  // Default values
  for(UInt_t i = 0; i < nelem; i++){
    time_min[i] = 0.0;
    time_max[i] = 500.0;
    gain[i] = 1.0;
  }

  err = LoadDB(file, date, request_par, GetPrefix());
  fclose(file); // no longer needed
  if( err )    
    return err;
  
  for(UInt_t i = 0; i < nelem; i++) {
    fCerInfo.push_back( {pmt_num[i], sig_type[i], time_min[i], time_max[i], gain[i]} );
  }
  
  return kOK;

}

//_____________________________________________________________________
Int_t HYPCherenkov::ReadGeometry(FILE* file, const TDatime& date, Bool_t required)
{
  vector<Double_t> position(3), size(3), angles(3);
  DBRequest request[] = {
    { "position", &position, kDoubleV, 0, required, 0, "detector position [m]"},
    { "size",     &size,     kDoubleV, 0, required, 0, "detector size [m]"},
    { "angle",    &angles,   kDoubleV, 0, true,     0, "detector angle [deg]"},
    {nullptr}
  };

  // Default values
  position[0] = 0.0;
  position[1] = 0.0;
  position[2] = 0.0;
  
  fSize[0] = 1.0; 
  fSize[1] = 0.1;
  fSize[1] = 0.02;
  
  Int_t err = LoadDB( file, date, request, GetPrefix());

  if( err )
    return kInitError;

  if( ! size.empty() ) {
    fSize[0] = size[0];
    fSize[1] = size[1];
    fSize[2] = size[2];    
  }

  return kOK;
}

//_____________________________________________________________________
Int_t HYPCherenkov::DefineVariables( EMode mode )
{

  RVarDef vars[] = {
    {"chan",        "Channel number for the hit", "fRawHits.HYPRawAdcHit.GetNum()"},
    {"pulseIntRaw", "Raw ADC pulse integral",     "fRawHits.HYPRawAdcHit.GetPulseIntRaw()"},
    {"pulseAmp",    "ADC pulse amplitude",        "fRawHits.HYPRawAdcHit.GetPulseAmp()"},
    {"pulseTime",   "ADC pulse time",             "fRawHits.HYPRawAdcHit.GetPulseTime()"},
    {"pulsePed",    "ADC pulse pedestal",         "fRawHits.HYPRawAdcHit.GetPulsePed()"},
    {"npeSum",      "Npe sum",                    "fNpeSum"},
    {nullptr}
  };

  return DefineVarsFromList(vars, mode);

}

//_____________________________________________________________________
OptUInt_t HYPCherenkov::LoadData( const THaEvData& evdata, 
				 const DigitizerHitInfo_t& hitinfo )
{
  // Use FADC module 
  if( hitinfo.modtype == Decoder::ChannelType::kMultiFunctionADC ) {
    return evdata.GetData(Decoder::kPulseIntegral, hitinfo.crate, hitinfo.slot, hitinfo.chan, hitinfo.hit);
  }

  // Generic THaDetBase::LoadData call
  return evdata.GetData(hitinfo.crate, hitinfo.slot, hitinfo.chan, hitinfo.hit);

}

//_____________________________________________________________________
Int_t HYPCherenkov::Decode( const THaEvData& evdata )
{
  return THaDetectorBase::Decode(evdata); 
}

//__________________________________________________________________
Int_t HYPCherenkov::StoreHit( const DigitizerHitInfo_t& hitinfo, UInt_t data )
{

  // Int_t nevents = fFADCData->AddHit(hitinfo);

  UInt_t chan = hitinfo.lchan; // logical channel
  auto* fadc = dynamic_cast<Decoder::Fadc250Module*>(hitinfo.module);
  assert(fadc);

  // Pulse mode data
  // Assume that the # of pulses for pulsetime, peak, pedestal are same
  UInt_t nevents = fadc->GetNumEvents(Decoder::kPulseIntegral, hitinfo.chan);
  for(UInt_t i = 0; i < nevents; i++) {
    UInt_t pulseint  = fadc->GetPulseIntegralData(hitinfo.chan, i);
    UInt_t pulsepeak = fadc->GetPulsePeakData(hitinfo.chan, i);
    UInt_t pulsetime = fadc->GetPulseTimeData(hitinfo.chan, i);
    UInt_t pulseped  = fadc->GetPulsePedestalData(hitinfo.chan, i);
    UInt_t coarsetime = fadc->GetPulseCoarseTimeData(hitinfo.chan, i);
    UInt_t finetime = fadc->GetPulseFineTimeData(hitinfo.chan, i);

    auto* rawhit = new((*fRawHits)[fNHits++]) HYPRawAdcHit();
    rawhit->SetData(chan, pulseint, pulsepeak, pulsetime, pulseped, coarsetime, finetime);
  }

  // Sample data
  UInt_t nsamples = fadc->GetNumEvents(Decoder::kSampleADC, hitinfo.chan);
  if(nsamples > 0) {
    fSampleData.push_back(chan);
    fSampleData.push_back(nsamples);
    for(UInt_t isamp = 0; isamp < nsamples; isamp++) {
      fSampleData.push_back(fadc->GetData(Decoder::kSampleADC, hitinfo.chan, isamp));
    }
  }

  return 0;
}

//_____________________________________________________________________
Int_t HYPCherenkov::CoarseProcess( TClonesArray& tracks )
{
  /*
  cout << "HYPTOFPlane::CoarseProcess" << endl;
  for(size_t i = 0; i < fFADCData->GetSize(); i++) {
    FADCHit& hit = fFADCData->GetData(i);
  }
  */

  // Coarse process: 
  // - Apply time cut using start time (from tof?) to select good hits
  // - Calculate npe = gain * pulse_int per PMT
  // - Calculate npe sum 

  return 0;

}

//_____________________________________________________________________
Int_t HYPCherenkov::FineProcess( TClonesArray& tracks )
{

  return 0;

}

//_____________________________________________________________________

ClassImp(HYPCherenkov)