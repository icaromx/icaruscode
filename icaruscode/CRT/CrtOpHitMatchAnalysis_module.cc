////////////////////////////////////////////////////////////////////////
// Class:       CrtOpHitMatchAnalysis
// Plugin Type: analyzer (art v3_05_00)
// File:        CrtOpHitMatchAnalysis_module.cc
//
// Generated at Tue Apr 14 19:49:18 2020 by Christopher Hilgenberg using cetskelgen
// from cetlib version v3_10_00.
////////////////////////////////////////////////////////////////////////

//Framework includes
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art_root_io/TFileService.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"

//LArSoft includes
#include "larcore/CoreUtils/ServiceUtil.h"
#include "lardata/DetectorInfoServices/DetectorClocksService.h"
#include "larsim/MCCheater/PhotonBackTrackerService.h"

//Data product includes
#include "nusimdata/SimulationBase/MCGeneratorInfo.h"
#include "nusimdata/SimulationBase/MCTruth.h"
#include "nusimdata/SimulationBase/MCParticle.h"
#include "lardataobj/RecoBase/OpHit.h"
#include "lardataobj/RecoBase/OpFlash.h"
#include "icaruscode/CRT/CRTProducts/CRTHit.hh"
#include "icaruscode/CRT/CRTProducts/CRTTrack.hh"
#include "icaruscode/CRT/CRTUtils/CRTCommonUtils.h"
#include "icaruscode/CRT/CRTUtils/CRTBackTracker.h"

//C++ includes
#include <vector>

//ROOT includes
#include "TTree.h"
#include "TVector3.h"

using std::vector;

class CrtOpHitMatchAnalysis;

