////////////////////////////////////////////////////////////////////////////////////
// Class:       CRTSimpleTrackProducer
// Module Type: producer
// File:        CRTSimpleTrackProducer_module.cc
// Description: Module for constructiong over-simplified CRT tracks.
// Copied from CRTTrackProducer by David Lorca Galindo 
//  Edited by Michelle Stancari April 3, 2018
//  Added some SBND specific stuff - Tom Brooks
//  Ported to and modified for use with icaruscode by Chris.Hilgenberg@colostate.edu
////////////////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Persistency/Common/FindManyP.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art_root_io/TFileService.h"
#include "art/Persistency/Common/PtrMaker.h"
#include "icaruscode/CRT/CRTProducts/CRTHit.hh"
#include "icaruscode/CRT/CRTProducts/CRTTrack.hh"
#include "icaruscode/CRT/CRTProducts/CRTTzero.hh"
#include "icaruscode/CRT/CRTUtils/CRTTrackRecoAlg.h"
#include "TTree.h"
#include "TVector3.h"
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <cmath> 
#include <memory>

using std::string;
using std::vector;
using std::pair;
using std::map;

namespace icarus {
namespace crt {

  class CRTTrackProducer : public art::EDProducer {
   public:
  
      explicit CRTTrackProducer(fhicl::ParameterSet const & p);
  
      // The destructor generated by the compiler is fine for classes
      // without bare pointers or other resource use.
      // Plugins should not be copied or assigned.
      CRTTrackProducer(CRTTrackProducer const &) = delete;
      CRTTrackProducer(CRTTrackProducer &&) = delete;
      CRTTrackProducer & operator = (CRTTrackProducer const &) = delete;
      CRTTrackProducer & operator = (CRTTrackProducer &&) = delete;
    
      // Required functions.
      void produce(art::Event & e) override;
    
      // Selected optional functions.
      void beginJob() override;
      void endJob() override;
    
   private:
  
      // Declare member data here.
      string       fDataLabelHits;
      string       fDataLabelTZeros;
      int          fTrackMethodType;
      int          fStoreTrack;
      bool         fUseTopPlane;
      CRTTrackRecoAlg trackAlg;
  }; // class CRTTrackProducer
  
  // Function to calculate average and rms from a vector of values
  void vmanip(vector<float> v, float* ave, float* rms);
  
  // Average crt hit structure
  struct CRTavehit{
  
      uint32_t ts0_ns;
      uint16_t ts0_ns_err;
      int32_t ts1_ns; 
      uint16_t ts1_ns_err;                                                        
      
      float x_pos;
      float x_err;
      float y_pos;
      float y_err;
      float z_pos;
      float z_err;
      float pe;
      int plane;
      string tagger;
    
  } tempah;
  
  // Function to make filling average hit easier
  CRTavehit fillme(uint32_t i, uint16_t j, int32_t k, uint16_t l, float a,
                   float b, float c, float d, float e, float f, float g, 
                   int p, string t);
  
  // Function to copy average hits
  CRTavehit copyme(CRTHit myhit);
  
  // Function to make creating CRTTracks easier
  CRTTrack shcut(CRTavehit ppA,CRTavehit ppb, uint32_t time0s,uint16_t terr);
  
  // Constructor
  CRTTrackProducer::CRTTrackProducer(fhicl::ParameterSet const & p)
    : EDProducer{p}, trackAlg(p.get<fhicl::ParameterSet>("TrackAlg"))
  {  
      // Initialize member data here.
      fDataLabelHits      = p.get<string>("DataLabelHits");      // CRTHit producer module name
      fDataLabelTZeros    = p.get<string>("DataLabelTZeros");    // CRTTzero producer module name
      fStoreTrack         = p.get<int>   ("StoreTrack");         // method 1 = all, method 2 = ave, method 3 = pure, method 4 = top plane
      fTrackMethodType    = p.get<int>   ("TrackMethodType");    // Print stuff
      fUseTopPlane        = p.get<bool>  ("UseTopPlane");        // Use hits from the top plane (SBND specific)
      
      // Call appropriate produces<>() functions here.
      if(fStoreTrack == 1){ 
          produces< vector<CRTTrack>   >();
          produces< art::Assns<CRTTrack , CRTHit> >();
      } 
    
  } // CRTTrackProducer()
  
