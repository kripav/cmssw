#include <memory>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/OwnVector.h"
#include "DataFormats/TrackCandidate/interface/TrackCandidateCollection.h"
#include "DataFormats/TrajectorySeed/interface/TrajectorySeedCollection.h"
#include "DataFormats/TrackerRecHit2D/interface/SiTrackerGSRecHit2DCollection.h" 
#include "DataFormats/TrackerRecHit2D/interface/SiTrackerGSMatchedRecHit2DCollection.h" 
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/TrackExtraFwd.h"

#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"

#include "FastSimulation/Tracking/interface/TrajectorySeedHitCandidate.h"
//#include "FastSimulation/Tracking/interface/TrackerRecHitSplit.h"
#include "FastSimulation/Tracking/plugins/TrackCandidateProducer.h"

#include <vector>

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "MagneticField/Engine/interface/MagneticField.h"

#include "SimDataFormats/Track/interface/SimTrack.h"

#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"
#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"
#include "TrackingTools/GeomPropagators/interface/Propagator.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"
#include "TrackingTools/MaterialEffects/interface/PropagatorWithMaterial.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"

TrackCandidateProducer::TrackCandidateProducer(const edm::ParameterSet& conf):thePropagator(0) 
{  
    
  if(conf.exists("keepFittedTracks")){
    std::cout << "TrackCandidateProducer no longer supports option keepFittedTracks" << std::endl;
    assert(false);
  }
  if(conf.exists("TrackProducers")){
    edm::LogError("TrackCandidateProducer") << "TrackCandidateProducer no longer supports TrackProducers" << std::endl;
    exit (0);
  }
 
  // The main product is a track candidate collection.
  produces<TrackCandidateCollection>();
  
  // The name of the seed producer
  seedProducer = conf.getParameter<edm::InputTag>("SeedProducer");

  // The name of the recHit producer
  hitProducer = conf.getParameter<edm::InputTag>("HitProducer");

  // The minimum number of crossed layers
  minNumberOfCrossedLayers = conf.getParameter<unsigned int>("MinNumberOfCrossedLayers");

  // The maximum number of crossed layers
  maxNumberOfCrossedLayers = conf.getParameter<unsigned int>("MaxNumberOfCrossedLayers");

  // Reject overlapping hits?
  rejectOverlaps = conf.getParameter<bool>("OverlapCleaning");

  // Split hits ?
  splitHits = conf.getParameter<bool>("SplitHits");

  // Reject tracks with several seeds ?
  // Typically don't do that at HLT for electrons, but do it otherwise
  seedCleaning = conf.getParameter<bool>("SeedCleaning");

  simTracks_ = conf.getParameter<edm::InputTag>("SimTracks");
  estimatorCut_= conf.getParameter<double>("EstimatorCut");

  // consumes
  seedToken = consumes<edm::View<TrajectorySeed> >(seedProducer);
  recHitRefToken = consumes<FastTMatchedRecHit2DCombinations>(edm::InputTag("theRecHitCombinations"));  
  edm::InputTag _label("famosSimHits");
  simVertexToken = consumes<edm::SimVertexContainer>(_label);
  simTrackToken = consumes<edm::SimTrackContainer>(_label);
}
  
// Virtual destructor needed.
TrackCandidateProducer::~TrackCandidateProducer() {

  if(thePropagator) delete thePropagator;
} 
 
void 
TrackCandidateProducer::beginRun(edm::Run const&, const edm::EventSetup & es) {

  //services
  edm::ESHandle<MagneticField>          magField;
  edm::ESHandle<TrackerGeometry>        geometry;

  es.get<IdealMagneticFieldRecord>().get(magField);
  es.get<TrackerDigiGeometryRecord>().get(geometry);

  theMagField = &(*magField);
  theGeometry = &(*geometry);

  thePropagator = new PropagatorWithMaterial(alongMomentum,0.105,&(*theMagField)); 
}
  
  // Functions that get called by framework every event