class CrtOpHitMatchAnalysis : public art::EDAnalyzer {
public:
  explicit CrtOpHitMatchAnalysis(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  CrtOpHitMatchAnalysis(CrtOpHitMatchAnalysis const&) = delete;
  CrtOpHitMatchAnalysis(CrtOpHitMatchAnalysis&&) = delete;
  CrtOpHitMatchAnalysis& operator=(CrtOpHitMatchAnalysis const&) = delete;
  CrtOpHitMatchAnalysis& operator=(CrtOpHitMatchAnalysis&&) = delete;

  // Required functions.
  void analyze(art::Event const& e) override;

  // Selected optional functions.
  void beginJob() override;
  void endJob() override;

private:

  bool HitCompare(const art::Ptr<CRTHit>& h1, const art::Ptr<CRTHit>& h2);
  void ClearVecs();
  //const detinfo::DetectorClocks* fClock;

  art::InputTag fOpHitModuleLabel;
  art::InputTag fOpFlashModuleLabel0;
  art::InputTag fOpFlashModuleLabel1;
  art::InputTag fCrtHitModuleLabel;
  art::InputTag fCrtTrackModuleLabel;

  double        fCoinWindow;
  double        fOpDelay;
  double        fCrtDelay;
  int           fFlashPeThresh;
  int           fHitPeThresh;
  double        fFlashVelocity;
  double        fFlashZOffset;
  double        fHitVelocityMax;
  double        fHitVelocityMin;

  CRTBackTracker* bt;
  CRTCommonUtils *crtutil;

  map<int,art::InputTag> fFlashLabels;

  TTree* fMatchTree;
  TTree* fHitTree;
  TTree* fFlashTree;

  //matchTree vars
  int                    fNCrt;         //number of CRT hits per event
  vector<vector<double>> fCrtXYZT;      //CRT hit x,y,z,t [cm/ns]
  vector<vector<double>> fCrtXYZErr;    //CRT hit x,y,z,t error [cm/ns]
  vector<int>            fCrtRegion;    //CRT hit region code
  vector<double>         fCrtPE;        //total PE's for CRT hit
  vector<double>         fTofHit;       //CRT - PMT TOF using OpHit
  vector<double>         fTofFlash;     //CRT - PMT TOF using OpFlash
  vector<double>         fTofFlashHit;
  vector<double>         fDistHit;      //CRT - PMT distance [cm]
  vector<double>         fDistFlash;    //CRT - flash barycenter distance [cm]
  vector<double>         fDistFlashHit;
  vector<double>         fTofPeHit;     //total PE for matched OpHit
  vector<double>         fTofPeFlash;   //total PE for matched OpFlash
  vector<double>         fTofPeFlashHit;
  vector<vector<double>> fTofXYZTHit;   //position/time [cm/ns] for matched OpHit
  vector<vector<double>> fTofXYZTFlash; //position/time [cm/ns] for matched OpFlash
  vector<vector<double>> fTofXYZTFlashHit;
  vector<int>            fTofTpcHit;    //TPC for matched OpHit
  vector<int>            fTofTpcFlash;  //TPC for matched OpFlash
  vector<bool>           fMatchHit;     //was CRT hit matched to OpHit?
  vector<bool>           fMatchFlash;   //was CRT hit matched to OpFlash?
  vector<double>         fTrueDist;     //true dist from reco CRT hit to PMT
  vector<double>         fTrueTOF;      //true TOF from reco CRT hit to AV entry to PMT
  vector<vector<double>> fEnterXYZT;
  vector<vector<double>> fPMTXYZT;
  vector<int>            fHitPDG;       //PDG of particle w/most dep'ed energy in hit
  vector<bool>           fIV;           //does particle ass'ed with hit enter IV
  vector<bool>           fAV;           //does particle ass'ed with hit enter AV
  vector<bool>           fFV;           //does particle ass'ed with hit enter FV
  bool                   fNu;           //is this a neutrino event
  bool                   fNuCC;         //is nu int CC
  double                 fNuE;          //true neutrino energy [GeV]
  double                 fNuXYZT[4];    //true neutrino vertex pos[cm]/int time[ns]
  bool                   fNuIV;         //is nu vertex in IV
  bool                   fNuAV;         //is nu vertex in AV
  bool                   fNuFV;         //is nu vertex in FV
  vector<bool>           fTrackFilt;    //excluded by track filter

  //hitTree vars
  int                    fNHit;     //number of OpHits per event
  vector<double>         fHitPE;    //PE for each OpHit
  vector<int>            fHitChan;  //OpHit channel ID
  vector<vector<double>> fHitXYZT;  //position and time for each OpHit [cm/ns]

  //flashTree vars
  int                    fNFlash;      //number of OpFlashes per event
  vector<int>            fFlashTPC;    //TPC where flash occured
  vector<vector<double>> fFlashXYZT;   //position and time of flash [cm/ns]
  vector<double>         fFlashPE;     //total PE's per flash
  vector<int>            fFlashNHit;   //number of OpHits per flash
  vector<double>         fFlashMeanPE; //mean OpHit PE averaged over all OpHits in flash
  vector<double>         fFlashRmsPE;  //RMS of OPHit PE distro. in flash
  vector<vector<double>> fFlashDelta;

  geo::GeometryCore const* fGeometryService;   ///< pointer to Geometry provider
};


CrtOpHitMatchAnalysis::CrtOpHitMatchAnalysis(fhicl::ParameterSet const& p)
  : EDAnalyzer{p} ,
    fOpHitModuleLabel(p.get<art::InputTag>("OpHitModuleLabel","ophit")),
    fOpFlashModuleLabel0(p.get<art::InputTag>("OpFlashModuleLabel0","opflashTPC0")),
    fOpFlashModuleLabel1(p.get<art::InputTag>("OpFlashModuleLabel1","opflashTPC1")),
    fOpFlashModuleLabel2(p.get<art::InputTag>("OpFlashModuleLabel2","opflashTPC2")),
    fOpFlashModuleLabel3(p.get<art::InputTag>("OpFlashModuleLabel3","opflashTPC3")),
    fCrtHitModuleLabel(p.get<art::InputTag>("CrtHitModuleLabel","crthit")),
    fCrtTrackModuleLabel(p.get<art::InputTag>("CrtTrackModuleLabel","crttrack")),
    fCoinWindow(p.get<double>("CoincidenceWindow",60.0)),
    fOpDelay(p.get<double>("OpDelay",55.1)) {
  // Call appropriate consumes<>() for any products to be retrieved by this module.
  
  art::ServiceHandle<art::TFileService> tfs;

  fMatchTree = tfs->make<TTree>("matchTree","CRTHit - OpHit/Flash matching analysis");
  fHitTree   = tfs->make<TTree>("hitTree", "OpHit info");
  fFlashTree = tfs->make<TTree>("flashTree", "OpFlash info");

  fHitTree->Branch("nhit",         &fNHit,   "nOpHit/I");
  fHitTree->Branch("xyzt",         &fHitXYZT);
  fHitTree->Branch("pe",           &fHitPE);
  fHitTree->Branch("chan",         &fHitChan);

  fFlashTree->Branch("nflash",     &fNFlash, "nOpFlash/I");
  fFlashTree->Branch("tpc",        &fFlashTPC);
  fFlashTree->Branch("xyzt",       &fFlashXYZT);
  fFlashTree->Branch("totpe",      &fFlashPE);
  fFlashTree->Branch("nhit",       &fFlashNHit);
  fFlashTree->Branch("meanpe",     &fFlashMeanPE);
  fFlashTree->Branch("rmspe",      &fFlashRmsPE);
  fFlashTree->Branch("delta",      &fFlashDelta);

  fMatchTree->Branch("ncrt",         &fNCrt,      "nCrtHit/I");
  fMatchTree->Branch("crtXYZT",      &fCrtXYZT);
  fMatchTree->Branch("crtXYZErr",    &fCrtXYZErr);
  fMatchTree->Branch("crtPE",        &fCrtPE);
  fMatchTree->Branch("crtRegion",    &fCrtRegion); 
  fMatchTree->Branch("tofHit",       &fTofHit);
  fMatchTree->Branch("tofFlash",     &fTofFlash);
  fMatchTree->Branch("tofFlashHit",  &fTofFlashHit);
  fMatchTree->Branch("peHit",        &fTofPeHit);
  fMatchTree->Branch("peFlash",      &fTofPeFlash);
  fMatchTree->Branch("peFlashHit",   &fTofPeFlashHit);
  fMatchTree->Branch("xyztHit",      &fTofXYZTHit);
  fMatchTree->Branch("xyztFlash",    &fTofXYZTFlash);
  fMatchTree->Branch("xyztFlashHit", &fTofXYZTFlashHit);
  fMatchTree->Branch("distHit",      &fDistHit);
  fMatchTree->Branch("distFlash",    &fDistFlash);
  fMatchTree->Branch("distFlashHit", &fDistFlashHit);
  fMatchTree->Branch("tpcHit",       &fTofTpcHit);
  fMatchTree->Branch("tpcFlash",     &fTofTpcFlash);
  fMatchTree->Branch("matchHit",     &fMatchHit);
  fMatchTree->Branch("matchFlash",   &fMatchFlash);
  fMatchTree->Branch("distTrue",     &fTrueDist);
  fMatchTree->Branch("tofTrue",      &fTrueTOF);
  fMatchTree->Branch("enterXYZT",    &fEnterXYZT);
  fMatchTree->Branch("pmtXYZT",      &fPMTXYZT);
  fMatchTree->Branch("crtPDG",       &fHitPDG);
  fMatchTree->Branch("crtIV",        &fIV);
  fMatchTree->Branch("crtAV",        &fAV);
  fMatchTree->Branch("crtFV",        &fFV);
  fMatchTree->Branch("nu",           &fNu,       "nu/O");
  fMatchTree->Branch("nuCC",         &fNuCC,     "nuCC/O");
  fMatchTree->Branch("nuE",          &fNuE,      "nuE/D");
  fMatchTree->Branch("nuXYZT",       fNuXYZT,    "nuXYZT[4]/D");
  fMatchTree->Branch("nuIV",         &fNuIV,     "nuIV/O");
  fMatchTree->Branch("nuAV",         &fNuAV,     "nuAV/O");
  fMatchTree->Branch("nuFV",         &fNuFV,     "nuFV/O");
  fMatchTree->Branch("trackfilt",    &fTrackFilt);

  //fClock = lar::providerFrom<detinfo::DetectorClocksService>();
}

void CrtOpHitMatchAnalysis::analyze(art::Event const& e)
{
  //art::ServiceHandle<cheat::PhotonBackTrackerService> pbt;
  //fTTrig = fClock->TriggerTime();

  geo::CryostatGeo const& cryo0 = fGeometryService->Cryostat(0);
  geo::CryostatGeo const& cryo1 = fGeometryService->Cryostat(1);
  geo::TPCGeo const& tpc00 = cryo0.TPC(0);
  geo::TPCGeo const& tpc01 = cryo0.TPC(1);
  geo::TPCGeo const& tpc10 = cryo1.TPC(0);
  geo::TPCGeo const& tpc11 = cryo1.TPC(1);
  ClearVecs();

  auto const& mctruths = //vector of MCTruths from GENIE
    *e.getValidHandle<vector<simb::MCTruth>>(fGenLabel);

  fNuIV = false;
  fNuAV = false;
  fNuFV = false;
  if(mctruths[0].GeneratorInfo().generator==simb::Generator_t::kGENIE){
      fNu = true;
      auto const& nu = mctruths[0].GetNeutrino();
      const TLorentzVector xyzt = nu.Nu().Position(0);
      fNuE = nu.Nu().E(); 
      fNuCC = nu.CCNC();
      fNuXYZT[0] = xyzt.X();
      fNuXYZT[1] = xyzt.Y();
      fNuXYZT[2] = xyzt.Z();
      fNuXYZT[3] = xyzt.T();
      double point[3];
      for(int i=0; i<3; i++) point[i] = fNuXYZT[i];
      if(cryo0.ContainsPosition(point) || cryo1.ContainsPosition(point)){
          fNuIV = true;
          if(tpc00.ContainsPosition(point) ||
             tpc01.ContainsPosition(point) ||
             tpc10.ContainsPosition(point) ||
             tpc11.ContainsPosition(point) ) {

              fNuIV = false;
              fNuAV = true;
              fNuFV = true;
          }
      }
  }
  else{
      fNu = false;
      fNuE = DBL_MAX;
      fNuCC = false;
      for(int i=0; i<4; i++)
          fNuXYZT[i] = DBL_MAX;
  }

  fEvent  = e.id().event();
  fRun    = e.run();
  fSubrun = e.subRun();

  fTTrig = fClock->TriggerTime();

      particleMap[particle.TrackId()] = &particle;

  }//G4 tracks