  void CRTTrackProducer::produce(art::Event & evt)
  {
      // For validation
      int nTrack = 0;
      int nCompTrack = 0;
      int nIncTrack = 0;
    
      // CRTTrack collection on this event                                                                         
      std::unique_ptr<vector<CRTTrack> > CRTTrackCol(new vector<CRTTrack>);
      std::unique_ptr< art::Assns<CRTTrack, CRTHit> > Trackassn( new art::Assns<CRTTrack, CRTHit>);
      art::PtrMaker<CRTTrack> makeTrackPtr(evt);
    
      // Implementation of required member function here.
      art::Handle< vector<CRTHit> > rawHandle;
      evt.getByLabel(fDataLabelHits, rawHandle); //what is the product instance name? no BernZMQ
    
      // Check to make sure the data we asked for is valid                                                                         
      if(!rawHandle.isValid()){
          mf::LogWarning("CRTTrackProducer")
            <<"No CRTHits from producer module "<<fDataLabelHits;
          return;
      }
    
      // Track method 4 = SBND method with top plane (doesn't use CRTTzero)
      if(fTrackMethodType == 4){
    
          //Get the CRT hits from the event
          vector<art::Ptr<CRTHit> > hitlist;
    
          if (evt.getByLabel(fDataLabelHits, rawHandle))
            art::fill_ptr_vector(hitlist, rawHandle);
    
          map<art::Ptr<CRTHit>, int> hitIds;
    
          for(size_t i = 0; i<hitlist.size(); i++){
              hitIds[hitlist[i]] = i;
          }
    
          vector<vector<art::Ptr<CRTHit>>> CRTTzeroVect = trackAlg.CreateCRTTzeros(hitlist);
    
          // Loop over tzeros
          for(size_t i = 0; i<CRTTzeroVect.size(); i++){
    
              //loop over hits for this tzero, sort by tagger
              map<string, vector<art::Ptr<CRTHit>>> hits;
    
              for (size_t ah = 0; ah< CRTTzeroVect[i].size(); ++ah){        
    
                  string ip = CRTTzeroVect[i][ah]->tagger;       
                  hits[ip].push_back(CRTTzeroVect[i][ah]);
              } // loop over hits
              
              //loop over planes and calculate average hits
              vector<pair<CRTHit, vector<int>>> allHits;
    
              for (auto &keyVal : hits){
    
                  string ip = keyVal.first;
                  vector<pair<CRTHit, vector<int>>> ahits = trackAlg.AverageHits(hits[ip], hitIds);
    
                  if(fUseTopPlane && ip == "volTaggerTopHigh_0"){ 
                      allHits.insert(allHits.end(), ahits.begin(), ahits.end());
                  }
    
                  else if(ip != "volTaggerTopHigh_0"){ 
                      allHits.insert(allHits.end(), ahits.begin(), ahits.end());
                  }
              }
    
              //Create tracks with hits at the same tzero
              vector<pair<CRTTrack, vector<int>>> trackCandidates = trackAlg.CreateTracks(allHits);
              nTrack += trackCandidates.size();
    
              for(size_t j = 0; j < trackCandidates.size(); j++){
    
                  CRTTrackCol->emplace_back(trackCandidates[j].first);
                  art::Ptr<CRTTrack> trackPtr = makeTrackPtr(CRTTrackCol->size()-1);
    
                  for (size_t ah = 0; ah< CRTTzeroVect[i].size(); ++ah){        
                      Trackassn->addSingle(trackPtr, CRTTzeroVect[i][ah]);
                  }
    
                  if(trackCandidates[j].first.complete) 
                      nCompTrack++;
                  else 
                      nIncTrack++;
              }
          }
    
      }//end if sbnd option
    
      //Older track reconstruction methods from MicroBooNE
      else{
          //Get list of tzeros             
          //  std::vector<crt::CRTHit> const& CRTHitCollection(*rawHandle);
          art::Handle< vector<CRTTzero> > rawHandletzero;
          evt.getByLabel(fDataLabelTZeros, rawHandletzero); //what is the product instance name? no BernZMQ
          
          //check to make sure the data we asked for is valid                                                                            
          if(!rawHandletzero.isValid()){
            mf::LogWarning("CRTTrackProducer")
              <<"No CRTTzeros from producer module "<<fDataLabelTZeros;
            return;
          }
          
          vector<art::Ptr<CRTTzero> > tzerolist;
          if (evt.getByLabel(fDataLabelTZeros,rawHandletzero))
            art::fill_ptr_vector(tzerolist, rawHandletzero);
     
          art::FindManyP<CRTHit> fmht(rawHandletzero, evt, fDataLabelTZeros);
     
          //loop over tzeros
          for(size_t tzIter = 0; tzIter < tzerolist.size(); ++tzIter){   
            
              //count planes with hits for this tzero
              int np =0 ; //int ipflag[7] = {}; // CHANGED FROM 4 TO 7
              int tothits =0;
              for (int ip=0;ip<7;++ip) { // CHANGED FROM 4 TO 7
                  if (tzerolist[tzIter]->nhits[ip]>0){ 
                      np++; 
                      //ipflag[ip]=1; 
                      tothits+=tzerolist[tzIter]->nhits[ip];
                  }  
              }
     
              if (np<2) continue;
              vector<art::Ptr<CRTHit> > hitlist=fmht.at(tzIter);
              //for(size_t hit_i = 0; hit_i < hitlist.size(); hit
              if (fTrackMethodType==1) {
                  double time_s_A = hitlist[0]->ts0_s;
     
                  // find pairs of hits in different planes
                  for (size_t ah = 0; ah< hitlist.size()-1; ++ah){        
                      CRTHit temphit=*hitlist[ah];
                      CRTavehit Ahit = copyme(temphit);
                      int planeA = hitlist[ah]->plane;
     
                      for (size_t bh = ah+1; bh< hitlist.size(); ++bh){        
                          int planeB = hitlist[bh]->plane;
     
                          if (planeB!=planeA && !((planeA==3&&planeB==4)||(planeA==4&&planeB==3))) {  // make a track               
                              temphit=*hitlist[bh];
                              CRTavehit Bhit = copyme(temphit);
                              CRTTrack CRTcanTrack=shcut(Ahit,Bhit,time_s_A,0);
                              CRTTrackCol->emplace_back(CRTcanTrack);
                          }
     
                      }
     
                  }
     
              }
     
              else if ((fTrackMethodType==2) || (fTrackMethodType==3 && np==2 && tothits==2)) {        
     
                  //loop over hits and get average x,y,z,pe for each plane CHANGED FROM 4 TO 7
                  vector<float> thittime0[7];
                  vector<float> thittime1[7];
                  vector<float> tx[7];
                  vector<float> ty[7];
                  vector<float> tz[7];
                  vector<float> pe[7];
                  
                  double time_s_A = hitlist[0]->ts0_s;
                  //      double time_s_err = hitlist[0]->ts0_s_err;
                  double time_s_err = 0.;
                  double time1_ns_A = hitlist[0]->ts1_ns;
                  double time0_ns_A = hitlist[0]->ts0_ns;
               
                  //loop over hits for this tzero, sort by plane
                  for (size_t ah = 0; ah< hitlist.size(); ++ah){        
                      int ip = hitlist[ah]->plane;       
                      thittime0[ip].push_back(hitlist[ah]->ts0_ns-time0_ns_A);
                      thittime1[ip].push_back(hitlist[ah]->ts1_ns-time1_ns_A);
                      tx[ip].push_back(hitlist[ah]->x_pos);
                      ty[ip].push_back(hitlist[ah]->y_pos);
                      tz[ip].push_back(hitlist[ah]->z_pos);
                      pe[ip].push_back(hitlist[ah]->peshit);        
                  } // loop over hits
     
                  CRTavehit aveHits[7];
                  //loop over planes and calculate average hits
                  for (int ip = 0; ip < 7; ip++){
                      if (tx[ip].size()>0){
                          uint32_t at0; int32_t at1; uint16_t rt0,rt1;
                          float totpe=0.0;
                          float avet1=0.0; float rmst1 =0.0; 
                          float avet0=0.0; float rmst0 =0.0; 
                          float avex=0.0; float rmsx =0.0; 
                          float avey=0.0; float rmsy =0.0; 
                          float avez=0.0; float rmsz =0.0;
                          vmanip(thittime0[ip],&avet0,&rmst0);
                          vmanip(thittime1[ip],&avet1,&rmst1);
                          at0 = (uint32_t)(avet0+time0_ns_A); rt0 = (uint16_t)rmst0;   
                          at1 = (int32_t)(avet1+time1_ns_A); rt1 = (uint16_t)rmst1;
                          vmanip(tx[ip],&avex,&rmsx);
                          vmanip(ty[ip],&avey,&rmsy);
                          vmanip(tz[ip],&avez,&rmsz);
                          totpe=std::accumulate(pe[ip].begin(), pe[ip].end(), 0.0);
                          CRTavehit aveHit = fillme(at0,rt0,at1,rt1,avex,rmsx,avey,rmsy,avez,rmsz,totpe,ip,"");
                          aveHits[ip] = aveHit;
                      }
                      else {
                          CRTavehit aveHit = fillme(0,0,0,0,-99999,-99999,-99999,-99999,-99999,-99999,-99999,ip,"");
                          aveHits[ip] = aveHit;
                      }
                  }  
     
                  // find pairs of hits in different planes
                  for (size_t ah = 0; ah< 6; ++ah){        
                      CRTavehit Ahit = aveHits[ah];
                      if( Ahit.x_pos==-99999 ) continue;
     
                      for (size_t bh = ah+1; bh< 7; ++bh){        
                          if ( aveHits[bh].x_pos==-99999 ) continue;
     
                          if (ah!=bh && !(ah==3&&bh==4)) {  // make a track               
                              CRTavehit Bhit = aveHits[bh];
                              CRTTrack CRTcanTrack=shcut(Ahit,Bhit,time_s_A,time_s_err);
                              CRTTrackCol->emplace_back(CRTcanTrack);
                              nTrack++;
                          }
     
                      }
     
                  }
     
              }
            
          }// loop over tzeros
      }//uBooNE method
      
      //store track collection into event
      if(fStoreTrack == 1){
          evt.put(std::move(CRTTrackCol));
          evt.put(std::move(Trackassn));
      }
      mf::LogInfo("CRTTrackProducer")
        <<"Number of tracks            = "<<"\n"
        <<"Number of complete tracks   = "<<"\n"
        <<"Number of incomplete tracks = "<<nIncTrack;
      
  } // CRTTrackProducer::produce()
  
