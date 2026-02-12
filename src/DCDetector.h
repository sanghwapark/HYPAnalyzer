#ifndef DCDetector_h
#define DCDetector_h

#include "THcDC.h"

class DCDetector : public THcDC {
  public:
    DCDetector( const char* name, const char* description = "",
          THaApparatus* a = NULL );
      virtual ~DCDetector(); 

  ClassDef(DCDetector,0)

};

#endif
