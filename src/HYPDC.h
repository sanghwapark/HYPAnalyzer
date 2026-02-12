#ifndef HYPDC_h
#define HYPDC_h

#include "THaTrackingDetector.h"
#include "THcHitList.h"
#include "THcRawDCHit.h"
#include "HYPDCChamber.h"
#include "HYPDCPlane.h"
#include "HYPSpacePoint.h"
#include "TMath.h"

class TClonesArray;

class HYPDC : public THaTrackingDetector, public THcHitList {
 public:
  HYPDC(const char* name, const char* description="",
     THaApparatus* apparatus = nullptr);
  virtual ~HYPDC();

  //virtual void    Clear( Option_t* opt="" );
  virtual Int_t   Decode( const THaEvData& );
  virtual EStatus Init( const TDatime& run_time );
  virtual Int_t   End(THaRunBase* run=0);
  virtual Int_t   CoarseTrack( TClonesArray& tracks );
  virtual Int_t   FineTrack( TClonesArray& tracks );

  Int_t    GetNWires(Int_t plane)      const { return fNWires[plane-1];}
  Int_t    GetNChamber(Int_t plane)    const { return fNChamber[plane-1];}
  Int_t    GetWireOrder(Int_t plane)   const { return fWireOrder[plane-1];}
  Double_t GetPitch(Int_t plane)       const { return fPitch[plane-1];}
  Double_t GetCentralWire(Int_t plane) const { return fCentralWire[plane-1];}
  Int_t    GetTdcWinMin(Int_t plane)   const { return fTdcWinMin[plane-1];}
  Int_t    GetTdcWinMax(Int_t plane)   const { return fTdcWinMax[plane-1];}
  Double_t GetXPos(Int_t plane)        const { return fXPos[plane-1];}
  Double_t GetYPos(Int_t plane)        const { return fYPos[plane-1];}
  Double_t GetZPos(Int_t plane)        const { return fZPos[plane-1];}
  Double_t GetAlphaAngle(Int_t plane)  const { return fAlphaAngle[plane-1];}
  Double_t GetBetaAngle(Int_t plane)   const { return fBetaAngle[plane-1];}
  Double_t GetGammaAngle(Int_t plane)  const { return fGammaAngle[plane-1];}

  Int_t    GetMinHits(Int_t chamber)   const { return fMinHits[chamber-1];}
  Int_t    GetMaxHits(Int_t chamber)   const { return fMaxHits[chamber-1];}
  Int_t    GetMinCombos(Int_t chamber) const { return fMinCombos[chamber-1];}
  Double_t GetSpacePointCriterion(Int_t chamber) const { return fSpace_Point_Criterion[chamber-1];}
  Double_t GetCentralTime(Int_t plane) const { return fCentralTime[plane-1];}
  Int_t    GetDriftTimeSign(Int_t plane) const { return fDriftTimeSign[plane-1];}
  Int_t    GetReadoutLR(Int_t plane)   const { return fReadoutLR[plane-1];}
  Int_t    GetReadoutTB(Int_t plane)   const { return fReadoutTB[plane-1];}
  Int_t    GetVersion()                const {return fVersion;}

  Double_t GetPlaneTimeZero(Int_t plane) const { return fPlaneTimeZero[plane-1];}
  Double_t GetSigma(Int_t plane) const { return fSigma[plane-1];}
  Double_t GetNSperChan() const { return fNSperChan;}
  Double_t GetCenter(Int_t plane) const {
    return
      fXPos[plane-1]*sin(fAlphaAngle[plane-1]) +
      fYPos[plane-1]*cos(fAlphaAngle[plane-1]);
  }

  HYPDC();  // for ROOT I/O

 protected:
  Int_t fdebuglinkstubs;
  Int_t fdebugprintrawdc;
  Int_t fdebugflagpr;
  Int_t fdebugflagstubs;
  Int_t fdebugtrackprint;
  Int_t fdebugprintdecodeddc;

  UInt_t fNDCTracks;
  TClonesArray* fDCTracks;     // Tracks found from stubs 

  char   fPrefix[2];
  Int_t  fNPlanes;              // Total number of DC planes
  char** fPlaneNames;
  UInt_t fNChambers;
 
