
#include "FemtoDstFormat/BranchWriter.h"
#include "FemtoDstFormat/TClonesArrayWriter.h"

#include "FemtoDstFormat/BranchReader.h"
#include "FemtoDstFormat/TClonesArrayReader.h"

#include "ChainLoader.h"

#include "TFile.h"
#include "TTree.h"

#include "TRandom3.h"

#include "FemtoDstFormat/FemtoEvent.h"
#include "FemtoDstFormat/FemtoTrackHelix.h"
#include "FemtoDstFormat/FemtoTrack.h"
#include "FemtoDstFormat/FemtoBTofPidTraits.h"
#include "FemtoDstFormat/FemtoMtdPidTraits.h"


#include "CandidateDstFormat/CandidateEvent.h"
#include "CandidateDstFormat/CandidateTrack.h"
#include "CandidateDstFormat/CandidateTrackMtdPidTraits.h"


#define LOGURU_IMPLEMENTATION 1
#include "loguru.h"

using namespace jdb;


void copy_event( CandidateEvent * ein, FemtoEvent * eout ){

	eout->mPrimaryVertex_mX1 = ein->mPrimaryVertex_mX1;
	eout->mPrimaryVertex_mX2 = ein->mPrimaryVertex_mX2;
	eout->mPrimaryVertex_mX3 = ein->mPrimaryVertex_mX3;
	eout->mWeight            = ein->mWeight           ;
	eout->mRunId             = ein->mRunId            ;
	eout->mEventId           = ein->mEventId          ;
	eout->mTriggerWord       = ein->mTriggerWord      ;
	eout->mTriggerWordMtd    = ein->mTriggerWordMtd   ;
	eout->mGRefMult          = ein->mGRefMult         ;
	eout->mPsi2              = ein->mPsi2             ;
	eout->mBin16             = ein->mBin16            ;
	eout->mRunIndex          = ein->mRunIndex         ;
}

void copy_track( CandidateTrack *trackin, FemtoTrack *trackout ){
	trackout->mPt                 = trackin->pMomentum().Pt();
	trackout->mEta                = trackin->pMomentum().Eta();
	trackout->mPhi                = trackin->pMomentum().Phi();
	trackout->mId                 = trackin->mId             ;
	trackout->mDedx               = trackin->mDedx           ;
	trackout->mNHitsFit           = trackin->mNHitsFit       ;
	trackout->mNHitsMax           = trackin->mNHitsMax       ;
	trackout->mNHitsDedx          = trackin->mNHitsDedx      ;
	trackout->mNSigmaPion         = trackin->mNSigmaPion     ;
	trackout->mNSigmaKaon         = trackin->mNSigmaKaon     ;
	trackout->mNSigmaProton       = trackin->mNSigmaProton   ;
	trackout->mNSigmaElectron     = trackin->mNSigmaElectron ;
	trackout->mDCA                = trackin->mDCA            ;
	
	// pidTraits
	trackout->mBTofPidTraitsIndex = trackin->mBTofPidTraitsIndex;
	trackout->mMtdPidTraitsIndex  = trackin->mMtdPidTraitsIndex;
	trackout->mEmcPidTraitsIndex  = -1;
	trackout->mHelixIndex         = trackin->mHelixIndex;
	trackout->mMcIndex            = -1;
}


void copy_mtdpid( CandidateTrackMtdPidTraits * mtdpidin, FemtoMtdPidTraits * mtdpidout ){
	mtdpidout->mDeltaY            = mtdpidin->mDeltaY            ;
	mtdpidout->mDeltaZ            = mtdpidin->mDeltaZ            ;
	mtdpidout->mDeltaTimeOfFlight = mtdpidin->mDeltaTimeOfFlight ;
	mtdpidout->mMatchFlag         = mtdpidin->mMatchFlag         ;
	mtdpidout->mMtdHitChan        = mtdpidin->mMtdHitChan        ;
	mtdpidout->mTriggerFlag       = mtdpidin->mTriggerFlag       ;
	mtdpidout->mIdTruth           = -1;
}


int main( int argc, char* argv[] ){
	
	loguru::init(argc, argv);

	TChain *chain = new TChain( "FemtoDst" );
	ChainLoader::loadList( chain, "./list.lis" );

	size_t totalEvents = chain->GetEntries();
	cout << "# Entries = " << totalEvents << endl;


	BranchReader<CandidateEvent> cer;
	TClonesArrayReader<CandidateTrack> ctr;
	TClonesArrayReader<CandidateTrackMtdPidTraits> cmtd;
	CandidateEvent * cevent = nullptr;
	CandidateTrack * ctrack = nullptr;
	CandidateTrackMtdPidTraits * cmtdpid = nullptr;

	cer.setup( chain, "Event" );
	ctr.setup( chain, "Tracks" );
	cmtd.setup( chain, "MtdPidTraits" );

	TFile *f = new TFile("FemtoDst.root", "RECREATE" );
	f->cd();

	BranchWriter<FemtoEvent> few;
	TClonesArrayWriter<FemtoTrack> ftw;
	TClonesArrayWriter<FemtoMtdPidTraits> fmtdw;
	
	TTree *tree = new TTree( "FemtoDst", "FemtoDst of STAR standard data format" );
	few.createBranch( tree, "Event" );
	ftw.createBranch( tree, "Tracks" );
	fmtdw.createBranch( tree, "MtdPidTraits" );
	
	FemtoEvent event;
	FemtoTrack track;
	FemtoMtdPidTraits mtdpid;

	size_t iEvent = 0;
	while (true){ // what could go wrong 
		Long64_t read = chain->GetEntry(iEvent);
		iEvent++;
		if ( read <= 0 )
			break;
		if ( iEvent >= totalEvents )
			break;

		if ( iEvent % 100000 == 0 ){
			float progress = (iEvent / (float)totalEvents) * 100;
			cout << "\r" << iEvent << " of " << totalEvents << " : " << setprecision(2) << progress << "%" << std::flush;	
		}
		
		
		
		ftw.reset();
		fmtdw.reset();

		track.reset();
		mtdpid.reset();


		cevent = cer.get();
		copy_event( cevent, &event );
		few.set( event );

		size_t nTracks = ctr.N();

		for ( size_t i = 0; i < nTracks; i++ ){
			ctrack = ctr.get( i );
			copy_track( ctrack, &track );

			if ( ctrack->mMtdPidTraitsIndex >= 0 ){
				cmtdpid = cmtd.get( ctrack->mMtdPidTraitsIndex );
				copy_mtdpid( cmtdpid, &mtdpid );
				fmtdw.add( mtdpid );
			}
			else 
				cmtdpid = nullptr;


			ftw.add( track );

		}

		tree->Fill();	
	}

	f->Write();
	return 0;
}