#include "FastSimulation/Tracking/plugins/TrackCandidateProducer.h"

#include <memory>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/OwnVector.h"
#include "DataFormats/TrackerRecHit2D/interface/SiTrackerGSRecHit2DCollection.h" 
#include "DataFormats/TrackerRecHit2D/interface/SiTrackerGSMatchedRecHit2DCollection.h" 
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/TrackExtraFwd.h"

#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"

#include "FastSimulation/Tracking/interface/TrajectorySeedHitCandidate.h"
//#include "FastSimulation/Tracking/interface/TrackerRecHitSplit.h"

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

TrackCandidateProducer::TrackCandidateProducer(const edm::ParameterSet& conf) 
{  
    
  // products
  produces<TrackCandidateCollection>();
  
  // input tags
  edm::InputTag seedProducerLabel = conf.getParameter<edm::InputTag>("SeedProducer");
  edm::InputTag simTkProducerLabel = conf.getParameter<edm::InputTag>("SimTracks");

  // Reject overlapping hits?
  rejectOverlaps = conf.getParameter<bool>("OverlapCleaning");

  // Split hits ?
  splitHits = conf.getParameter<bool>("SplitHits");

  // consumes
  seedCollectionToken = consumes<TrajectorySeedCollection>(seedProducerLabel);
  recHitCombinationsToken = consumes<FastTMatchedRecHit2DCombinations>(seedProducerLabel);
  simVtxCollectionToken = consumes<edm::SimVertexContainer>(simTkProducerLabel);
  simTkCollectionToken = consumes<edm::SimTrackContainer>(simTkProducerLabel);
}
  
 
void 
TrackCandidateProducer::beginRun(edm::Run const&, const edm::EventSetup & es) {

  //services
  edm::ESHandle<MagneticField>          magField;
  edm::ESHandle<TrackerGeometry>        geometry;
  edm::ESHandle<TrackerTopology>        tTopo;

  es.get<IdealMagneticFieldRecord>().get(magField);
  es.get<TrackerDigiGeometryRecord>().get(geometry);
  es.get<IdealGeometryRecord>().get(tTopo);

  theMagField = &(*magField);
  theGeometry = &(*geometry);
  theTTopo = &(*tTopo);

  // TODO: get the propagator from the event
  thePropagator = std::make_shared<PropagatorWithMaterial>(alongMomentum,0.105,theMagField);
}