  void CRTTrackProducer::beginJob()
  {
  } // CRTTrackProducer::beginJob()
  
  void CRTTrackProducer::endJob()
  {
  } // CRTTrackProducer::endJob()
  
  // Function to calculate the mean and rms from a vector of values
  void vmanip(std::vector<float> v, float* ave, float* rms)
  {
    *ave=0.0; *rms =0.0;
    if (v.size()>0) {
      //  find the mean and *rms of all the vector elements
      double sum = std::accumulate(v.begin(), v.end(), 0.0);
      double mean = sum / v.size();
      *ave=mean;
      
      if (v.size()>1) {
      double sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
      double stdev = std::sqrt(sq_sum / v.size() - mean * mean);
      *rms=stdev;
      }
    }
  } // vmanip()
  
  // Function to make creating average crt hits easier
  CRTavehit fillme(uint32_t ts0_ns, uint16_t ts0_ns_err, int32_t ts1_ns, uint16_t ts1_ns_err, 
                   float x_pos, float x_err, float y_pos, float y_err, float z_pos, float z_err, 
                   float pe, int plane, string tagger)
  {
    CRTavehit h;
    h.ts0_ns     = ts0_ns;
    h.ts0_ns_err = ts0_ns_err;
    h.ts1_ns     = ts1_ns; 
    h.ts1_ns_err = ts1_ns_err;                                                        
    
    h.x_pos      = x_pos;
    h.x_err      = x_err;
    h.y_pos      = y_pos;
    h.y_err      = y_err;
    h.z_pos      = z_pos;
    h.z_err      = z_err;
    h.pe         = pe;
    h.plane      = plane;
    h.tagger     = tagger;
    return(h);
  } // fillme()
  
