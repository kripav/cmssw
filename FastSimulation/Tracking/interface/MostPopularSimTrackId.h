#ifndef FastSimulation_Tracking_MostPopularSimTrackId_h
#define FastSimulation_Tracking_MostPopularSimTrackId_h

#include <vector>
#include <string>
#include <map>
#include "DataFormats/TrackerRecHit2D/interface/SiTrackerGSMatchedRecHit2D.h"

using namespace std;

/*
write 2 standalone functions:
               getMostPopularSimTrackId(TrackerRecHitCombination) 
               getMostPopularSimTrackId(RecoTrack)

 that returns the most popular SimTrackId over all RecHits in the combination or on the track 
*/


mostPopularSimTrackId(const FastTMatchedRecHit2DCombination TrackerRecHitCombination){

  std::map<unsigned,unsigned> SimTrackIds;

  for (unsigned int i=0; i < TrackerRecHitCombination.size(); ++i){
    const SiTrackerGSMatchedRecHit2D& theRecHit = *TrackerRecHitCombination[i];
 
    // what's the simtrackid of this rechit?
    int currentSimTrackId = theRecHit->simtrackId();
   
    //how do I store the simtrackid in my map?
      SimTrackIds[currentSimTrackId]+=1;
  }

  // loop over every pair inside the map
  for( std::map<unsigned,unsigned>::iterator ii=SimTrackIds.begin(); ii!=SimTrackIds.end(); ++ii)
	   {	     
	     cout << "First Entry: "<<(*ii).first << "    Second Entry: " << (*ii).second << endl;
	     //SimTrackIds.count((*ii).first);
	     
	     
	   }

  // SimTrackIds.begin()->first ; //should be simtrack id
  //SimTrackIds.begin()->second; // should be number of rechits
  return true;
}










//mostPopularSimTrackId(const reco::TrackCollection recoTrack){
//}






