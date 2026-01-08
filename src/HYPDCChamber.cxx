#include "HYPDCChamber.h"
#include "THaApparatus.h"
#include <sstream>
#include <string>

//____________________________________________________
HYPDCChamber::HYPDCChamber( const char* name, const char* description, 
  THaDetectorBase* parent) :
  THaSubDetector(name, description, parent),
  fNPlanes(0), fNHits(0)
{
  // constructor
}

//____________________________________________________
HYPDCChamber::~HYPDCChamber()
{
  // destructor
  for(auto plane : fPlanes )
    delete plane;
  fPlanes.clear();

}

//____________________________________________________
void HYPDCChamber::MakePrefix()
{
  TString prefix =  GetApparatus()->GetName()[0];
  prefix.Append(".");
  prefix += GetName();
  prefix.Append(".");

  delete [] fPrefix;
  fPrefix = new char[ prefix.Length()+1 ];
  strcpy( fPrefix, prefix.Data() );
}

//____________________________________________________
THaAnalysisObject::EStatus HYPDCChamber::Init(const TDatime &date)
{

  // cout << "HYPDCChamber::Init()" << endl;

  EStatus status;
  // calls ReadDatabaseas and DefineVariables
  if( (status = THaSubDetector::Init(date)) )
    return fStatus = status;

  // Init planes
  FILE* file = OpenFile( date );
  if( !file ) return kFileError;

  string plane_name_str;
    DBRequest request[] = {
        {"nplanes", &fNPlanes, kInt},
        {"plane_names", &plane_name_str, kString},
        {nullptr}
    };

  Int_t err = LoadDB(file, date, request, GetPrefix());
  if(err) {
    fclose(file);
    return kInitError;
  }

  // Get the plane names: 1x1, 1x2, 1u1, ...
  vector<string> plane_names;
  // remove ""
  plane_name_str.erase(0,1);
  plane_name_str.pop_back();
  stringstream ss(plane_name_str);
  string temp_name;
  while(ss >> temp_name)
    plane_names.emplace_back(temp_name);

  // Define planes
  for(Int_t i = 0; i < fNPlanes; i++) {
    // cout << "Plane Names: " << plane_names[i] << endl;
    fPlanes.emplace_back(new HYPDCPlane(Form("%s", plane_names[i].c_str()), Form("DC Plane %s", plane_names[i].c_str()), this));
  }

  for(auto &plane : fPlanes ){
    status = plane->Init(date);
    if(status != kOK)
      return fStatus = status;
  }

  return fStatus = kOK;
}

//____________________________________________________
void HYPDCChamber::Clear( Option_t* opt )
{
  fHits.clear();
}

//____________________________________________________
Int_t HYPDCChamber::Decode( const THaEvData& evdata )
{
  for(Int_t i = 0; i < fNPlanes; i++){
    fPlanes[i]->Decode(evdata);
  }

  return 0;
}

//____________________________________________________
Int_t HYPDCChamber::DefineVariables( EMode mode )
{
  // mainly for space point output variables 
  return 0;
}
//____________________________________________________
Int_t HYPDCChamber::CoarseTrack( TClonesArray& tracks )
{

  FindSpacePoints(); 

  return 0;
}

//____________________________________________________
Int_t HYPDCChamber::FineTrack( TClonesArray& tracks )
{
  return 0;
}


//____________________________________________________
void HYPDCChamber::ProcessHits()
{
  for(Int_t i = 0; i < fNPlanes; i++) {
    TClonesArray* hitarray = fPlanes[i]->GetHits();
    for(Int_t ihit = 0; ihit < fPlanes[i]->GetNHits(); ihit++) {
      fHits.push_back(static_cast<HYPDCHit*>(hitarray->At(ihit)));
      fNHits++;
    }
  }

}

//____________________________________________________
Int_t HYPDCChamber::FindSpacePoints()
{

  return 0;

}

ClassImp(HYPDCChamber)