void 
TrackCandidateProducer::produce(edm::Event& e, const edm::EventSetup& es) {        

  // The produced objects
  std::auto_ptr<TrackCandidateCollection> output(new TrackCandidateCollection);    
  
  // Get the seeds
  edm::Handle<edm::View<TrajectorySeed> > theSeedCollection;
  e.getByToken(seedCollectionToken,theSeedCollection);

  // No seed -> output an empty track collection
  if(theSeedCollection->size() == 0) {
    e.put(output);
    return;
  }


  // SimTracks, SimVertices and RecHitCombinations
  edm::Handle<edm::SimVertexContainer> theSimVtxCollection;
  e.getByToken(simVtxCollectionToken,theSimVtxCollection);
  edm::Handle<edm::SimTrackContainer> theSimTkCollection;
  e.getByToken(simTkCollectionToken,theSimTkCollection);
  edm::Handle<FastTMatchedRecHit2DCombinations> theRecHitCombinations;
  e.getByToken(recHitCombinationsToken, theRecHitCombinations);

      
  // Produce a trackcandidate for each given seed
  for (unsigned seednr = 0; seednr < theSeedCollection->size(); ++seednr){

    // a reference to the RecHitCombination from which the seed was produced
    const FastTMatchedRecHit2DCombination & theRecHitCombination = theRecHitCombinations->at(seednr);

    // get the list of hits from which to produce the TrackCandidate
    std::vector<TrajectorySeedHitCandidate> theTrackerRecHits;
    TrajectorySeedHitCandidate theCurrentRecHit, thePreviousRecHit;
    for (unsigned h_rechit=0; h_rechit < theRecHitCombination.size(); ++h_rechit){

      theCurrentRecHit = TrajectorySeedHitCandidate(theRecHitCombination[h_rechit].get(),theGeometry,theTTopo);
    
      if ( !rejectOverlaps || 
	   h_rechit == 0 || 
	   theCurrentRecHit.subDetId()    != thePreviousRecHit.subDetId() || 
	   theCurrentRecHit.layerNumber() != thePreviousRecHit.layerNumber() ) {  
	addHits(theCurrentRecHit,theTrackerRecHits);       // insert the current hit
	thePreviousRecHit = theCurrentRecHit;
      } 
      else if ( theCurrentRecHit.localError() < thePreviousRecHit.localError() ) { 
	removeHits(thePreviousRecHit,theTrackerRecHits);   // get rid of the previous hit
	addHits(theCurrentRecHit,theTrackerRecHits);       // insert instead the current hit
	thePreviousRecHit = theCurrentRecHit;
      } 
      else {	  
	theCurrentRecHit = thePreviousRecHit;              // skip the current hit
      }
    }
    
    // convert TrajectorySeedHitCandidate to TrackingRecHit 
    edm::OwnVector<TrackingRecHit> recHits;
    unsigned nTrackerHits = theTrackerRecHits.size();
    recHits.reserve(nTrackerHits); 
    for ( unsigned ih=0; ih<nTrackerHits; ++ih ) {
      TrackingRecHit* aTrackingRecHit = theTrackerRecHits[ih].hit()->clone();
      recHits.push_back(aTrackingRecHit);
    }

    // find the SimTrack that corresponds to the RecHitCombination
    const SimTrack & simTk = theSimTkCollection->at(theRecHitCombination[0].get()->simtrackId1());

    // find the corresponding SimVertex
    const SimVertex & simVtx = theSimVtxCollection->at(simTk.vertIndex());

    //   a) origin vertex
    GlobalPoint position(simVtx.position().x(), simVtx.position().y(), simVtx.position().z());

    //   b) initial momentum
    GlobalVector momentum( simTk.momentum().x(), simTk.momentum().y(), simTk.momentum().z());

    //   c) electric charge
    float        charge   = simTk.charge();

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
    PTrajectoryStateOnDet PTSOD = trajectoryStateTransform::persistentState(initialTSOS,recHits.front().geographicalId().rawId()); 
    
    // add a new track candidate to the list
    output->push_back(TrackCandidate(recHits,theSeedCollection->at(seednr),PTSOD,edm::RefToBase<TrajectorySeed>(theSeedCollection,seednr)));
  }

  // Save the track candidates
  e.put(output);
}


void TrackCandidateProducer::addHits(const TrajectorySeedHitCandidate& theCurrentRecHit,
				     std::vector<TrajectorySeedHitCandidate>& theTrackerRecHits ){
  if (splitHits && theCurrentRecHit.matchedHit()->isMatched()){
      const SiTrackerGSRecHit2D* mHit = theCurrentRecHit.matchedHit()->monoHit();
      const SiTrackerGSRecHit2D* sHit = theCurrentRecHit.matchedHit()->stereoHit();
      
      // Add the new hits
      //  if( mHit->simhitId() < sHit->simhitId() ) {
	
	theTrackerRecHits.push_back(TrajectorySeedHitCandidate(mHit,theCurrentRecHit));
	theTrackerRecHits.push_back(TrajectorySeedHitCandidate(sHit,theCurrentRecHit));
	
	//  } else {
	
	//	theTrackerRecHits.push_back(TrajectorySeedHitCandidate(sHit,theCurrentRecHit));
	//	theTrackerRecHits.push_back(TrajectorySeedHitCandidate(mHit,theCurrentRecHit));
	
	//  }
  }
  else
    theTrackerRecHits.push_back(theCurrentRecHit);
}


void TrackCandidateProducer::removeHits(const TrajectorySeedHitCandidate& thePreviousRecHit,
				     std::vector<TrajectorySeedHitCandidate>& theTrackerRecHits ){
  theTrackerRecHits.pop_back();
  if ( splitHits && thePreviousRecHit.matchedHit()->isMatched() ) theTrackerRecHits.pop_back();
}
