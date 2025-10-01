#include "HKSSpectrometer.h"
#include "THaTrack.h"

#include <iostream>
#include <fstream>

using namespace std;

ClassImp(HKSSpectrometer)

//_____________________________________________________________________________
HKSSpectrometer::HKSSpectrometer( const char* name, const char* description ) :
  THaSpectrometer( name, description )
{
  //Default constructor
}

//_____________________________________________________________________________
HKSSpectrometer::~HKSSpectrometer()
{
  // Destructor
  DefineVariables( kDelete );

}

//_____________________________________________________________________________
Int_t HKSSpectrometer::ReadDatabase( const TDatime& date )
{

  // Load parameters 

  #ifdef WITH_DEBUG
    cout << "HKSSpectrometer::ReadDatabase()" << endl;
  #endif

  return kOK;
  
}

//_____________________________________________________________________________
Int_t HKSSpectrometer::ReadRunDatabase( const TDatime& date )
{

  return kOK;
  
}

//_____________________________________________________________________________
Int_t HKSSpectrometer::DefineVariables( EMode mode )
{

  if( mode == kDefine && fIsSetup ) return kOK;
  THaSpectrometer::DefineVariables( mode );
  fIsSetup = ( mode == kDefine );
  // Add variables here
  /*
  RVarDef vars[] = {
    {},
    { 0 }    
  };
  
  return DefineVarsFromList(vars, mode);
    */

  return kOK;

}

//_____________________________________________________________________________
Int_t HKSSpectrometer::FindVertices( TClonesArray& tracks )
{

  return 0;
}

//_____________________________________________________________________________
Int_t HKSSpectrometer::TrackCalc()
{

  return 0;
}
