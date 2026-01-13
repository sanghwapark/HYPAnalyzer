#include "HYPDCChamber.h"
#include "HYPDCCluster.h"
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
  fUClusters = new TClonesArray("HYPDCCluster", 10);
  fVClusters = new TClonesArray("HYPDCCluster", 10);
  fXClusters = new TClonesArray("HYPDCCluster", 10);
  fUXClusters = new TClonesArray("HYPDCCluster", 10);
  fVXClusters = new TClonesArray("HYPDCCluster", 10);
}

//____________________________________________________
HYPDCChamber::~HYPDCChamber()
{
  // destructor
  for(auto plane : fPlanes )
    delete plane;
  fPlanes.clear();

  delete fUClusters;
  delete fVClusters;
  delete fXClusters;
  delete fUXClusters;
  delete fVXClusters;

  delete [] fXsp; fXsp = nullptr;
  delete [] fYsp; fYsp = nullptr;
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
        {"nplanes",     &fNPlanes, kInt},
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
    HYPDCPlane* plane = new HYPDCPlane(Form("%s", plane_names[i].c_str()), Form("DC Plane %s", plane_names[i].c_str()), this);
    plane->SetPlaneNum(i);
    fPlanes.emplace_back(plane);
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
  fNHits = 0;
  fHits.clear();
}

//____________________________________________________
Int_t HYPDCChamber::Decode( const THaEvData& evdata )
{
  for(Int_t i = 0; i < fNPlanes; i++){
    fPlanes[i]->Decode(evdata);

  TClonesArray* hitarray = fPlanes[i]->GetHits();
  for(Int_t ihit = 0; ihit < fPlanes[i]->GetNHits(); ihit++) {
    fHits.push_back(static_cast<HYPDCHit*>(hitarray->At(ihit)));
    fNHits++;
  }
}
  return 0;
}

//________________________________________________________________
Int_t HYPDCChamber::ReadDatabase( const TDatime& date )
{
  // Read parameters 
  // Called by ThaDetectorBase::Init()   

  FILE* file = OpenFile( date );
  if( !file ) return kFileError;

  vector<Double_t> fAlphas;
  vector<Double_t> fBetas;
  vector<Double_t> fGammas;
  
  DBRequest request[] = {
    {"min_hits", &fMinHitCut, kInt},
    {"max_hits", &fMaxHitCut, kInt},
    {"alpha",    &fAlphas,    kDoubleV},
    {"beta",     &fBetas,    kDoubleV},
    {"gamma",    &fGammas,    kDoubleV},
    {nullptr}
  };
  Int_t err = LoadDB(file, date, request,GetPrefix());
  if(err) {
    fclose(file);
    return kInitError;
  }

  fXsp = new Double_t[fNPlanes];
  fYsp = new Double_t[fNPlanes];
  // Calculate geometry parameters
  for(Int_t i = 0; i < fNPlanes; i++) {
    Double_t alpha = fAlphas[i];
    Double_t beta = fBetas[i];
    Double_t gamma = fGammas[i];
    Double_t z0 = fPlanes[i]->GetZ();

    Double_t cosalpha = TMath::Cos(alpha);
    Double_t sinalpha = TMath::Sin(alpha);
    Double_t cosbeta = TMath::Cos(beta);
    Double_t sinbeta = TMath::Sin(beta);
    Double_t cosgamma = TMath::Cos(gamma);
    Double_t singamma = TMath::Sin(gamma);

    Double_t hzchi = -cosalpha*sinbeta + sinalpha*cosbeta*singamma;
    Double_t hzpsi =  sinalpha*sinbeta + cosalpha*cosbeta*singamma;
    Double_t hxchi = -cosalpha*cosbeta - sinalpha*sinbeta*singamma;
    Double_t hxpsi =  sinalpha*cosbeta - cosalpha*sinbeta*singamma;
    Double_t hychi =  sinalpha*cosgamma;
    Double_t hypsi =  cosalpha*cosgamma;
    Double_t stubxchi = -cosalpha;
    Double_t stubxpsi = sinalpha;
    Double_t stubychi = sinalpha;
    Double_t stubypsi = cosalpha;

    Double_t sumsqupsi = hzpsi*hzpsi+hxpsi*hxpsi+hypsi*hypsi;
    Double_t sumsquchi = hzchi*hzchi+hxchi*hxchi+hychi*hychi;
    Double_t sumcross = hzpsi*hzchi + hxpsi*hxchi + hypsi*hychi;
    Double_t denom1 = sumsqupsi*sumsquchi-sumcross*sumcross;
    Double_t fPsi0 = (-z0*hzpsi*sumsquchi
          +z0*hzchi*sumcross) / denom1;
    Double_t hchi0 = (-z0*hzchi*sumsqupsi
          +z0*hzpsi*sumcross) / denom1;
    Double_t hphi0 = TMath::Sqrt(pow(z0+hzpsi*fPsi0+hzchi*hchi0,2)
              + pow(hxpsi*fPsi0+hxchi*hchi0,2)
              + pow(hypsi*fPsi0+hychi*hchi0,2) );
    if(z0 < 0.0) hphi0 = -hphi0;
    
    Double_t denom2 = stubxpsi*stubychi - stubxchi*stubypsi;
    fXsp[i] = hychi/denom2;		// sin(a)
    fYsp[i] = -hxchi/denom2;		// cos(a)

    fPlanes[i]->SetXsp(fXsp[i]);
    fPlanes[i]->SetYsp(fYsp[i]);
    }
  
  return kOK;
}
//____________________________________________________
Int_t HYPDCChamber::DefineVariables( EMode mode )
{
  return 0;
}
//____________________________________________________
Int_t HYPDCChamber::CoarseTrack( TClonesArray& tracks )
{

  return 0;
}

