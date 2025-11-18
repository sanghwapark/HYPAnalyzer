#include "HYPTOFPlane.h"
#include "THaDetMap.h"
#include "THaEvData.h"
#include "VarType.h"
#include "vfTDC.h"
#include <cstdlib>

using namespace std;

//__________________________________________________________________
HYPTOFPlane::HYPTOFPlane( const char* name, const char* description, 
			  const Int_t iplane, THaDetectorBase *parent) :
  THaSubDetector(name, description, parent),
  fNHits(0), fFADCData(nullptr)
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

  //  if( FillDetMap(detmap, THaDetMap::kFillLogicalChannel, here) <= 0 )
  if( FillDetMap(detmap, THaDetMap::kFillModel | THaDetMap::kFillRefChan, here) <= 0 )
    return kInitError;

  /*
  if( ! err ) {
    UInt_t tot_nchan = fDetMap->GetTotNumChan();
    if( tot_nchan != 2 * nelem ) {
      Error(Here(here), "Number of detector map channels (%u) "
	    "inconsistent with 4*number of paddles (%d)",
	    tot_nchan, 4*nelem);
      err = kInitError;
    }
  }
  */

  // Check parent detector initialization
  if ( fTOF ) {
    assert(fTOF->Status() == kOK);
  }

  // For vfTDC modules
  for(UInt_t i = 0; i < (UInt_t)fDetMap->GetSize(); i++ ){
    THaDetMap::Module* d = fDetMap->GetModule(i);
    if( d->GetModel() == 527 )
      d->MakeTDC();
  }

  // Init FADCData
  fFADCData = new FADCData();

  return kOK;
}

//__________________________________________________________________
Int_t HYPTOFPlane::ReadGeometry(FILE* file, const TDatime& date, Bool_t required)
{
  // cout << "HYPTOFPlane::ReadGeometry" << endl;

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
//__________________________________________________________________
Int_t HYPTOFPlane::DefineVariables( EMode mode )
{

  // To-dos:
  // Define TOF hit obj and better manage the output vars
  
  // Raw FADC hits
  /*
  RVarDef vars[] = {
    {0}
  };

  return DefineVarsFromList(vars, mode);
  */
  return 0;
}

//__________________________________________________________________
void HYPTOFPlane::Clear( Option_t* opt )
{
  THaSubDetector::Clear(opt);
  fNHits = 0;
}

//__________________________________________________________________
OptUInt_t HYPTOFPlane::LoadData( const THaEvData& evdata, 
				 const DigitizerHitInfo_t& hitinfo )
{
  // Use FADC module 
  if( hitinfo.modtype == Decoder::ChannelType::kMultiFunctionADC ) {
    return evdata.GetData(Decoder::kPulseIntegral, hitinfo.crate, hitinfo.slot, hitinfo.chan, hitinfo.hit);
  }

  // Generic THaDetBase::LoadData call
  return evdata.GetData(hitinfo.crate, hitinfo.slot, hitinfo.chan, hitinfo.hit);

}

//__________________________________________________________________
Int_t HYPTOFPlane::Decode( const THaEvData& evdata )
{
  // Loop over all channels (hits) in evdata that are 
  // associated with this detector's detector map
  // For each hit:
  // LoadData(evdata, hitinfo) hitinfo for each channel
  // StoreData(hitinfo,data) save data in member variables

  fFADCData->Clear();

  // FIXME: Need to add part for ref time

  for( UInt_t i = 0; i < fDetMap->GetSize(); i++ ) {
    THaDetMap::Module* d = fDetMap->GetModule(i);    
    if( d->IsADC() ) {
      // cout << "ADC: ROC: SLOT: " << d->crate << " " << d->slot << endl;
      Int_t nhits = THaDetectorBase::Decode(evdata);
      fNHits++;
    }
    else {
      // cout << "This is TDC: " << d->crate << " " << d->slot << " " << d->GetModel() << endl;
      for(UInt_t j = 0; j < evdata.GetNumChan(d->crate, d->slot); j++) {
        UInt_t chan = evdata.GetNextChan(d->crate, d->slot, j);
        if( chan < d->lo || chan > d->hi ) continue;
        Int_t lchan = d->ConvertToLogicalChannel(chan);

        UInt_t nhits = evdata.GetNumHits(d->crate, d->slot, chan);
        for(UInt_t ihit = 0; ihit < nhits;  ihit++) {
          UInt_t data = evdata.GetData(d->crate, d->slot, chan, ihit);
          
#ifdef WITH_DEBUG
	  if( fDebug > 1)
	    cout << "HYPTOFPlane::Decode " 
		 << "chan, lchan, nhit, hit, data: "
		 << chan << " " << lchan << " " << nhits << " " << ihit << " " << data << endl;
#endif
          fNHits++;
        }
      }

    }// else
  }// for modules

  return fNHits;
}

//__________________________________________________________________
Int_t HYPTOFPlane::StoreHit( const DigitizerHitInfo_t& hitinfo, UInt_t data )
{
  Int_t nevents = fFADCData->AddHit(hitinfo);

  return nevents;
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
