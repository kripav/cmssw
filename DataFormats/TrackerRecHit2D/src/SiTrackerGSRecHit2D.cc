#include "DataFormats/TrackerRecHit2D/interface/SiTrackerGSRecHit2D.h"


SiTrackerGSRecHit2D::SiTrackerGSRecHit2D( const LocalPoint& pos, const LocalError& err,
					  GeomDet const & idet,
					  const int Id               ,
					  const int simtrackId1      ,
					  const int simtrackId2      ,
					  const uint32_t eeId        ,
					  ClusterRef const&  cluster ,
					  const int pixelMultiplicityX = -1,
					  const int pixelMultiplicityY = -1 
					   ): 
  GSSiTrackerRecHit2DLocalPos(pos,err,idet) ,
  Id_(Id) ,
  simtrackId1_(simtrackId1) ,
  simtrackId2_(simtrackId2) ,
  eeId_(eeId) ,
  cluster_(cluster), 
  pixelMultiplicityAlpha_(pixelMultiplicityX), 
  pixelMultiplicityBeta_(pixelMultiplicityY){}
