void replay(int RunNumber=111, int MaxEvent=100)
{

  const char* RunFileNamePattern = "run_%03d.dat.0.0";
  vector<TString> pathList;
  pathList.push_back("./rawdata");

  gHcParms->Define("gen_run_number", "Run_Number", RunNumber);
  gHcParms->AddString("g_ctp_database_filename", "DB/standard.database");
  gHcParms->Load(gHcParms->GetString("g_ctp_database_filename"), RunNumber);
  gHcParms->Load(gHcParms->GetString("g_ctp_kinematics_filename"), RunNumber);  
  gHcParms->Load("DB/scin_geo.param");
  gHcParms->Load("DB/scin_cuts.param");

  gHcDetectorMap = new THcDetectorMap();
  gHcDetectorMap->Load("DB/kscin.map");

  HKSSpectrometer* hks = new HKSSpectrometer("K", "HKS");
  gHaApps->Add(hks);

  HYPScintDetector* tof = new HYPScintDetector("scin", "scin");
  hks->AddDetector(tof);

  THcAnalyzer* analyzer = new THcAnalyzer;
  THaEvent* event = new THaEvent;

  THcRun* run = new THcRun(pathList, Form(RunFileNamePattern, RunNumber) );
  run->SetRunParamClass("THcRunParameters");
  run->SetEventRange(1, MaxEvent);
  run->SetDataRequired(THaRunBase::kDate|THaRunBase::kRunNumber);
  run->Print();

  analyzer->SetEvent(event);
  analyzer->SetCountMode(2); // 2 = counter is event number

  analyzer->SetCrateMapFileName("DB/db_cratemap.dat");
  analyzer->SetOutFile("test.root");
  analyzer->SetCutFile("test_cuts.def");
  analyzer->SetOdefFile("test.def");

  analyzer->Process(run);

}
