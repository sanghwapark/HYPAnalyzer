#include "HYPDCPlane.h"
#include "HYPDCHit.h"
#include "THaDetMap.h"
#include "THaEvData.h"
#include "VarType.h"
#include "vfTDC.h"
#include <cstdlib>

using namespace std;

//__________________________________________________________________
HYPDCPlane::HYPDCPlane( const char* name, const char* description, 
  THaDetectorBase *parent) :
  THaSubDetector(name, description, parent),
  fNHits(0), fAxis(0)
{
  // constructor

  fHits = new TClonesArray("HYPDCHit", 100);
  fWires = new TClonesArray("HYPDCWire", 160);
}

//__________________________________________________________________
HYPDCPlane::~HYPDCPlane()
{
  RemoveVariables();

  delete fHits;
  delete fWires;
}

//__________________________________________________________________
THaAnalysisObject::EStatus HYPDCPlane::Init(const TDatime &date)
{

  EStatus status;
  if( (status = THaSubDetector::Init(date)) )
    return fStatus = status;
  
  return fStatus = kOK;

}

//__________________________________________________________________
Int_t HYPDCPlane::ReadDatabase( const TDatime& date )
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
  Int_t nelem;
  DBRequest request[] = {
    {"detmap",  &detmap,  kIntV},
    {"nwires",   &nelem,   kInt},
    {nullptr}
  };
  
  err = LoadDB(file, date, request, GetPrefix());
  fclose(file);
  if( err )
    return err;

//  if( FillDetMap(detmap, THaDetMap::kFillLogicalChannel, here) <= 0 )
  if( FillDetMap(detmap, THaDetMap::kFillModel | THaDetMap::kFillRefChan, here) <= 0 )
    return kInitError;

  // Check parent detector initialization
  if ( fDC ) {
    assert(fDC->Status() == kOK);
  }

  // For vfTDC modules
  for(UInt_t i = 0; i < (UInt_t)fDetMap->GetSize(); i++ ){
    THaDetMap::Module* d = fDetMap->GetModule(i);
    cout << "Module: " << d->crate << " " << d->slot << " " << d->GetModel() << endl;
    if( d->GetModel() == 527 )
      d->MakeTDC();
  }

  // define wires
  for(int i = 0; i < nelem; i++) {
    Double_t pos = 0.0; // FIXME: calculate wire position 
    Double_t offset = 0.0; // FIXME: tdc offset -- read from DB?
    new((*fWires)[i]) HYPDCWire(i+1, fAxis, pos, offset);
  }

  return kOK;
}

//__________________________________________________________________
Int_t HYPDCPlane::ReadGeometry(FILE* file, const TDatime& date, Bool_t required)
{
  // cout << "HYPDCPlane::ReadGeometry" << endl;

  vector<Double_t> position(3), size(3), angles(3);
  DBRequest request[] = {
    { "position", &position, kDoubleV, 0, required, 0, "detector position [m]"},
    { "size",     &size,     kDoubleV, 0, required, 0, "detector size [m]"},
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
Int_t HYPDCPlane::DefineVariables( EMode mode )
{

  RVarDef vars[] = {
    {"nhits",   "Number of raw TDC hits", "fNHits"},
    {"chan",    "channel for a given hit","v_Chan"},
    {"tdcraw",  "Raw TDC",                "v_RawHitTDC"},
    {"tdcopt",  "TDC Option",             "v_RawHitOpt"},
    {"wire",    "Wire numbers with hits", "fHits.HYPDCHit.GetWireNum()"},
    {"time_nc", "Time no ref corrected",  "fHits.HYPDCHit.GetTime()"},
    {nullptr}
  };

  return DefineVarsFromList(vars, mode);

}

//__________________________________________________________________
void HYPDCPlane::Clear( Option_t* opt )
{
  cout << "HYPDCPlane::Clear" << endl;

  THaSubDetector::Clear(opt);
  fNHits = 0;
  fHits->Clear();

  v_RawHitTDC.clear();
  v_RawHitOpt.clear();
  v_Chan.clear();

}

//__________________________________________________________________
Int_t HYPDCPlane::Decode( const THaEvData& evdata )
{
  const char* const here = "Decode";
  //  UInt_t evnum = evdata.GetEvNum();
  //  cout << "Event Number: " << evnum << endl;

  // Loop over hits, allowing multi hits for a channel
  auto hitIter = fDetMap->MakeMultiHitIterator(evdata);
  while(hitIter) {
    const auto& hitinfo = *hitIter;
    OptUInt_t data = LoadData(evdata, hitinfo);
    if( ! data ) {
      // Data could not be retrieved (probably decoder bug)
      DataLoadWarning(hitinfo, here);
      continue;
    }
    
    //cout << "SLOT CH lCH NHIT HIT DATA: " << hitinfo.slot << " "
    // << hitinfo.chan << " " << hitinfo.chan << " " << hitinfo.nhit << " " << hitinfo.hit << " " << evdata.GetData(hitinfo.crate, hitinfo.slot, hitinfo.chan, hitinfo.hit) << endl;
    UInt_t chan = hitinfo.chan;
    UInt_t tdc = evdata.GetData(hitinfo.crate, hitinfo.slot, hitinfo.chan, hitinfo.hit);
    UInt_t tdc_opt = evdata.GetOpt(hitinfo.crate, hitinfo.slot, hitinfo.chan, hitinfo.hit);

    // Temporary output containers
    v_Chan.emplace_back(chan);
    v_RawHitTDC.emplace_back(tdc);
    v_RawHitOpt.emplace_back(tdc_opt);

    HYPDCWire* wire = GetWire(hitinfo.chan);
    Double_t time = tdc * 1.0; // FIXME: calculate time
    //Double_t time = - rawtdc*fNSperChan + fPlaneTimeZero - wire->GetTOffset(); // fNSperChan > 0 for 1877
    new((*fHits)[fNHits]) HYPDCHit(wire, tdc, time);

    ++hitIter;
    fNHits++;
  }

  return fNHits;
}

ClassImp(HYPDCPlane)
