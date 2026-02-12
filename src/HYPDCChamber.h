#ifndef HYPDCChamber_h
#define HYPDCChamber_h

#include "THaSubDetector.h"
#include "HYPDCPlane.h"
#include "HYPDCHit.h"
#include "HYPDCCluster.h"
#include "TClonesArray.h"
#include "TMatrixD.h"

class HYPSpacePoint;

class HYPDCChamber : public THaSubDetector {

public:
  HYPDCChamber( const char* name, const char* description, Int_t chambernum,
		   THaDetectorBase* parent = nullptr );
  virtual ~HYPDCChamber();

  virtual void    Clear( Option_t* opt="" );
  virtual Int_t   Decode( const THaEvData& );
  virtual EStatus Init( const TDatime& run_time );

  virtual void    AddPlane(HYPDCPlane *plane);
  virtual void    ProcessHits( void );
  virtual Int_t   FindSpacePoints( void ) ;
  virtual void    PrintDecode( void ) ;
  virtual void    CorrectHitTimes( void ) ;
  virtual void    LeftRight(void);
  
  Int_t GetNPlanes()      const { return fNPlanes; }
  Int_t GetNHits()        const { return fNHits; }
  Int_t GetNSpacePoints() const { return(fNSpacePoints);}
  Int_t GetNTracks()      const { return fTrackProj->GetLast()+1; }
  Int_t GetChamberNum()   const { return fChamberNum;}
  Double_t GetZPos()      const { return fZPos;}

  TClonesArray* GetTrackHits() const { return fTrackProj; }
  TClonesArray* GetSpacePointsP() const { return(fSpacePoints);}

  HYPDCChamber();

protected:

  Int_t fNPlanes;
  Int_t fNHits;
  Int_t fChamberNum;

  Int_t XPlaneInd; 		// Index of Xplane for this chamber
  Int_t XPlanePInd; 	// Index of Xplanep for this chamber
  Int_t XPlaneNum;		// Absolute plane number of Xplane
  Int_t XPlanePNum;		// Absolute plane number of Xplanep

  Int_t fMinHits;       // Minimum hits required to find space point
  Int_t fMaxHits;       // Maximum required to find space point
  Int_t fMinCombos;     // Minimum # pairs in a space point
  Double_t fWireVelocity;
  Int_t fSmallAngleApprox;
  Double_t fStubMaxXPDiff;
  Int_t fhdebugflagpr;
  Int_t fdebugstubchisq;
  Double_t fRatio_xpfp_to_xfp; // Used in selecting stubs 
  Double_t fZPos;
  Double_t fXCenter;
  Double_t fYCenter;
  Double_t fSpacePointCriterion;
  Double_t fMaxDist; 		// Max dist used in EasySpacePoint methods
  Double_t* fSinBeta;
  Double_t* fCosBeta;
  Double_t* fTanBeta;
  Double_t* fSigma;
  Double_t* fPsi0;
  Double_t** fStubCoefs;

  std::map<int,TMatrixD> fAA3Inv;

  TClonesArray* fUClusters;
  TClonesArray* fVClusters;
  TClonesArray* fXClusters;
  TClonesArray* fUXClusters;
  TClonesArray* fVXClusters;
  TClonesArray* fSpacePoints;

  // FIXME: Try to avoid having duplicated data 
  struct SpacePointOutputData {
    std::vector<Int_t> SpNHits;            //< [] Number of Hits in space point
    std::vector<Int_t> SpHitIndex;         //< []*SpNHits  Hit index for each hit in sp point
    void clear() {
      SpNHits.clear();
      SpHitIndex.clear();
    }
  };

  Int_t fNSpacePoints;
  SpacePointOutputData fSpHit;     //< Space point hit structure

  std::vector<HYPDCPlane*> fPlanes;
  std::vector<HYPDCHit*> fHits;
  
  TClonesArray*  fTrackProj;  // projection of track onto scintillator plane
                              // and estimated match to TOF paddle

  virtual Int_t  ReadDatabase( const TDatime &date );
  virtual Int_t  DefineVariables( EMode mode = kDefine );

  void       DeleteArrays();
  UInt_t     Count1Bits(UInt_t x);
  Double_t   FindStub(Int_t nhits, HYPSpacePoint *sp, 
    Int_t* plane_list, UInt_t bitpat, Int_t* plusminus, Double_t* stub);

  THaDetectorBase* fParent;

  ClassDef(HYPDCChamber,0)
};

#endif