  Int_t fProjectToChamber;	     // If 1, project y position each stub back to it's own
                                // chamber before comparing y positions in LinkStubs
                                // Was used for SOS in ENGINE.
                                // FIXME: Remove
  Int_t     fTDC_RefTimeCut;
  Double_t  fTrackLargeResidCut;

  // Per-event data
  Int_t fStubTest;
  Int_t fNhits;
  Int_t fNthits;
  Int_t fN_True_RawHits;
  Int_t fNSp;                   // Number of space points
  Int_t fNsp_best;                   // Number of space points for gloden track
  Double_t* fDist_best;        //[fNPlanes] 
  Double_t* fLR_best;         //[fNPlanes]
  Double_t* fPos_best;         //[fNPlanes]
  Double_t* fResiduals;         //[fNPlanes] Array of residuals
  Double_t* fResidualsExclPlane;         //[fNPlanes] Array of residuals with plane excluded
  Double_t* fWire_hit_did;      //[fNPlanes]
  Double_t* fWire_hit_should;   //[fNPlanes]

  Double_t fNSperChan;		/* TDC bin size */
  Double_t fWireVelocity;
  Int_t fSingleStub;		/* If 1, single stubs make tracks */
  Int_t fNTracksMaxFP;
  Double_t fXtTrCriterion;
  Double_t fYtTrCriterion;
  Double_t fXptTrCriterion;
  Double_t fYptTrCriterion;
  Int_t fUseNewLinkStubs;
  Int_t fUseNewTrackFit; 
  Int_t fVersion;

  // Each of these will be dimensioned with the number of chambers
  Double_t* fXCenter;
  Double_t* fYCenter;
  Int_t* fMinHits;
  Int_t* fMaxHits;
  Int_t* fMinCombos;
  Double_t* fSpace_Point_Criterion;

  // Each of these will be dimensioned with the number of planes
  // A THcDCPlane class object will need to access the value for
  // its plane number.  Should we have a Get method for each or
  Int_t* fTdcWinMin;
  Int_t* fTdcWinMax;
  Double_t* fCentralTime;
  Int_t* fNWires;		// Number of wires per plane
  Int_t* fNChamber;  // FIXME: the name is confusing, this is the chamber number for each plane
  Int_t* fWireOrder;
  Int_t* fDriftTimeSign;
  Int_t* fReadoutTB;
  Int_t* fReadoutLR;

  Double_t* fXPos;
  Double_t* fYPos;
  Double_t* fZPos;
  Double_t* fAlphaAngle;
  Double_t* fBetaAngle;
  Double_t* fGammaAngle;
  Double_t* fPitch;
  Double_t* fCentralWire;
  Double_t* fPlaneTimeZero;
  Double_t* fSigma;
  Double_t** fPlaneCoeffs;
  //
  Double_t fX_fp_best;
  Double_t fY_fp_best;
  Double_t fXp_fp_best;
  Double_t fYp_fp_best;
  Double_t fChisq_best;
  Int_t fSp1_ID_best;
  Int_t fSp2_ID_best;
  Bool_t fInSideDipoleExit_best;

 // For accumulating statitics for efficiencies
  Int_t  fTotEvents;
  Int_t* fNChamHits;
  Int_t* fPlaneEvents;

  // Pointer to global var indicating whether this spectrometer is triggered
  // for this event.
  Bool_t* fPresentP;

  // Intermediate structure for building
  static const UInt_t MAXTRACKS = 100;

  std::vector<HYPDCPlane*> fPlanes; // List of plane objects
  std::vector<HYPDCChamber*> fChambers; // List of chamber objects

  TClonesArray*  fTrackProj;  // projection of track onto scintillator plane
                              // and estimated match to TOF paddle
                              // FIXME: this is not really used -- remove?
                              
  void           ClearEvent();
  void           DeleteArrays();
  virtual Int_t  ReadDatabase( const TDatime& date );
  virtual Int_t  DefineVariables( EMode mode = kDefine );
  void           LinkStubs();
  void           NewLinkStubs();
  void           TrackFit();
  void           NewTrackFit(UInt_t TrackIndex);
  Double_t       DpsiFun(Double_t ray[4], Int_t plane);
  void           EffInit();
  void           Eff();

  void Setup(const char* name, const char* description);
  void PrintSpacePoints();
  void PrintStubs();
  void PrintTrack();
  void EfficiencyPerWire(Int_t golden_track_index);

  ClassDef(HYPDC,0)

};

#endif
