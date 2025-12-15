#include "HYPCherenkov.h"
#include "Fadc250Module.h"

using namespace std;

//_____________________________________________________________________
HYPCherenkov::HYPCherenkov( const char* name, const char* description,
		      THaApparatus* apparatus) :
  THaNonTrackingDetector(name, description, apparatus), 
  fNHits(0), fFADCData(nullptr)
{

}

//_____________________________________________________________________
HYPCherenkov::~HYPCherenkov()
{
  // Destructor
  RemoveVariables();

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
  fFADCData->Clear();
  fNHits = 0;
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
  DBRequest request[] = {
    {"detmap",  &detmap,  kIntV},
    {"nelem",   &nelem,   kInt},
    {nullptr}
  };
  
  err = LoadDB(file, date, request, GetPrefix());
  fclose(file);
  if( err )
    return err;

  if( FillDetMap(detmap, THaDetMap::kFillModel | THaDetMap::kFillRefChan, here) <= 0 )
    return kInitError;

  // Init FADCData
  fFADCData = new FADCData();

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

  return 0;

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
  Int_t nevents = fFADCData->AddHit(hitinfo);

  return nevents;
}

//_____________________________________________________________________
Int_t HYPCherenkov::CoarseProcess( TClonesArray& tracks )
{

  return 0;

}

//_____________________________________________________________________
Int_t HYPCherenkov::FineProcess( TClonesArray& tracks )
{

  return 0;

}

//_____________________________________________________________________

ClassImp(HYPCherenkov)