  //SimPhotons
  /*art::Handle< std::vector<sim::SimPhotons>> photHandle;
  std::vector< art::Ptr<sim::SimPhotons> >   photList;
  if( e.getByLabel(fPhotLabel, photHandle) )
      art::fill_ptr_vector(photList, photHandle);

  fNPhot = photList.size();

  for(auto const& phot : photList){
      fPhotChan.push_back(phot->OpChannel());
      fNPhotChan.push_back(phot->size());
      double pos[3];
      fGeometryService->OpDetGeoFromOpChannel(phot->OpChannel()).GetCenter(pos);
      for(size_t i=0; i<phot->size(); i++) {
          vector<double> xyz = {pos[0],pos[1],pos[2],phot->at(i).Time};
          fPhotPos.push_back(xyz);
      }
  }

  fPhotTree->Fill();*/

  //OpDet waveforms
  /*art::Handle< std::vector<raw::OpDetWaveform> > wfHandle;
  std::vector< art::Ptr<raw::OpDetWaveform> > wfList;
  if( e.getByLabel(fWFLabel,wfHandle) )
      art::fill_ptr_vector(wfList, wfHandle);

  fNWFs = wfList.size();
  for(auto const& wf : wfList){
      fWFChans.push_back(wf->ChannelNumber());
      fWFTime.push_back(wf->TimeStamp());
  }
  fWFTree->Fill();*/

