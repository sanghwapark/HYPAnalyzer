#include "HYPTOFDetector.h"
#include "HYPTOFPlane.h"
#include "THaApparatus.h"
#include "VarDef.h"
#include "VarType.h"
#include "THcParmList.h"
#include "THcGlobals.h"
#include "THcDetectorMap.h"

#include <iostream>

using namespace std;

//____________________________________________________________________________________
HYPTOFDetector::HYPTOFDetector( const char* name, const char* description, 
    THaApparatus* apparatus) :
    THaNonTrackingDetector(name, description, apparatus)
{
    fNPlanes = 0;
}

//____________________________________________________________________________________
HYPTOFDetector::~HYPTOFDetector()
{
    RemoveVariables();
    for(auto plane : fPlanes )
        delete plane;
    fPlanes.clear();
}

//____________________________________________________________________________________
void HYPTOFDetector::Clear( Option_t* opt )
{
    fNhits = 0;

}

//____________________________________________________________________________________
THaAnalysisObject::EStatus HYPTOFDetector::Init( const TDatime & date )
{
    cout << "HYPTOFDetector::Init" << endl;
    // Init subdetectors
    char prefix[2];
    prefix[0] = tolower(GetApparatus()->GetName()[0]);
    prefix[1] = '\0';
    Bool_t optional = true;

    string planenamelist;
    DBRequest list[] = {
        {"tof_num_planes",  &fNPlanes, kInt},
        {"tof_plane_names", &planenamelist, kString},
        {"tof_tdcrefcut",   &fTDC_RefTimeCut, kInt, 0, optional},
        {"tof_adcrefcut",   &fADC_RefTimeCut, kInt, 0, optional},
        {nullptr}
    };

    fTDC_RefTimeCut = 0;
    fADC_RefTimeCut = 0;

    gHcParms->LoadParmValues((DBRequest*)&list, prefix);

    vector<string> plane_names = Podd::vsplit(planenamelist);
    if(plane_names.size() != (UInt_t) fNPlanes) {
        cout << "ERROR: Number of planes " << fNPlanes << " doesn't agree with number of plane names " << plane_names.size() << endl;
        return kInitError;
    }

    for(Int_t i = 0; i < fNPlanes; i++) {
        HYPTOFPlane* newplane = new HYPTOFPlane(plane_names[i].c_str(), Form("TOF Plane %s", plane_names[i].c_str()), i+1, this);
        fPlanes.push_back(newplane);
    }

    // In detector map, the detector ID should be defined 
    // e.g. KDC is for HKS DC
    string EngineDID = string(GetApparatus()->GetName()).substr(0, 1) + GetName();
    std::transform(EngineDID.begin(), EngineDID.end(), EngineDID.begin(), ::toupper);
    if(gHcDetectorMap->FillMap(fDetMap, EngineDID.c_str()) < 0) {
        static const char* const here = "Init()";
        Error(Here(here), "Error filling detectormap for %s.", EngineDID.c_str());
        return kInitError;
    }

    InitHitList(fDetMap, "THcRawHodoHit", fDetMap->GetTotNumChan()+1,
        fTDC_RefTimeCut, fADC_RefTimeCut);

    EStatus status;       
    if ((status = THaNonTrackingDetector::Init(date)))
        return fStatus = status;

    for(Int_t ip=0;ip<fNPlanes;ip++) {
        if((status = fPlanes[ip]->Init( date ))) {
            return fStatus=status;
        }
    }

    fPresentP = 0;
    THaVar* vpresent = gHaVars->Find(Form("%s.present",GetApparatus()->GetName()));
    if(vpresent) {
        fPresentP = (Bool_t *) vpresent->GetValuePointer();
    }

    return fStatus = kOK;
}

//____________________________________________________________________________________
Int_t HYPTOFDetector::Decode( const THaEvData& evdata )
{        
    //cout << "HYPTOFDetector::Decode" << endl;
    Bool_t present = kTRUE;  // suppress reference time warnings
    if(fPresentP) {          // if this spectrometer not part of trigger
        present = *fPresentP;
    }
    
    fNhits = DecodeToHitList(evdata, !present);

    Int_t nexthit = 0;
    for(Int_t ip = 0; ip < fNPlanes; ip++) {
        nexthit = fPlanes[ip]->ProcessHits(fRawHitList, nexthit);
    }

    return fNhits;
}

//____________________________________________________________________________________
Int_t HYPTOFDetector::ReadDatabase( const TDatime & date )
{
    // cout << "HYPTOFDetector::ReadDatabase" << endl;
    return 0;
}
//____________________________________________________________________________________
Int_t HYPTOFDetector::DefineVariables( EMode mode)
{
    return 0;
}

//____________________________________________________________________________________
Int_t HYPTOFDetector::CoarseProcess( TClonesArray& tracks )
{

    return 0;
}

//____________________________________________________________________________________
Int_t HYPTOFDetector::FineProcess( TClonesArray& tracks )
{
    return 0;
}

//____________________________________________________________________________________

ClassImp(HYPTOFDetector)