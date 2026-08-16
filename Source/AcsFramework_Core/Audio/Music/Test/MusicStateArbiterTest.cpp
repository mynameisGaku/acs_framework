// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Audio/Music/MusicStateArbiter.h"
#include "Common/Test/TestHarness.h"

namespace
{
	/** 申告を 1 件作る。 */
	FMusicStateRequest MakeRequest( EMusicState State, EMusicPriority Priority, f32 Intensity = 0.0f ) noexcept
	{
		FMusicStateRequest Request;
		Request.State = State;
		Request.Priority = Priority;
		Request.Intensity = Intensity;
		return Request;
	}
}


void RunMusicStateArbiterTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CMusicStateArbiter / 何も無ければ決まらない" );

	{
		CMusicStateArbiter Arbiter;

		FMusicStateRequest Winner;
		Harness.Check( !Arbiter.ResolveWinner( Winner ), "空なら false" );
		Harness.CheckEqualU64( Arbiter.Num(), 0u, "件数 0" );
	}

	Harness.BeginSuite( "CMusicStateArbiter / 強いほうが勝つ" );

	{
		CMusicStateArbiter Arbiter;
		Arbiter.AddRequest( MakeRequest( EMusicState::Calm, EMusicPriority::Ambient ) );
		Arbiter.AddRequest( MakeRequest( EMusicState::Combat, EMusicPriority::Gameplay ) );

		FMusicStateRequest Winner;
		Harness.Check( Arbiter.ResolveWinner( Winner ), "決まる" );
		Harness.Check( Winner.State == EMusicState::Combat, "Gameplay が Ambient に勝つ" );
	}

	{
		// 並び順に依らないこと (強いほうを先に入れても結果が同じ)。
		CMusicStateArbiter Arbiter;
		Arbiter.AddRequest( MakeRequest( EMusicState::Combat, EMusicPriority::Gameplay ) );
		Arbiter.AddRequest( MakeRequest( EMusicState::Calm, EMusicPriority::Ambient ) );

		FMusicStateRequest Winner;
		Arbiter.ResolveWinner( Winner );
		Harness.Check( Winner.State == EMusicState::Combat, "入れる順を変えても同じ" );
	}

	{
		CMusicStateArbiter Arbiter;
		Arbiter.AddRequest( MakeRequest( EMusicState::Combat, EMusicPriority::Gameplay ) );
		Arbiter.AddRequest( MakeRequest( EMusicState::Victory, EMusicPriority::Cinematic ) );
		Arbiter.AddRequest( MakeRequest( EMusicState::Calm, EMusicPriority::Ambient ) );

		FMusicStateRequest Winner;
		Arbiter.ResolveWinner( Winner );
		Harness.Check( Winner.State == EMusicState::Victory, "Cinematic が一番強い" );
	}

	Harness.BeginSuite( "CMusicStateArbiter / 同じ強さなら後から来たほう" );

	{
		// 直近の状況を優先する取り決め。ここが逆だと、古い申告に引きずられる。
		CMusicStateArbiter Arbiter;
		Arbiter.AddRequest( MakeRequest( EMusicState::Calm, EMusicPriority::Gameplay ) );
		Arbiter.AddRequest( MakeRequest( EMusicState::Tension, EMusicPriority::Gameplay ) );

		FMusicStateRequest Winner;
		Arbiter.ResolveWinner( Winner );
		Harness.Check( Winner.State == EMusicState::Tension, "後勝ち" );
	}

	Harness.BeginSuite( "CMusicStateArbiter / 強さも一緒に運ぶ" );

	{
		CMusicStateArbiter Arbiter;
		Arbiter.AddRequest( MakeRequest( EMusicState::Combat, EMusicPriority::Gameplay, 0.75f ) );

		FMusicStateRequest Winner;
		Arbiter.ResolveWinner( Winner );
		Harness.CheckEqualF32( Winner.Intensity, 0.75f, "Intensity が運ばれる" );
	}

	Harness.BeginSuite( "CMusicStateArbiter / フレームごとに捨てる" );

	{
		CMusicStateArbiter Arbiter;
		Arbiter.AddRequest( MakeRequest( EMusicState::Combat, EMusicPriority::Gameplay ) );
		Harness.CheckEqualU64( Arbiter.Num(), 1u, "溜まる" );

		Arbiter.ClearFrame();
		Harness.CheckEqualU64( Arbiter.Num(), 0u, "捨てられる" );

		FMusicStateRequest Winner;
		Harness.Check( !Arbiter.ResolveWinner( Winner ), "捨てた後は決まらない" );
	}
}
