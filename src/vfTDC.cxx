/** \class vfTDC Module
    \author Stephen Wood
    \author Simona Malace
    \author Brad Sawatzky
    \author Eric Pooser

    Decoder module to retrieve vfTDCs.
*/

#include "vfTDC.h"
#include "THaSlotData.h"
#include "jlabdec.h"
#include <iostream>

using namespace std;

//#define DEBUG
//#define WITH_DEBUG

namespace Decoder {

  const UInt_t NTDCCHAN = 256;
  const UInt_t MAXHIT = 100;

  Module::TypeIter_t vfTDCModule::fgThisType =
    DoRegister( ModuleType( "Decoder::vfTDCModule" , 527 ));

  //_________________________________________________________________________________  
  vfTDCModule::vfTDCModule(Int_t crate, Int_t slot)
    : VmeModule(crate, slot), fNumHits(NTDCCHAN), fTdcData(NTDCCHAN*MAXHIT),
      fTdcOpt(NTDCCHAN*MAXHIT), slot_data(nullptr)
  {
    vfTDCModule::Init();
  }

  //_________________________________________________________________________________  
  void vfTDCModule::Init() {
    VmeModule::Init();
    fNumHits.resize(NTDCCHAN);
    fTdcData.resize(NTDCCHAN*MAXHIT);
    fTdcOpt.resize(NTDCCHAN*MAXHIT);
    fCoarseTime.resize(NTDCCHAN*MAXHIT);
    fFineTime.resize(NTDCCHAN*MAXHIT);
    f2nsBit.resize(NTDCCHAN*MAXHIT);

    Clear();
    IsInit = true;
    fName = "vfTDC Module";
  }

  //_________________________________________________________________________________  
  UInt_t vfTDCModule::LoadSlot( THaSlotData* sldat, const UInt_t* evbuffer,
				const UInt_t *pstop) {
    // This is a simple, default method for loading a slot
    const UInt_t *p = evbuffer;
    slot_data = sldat;
    fWordsSeen = 0; 		// Word count including global header  
    Int_t glbl_trl = 0;
    while(p <= pstop && glbl_trl == 0) {
      glbl_trl = Decode(p);
      fWordsSeen++;
      ++p;
    }
    return fWordsSeen;
  }

  //_________________________________________________________________________________  
  UInt_t vfTDCModule::LoadSlot( THaSlotData* sldat, const UInt_t* evbuffer,
				UInt_t pos, UInt_t len ) {
    // Fill data structures of this class, utilizing bank structure
    // Read until out of data or until decode says that the slot is finished
    // len = ndata in event, pos = word number for block header in event
    slot_data = sldat;
    fWordsSeen = 0;
    while(fWordsSeen < len) {
      Decode(&evbuffer[pos+fWordsSeen]);
      fWordsSeen++;
    }
    return fWordsSeen;
  }

