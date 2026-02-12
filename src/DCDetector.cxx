#include "DCDetector.h"

using namespace std;

//____________________________________________________
DCDetector::DCDetector( const char* name, const char* description,
		   THaApparatus* a)
  : THcDC(name, description, a)
{

}

//____________________________________________________
DCDetector::~DCDetector()
{

}

ClassImp(DCDetector)