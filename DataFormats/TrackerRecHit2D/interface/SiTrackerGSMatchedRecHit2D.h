#ifndef SiTrackerGSMatchedRecHit2D_H
#define SiTrackerGSMatchedRecHit2D_H

#include "DataFormats/TrackerRecHit2D/interface/GSSiTrackerRecHit2DLocalPos.h"
#include "DataFormats/TrackerRecHit2D/interface/SiTrackerGSRecHit2D.h"
class SiTrackerGSRecHit2D;

class SiTrackerGSMatchedRecHit2D : public GSSiTrackerRecHit2DLocalPos{
  
public:
  
  SiTrackerGSMatchedRecHit2D(): GSSiTrackerRecHit2DLocalPos(),
    Id_(),
    simtrackId1_(),
    simtrackId2_(),
    eeId_(),
    cluster_(),  
    pixelMultiplicityAlpha_(), 
    pixelMultiplicityBeta_(),
    isMatched_(), 
    componentMono_(),
    componentStereo_() {}
  
  ~SiTrackerGSMatchedRecHit2D() {}
  
 typedef edm::Ref<FastTrackerClusterCollection, FastTrackerCluster > ClusterRef;
 typedef edm::RefProd<FastTrackerClusterCollection> ClusterRefProd;


  SiTrackerGSMatchedRecHit2D( const LocalPoint&, const LocalError&,
			      GeomDet const & idet,
			      const int Id,
			      const int simtrackId1,			     
			      const int simtrackId2,
			      const uint32_t eeId,
			      ClusterRef const&  cluster,
			      const int pixelMultiplicityX,
			      const int pixelMultiplicityY,
			      const bool isMatched,
			      const SiTrackerGSRecHit2D* rMono, 
			      const SiTrackerGSRecHit2D* rStereo 
			      );  

  SiTrackerGSMatchedRecHit2D( const LocalPoint&, const LocalError&,
			      GeomDet const & idet,
			      const int Id,
			      const int simtrackId1,
			      const int simtrackId2,
			      const uint32_t eeId,
			      ClusterRef const&  cluster,
			      const int pixelMultiplicityX,
			      const int pixelMultiplicityY
			      );  

  virtual SiTrackerGSMatchedRecHit2D * clone() const {SiTrackerGSMatchedRecHit2D * p =  new SiTrackerGSMatchedRecHit2D( * this); p->load(); return p;}
  const int& Id()           const { return Id_;}
  const int& simtrackId1()  const { return simtrackId1_;}
  const int& simtrackId2()  const { return simtrackId2_;}
  const uint32_t& eeId()   const { return eeId_;}
  const int& simMultX()    const { return pixelMultiplicityAlpha_;}
  const int& simMultY()    const { return pixelMultiplicityBeta_;}
  const bool& isMatched()  const { return isMatched_;}
  const SiTrackerGSRecHit2D *monoHit() const { return &componentMono_;}
  const SiTrackerGSRecHit2D *stereoHit() const { return &componentStereo_;}

 ClusterRef const& cluster() const { return cluster_;}
  void setClusterRef(const ClusterRef &ref) { cluster_  = ref; }
  void setId(int Id) {Id_ = Id; }
  void setEeId(uint32_t eeId){eeId_ = eeId;}

  virtual bool sharesInput( const TrackingRecHit* other, SharedInputType what) const;
 
private:
  int Id_;
  int const simtrackId1_;
  int const simtrackId2_;
  uint32_t eeId_;
  ClusterRef cluster_;
  int const pixelMultiplicityAlpha_;
  int const pixelMultiplicityBeta_;
  bool isMatched_;

  SiTrackerGSRecHit2D componentMono_;
  SiTrackerGSRecHit2D componentStereo_;
};

typedef SiTrackerGSMatchedRecHit2D FastTMatchedRecHit2D; //FastT stands for FastSim Tracker
typedef std::vector<FastTMatchedRecHit2D> FastTMatchedRecHit2DCollection;
typedef edm::Ref<FastTMatchedRecHit2DCollection> FastTMatchedRecHit2DRef;
typedef std::vector<FastTMatchedRecHit2DRef> FastTMatchedRecHit2DCombination;  
typedef std::vector<FastTMatchedRecHit2DCombination> FastTMatchedRecHit2DCombinations;  



#endif