void 
TrackCandidateProducer::produce(edm::Event& e, const edm::EventSetup& es) {        

  // Useful typedef's to avoid retyping
  typedef std::pair<reco::TrackRef,edm::Ref<std::vector<Trajectory> > > TrackPair;

  // The produced objects
  std::auto_ptr<TrackCandidateCollection> output(new TrackCandidateCollection);    
  
  // Get the seeds
  edm::Handle<edm::View<TrajectorySeed> > theSeeds;
  e.getByToken(seedToken,theSeeds);

  //Retrieve tracker topology from geometry
  edm::ESHandle<TrackerTopology> tTopoHand;
  es.get<IdealGeometryRecord>().get(tTopoHand);
  const TrackerTopology *tTopo=tTopoHand.product();


  // No seed -> output an empty track collection
  if(theSeeds->size() == 0) {
    e.put(output);
    return;
  }

  // SimTracks, SimVertices and RecHitCombinations
  edm::Handle<edm::SimVertexContainer> theSimVtx;
  e.getByToken(simVertexToken,theSimVtx);
  edm::Handle<edm::SimTrackContainer> theSTC;
  e.getByToken(simTrackToken,theSTC);
  edm::Handle<FastTMatchedRecHit2DCombinations> theRecHitCombinations;
  e.getByToken(recHitRefToken, theRecHitCombinations);

  const edm::SimTrackContainer* theSimTracks = &(*theSTC);
      
  // Loop over the seeds
  int currentTrackId = -1;
 
  unsigned seed_size = theSeeds->size(); 
  for (unsigned seednr = 0; seednr < seed_size; ++seednr){
    // The seed
    const BasicTrajectorySeed* aSeed = &((*theSeeds)[seednr]);

    std::vector<TrajectorySeedHitCandidate> theTrackerRecHits;
    TrajectorySeedHitCandidate theCurrentRecHit, thePreviousRecHit;
    const FastTMatchedRecHit2DCombination & theRecHitCombination = theRecHitCombinations->at(seednr);
     
    for (unsigned h_rechit=0; h_rechit < theRecHitCombination.size(); ++h_rechit){

      theCurrentRecHit = TrajectorySeedHitCandidate(theRecHitCombination[h_rechit].get(),theGeometry,tTopo);
    
      if ( !rejectOverlaps || 
	   h_rechit == 0 || 
	   theCurrentRecHit.subDetId()    != thePreviousRecHit.subDetId() || 
	   theCurrentRecHit.layerNumber() != thePreviousRecHit.layerNumber() ) {  
	addHits(theCurrentRecHit,theTrackerRecHits);     // No splitting   
	thePreviousRecHit = theCurrentRecHit;
      } 
      else if ( theCurrentRecHit.localError() < thePreviousRecHit.localError() ) { 
	removeHits(thePreviousRecHit,theTrackerRecHits);   // get rid of the previous hit
	addHits(theCurrentRecHit,theTrackerRecHits);       // insert the current hit
	thePreviousRecHit = theCurrentRecHit;
      } 
      else {	  
	// skipp the current hit
	theCurrentRecHit = thePreviousRecHit;
      }
    }// End of loop over the track rechits
    
    int simTrackId = theRecHitCombination[0].get()->simtrackId();
    edm::OwnVector<TrackingRecHit> recHits;
    unsigned nTrackerHits = theTrackerRecHits.size();
    recHits.reserve(nTrackerHits); 
    
    for ( unsigned ih=0; ih<nTrackerHits; ++ih ) {
      TrackingRecHit* aTrackingRecHit = theTrackerRecHits[ih].hit()->clone();
      recHits.push_back(aTrackingRecHit);
    }

    PTrajectoryStateOnDet PTSOD;
      int vertexIndex = (*theSimTracks)[currentTrackId].vertIndex();
      //   a) origin vertex
      GlobalPoint  position((*theSimVtx)[vertexIndex].position().x(),
			    (*theSimVtx)[vertexIndex].position().y(),
			    (*theSimVtx)[vertexIndex].position().z());
      //   b) initial momentum
      GlobalVector momentum( (*theSimTracks)[currentTrackId].momentum().x() , 
			     (*theSimTracks)[currentTrackId].momentum().y() , 
			     (*theSimTracks)[currentTrackId].momentum().z() );
      //   c) electric charge
      float        charge   = (*theSimTracks)[simTrackId].charge();
      //  -> inital parameters
      GlobalTrajectoryParameters initialParams(position,momentum,(int)charge,theMagField);
      //  -> large initial errors
      AlgebraicSymMatrix55 errorMatrix= AlgebraicMatrixID();    
      CurvilinearTrajectoryError initialError(errorMatrix);
      // -> initial state
      FreeTrajectoryState initialFTS(initialParams, initialError);      
      const GeomDet* initialLayer = theGeometry->idToDet(recHits.front().geographicalId());  
      const TrajectoryStateOnSurface initialTSOS = thePropagator->propagate(initialFTS,initialLayer->surface()) ;
      if (!initialTSOS.isValid()) continue;       
      PTSOD = trajectoryStateTransform::persistentState(initialTSOS,recHits.front().geographicalId().rawId()); 
    
        
    TrackCandidate newTrackCandidate(recHits,*aSeed,PTSOD,edm::RefToBase<TrajectorySeed>(theSeeds,seednr));
    output->push_back(newTrackCandidate);
  }//loop over all possible seeds.

  // Save the track candidates
  e.put(output);
}


void TrackCandidateProducer::addHits(const TrajectorySeedHitCandidate& theCurrentRecHit,
				     std::vector<TrajectorySeedHitCandidate>& theTrackerRecHits ){
  
  if (splitHits && theCurrentRecHit.matchedHit()->isMatched()){
      const SiTrackerGSRecHit2D* mHit = theCurrentRecHit.matchedHit()->monoHit();
      const SiTrackerGSRecHit2D* sHit = theCurrentRecHit.matchedHit()->stereoHit();
      
      // Add the new hits
      if( mHit->simhitId() < sHit->simhitId() ) {
	
	theTrackerRecHits.push_back(TrajectorySeedHitCandidate(mHit,theCurrentRecHit));
	theTrackerRecHits.push_back(TrajectorySeedHitCandidate(sHit,theCurrentRecHit));
	
      } else {
	
	theTrackerRecHits.push_back(TrajectorySeedHitCandidate(sHit,theCurrentRecHit));
	theTrackerRecHits.push_back(TrajectorySeedHitCandidate(mHit,theCurrentRecHit));
	
      }
  }
  else
    theTrackerRecHits.push_back(theCurrentRecHit);
}


void TrackCandidateProducer::removeHits(const TrajectorySeedHitCandidate& thePreviousRecHit,
				     std::vector<TrajectorySeedHitCandidate>& theTrackerRecHits ){

  theTrackerRecHits.pop_back();
  if ( splitHits && thePreviousRecHit.matchedHit()->isMatched() ) theTrackerRecHits.pop_back();
}