  // Function to copy average CRT hits
  CRTavehit copyme(CRTHit myhit)
  {
    CRTavehit h;
    h.ts0_ns     = myhit.ts0_ns;
    h.ts0_ns_err = 0;
    h.ts1_ns     = myhit.ts1_ns;; 
    h.ts1_ns_err = 0;       
    h.x_pos      = myhit.x_pos;
    h.x_err      = myhit.x_err;
    h.y_pos      = myhit.y_pos;
    h.y_err      = myhit.y_err;
    h.z_pos      = myhit.z_pos;
    h.z_err      = myhit.z_err;
    h.pe         = myhit.peshit;
    h.plane      = myhit.plane;
    h.tagger     = myhit.tagger;
    return(h);
  } // copyme()
  
  // Function to make CRTTrack
  CRTTrack shcut(CRTavehit ppA, CRTavehit ppB, uint32_t time0s, uint16_t terr)
  {
    CRTTrack newtr;
    newtr.ts0_s         = time0s;
    newtr.ts0_s_err     = terr;
    newtr.ts0_ns_h1     = ppA.ts0_ns;
    newtr.ts0_ns_err_h1 = ppA.ts0_ns_err;
    newtr.ts0_ns_h2     = ppB.ts0_ns;
    newtr.ts0_ns_err_h2 = ppB.ts0_ns_err;
    newtr.ts0_ns        = (uint32_t)(0.5*(ppA.ts0_ns+ppB.ts0_ns));
    newtr.ts0_ns_err    = (uint16_t)(0.5*sqrt(ppA.ts0_ns_err*ppA.ts0_ns_err+ppB.ts0_ns_err*ppB.ts0_ns_err));
    newtr.ts1_ns        = (int32_t)(0.5*(ppA.ts1_ns+ppB.ts1_ns));
    newtr.ts1_ns_err    = (uint16_t)(0.5*sqrt(ppA.ts0_ns_err*ppA.ts0_ns_err+ppB.ts0_ns_err*ppB.ts0_ns_err));
    newtr.peshit        = ppA.pe+ppB.pe;
    newtr.x1_pos        = ppA.x_pos;
    newtr.x1_err        = ppA.x_err;
    newtr.y1_pos        = ppA.y_pos;
    newtr.y1_err        = ppA.y_err;
    newtr.z1_pos        = ppA.z_pos;
    newtr.z1_err        = ppA.z_err;
    newtr.x2_pos        = ppB.x_pos;
    newtr.x2_err        = ppB.x_err;
    newtr.y2_pos        = ppB.y_pos;
    newtr.y2_err        = ppB.y_err;
    newtr.z2_pos        = ppB.z_pos;
    newtr.z2_err        = ppB.z_err;
    float deltax        = ppA.x_pos-ppB.x_pos;
    float deltay        = ppA.y_pos-ppB.y_pos;
    float deltaz        = ppA.z_pos-ppB.z_pos;
    newtr.length        = sqrt(deltax*deltax+deltay*deltay+deltaz*deltaz);
    newtr.thetaxy       = atan2(deltax,deltay);
    newtr.phizy         = atan2(deltaz,deltay);
    newtr.plane1        = ppA.plane;
    newtr.plane2        = ppB.plane;

    return(newtr);

  } // shcut()
  
  DEFINE_ART_MODULE(CRTTrackProducer)

}// namespace crt  
}// namespace icarus