//____________________________________________________
Int_t HYPDCChamber::FineTrack( TClonesArray& tracks )
{
  return 0;
}

//____________________________________________________
Int_t HYPDCChamber::FindSpacePoints()
{
  // cluster arrays
  fUClusters->Clear("C");
  fVClusters->Clear("C");
  fXClusters->Clear("C");
  fUXClusters->Clear("C");
  fVXClusters->Clear("C");

  Int_t nuclus = 0;
  Int_t nvclus = 0;
  Int_t nxclus = 0;

  // U, X, V plane clusters
  if(fNHits >= fMinHitCut && fNHits < fMaxHitCut ) {
    for(Int_t ihit = 0; ihit < fNHits; ihit++) {
      HYPDCHit* hit1 = fHits[ihit];
      for(Int_t jhit = ihit+1; jhit < fNHits; jhit++) {
        HYPDCHit* hit2 = fHits[jhit];
        if( hit1->GetPlaneNum() != hit2->GetPlaneNum() && abs(hit1->GetPos() - hit2->GetPos()) < 0.6) {

          Double_t x = (hit1->GetPos() + hit2->GetPos())/2.0;
          Double_t y = 0.0;

          HYPDCCluster* clust = nullptr;
          if(hit1->GetAxis() == DC::kU)
            clust = new((*fUClusters)[nuclus++]) HYPDCCluster(x, y);
          else if(hit1->GetAxis() == DC::kV)
            clust = new((*fVClusters)[nvclus++]) HYPDCCluster(x, y);
          else 
            clust = new((*fXClusters)[nxclus++]) HYPDCCluster(x, y);

            clust->AddHit(hit1);
            clust->AddHit(hit2);
            clust->SetCoord(hit1->GetAxis());

            hit1->IncreasePlaneClust();
            hit2->IncreasePlaneClust();
          }
        } // hit2

      // single plane cluster
      if(hit1->GetNPlaneClust() == 0) {
          Double_t x = hit1->GetPos();
          Double_t y = 0.0;

          HYPDCCluster* clust;
          if(hit1->GetAxis() == DC::kU)
            clust = new((*fUClusters)[nuclus++]) HYPDCCluster(x, y);
          else if(hit1->GetAxis() == DC::kV)
            clust = new((*fVClusters)[nvclus++]) HYPDCCluster(x, y);
          else 
            clust = new((*fXClusters)[nxclus++]) HYPDCCluster(x, y);

            clust->AddHit(hit1);
            clust->SetCoord(hit1->GetAxis());
            hit1->IncreasePlaneClust();
      }
    } // hit1
  }

  // now form UX, VX pairs
  Int_t nuxclus = 0;
  for(Int_t iu = 0; iu < nuclus; iu++) {
    HYPDCCluster *uclus = static_cast<HYPDCCluster*>(fUClusters->At(iu));
    Double_t upos = uclus->GetX();
    for(Int_t ix = 0; ix < nxclus; ix++) {
      HYPDCCluster *xclus = static_cast<HYPDCCluster*>(fXClusters->At(ix));
      Double_t xpos = xclus->GetX();

      HYPDCHit* uhit = uclus->GetHit(0); // first hit in the cluster
      HYPDCHit* xhit = xclus->GetHit(0); // first hit in the cluster
      // calculate pos
      Double_t determinate = fXsp[uhit->GetPlaneNum()]*fYsp[xhit->GetPlaneNum()] -  fYsp[uhit->GetPlaneNum()]*fXsp[xhit->GetPlaneNum()];
      Double_t x = (upos*fYsp[xhit->GetPlaneNum()] - xpos*fYsp[uhit->GetPlaneNum()]);
      Double_t y = (xpos*fXsp[uhit->GetPlaneNum()] - upos*fXsp[xhit->GetPlaneNum()]);
      HYPDCCluster* clust = new((*fUXClusters)[nuxclus++]) HYPDCCluster(x, y);
    }
  }

  Int_t nvxclus = 0;
  for(Int_t iv = 0; iv < nvclus; iv++) {
    HYPDCCluster *vclus = static_cast<HYPDCCluster*>(fUClusters->At(iv));
    Double_t vpos = vclus->GetX();

    for(Int_t ix = 0; ix < nxclus; ix++) {
      HYPDCCluster *xclus = static_cast<HYPDCCluster*>(fXClusters->At(ix));
      Double_t xpos = xclus->GetX();

      HYPDCHit* vhit = vclus->GetHit(0); // first hit in the cluster
      HYPDCHit* xhit = xclus->GetHit(0); // first hit in the cluster
      // calculate pos
      Double_t determinate = fXsp[vhit->GetPlaneNum()]*fYsp[xhit->GetPlaneNum()] -  fYsp[vhit->GetPlaneNum()]*fXsp[xhit->GetPlaneNum()];
      Double_t x = (vpos*fYsp[xhit->GetPlaneNum()] - xpos*fYsp[vhit->GetPlaneNum()]);
      Double_t y = (xpos*fXsp[vhit->GetPlaneNum()] - vpos*fXsp[xhit->GetPlaneNum()]);  
      HYPDCCluster* clust = new((*fVXClusters)[nvxclus++]) HYPDCCluster(x, y);
    }
  }

  // Loop over ux and vx cluster pairs
  for(Int_t i = 0; i < nuxclus; i++) {
    HYPDCCluster* clust1 = static_cast<HYPDCCluster*>(fUXClusters->At(i));
    Double_t pos_x1 = clust1->GetX();
    Double_t pos_y1 = clust1->GetY();

    for(Int_t j = 0; j < nvxclus; j++) {
      HYPDCCluster* clust2 = static_cast<HYPDCCluster*>(fVXClusters->At(j));
      Double_t pos_x2 = clust2->GetX();
      Double_t pos_y2 = clust2->GetY();

      Double_t dist2 = pow(pos_x1-pos_x2,2) + pow(pos_y1-pos_y2,2);
      // FIXME: add cuts on dist2, xhit match, min number of hits

      Double_t x = (pos_x1 + pos_x2) / 2.0;
      Double_t y = (pos_y1 + pos_y2) / 2.0;
      
      cout << "UXCLUS VXCLUS X Y: " << i << " " << j << " " << x << " " << y << endl;
      // FIXME: Add to SpacePoint list
    }
  }

  return 0;

}

ClassImp(HYPDCChamber)