  //OpHits
  art::Handle< std::vector<recob::OpHit> > opHitListHandle;
  std::vector< art::Ptr<recob::OpHit> >    opHitList;
  if( e.getByLabel(fOpHitModuleLabel,opHitListHandle) )
      art::fill_ptr_vector(opHitList, opHitListHandle);

  fNOpHit = opHitList.size();

  for(auto const& ophit : opHitList){
        double t = ophit->PeakTime()*1e3-fOpDelay;
        fHitPE.push_back(ophit->PE());
        fHitChan.push_back(ophit->OpChannel());
        double pos[3];
        fGeometryService->OpDetGeoFromOpChannel(ophit->OpChannel()).GetCenter(pos);
        vector<double> xyzt = {pos[0],pos[1],pos[2],t};
        fHitXYZT.push_back(xyzt);
  }

  //OpFlash
  art::Handle< std::vector<recob::OpFlash> > opFlashListHandle0;
  art::Handle< std::vector<recob::OpFlash> > opFlashListHandle1;
  std::vector< art::Ptr<recob::OpFlash> >    opFlashList;
  if( e.getByLabel(fOpFlashModuleLabel0,opFlashListHandle0) )
      art::fill_ptr_vector(opFlashList, opFlashListHandle0);
  if( e.getByLabel(fOpFlashModuleLabel1,opFlashListHandle1) )
      art::fill_ptr_vector(opFlashList, opFlashListHandle1);