  //_________________________________________________________________________________  
  Int_t vfTDCModule::Decode(const UInt_t *p) {

    Int_t glbl_trl = 0;
    UInt_t data_type_cont =(*p >> 31) & 0x1; 
    UInt_t data_type_def =(*p >> 27) & 0xF; 

    if (data_type_def ==0 && data_type_cont==0) data_type_def = 3; // trigger time continuation 
    
    static uint32_t type_last = 15;
    static int new_type = 0;
    int type_current = 0;
    generic_data_word_t gword;

    gword.raw = *p;

    if(gword.bf.data_type_defining) /* data type defining word */
      {
	new_type = 1;
	type_current = gword.bf.data_type_tag;
      }
    else
      {
	new_type = 0;
	type_current = type_last;
      }

    if (data_type_def ==3 && data_type_cont==0) type_current = 3; // trigger time continuation 

    switch(type_current) {
    case 0: // block header
      //std::cout << "Block header word = " << std::hex << *p << " shifted = " << std::hex << ((*p>>22)&0x1F) << std::endl; 
      tdc_data.glb_hdr_slno = (*p >> 22) & 0x1F; // 
      //	tdc_data.glb_hdr_slno = (*p >> 8) & 0x3FF;       // bits 
      if (tdc_data.glb_hdr_slno == fSlot) {
#ifdef WITH_DEBUG
	if (fDebugFile)
	  *fDebugFile << "vfTDCModule:: Block HEADER >> data = " 
		      << hex << *p << " >> event number = " << dec 
		      << " >> slot number = "  
		      << tdc_data.glb_hdr_slno << endl;
#endif
      }
      break;
    case 1: // block trailer
      glbl_trl=1;
      break;
    case 2: // event header
      //std::cout << "Event header word = " << std::hex << *p << " shifted = " << std::hex << ((*p>>22)&0x1F) << std::endl; 
      tdc_data.ev_hdr_slno = (*p >> 22) & 0x1F; // 
      tdc_data.evh_trig_num = *p & 0x7FFFFF;  // Event header trigger number
      break;
    case 3: // trigger time low 24
      if (tdc_data.ev_hdr_slno == fSlot) {
        if (data_type_cont==1) {
          tdc_data.trig_time_l = *p & 0xFFFFFF;  // Event header trigger time low 24
	  //std::cout << "Low Trigger: Slot = " << tdc_data.glb_hdr_slno << "  Trigger Time L = " << tdc_data.trig_time_l << endl;
        } else {
          tdc_data.trig_time_h = *p & 0xFFFFFF;  // Event header trigger time high 24
	  //std::cout << "High Trigger: Slot = " << tdc_data.glb_hdr_slno << "  Trigger Time H = " << tdc_data.trig_time_h << endl;
	  tdc_data.trig_time = (((tdc_data.trig_time_h << 24) | tdc_data.trig_time_l)%1024)*4000 - 10000;
	  //std::cout << "Trigger: Slot = " << tdc_data.glb_hdr_slno << "  Trigger Time = " << dec << tdc_data.trig_time << endl;
        }
      }
      break;
    case 7: // TDC hit
      if (tdc_data.ev_hdr_slno == fSlot) {
        Int_t group  = ((*p) & 0x07000000)>>24;
        Int_t chan   = ((*p) & 0x00F80000)>>19;
        Int_t edgeD  = ((*p) & 0x00040000)>>18;
        Int_t coarse = ((*p) & 0x0003FF00)>>8;
        Int_t two_ns = ((*p) & 0x00000080)>>7;
        Int_t fine   = ((*p) & 0x0000007F);

        tdc_data.opt = edgeD;

        tdc_data.chan = (group*32 + chan); //this is the original cable map
        //
        //The assumption at this point, after looking at missing pixel maps
        //is that for front panel connections, there is a 0-15 vs. 16-31 switching
        //that is needed, but for back panel connections it is as originally expected.
        //
        //EJB and BS: May 4, 2025

	// SP: Use the original map for now 
	/*
	  if(group >=1 and group <=4) {
	  if (chan <= 15) {
	  tdc_data.chan = (group*32 + chan + 16); // this is the flipped cable map
	  } else {
	  tdc_data.chan = (group*32 + chan - 16); // this is the flipped cable map
	  }
	  } else {
	  tdc_data.chan = group*32 + chan;
	  }
	*/
	tdc_data.raw = coarse*4 + two_ns*2 + fine*2/124.87; // time in ns
	
  if (tdc_data.raw < tdc_data.trig_time) {
	  tdc_data.raw = tdc_data.raw + 1024*4;
	}

  tdc_data.raw = tdc_data.raw - tdc_data.trig_time/1000.; // time in ns
  
  // only using LE signal for now. Need to give an option to choose? 
  if(tdc_data.opt == 1)
  	tdc_data.status = slot_data->loadData("tdc", tdc_data.chan, tdc_data.raw, tdc_data.opt);

#ifdef WITH_DEBUG
	if (fDebugFile)
	  *fDebugFile << "vfTDCModule:: MEASURED DATA >> data = " 
		      << hex << *p << " >> channel = " << dec
		      << tdc_data.chan << " >> edge = "
		      << tdc_data.opt  << " >> raw time = "
		      << tdc_data.raw << " >> status = "
		      << tdc_data.status << endl;
#endif

	/*
	//std::cout << std::endl;
	std::cout << "vfTDCModule:: MEASURED DATA >> data = " 
	<< hex << *p << " >> channel = " << dec
	<< tdc_data.chan << " >> slot = " << tdc_data.ev_hdr_slno << " >> edge = "
	<< tdc_data.opt  << " >> status = "
	<< tdc_data.status << " >> raw time = "
	<< tdc_data.raw << " original raw = "
	<< original_raw << " >> trigtime = "
	<< tdc_data.trig_time << std::endl;
	*/

        if(tdc_data.chan < NTDCCHAN &&
           fNumHits[tdc_data.chan] < MAXHIT) {
          fTdcData[tdc_data.chan * MAXHIT + fNumHits[tdc_data.chan]] = tdc_data.raw;
          fTdcOpt[tdc_data.chan * MAXHIT + fNumHits[tdc_data.chan]] = tdc_data.opt;
          fCoarseTime[tdc_data.chan * MAXHIT + fNumHits[tdc_data.chan]] = coarse;
          fFineTime[tdc_data.chan * MAXHIT + fNumHits[tdc_data.chan]] = fine;
          f2nsBit[tdc_data.chan * MAXHIT + fNumHits[tdc_data.chan]] = two_ns;

          fNumHits[tdc_data.chan]++;
        }
        if (tdc_data.status != SD_OK ) return -1;
      }
      break;
    case 14 : // invalid data
      break;
    case 15 : // buffer alignment filler word; skip
      break;
    default:	// unknown word
      cout << "unknown word for vfTDCModule: " << hex << (*p) << dec << endl;
      cout << "according to global header ev. nr. is: " << " " << tdc_data.glb_hdr_evno << endl;
      break;
    }
    return glbl_trl;  
  }

