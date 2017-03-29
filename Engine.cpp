
#include "FemtoDstFormat/BranchWriter.h"
#include "FemtoDstFormat/TClonesArrayWriter.h"

#include "FemtoDstFormat/BranchReader.h"
#include "FemtoDstFormat/TClonesArrayReader.h"


#include "TFile.h"
#include "TTree.h"

#include "TRandom3.h"

#include "FemtoDstFormat/FemtoEvent.h"
#include "FemtoDstFormat/FemtoTrackHelix.h"
#include "FemtoDstFormat/FemtoTrack.h"
#include "FemtoDstFormat/FemtoBTofPidTraits.h"

#define LOGURU_IMPLEMENTATION 1
#include "loguru.h"


int main( int argc, char* argv[] ){
	
	loguru::init(argc, argv);

	TFile *f = new TFile( "FemtoDst.root", "RECREATE" );
	TTree *tree = new TTree( "FemtoDst", "FemtoDst of STAR standard data format" );

	gRandom = new TRandom3();
	gRandom->SetSeed( 1 );

	FemtoBranchWriter<FemtoEvent> few;
	few.createBranch( tree, "Event" );

	// FemtoTracksWriter ftw;
	FemtoTClonesArrayWriter<FemtoTrack> ftw;
	ftw.createBranch( tree, "Tracks" );

	FemtoTClonesArrayWriter<FemtoTrackHelix> fthw;
	fthw.createBranch( tree, "TrackHelices" );

	FemtoTClonesArrayWriter<FemtoBTofPidTraits> fbtofw;
	fbtofw.createBranch( tree, "BTofPidTraits" );


	FemtoEvent event;
	FemtoTrack track;
	FemtoTrackHelix tHelix;
	FemtoBTofPidTraits btof;
	for ( int i = 0; i < 10000; i++ ){
		event.mPrimaryVertex_mX1 = gRandom->Gaus( 0, 1);
		event.mPrimaryVertex_mX2 = gRandom->Gaus( 0, 1);
		event.mPrimaryVertex_mX3 = gRandom->Gaus( 0, 100);
		few.set( event );

		ftw.reset();
		fthw.reset();
		fbtofw.reset();
		// make some tracks
		for ( int j = 0; j < 100; j++ ){
			track.reset();
			track.pMomentum( gRandom->Uniform( -1, 1 ), gRandom->Uniform( -1, 1 ), gRandom->Uniform( -1, 1 ) );

			track.gDCA( gRandom->Gaus( 1.0, 3.0 ) );

			tHelix.mMap0 = 1221434;

			btof.mBTofMatchFlag=2;
			btof.yLocal(gRandom->Uniform( -3.0, 3.0 ));
			btof.zLocal(gRandom->Uniform( -3.0, 3.0 ));

			track.mHelixIndex = fthw.N();
			track.mBTofPidTraitsIndex = fbtofw.N();

			fbtofw.add( btof );
			fthw.add( tHelix );
			ftw.add( track );
		}

		tree->Fill();
	}
	

	f->Write();
	f->Close();
	delete f;
	


	FemtoBranchReader<FemtoEvent> fer;
	FemtoTClonesArrayReader<FemtoTrack> ftr;
	f = new TFile( "FemtoDst.root", "READ" );
	tree = (TTree*)f->Get( "FemtoDst" );
	fer.setup( tree, "Event" );
	ftr.setup( tree, "Tracks" );
	for ( int i = 0; i < tree->GetEntries(); i++ ){
		tree->GetEntry(i);

		LOG_F( INFO, "vZ=%f", fer.get()->mPrimaryVertex_mX3 );
		UInt_t nTracks = ftr.N();
		for ( int j = 0; j < nTracks; j++ ){
			LOG_F( INFO, "Track[%d].gDCA() = %f", j, ftr.get(j)->gDCA() );
		}
	}
	


	f->Close();



	return 0;
}