  fNOpFlash = opFlashList.size();

  for(auto const& flash : opFlashList){
        vector<double> xyzt;
        xyzt.push_back(0.); 
        xyzt.push_back(flash->YCenter());
        xyzt.push_back(flash->ZCenter());
        xyzt.push_back(flash->Time()*1e3-fOpDelay);
        fFlashXYZT.push_back(xyzt);
        fPeFlash.push_back(flash->TotalPE());
  }

  fFlashTree->Fill();

  //CRTracks
  art::Handle< std::vector<CRTTrack> > crtTrackListHandle;
  std::vector< art::Ptr<CRTTrack> >    crtTrackList;
  if( e.getByLabel(fCrtTrackModuleLabel,crtTrackListHandle))
      art::fill_ptr_vector(crtTrackList, crtTrackListHandle);

  art::FindManyP<CRTHit> findManyHits(
          crtTrackListHandle, e, fCrtTrackModuleLabel);


  //get time-ordered(ascending) vector of hits for each track
  std::vector<std::vector<art::Ptr<CRTHit>>> trackhits; 
  for(size_t itrk=0; itrk<crtTrackList.size(); itrk++){

      std::vector<art::Ptr<CRTHit>> trkhits = findManyHits.at(itrk);
      std::sort(trkhits.begin(),trkhits.end(),
        [](const art::Ptr<CRTHit>& a, const art::Ptr<CRTHit>& b)->bool
        { 
           return a->ts0_ns < b->ts0_ns; 
        });

      trackhits.push_back(trkhits);
       
  }//for tracks


  //CRTHits
  art::Handle< std::vector<icarus::crt::CRTHit> > crtHitListHandle;
  std::vector< art::Ptr<icarus::crt::CRTHit> >    crtHitList;
  if( e.getByLabel(fCrtHitModuleLabel,crtHitListHandle))
      art::fill_ptr_vector(crtHitList, crtHitListHandle);

  fNCrtHit = crtHitList.size();