  //_________________________________________________________________________________  
  UInt_t vfTDCModule::GetData( UInt_t chan, UInt_t hit ) const {
    if( hit >= fNumHits[chan] ) return 0;
    UInt_t idx = chan * MAXHIT + hit;
    if( idx > MAXHIT * NTDCCHAN ) return 0;
    return fTdcData[idx];
  }

  //_________________________________________________________________________________  
  UInt_t vfTDCModule::GetOpt( UInt_t chan, UInt_t hit ) const {
    if( hit >= fNumHits[chan] ) return 0;
    UInt_t idx = chan * MAXHIT + hit;
    if( idx > MAXHIT * NTDCCHAN ) return 0;
    return fTdcOpt[idx];
  }

  //_________________________________________________________________________________  
  UInt_t vfTDCModule::GetTriggerTime() const {
    return tdc_data.trig_time;
  }

  //_________________________________________________________________________________  
  UInt_t vfTDCModule::GetCoarseTime( UInt_t chan, UInt_t hit ) const {
    if( hit >= fNumHits[chan] ) return 0;
    UInt_t idx = chan * MAXHIT + hit;
    if( idx > MAXHIT * NTDCCHAN ) return 0;
    return fCoarseTime[idx];
  }

  //_________________________________________________________________________________  
  UInt_t vfTDCModule::GetFineTime( UInt_t chan, UInt_t hit ) const {
    if( hit >= fNumHits[chan] ) return 0;
    UInt_t idx = chan * MAXHIT + hit;
    if( idx > MAXHIT * NTDCCHAN ) return 0;
    return fFineTime[idx];
  }

  //_________________________________________________________________________________  
  UInt_t vfTDCModule::Get2nsBit( UInt_t chan, UInt_t hit ) const {
    if( hit >= fNumHits[chan] ) return 0;
    UInt_t idx = chan * MAXHIT + hit;
    if( idx > MAXHIT * NTDCCHAN ) return 0;
    return f2nsBit[idx];
  }

  //_________________________________________________________________________________  
  void vfTDCModule::Clear( Option_t* ) {
    fNumHits.assign(NTDCCHAN, 0);
    fTdcData.assign(NTDCCHAN * MAXHIT, 0);
    fTdcOpt.assign(NTDCCHAN * MAXHIT, 0);
    fCoarseTime.assign(NTDCCHAN * MAXHIT, 0);
    fFineTime.assign(NTDCCHAN * MAXHIT, 0);
    f2nsBit.assign(NTDCCHAN * MAXHIT, 0);
  }
}

ClassImp(Decoder::vfTDCModule)


