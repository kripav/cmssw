#ifndef SiTrackerGSRecHit2D_H
#define SiTrackerGSRecHit2D_H

#include "DataFormats/TrackerRecHit2D/interface/GSSiTrackerRecHit2DLocalPos.h"
#include "DataFormats/Common/interface/Ref.h"
#include "FastSimDataFormats/External/interface/FastTrackerClusterCollection.h" 

// typedef edm::Ref<FastTrackerClusterCollection, FastTrackerCluster > ClusterRef;
// typedef edm::RefProd<FastTrackerClusterCollection> ClusterRefProd;

class SiTrackerGSRecHit2D : public GSSiTrackerRecHit2DLocalPos{
  
public:
  
 
  
 SiTrackerGSRecHit2D(): GSSiTrackerRecHit2DLocalPos(),
    Id_(),
    simtrackId1_(),
    simtrackId2_(),
    eeId_(),
    cluster_(),  
    pixelMultiplicityAlpha_(), 
    pixelMultiplicityBeta_() {}
  
  ~SiTrackerGSRecHit2D() {}
  
 typedef edm::Ref<FastTrackerClusterCollection, FastTrackerCluster > ClusterRef;
 typedef edm::RefProd<FastTrackerClusterCollection> ClusterRefProd;


  SiTrackerGSRecHit2D( const LocalPoint&, const LocalError&,
		       GeomDet const & idet,
		       const int Id,
		       const int simtrackId1,
		       const int simtrackId2,
		       const uint32_t eeId,
		       ClusterRef const&  cluster,
		       const int pixelMultiplicityX,
		       const int pixelMultiplicityY);     
  
  virtual SiTrackerGSRecHit2D * clone() const {SiTrackerGSRecHit2D * p = new SiTrackerGSRecHit2D( * this); p->load(); return p;}
  
  const int& Id()          const { return Id_;}
  const int& simtrackId1()   const { return simtrackId1_;}
  const int& simtrackId2()   const { return simtrackId2_;}
  const uint32_t& eeId()   const { return eeId_;}
  const int& simMultX()    const { return pixelMultiplicityAlpha_;}
  const int& simMultY()    const { return pixelMultiplicityBeta_;}

  ClusterRef const& cluster() const { return cluster_;}
  void setClusterRef(const ClusterRef &ref) { cluster_  = ref; }
  void setId(int Id) {Id_ = Id; }
  void setEeId(uint32_t eeId){eeId_ = eeId;}

  virtual bool sharesInput( const TrackingRecHit* other, SharedInputType what) const {return false;}
  
 private:
  int Id_;
  int simtrackId1_;
  int simtrackId2_;
  uint32_t eeId_;
  ClusterRef cluster_;
  int pixelMultiplicityAlpha_;
  int pixelMultiplicityBeta_;
  
};

// Comparison operators
inline bool operator<( const SiTrackerGSRecHit2D& one, const SiTrackerGSRecHit2D& other) {
  if ( one.geographicalId() < other.geographicalId() ) {
    return true;
  } else {
    return false;
  }
}

typedef SiTrackerGSRecHit2D FastTRecHit2D; //FastT stands for FastSim Tracker


typedef SiTrackerGSRecHit2D FastTRecHit2D; //FastT stands for FastSim Tracker
typedef std::vector<FastTRecHit2D> FastTRecHit2DCollection;
typedef edm::Ref<FastTRecHit2DCollection> FastTRecHit2DRef;
typedef std::vector<FastTRecHit2DRef> FastTRecHit2DCombination;  
typedef std::vector<FastTRecHit2DCombination> FastTRecHit2DCombinations;  

#endif