  for(int icrt=0; icrt<fNCrtHit; icrt++){

      auto const& crthit = crtHitList[icrt];

      bool trackfilt=false;
      for(auto const& trkhits: trackhits){
          for(size_t ihit=1; ihit<trkhits.size(); ihit++) {
              if(HitCompare(trkhits[ihit],crt)){
                  trackfilt = true;
                  break;
              }
          }
          if(trackfilt)
              break;
      }
      fTrackFilt.push_back(trackfilt);

      vector<double> xyzt, xyzerr;
      TVector3 rcrt(crthit->x_pos,crthit->y_pos,crthit->z_pos);

      xyzt.push_back(rcrt.X());
      xyzt.push_back(rcrt.Y());
      xyzt.push_back(rcrt.Z());
      double tcrt = (int32_t)crt->ts0_ns - fCrtDelay;
      xyzt.push_back(tcrt);
      fCrtXYZT.push_back(xyzt);

      xyzerr.push_back(crt->x_err);
      xyzerr.push_back(crt->y_err);
      xyzerr.push_back(crt->z_err);
      fCrtXYZErr.push_back(xyzerr);

      fCrtPE.push_back(crt->peshit);
      fCrtRegion.push_back(crtutil->AuxDetRegionNameToNum(crt->tagger));

      // -- get truth info --
      //get trackID associated with CRTHit
      int trackID = bt->TrueIdFromTotalEnergy(e,*crt);
      bool firstIV=false, firstAV=false, firstFV=false;

      //if trackID exists in particle map
      if(particleMap.find(abs(trackID))!=particleMap.end()){
          auto const& particle = particleMap[abs(trackID)];
          fHitPDG.push_back(particle->PdgCode());
          for(size_t i=0; i<particle->NumberTrajectoryPoints(); i++){
              const TLorentzVector& pos = particle->Position(i);
              double point[3] = {pos.X(),pos.Y(),pos.Z()};
              if(cryo0.ContainsPosition(point)) {
                  firstIV = true;
                  if(!firstAV && (tpc00.ContainsPosition(point) ||
                     tpc01.ContainsPosition(point)) ) {
                      double opDetPos[3];
                      (cryo0.OpDet(cryo0.GetClosestOpDet(point))).GetCenter(opDetPos);
                      double ddirect = sqrt(pow(opDetPos[0]-rcrt.X(),2)
                                          + pow(opDetPos[1]-rcrt.Y(),2)
                                          + pow(opDetPos[2]-rcrt.Z(),2));
                      double dprop = sqrt(pow(opDetPos[0]-pos[0],2)
                                        + pow(opDetPos[1]-pos[1],2)
                                        + pow(opDetPos[2]-pos[2],2));
                      double tprop = pos.T() + dprop*LAR_PROP_DELAY;
                      fTrueDist.push_back(ddirect);
                      fTrueTOF.push_back(tcrt-tprop);
                      vector<double> tmp1 = {pos.X(),pos.Y(),pos.Z(),pos.T()};
                      vector<double> tmp2 = {opDetPos[0],opDetPos[1],opDetPos[2],tprop};
                      fEnterXYZT.push_back(tmp1);
                      fPMTXYZT.push_back(tmp2);
                      firstIV = false;
                      firstAV = true;
                      firstFV = true;
                  }//if AV
              }//if IV
              if(!firstAV && cryo1.ContainsPosition(point)){
                  firstIV=true;
                  if( !firstAV && (tpc10.ContainsPosition(point) ||
                      tpc11.ContainsPosition(point)) ) {
                      double opDetPos[3];
                      (cryo1.OpDet(cryo1.GetClosestOpDet(point))).GetCenter(opDetPos);
                      double ddirect = sqrt(pow(opDetPos[0]-rcrt.X(),2)
                                          + pow(opDetPos[1]-rcrt.Y(),2)
                                          + pow(opDetPos[2]-rcrt.Z(),2));
                      double dprop = sqrt(pow(opDetPos[0]-pos[0],2)
                                        + pow(opDetPos[1]-pos[1],2)
                                        + pow(opDetPos[2]-pos[2],2));
                      double tprop = pos.T() + dprop*LAR_PROP_DELAY;
                      fTrueDist.push_back(ddirect);
                      fTrueTOF.push_back(tcrt-tprop);
                      firstIV=false;
                      firstAV=true;
                      firstFV=true;
                  }
              }
              if(firstAV) break;
          }//for traj points
      }
      else {
          std::cout << "CRTHit trackID " << trackID << " not found in particle map!" << std::endl;
          fHitPDG.push_back(INT_MAX);
          fTrueDist.push_back(DBL_MAX);
          fTrueTOF.push_back(DBL_MAX);
          fEnterXYZT.push_back({DBL_MAX,DBL_MAX,DBL_MAX,DBL_MAX});
          fPMTXYZT.push_back({DBL_MAX,DBL_MAX,DBL_MAX,DBL_MAX});
      }
      fIV.push_back(firstIV);
      fAV.push_back(firstAV);
      fFV.push_back(firstFV);
      if(!firstAV&&fHitPDG.back()!=INT_MAX){
          fTrueDist.push_back(DBL_MAX);
          fTrueTOF.push_back(DBL_MAX);
          fEnterXYZT.push_back({DBL_MAX,DBL_MAX,DBL_MAX,DBL_MAX});
          fPMTXYZT.push_back({DBL_MAX,DBL_MAX,DBL_MAX,DBL_MAX});
      }//end get truth info

      // -- flash match --
      int matchtpc = -1;
      double tdiff = DBL_MAX, rdiff=DBL_MAX, peflash=DBL_MAX;
      bool matched=false;
      xyzt.clear();
      double flashHitT = DBL_MAX, flashHitPE=DBL_MAX, flashHitDiff=DBL_MAX;
      vector<double> flashHitxyzt;

      for(auto const& flashList : opFlashLists) {

          art::FindManyP<recob::OpHit> findManyHits(
                    flashHandles[flashList.first], e, fFlashLabels[flashList.first]);

          for(size_t iflash=0; iflash<flashList.second.size(); iflash++) { 

              auto const& flash = flashList.second[iflash];
              if(flash->TotalPE()<fFlashPeThresh){
                  continue;
              }

              double tflash = flash->Time()*1e3-fOpDelay;
              TVector3 rflash(0,flash->YCenter(),flash->ZCenter());
              TVector3 vdiff = rcrt-rflash;
              if(abs(tcrt-tflash)<abs(tdiff)) {
                  peflash = flash->TotalPE();
                  tdiff = tcrt-tflash;
                  rdiff = vdiff.Mag();
                  xyzt.clear();
                  xyzt.push_back(rflash.X());
                  xyzt.push_back(rflash.Y());
                  xyzt.push_back(rflash.Z());
                  xyzt.push_back(tflash);
                  matched = true;
                  matchtpc = flashList.first;

                  vector<art::Ptr<recob::OpHit>> hits = findManyHits.at(iflash);
                  for(auto const& hit : hits) {
                      double tPmt = hit->PeakTime()*1.e3-fOpDelay;
                      if( tPmt < flashHitT) {
                          flashHitT = tPmt;
                          flashHitPE = hit->PE();

                          //FlashHit position/time
                          double pos[3];
                          fGeometryService->OpDetGeoFromOpChannel(hit->OpChannel()).GetCenter(pos);
                          flashHitxyzt.clear();
                          for(int i=0; i<3; i++) flashHitxyzt.push_back(pos[i]);
                          flashHitxyzt.push_back(flashHitT);

                          //FlashHit distance
                          TVector3 rflashHit(pos[0],pos[1],pos[2]);
                          TVector3 vdiffHit = rcrt-rflashHit;
                          flashHitDiff = vdiffHit.Mag();
                      }
                  }//loop over flash hits

              }//if minimum tdiff
          }//for OpFlash in this flash list
      }//for flash lists
      if(!matched) {
          peflash = DBL_MAX;
          for(int i=0; i<4; i++) xyzt.push_back(DBL_MAX);
      }

      // -- match OpHits to CRTHits -- 
      tdiff = DBL_MAX;
      matched = false;
      peflash = DBL_MAX;
      rdiff = DBL_MAX;
      xyzt.clear();

      double pemax = 0.;
      for(auto const& hit : opHitList) {
          double thit = hit->PeakTime()*1e3-fOpDelay;
          if(hit->PE()<fHitPeThresh){
                continue;
          }

          //if(abs(tcrt-thit)<abs(tdiff)) {
          if(abs(tcrt-thit)<fCoinWindow && hit->PE()>pemax) {
              pemax = hit->PE();

              //hitXYZT
              double pos[3];
              fGeometryService->OpDetGeoFromOpChannel(hit->OpChannel()).GetCenter(pos);

              //distHit
              TVector3 rhit (pos[0],pos[1],pos[2]);
              TVector3 vdiff = rcrt-rhit;

              //double vel = abs(vdiff.Mag()/(tcrt-thit));
              //if(vel>fHitVelocityMax || vel<fHitVelocityMin)
              //    continue;
              rdiff = vdiff.Mag();
              peflash = hit->PE();
              tdiff = tcrt-thit;
              xyzt.clear();
              for(int i=0; i<3; i++) xyzt.push_back(pos[i]);
              xyzt.push_back(thit);

              matched = true;
                
          }//if min tof
      }//for OpHits
      if(!matched) {
          tdiff = DBL_MAX;
          peflash = DBL_MAX;
          rdiff = DBL_MAX;
          xyzt.clear();
          for(int i=0; i<4; i++) xyzt.push_back(DBL_MAX);
      }
      if(!matched) continue;
      fTofHit.push_back(tdiff);
      //fTofPe.push_back(fPeFlash[imatch])'
  }//for CRTHits

