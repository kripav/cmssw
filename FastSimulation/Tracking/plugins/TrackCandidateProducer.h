#ifndef FastSimulation_Tracking_TrackCandidateProducer_h
#define FastSimulation_Tracking_TrackCandidateProducer_h

#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/TrackCandidate/interface/TrackCandidateCollection.h"
#include "DataFormats/TrajectorySeed/interface/TrajectorySeedCollection.h"
#include "DataFormats/TrackerRecHit2D/interface/SiTrackerGSMatchedRecHit2D.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "TrackingTools/PatternTools/interface/Trajectory.h"
#include "TrackingTools/PatternTools/interface/TrajTrackAssociation.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "SimDataFormats/Vertex/interface/SimVertexContainer.h"

#include "FastSimulation/Tracking/interface/TrajectorySeedHitCandidate.h"

class TrackerGeometry;
class PropagatorWithMaterial;

namespace edm { 
  class ParameterSet;
  class Event;
  class EventSetup;
}

namespace reco { 
  class Track;
}


class TrackingRecHit;

#include <vector>

class TrackCandidateProducer : public edm::stream::EDProducer <>
{
 public:
  
  explicit TrackCandidateProducer(const edm::ParameterSet& conf);
  virtual ~TrackCandidateProducer(){};
  virtual void beginRun(edm::Run const& run, const edm::EventSetup & es) override;
  virtual void produce(edm::Event& e, const edm::EventSetup& es) override;
  
 private:

  void addHits(const TrajectorySeedHitCandidate&, std::vector<TrajectorySeedHitCandidate>&);
  void removeHits(const TrajectorySeedHitCandidate&, std::vector<TrajectorySeedHitCandidate>&);  
  const TrackerGeometry*  theGeometry;
  const MagneticField*  theMagField;
  const TrackerTopology * theTTopo;
  std::shared_ptr<PropagatorWithMaterial> thePropagator;

  bool rejectOverlaps;
  bool splitHits;
 
  // tokens
  edm::EDGetTokenT<TrajectorySeedCollection> seedCollectionToken;
  edm::EDGetTokenT<FastTMatchedRecHit2DCombinations> recHitCombinationsToken;
  edm::EDGetTokenT<edm::SimVertexContainer> simVtxCollectionToken;
  edm::EDGetTokenT<edm::SimTrackContainer> simTkCollectionToken;
};

#endif
