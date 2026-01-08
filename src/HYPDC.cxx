#include "HYPDC.h"

//________________________________________________________________
HYPDC::HYPDC(const char* name, const char* description,
       THaApparatus* apparatus) :
  THaTrackingDetector(name, description, apparatus), 
  fNChambers(1), fNTracks(0)
{

}

//________________________________________________________________
HYPDC::~HYPDC()
{
  // Destructor
  RemoveVariables();
  for(auto chamber : fChambers )
    delete chamber;
  fChambers.clear();

}

//________________________________________________________________
THaAnalysisObject::EStatus HYPDC::Init( const TDatime& date )
{

  // std::cout << "HYPDC::Init" << std::endl;

  EStatus status;
  if((status = THaTrackingDetector::Init( date )))
    return fStatus = status;

  // Define and Init Chambers
  for(Int_t i = 0; i < fNChambers; i++) {
    fChambers.emplace_back(new HYPDCChamber(Form("dc%d",i+1), Form("DC Chamber %d", i+1), this));
  }
  for(auto &chamber : fChambers ) {
    status = chamber->Init(date);
    if(status != kOK) return fStatus = status;
  }

  return fStatus = kOK;
}
//________________________________________________________________
void HYPDC::Clear( Option_t* opt )
{
  fNTracks = 0;
}

//________________________________________________________________
Int_t HYPDC::ReadDatabase( const TDatime& date )
{
  // Read parameters 
  // Called by ThaDetectorBase::Init()   

  FILE* file = OpenFile( date );
  if( !file ) return kFileError;

  DBRequest request[] = {
    {"nchambers", &fNChambers, kInt},
    {nullptr}
  };
  Int_t err = LoadDB(file, date, request,GetPrefix());
  if(err) {
    fclose(file);
    return kInitError;
  }

  return kOK;
}

//________________________________________________________________
Int_t HYPDC::Decode( const THaEvData& evdata )
{
   for(Int_t i = 0; i < fNChambers; i++){
    fChambers[i]->Decode(evdata);
  }

  return 0;
}

//________________________________________________________________
Int_t HYPDC::DefineVariables( EMode mode )
{

  return 0;
}

//________________________________________________________________
Int_t HYPDC::CoarseTrack( TClonesArray& tracks )
{

  return 0;
}

//________________________________________________________________
Int_t HYPDC::FineTrack( TClonesArray& tracks )
{
  return 0;
}

//________________________________________________________________

ClassImp(HYPDC)