  fTree->Fill();

}//analyze

void CrtOpHitMatchAnalysis::beginJob()
{
}

void CrtOpHitMatchAnalysis::endJob()
{
    //matchTree
    fCrtXYZT.clear();
    fCrtXYZErr.clear();  
    fCrtRegion.clear();  
    fCrtPE.clear();      
    fTofHit.clear();     
    fTofFlash.clear();   
    fTofFlashHit.clear();
    fDistHit.clear();    
    fDistFlash.clear(); 
    fDistFlashHit.clear();
    fTofPeHit.clear();   
    fTofPeFlash.clear(); 
    fTofPeFlashHit.clear();
    fTofXYZTHit.clear();
    fTofXYZTFlash.clear();
    fTofXYZTFlashHit.clear();
    fTofTpcHit.clear();  
    fTofTpcFlash.clear();
    fMatchHit.clear();   
    fMatchFlash.clear(); 
    fTrueDist.clear();
    fTrueTOF.clear();
    fEnterXYZT.clear();
    fPMTXYZT.clear();
    fHitPDG.clear();
    fIV.clear();
    fAV.clear();
    fFV.clear();
    fTrackFilt.clear();

    //hitTree
    fHitXYZT.clear();
    fHitPE.clear();    
    fHitChan.clear();

    //flashTree
    fFlashTPC.clear(); 
    fFlashXYZT.clear();
    fFlashPE.clear();  
    fFlashMeanPE.clear();
    fFlashRmsPE.clear();
    fFlashDelta.clear();

}


bool CrtOpHitMatchAnalysis::HitCompare(const art::Ptr<CRTHit>& hit1, const art::Ptr<CRTHit>& hit2) {

      if(hit1->ts1_ns != hit2->ts1_ns) return false;
      if(hit1->plane  != hit2->plane) return false;
      if(hit1->x_pos  != hit2->x_pos) return false;
      if(hit1->y_pos  != hit2->y_pos) return false;
      if(hit1->z_pos  != hit2->z_pos) return false;
      if(hit1->x_err  != hit2->x_err) return false;
      if(hit1->y_err  != hit2->y_err) return false;
      if(hit1->z_err  != hit2->z_err) return false;
      if(hit1->tagger != hit2->tagger) return false;

      return true;

}

DEFINE_ART_MODULE(CrtOpHitMatchAnalysis)
