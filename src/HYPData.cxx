#include "HYPData.h"
#include "Decoder.h"
#include "Fadc250Module.h"

using namespace std;

namespace HYPData {

  //_______________________________________________________________
  Int_t FADCData::Decode( const THaEvData& evdata, THaDetMap::Module *d)
  {

    return 0;
  }

  //_______________________________________________________________
  Int_t FADCData::AddHit( const DigitizerHitInfo_t& hitinfo )
  {
    // Modes for multi-function module
    // enum EModuleType { kSampleADC, kPulseIntegral, kPulseTime,
    // kPulsePeak, kPulsePedestal, kCoarseTime, kFineTime };

    UInt_t chan = hitinfo.lchan; // logical channel
    auto* fadc = dynamic_cast<Decoder::Fadc250Module*>(hitinfo.module);
    assert(fadc);

    // Pulse mode data
    // Assume that the # of pulses for pulsetime, peak, pedestal are same
    UInt_t nevents = fadc->GetNumEvents(Decoder::kPulseIntegral, hitinfo.chan);
    // cout << "Hit info lch: " << chan << " ch:" << hitinfo.chan << endl;
    // cout << "Number of pulses: " << nevents << endl;    

    for(UInt_t i = 0; i < nevents; i++) {
      UInt_t pulseint  = fadc->GetPulseIntegralData(hitinfo.chan, i);
      UInt_t pulsepeak = fadc->GetPulsePeakData(hitinfo.chan, i);
      UInt_t pulsetime = fadc->GetPulseTimeData(hitinfo.chan, i);
      UInt_t pulseped  = fadc->GetPulsePedestalData(hitinfo.chan, i);
      UInt_t coarsetime = fadc->GetPulseCoarseTimeData(hitinfo.chan, i);
      UInt_t finetime = fadc->GetPulseFineTimeData(hitinfo.chan, i);

    /*
        cout << "Integral: " << pulseint << endl;
        cout << "PulseAmp: " << pulsepeak << endl;
        cout << "PulseTime: " << pulsetime << endl;
        cout << "Pedestal: " << pulseped << endl;
        cout << "CoarseTime:" << coarsetime << endl;
        cout << "FineTime:" << finetime << endl;
    */
        fPulseData.push_back(FADCHit{chan, pulseint, pulsepeak, pulsetime, pulseped, coarsetime, finetime});
    }

    // Sample data
    UInt_t nsamples = fadc->GetNumEvents(Decoder::kSampleADC, hitinfo.chan);
    for(UInt_t isamp = 0; isamp < nsamples; isamp++) {
      fSampleData.push_back(fadc->GetData(Decoder::kSampleADC, hitinfo.chan, isamp));
    }

    fNHits++;

    return nevents;
  }

  //_______________________________________________________________
  void FADCData::Clear()
  {
    fPulseData.clear();
    fNHits = 0;
  }
}
