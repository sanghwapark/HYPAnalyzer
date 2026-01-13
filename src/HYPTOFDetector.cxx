#include "HYPTOFDetector.h"

#include <iostream>

using namespace std;

//____________________________________________________________________________________
HYPTOFDetector::HYPTOFDetector( const char* name, const char* description, 
    THaApparatus* apparatus) :
    THaNonTrackingDetector(name, description, apparatus)
{
    fNPlanes = 1;
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
    fNHits = 0;

}

//____________________________________________________________________________________
THaAnalysisObject::EStatus HYPTOFDetector::Init( const TDatime & date )
{
 
    EStatus status;       
    if ((status = THaNonTrackingDetector::Init(date)))
        return fStatus = status;

    // Init sub detectors
    for(auto &plane : fPlanes) {
        status = plane->Init(date);
        if(status != kOK)
            return fStatus = status;
    }

    return fStatus = kOK;
}

//____________________________________________________________________________________
Int_t HYPTOFDetector::Decode( const THaEvData& evdata )
{        
    // cout << "HYPTOFDetector::Decode" << endl;
    // Decode for all planes
    for(Int_t i = 0; i < fNPlanes; i++)
        fPlanes[i]->Decode(evdata);

    return 0;
}

//____________________________________________________________________________________
Int_t HYPTOFDetector::ReadDatabase( const TDatime & date )
{
    // cout << "HYPTOFDetector::ReadDatabase" << endl;

    FILE* file = OpenFile( date );
    if( !file ) return kFileError;

    string plane_names;
    DBRequest request[] = {
        {"nplanes", &fNPlanes, kInt},
        {"names",   &plane_names, kString},
        {nullptr}
    };

    Int_t err = LoadDB(file, date, request);
    if(err) {
        fclose(file);
        return err;
    }

    // Define TOF planes
    for(Int_t i = 0; i < fNPlanes; i++) {
        HYPTOFPlane* new_plane = new HYPTOFPlane(Form("tof%d",i), Form("tof%d", i), i, this);
        fPlanes.push_back(new_plane);
    }
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
    for(Int_t i = 0; i < fNPlanes; i++)
        fPlanes[i]->CoarseProcess(tracks);

    return 0;
}

//____________________________________________________________________________________
Int_t HYPTOFDetector::FineProcess( TClonesArray& tracks )
{
    return 0;
}

//____________________________________________________________________________________

ClassImp(HYPTOFDetector)