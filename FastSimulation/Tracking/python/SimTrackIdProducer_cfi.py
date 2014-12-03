import FWCore.ParameterSet.Config as cms

simTrackIdProducer = cms.EDProducer("SimTrackIdProducer",
                                trackCollection = cms.InputTag("iterativeInitialTracks")                
                                )
