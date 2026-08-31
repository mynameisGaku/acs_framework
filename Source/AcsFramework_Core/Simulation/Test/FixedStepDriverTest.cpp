// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/DeterministicRandom.h"
#include "AcsFramework_Core/Simulation/FixedStepDriver.h"
#include "Common/Test/TestHarness.h"

#include <cmath>
#include <limits>


void RunFixedStepDriverTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CFixedStepDriver / 既定で回る" );

	{
		// 設定を忘れたまま 0 ステップしか進まないと、原因が時計にあると気付きにくい。
		CFixedStepDriver Driver;

		Harness.CheckNearF32( Driver.GetStepSeconds(), 1.0f / 60.0f, 0.0001f, "既定は 1/60 秒" );
		Harness.Check( Driver.Advance( 1.0 / 60.0 ) >= 1u, "設定しなくても進む" );
	}

	Harness.BeginSuite( "CFixedStepDriver / 実時間をステップへ割り直す" );

	{
		CFixedStepDriver Driver;
		Harness.Check( Driver.Configure( 0.1, 8u ), "設定できる" );

		Harness.CheckEqualU64( Driver.Advance( 0.05 ), 0u, "半分では進まない" );
		Harness.CheckEqualU64( Driver.Advance( 0.05 ), 1u, "溜まると 1 ステップ" );
		Harness.CheckEqualU64( Driver.Advance( 0.35 ), 3u, "溜まったぶんだけまとめて" );

		Harness.CheckEqualU64( Driver.GetTick(), 0u, "進めたのは呼ぶ側なので、まだ 0" );

		Driver.AdvanceTick();
		Driver.AdvanceTick();
		Harness.CheckEqualU64( Driver.GetTick(), 2u, "進めたぶんだけ数える" );
	}

	Harness.BeginSuite( "CFixedStepDriver / 上限で頭打ちにする" );

	{
		// 重い 1 フレームを «取り戻そう» として更に重くなるのを避ける歯止め。
		CFixedStepDriver Driver;
		Driver.Configure( 0.01, 4u );

		const u32 Steps = Driver.Advance( 10.0 );

		Harness.CheckEqualU64( Steps, 4u, "上限で止まる" );
		Harness.Check( Driver.WasClamped(), "頭打ちになったと分かる" );
		Harness.Check( Driver.GetDroppedSeconds() > 0.0, "捨てた秒数が残る" );

		Driver.Advance( 0.0 );
		Harness.Check( !Driver.WasClamped(), "次に普通の dt を渡せば戻る" );
	}

	Harness.BeginSuite( "CFixedStepDriver / 受け付けない設定" );

	{
		CFixedStepDriver Driver;

		Harness.Check( !Driver.Configure( 0.0, 8u ), "0 秒は受け付けない" );
		Harness.Check( !Driver.Configure( -1.0, 8u ), "負の秒も受け付けない" );
		Harness.Check( !Driver.Configure( 0.016, 0u ), "上限 0 は受け付けない" );

		Harness.CheckNearF32( Driver.GetStepSeconds(), 1.0f / 60.0f, 0.0001f, "弾いた設定は反映されない" );
	}

	Harness.BeginSuite( "CFixedStepDriver / 補間位置と巻き戻し" );

	{
		CFixedStepDriver Driver;
		Driver.Configure( 0.1, 8u );

		Driver.Advance( 0.05 );
		const f32 Alpha = Driver.GetAlpha();
		Harness.Check( Alpha >= 0.0f && Alpha <= 1.0f, "0..1 に収まる" );

		Driver.AdvanceTick();
		Driver.AdvanceTick();

		FFixedStepClockSnapshot State{};
		u32 Tick = 0u;
		Harness.Check( Driver.TryCaptureSnapshot( State, Tick ), "写せる" );
		Harness.CheckEqualU64( Tick, 2u, "ティックも写る" );

		Driver.Advance( 0.5 );
		Driver.AdvanceTick();

		Harness.Check( Driver.TryRestoreSnapshot( State, Tick ), "戻せる" );
		Harness.CheckEqualU64( Driver.GetTick(), 2u, "ティックが戻る" );

		Driver.Reset();
		Harness.CheckEqualU64( Driver.GetTick(), 0u, "初期化できる" );
	}

	Harness.BeginSuite( "CDeterministicRandom / 種と引いた回数" );

	{
		CDeterministicRandom A;
		CDeterministicRandom B;

		A.Reseed( 12345u );
		B.Reseed( 12345u );

		Harness.CheckEqualU64( A.GetSeed(), 12345u, "種を覚えている" );
		Harness.CheckEqualU64( A.GetDrawCount(), 0u, "蒔き直すと 0 から" );

		bool bSame = true;
		for ( u32 Index = 0u; Index < 32u; ++Index )
		{
			if ( A.NextU32() != B.NextU32() ) bSame = false;
		}

		Harness.Check( bSame, "同じ種なら同じ並び" );
		Harness.CheckEqualU64( A.GetDrawCount(), 32u, "引いた回数を数えている" );

		CDeterministicRandom C;
		C.Reseed( 12346u );
		Harness.Check( C.NextU32() != A.NextU32() || C.NextU32() != A.NextU32(), "種が違えば並びも違う" );
	}

	Harness.BeginSuite( "CDeterministicRandom / 写して戻す" );

	{
		CDeterministicRandom Random;
		Random.Reseed( 777u );

		for ( u32 Index = 0u; Index < 5u; ++Index ) Random.NextU32();

		FRandomSnapshot State{};
		u64 DrawCount = 0u;
		Random.CaptureSnapshot( State, DrawCount );
		Harness.CheckEqualU64( DrawCount, 5u, "そのときの回数も写る" );

		const u32 Expected = Random.NextU32();

		for ( u32 Index = 0u; Index < 10u; ++Index ) Random.NextU32();

		Harness.Check( Random.TryRestoreSnapshot( State, DrawCount ), "戻せる" );
		Harness.CheckEqualU64( Random.GetDrawCount(), 5u, "回数も戻る" );
		Harness.CheckEqualU64( Random.NextU32(), Expected, "続きの出目が一致する" );
	}

	Harness.BeginSuite( "CDeterministicRandom / 範囲" );

	{
		CDeterministicRandom Random;
		Random.Reseed( 2026u );

		bool bInUnit = true;
		bool bInRange = true;
		bool bInIntRange = true;

		for ( u32 Index = 0u; Index < 200u; ++Index )
		{
			const f32 Unit = Random.NextUnitFloat();
			if ( Unit < 0.0f || Unit > 1.0f ) bInUnit = false;

			const f32 Ranged = Random.NextRangeFloat( -2.0f, 5.0f );
			if ( Ranged < -2.0f || Ranged > 5.0f ) bInRange = false;

			const i32 Integer = Random.NextRangeInt( 3, 7 );
			if ( Integer < 3 || Integer > 7 ) bInIntRange = false;
		}

		Harness.Check( bInUnit, "0..1 に収まる" );
		Harness.Check( bInRange, "指定した範囲に収まる" );
		Harness.Check( bInIntRange, "整数も範囲に収まる" );
		Harness.CheckEqualU64( Random.GetDrawCount(), 600u, "引いた回数が合う" );
	}

	Harness.BeginSuite( "CDeterministicRandom / 成功確率を判定する" );

	{
		constexpr usize kSampleCount = 4096u;
		/** 32bit全出目の先頭25%に当たる排他的上限。 */
		constexpr u64 kQuarterThreshold = 1ull << 30u;
		CDeterministicRandom Random;
		CDeterministicRandom Reference;
		Random.Reseed( 20260831u );
		Reference.Reseed( 20260831u );
		usize SuccessCount = 0u;
		bool bMatchesReference = true;

		for ( usize SampleIndex = 0u; SampleIndex < kSampleCount;
			++SampleIndex )
		{
			bool bOccurred = false;
			const bool bExpected = static_cast<u64>( Reference.NextU32() )
				< kQuarterThreshold;
			if ( !Random.TryChance( 0.25f, bOccurred )
				|| bOccurred != bExpected ) bMatchesReference = false;
			if ( bOccurred ) ++SuccessCount;
		}

		Harness.Check( bMatchesReference,
			"25%を32bit出目の先頭4分の1へ正確に割り当てる" );
		Harness.Check( SuccessCount > 900u && SuccessCount < 1150u,
			"十分な回数では成功数が指定確率の近くへ集まる" );
		Harness.CheckEqualU64( Random.GetDrawCount(), kSampleCount,
			"途中確率1回につき32bit乱数を1個進める" );
	}

	{
		const f32 InvalidProbabilities[] = {
			-0.01f,
			1.01f,
			std::numeric_limits<f32>::quiet_NaN(),
			std::numeric_limits<f32>::infinity() };
		CDeterministicRandom Random;
		CDeterministicRandom Expected;
		Random.Reseed( 42u );
		Expected.Reseed( 42u );
		bool bOccurred = true;
		const bool bZeroAccepted = Random.TryChance( 0.0f, bOccurred )
			&& !bOccurred;
		const bool bOneAccepted = Random.TryChance( 1.0f, bOccurred )
			&& bOccurred;
		bool bInvalidRejectedAtomically = true;

		for ( usize ProbabilityIndex = 0u;
			ProbabilityIndex < sizeof( InvalidProbabilities )
				/ sizeof( InvalidProbabilities[0] );
			++ProbabilityIndex )
		{
			bOccurred = ( ProbabilityIndex % 2u ) == 0u;
			const bool bBefore = bOccurred;
			if ( Random.TryChance( InvalidProbabilities[ProbabilityIndex],
				bOccurred ) || bOccurred != bBefore )
			{
				bInvalidRejectedAtomically = false;
			}
		}

		Harness.Check( bZeroAccepted && bOneAccepted,
			"0と1は乱数を使わず確定結果にする" );
		Harness.Check( bInvalidRejectedAtomically,
			"範囲外と非有限の確率では出力を変えない" );
		Harness.CheckEqualU64( Random.GetDrawCount(), 0u,
			"確定結果と拒否では乱数位置を進めない" );
		Harness.CheckEqualU64( Random.NextU32(), Expected.NextU32(),
			"確定結果と拒否後も次の乱数値が変わらない" );
	}

	{
		constexpr usize kReplayCount = 64u;
		CDeterministicRandom Random;
		Random.Reseed( 271828u );
		bool bWarmupOccurred = false;
		Random.TryChance( 0.4f, bWarmupOccurred );
		FRandomSnapshot Snapshot{};
		u64 SnapshotDrawCount = 0u;
		Random.CaptureSnapshot( Snapshot, SnapshotDrawCount );
		bool Expected[kReplayCount]{};
		for ( usize Index = 0u; Index < kReplayCount; ++Index )
		{
			Random.TryChance( 0.4f, Expected[Index] );
		}

		const bool bRestored = Random.TryRestoreSnapshot(
			Snapshot, SnapshotDrawCount );
		bool bReplayed = bRestored;
		for ( usize Index = 0u; Index < kReplayCount; ++Index )
		{
			bool bOccurred = false;
			if ( !Random.TryChance( 0.4f, bOccurred )
				|| bOccurred != Expected[Index] ) bReplayed = false;
		}

		Harness.Check( SnapshotDrawCount == 1u,
			"確率判定後の乱数位置を写す" );
		Harness.Check( bReplayed,
			"復元後に同じ確率判定列を再生する" );
		Harness.CheckEqualU64( Random.GetDrawCount(),
			SnapshotDrawCount + kReplayCount,
			"復元後の確率判定でも消費数が一致する" );
	}

	Harness.BeginSuite( "CDeterministicRandom / 均等な添字抽選" );

	{
		CDeterministicRandom A;
		CDeterministicRandom B;
		A.Reseed( 20260831u );
		B.Reseed( 20260831u );
		bool bSameAndInRange = true;
		for ( usize Draw = 0u; Draw < 128u; ++Draw )
		{
			usize AIndex = 99u;
			usize BIndex = 99u;
			if ( !A.TryChooseIndex( 7u, AIndex )
				|| !B.TryChooseIndex( 7u, BIndex )
				|| AIndex != BIndex || AIndex >= 7u ) bSameAndInRange = false;
		}

		Harness.Check( bSameAndInRange,
			"同じ種と件数から同じ範囲内の添字を選ぶ" );
		Harness.CheckEqualU64( A.GetDrawCount(), 128u,
			"棄却のない小さい件数では成功ごとに1個進める" );
	}

	{
		constexpr usize kWideItemCount =
			static_cast<usize>( 0x80000001u );
		constexpr u64 kSourceValueCount = 1ull << 32u;
		const u64 AcceptanceLimit = kSourceValueCount
			- ( kSourceValueCount % static_cast<u64>( kWideItemCount ) );
		CDeterministicRandom SeedFinder;
		u64 RejectionSeed = 0u;
		for ( u64 CandidateSeed = 1u; CandidateSeed <= 1024u;
			++CandidateSeed )
		{
			SeedFinder.Reseed( CandidateSeed );
			if ( static_cast<u64>( SeedFinder.NextU32() ) >= AcceptanceLimit )
			{
				RejectionSeed = CandidateSeed;
				break;
			}
		}

		CDeterministicRandom Reference;
		Reference.Reseed( RejectionSeed );
		u32 AcceptedSample = 0u;
		u64 ExpectedDrawCount = 0u;
		do
		{
			AcceptedSample = Reference.NextU32();
			++ExpectedDrawCount;
		}
		while ( static_cast<u64>( AcceptedSample ) >= AcceptanceLimit );
		const usize ExpectedIndex = static_cast<usize>(
			static_cast<u64>( AcceptedSample ) % kWideItemCount );

		CDeterministicRandom Random;
		Random.Reseed( RejectionSeed );
		usize Index = 99u;
		const bool bChosen = Random.TryChooseIndex( kWideItemCount, Index );
		Harness.Check( RejectionSeed != 0u && ExpectedDrawCount > 1u,
			"広い件数で棄却が起きる種を固定範囲から見つける" );
		Harness.Check( bChosen && Index == ExpectedIndex,
			"棄却後の最初の受理値を均等な添字へ変える" );
		Harness.CheckEqualU64( Random.GetDrawCount(), ExpectedDrawCount,
			"棄却した32bit乱数も消費数へ含める" );
	}

	{
		CDeterministicRandom Random;
		CDeterministicRandom Expected;
		Random.Reseed( 42u );
		Expected.Reseed( 42u );
		usize Index = 77u;
		constexpr usize kTooManyItems =
			static_cast<usize>( 0xffffffffu ) + 1u;

		const bool bRejected = !Random.TryChooseIndex( 0u, Index )
			&& !Random.TryChooseIndex( kTooManyItems, Index );
		usize SingleIndex = 99u;
		const bool bSingle = Random.TryChooseIndex( 1u, SingleIndex );
		Harness.Check( bRejected && Index == 77u
			&& bSingle && SingleIndex == 0u,
			"空と過大件数を拒否し、1件を乱数なしで選ぶ" );
		Harness.CheckEqualU64( Random.GetDrawCount(), 0u,
			"選べない件数と1件では乱数を進めない" );
		Harness.CheckEqualU64( Random.NextU32(), Expected.NextU32(),
			"拒否後も次の乱数値が変わらない" );
	}

	Harness.BeginSuite( "CDeterministicRandom / 重み付き抽選" );

	{
		constexpr f32 Weights[] = { 1.0f, 3.0f, 6.0f };
		CDeterministicRandom A;
		CDeterministicRandom Reference;
		A.Reseed( 20260831u );
		Reference.Reseed( 20260831u );

		bool bExpectedSequence = true;
		bool bInRange = true;
		for ( u32 Draw = 0u; Draw < 128u; ++Draw )
		{
			usize AIndex = 99u;
			const u64 RandomBits =
				( static_cast<u64>( Reference.NextU32() ) << 21u )
				| static_cast<u64>( Reference.NextU32() >> 11u );
			constexpr f64 kUnitDivisor = 9007199254740992.0;
			const f64 Unit = static_cast<f64>( RandomBits ) / kUnitDivisor;
			const usize ExpectedIndex = Unit < 0.1 ? 0u : ( Unit < 0.4 ? 1u : 2u );
			if ( !A.TryChooseWeightedIndex( Weights, 3u, AIndex )
				|| AIndex != ExpectedIndex ) bExpectedSequence = false;
			if ( AIndex >= 3u ) bInRange = false;
		}

		Harness.Check( bExpectedSequence,
			"同じ種の単位乱数を重み区間へ割り当てる" );
		Harness.Check( bInRange, "重み配列の範囲に収まる" );
		Harness.CheckEqualU64( A.GetDrawCount(), 256u,
			"成功1回につき32bit乱数を2個進める" );
	}

	{
		constexpr f32 SinglePositive[] = { 0.0f, 0.0f, 2.0f, 0.0f };
		CDeterministicRandom Random;
		Random.Reseed( 9u );

		bool bOnlyPositiveSelected = true;
		for ( u32 Draw = 0u; Draw < 32u; ++Draw )
		{
			usize Index = 99u;
			if ( !Random.TryChooseWeightedIndex( SinglePositive, 4u, Index )
				|| Index != 2u ) bOnlyPositiveSelected = false;
		}

		Harness.Check( bOnlyPositiveSelected, "0の項目を選ばない" );
	}

	{
		const f32 Negative[] = { 1.0f, -1.0f };
		const f32 NotFinite[] = {
			1.0f, std::numeric_limits<f32>::infinity() };
		const f32 NotANumber[] = {
			1.0f, std::numeric_limits<f32>::quiet_NaN() };
		const f32 AllZero[] = { 0.0f, 0.0f };
		CDeterministicRandom Random;
		CDeterministicRandom Expected;
		Random.Reseed( 42u );
		Expected.Reseed( 42u );
		usize Index = 77u;

		const bool bRejected =
			!Random.TryChooseWeightedIndex( nullptr, 2u, Index )
			&& !Random.TryChooseWeightedIndex( AllZero, 0u, Index )
			&& !Random.TryChooseWeightedIndex( Negative, 2u, Index )
			&& !Random.TryChooseWeightedIndex( NotFinite, 2u, Index )
			&& !Random.TryChooseWeightedIndex( NotANumber, 2u, Index )
			&& !Random.TryChooseWeightedIndex( AllZero, 2u, Index );

		Harness.Check( bRejected && Index == 77u,
			"空、負、非有限、全0を出力変更なしで拒否する" );
		Harness.CheckEqualU64( Random.GetDrawCount(), 0u,
			"不正入力では乱数を進めない" );
		Harness.CheckEqualU64( Random.NextU32(), Expected.NextU32(),
			"拒否後も次の乱数値が変わらない" );
	}

	Harness.BeginSuite( "CDeterministicRandom / 重み付き抽選を写して戻す" );

	{
		constexpr f32 Weights[] = { 2.0f, 5.0f, 3.0f };
		CDeterministicRandom Random;
		Random.Reseed( 314159u );

		usize WarmupIndex = 0u;
		for ( u32 Draw = 0u; Draw < 3u; ++Draw )
		{
			Random.TryChooseWeightedIndex( Weights, 3u, WarmupIndex );
		}

		FRandomSnapshot Snapshot{};
		u64 SnapshotDrawCount = 0u;
		Random.CaptureSnapshot( Snapshot, SnapshotDrawCount );
		constexpr usize kExpectedCount = 16u;
		usize ExpectedIndices[kExpectedCount]{};
		bool bExpectedCreated = true;
		for ( usize Index = 0u; Index < kExpectedCount; ++Index )
		{
			if ( !Random.TryChooseWeightedIndex(
					Weights, 3u, ExpectedIndices[Index] ) ) bExpectedCreated = false;
		}

		const bool bRestored =
			Random.TryRestoreSnapshot( Snapshot, SnapshotDrawCount );
		bool bReplayed = bRestored;
		for ( usize Index = 0u; Index < kExpectedCount; ++Index )
		{
			usize ReplayedIndex = 99u;
			if ( !Random.TryChooseWeightedIndex( Weights, 3u, ReplayedIndex )
				|| ReplayedIndex != ExpectedIndices[Index] ) bReplayed = false;
		}

		Harness.Check( bExpectedCreated && SnapshotDrawCount == 6u,
			"抽選後の再生位置を32bit消費数で写す" );
		Harness.Check( bReplayed,
			"復元後に同じ重み付き抽選列を再生する" );
		Harness.CheckEqualU64( Random.GetDrawCount(),
			SnapshotDrawCount + kExpectedCount * 2u,
			"復元後の抽選でも消費数が一致する" );
	}

	Harness.BeginSuite( "CDeterministicRandom / 配列を偏りなく並べ替える" );

	{
		constexpr usize kItemCount = 8u;
		u32 A[kItemCount] = { 0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u };
		u32 B[kItemCount] = { 0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u };
		CDeterministicRandom RandomA;
		CDeterministicRandom RandomB;
		RandomA.Reseed( 20260831u );
		RandomB.Reseed( 20260831u );

		const bool bShuffledA = RandomA.TryShuffle( A, kItemCount );
		const bool bShuffledB = RandomB.TryShuffle( B, kItemCount );
		bool bSame = bShuffledA && bShuffledB;
		bool bChanged = false;
		bool bPermutation = true;
		bool bSeen[kItemCount]{};
		for ( usize Index = 0u; Index < kItemCount; ++Index )
		{
			if ( A[Index] != B[Index] ) bSame = false;
			if ( A[Index] != Index ) bChanged = true;
			if ( A[Index] >= kItemCount || bSeen[A[Index]] )
			{
				bPermutation = false;
				continue;
			}
			bSeen[A[Index]] = true;
		}

		Harness.Check( bSame, "同じ種と項目数なら同じ並びになる" );
		Harness.Check( bChanged && bPermutation,
			"全項目を重複や欠落なしで並べ替える" );
		Harness.CheckEqualU64( RandomA.GetDrawCount(), 7u,
			"棄却のない8項目では32bit乱数を7個進める" );
	}

	{
		CDeterministicRandom Random;
		Random.Reseed( 314159u );
		u32 Warmup[] = { 0u, 1u, 2u };
		Random.TryShuffle( Warmup, 3u );

		FRandomSnapshot Snapshot{};
		u64 SnapshotDrawCount = 0u;
		Random.CaptureSnapshot( Snapshot, SnapshotDrawCount );
		u32 Expected[] = { 10u, 20u, 30u, 40u, 50u, 60u };
		u32 Replayed[] = { 10u, 20u, 30u, 40u, 50u, 60u };
		const bool bExpected = Random.TryShuffle( Expected, 6u );
		const u64 ExpectedDrawCount = Random.GetDrawCount();
		const bool bRestored = Random.TryRestoreSnapshot(
			Snapshot, SnapshotDrawCount );
		const bool bReplayed = Random.TryShuffle( Replayed, 6u );
		bool bSame = bExpected && bRestored && bReplayed;
		for ( usize Index = 0u; Index < 6u; ++Index )
		{
			if ( Expected[Index] != Replayed[Index] ) bSame = false;
		}

		Harness.Check( bSame,
			"途中状態へ戻すと同じ並べ替えを再生する" );
		Harness.CheckEqualU64( Random.GetDrawCount(), ExpectedDrawCount,
			"復元後の並べ替えでも消費数が一致する" );
	}

	{
		u32 Items[] = { 4u, 5u, 6u };
		CDeterministicRandom Random;
		CDeterministicRandom Expected;
		Random.Reseed( 42u );
		Expected.Reseed( 42u );
		constexpr usize kTooManyItems =
			static_cast<usize>( 0xffffffffu ) + 1u;

		const bool bRejected =
			!Random.TryShuffle<u32>( nullptr, 1u )
			&& !Random.TryShuffle( Items, kTooManyItems );
		const bool bEmptyAccepted = Random.TryShuffle<u32>( nullptr, 0u );
		const bool bSingleAccepted = Random.TryShuffle( Items, 1u );

		Harness.Check( bRejected && bEmptyAccepted && bSingleAccepted,
			"null、過大件数を拒否し、空と1件を乱数なしで受理する" );
		Harness.Check( Items[0] == 4u && Items[1] == 5u && Items[2] == 6u,
			"拒否または1件では配列を変えない" );
		Harness.CheckEqualU64( Random.GetDrawCount(), 0u,
			"並べ替えない入力では乱数を進めない" );
		Harness.CheckEqualU64( Random.NextU32(), Expected.NextU32(),
			"拒否後も次の乱数値が変わらない" );
	}

	Harness.BeginSuite( "CDeterministicRandom / 3D箱内部を各軸へ均等に選ぶ" );

	{
		constexpr usize kSampleCount = 4096u;
		/** 各軸で異なる範囲を確認する箱の半寸法。 */
		const FVec3 HalfExtents{ 2.0f, 3.0f, 4.0f };
		CDeterministicRandom A;
		CDeterministicRandom B;
		A.Reseed( 20260831u );
		B.Reseed( 20260831u );
		bool bSameAndInside = true;
		f64 SumX = 0.0;
		f64 SumY = 0.0;
		f64 SumZ = 0.0;
		f64 SumNormalizedX2 = 0.0;
		f64 SumNormalizedY2 = 0.0;
		f64 SumNormalizedZ2 = 0.0;
		f64 SumNormalizedXY = 0.0;
		f64 SumNormalizedXZ = 0.0;
		f64 SumNormalizedYZ = 0.0;
		/** 3軸それぞれを8等分した度数。 */
		u32 AxisBinCounts[3][8]{};
		for ( usize SampleIndex = 0u; SampleIndex < kSampleCount;
			++SampleIndex )
		{
			FVec3 PointA{};
			FVec3 PointB{};
			if ( !A.TryPointInBox3D( HalfExtents, PointA )
				|| !B.TryPointInBox3D( HalfExtents, PointB )
				|| !std::isfinite( PointA.x ) || !std::isfinite( PointA.y )
				|| !std::isfinite( PointA.z )
				|| PointA.x != PointB.x || PointA.y != PointB.y
				|| PointA.z != PointB.z
				|| std::abs( PointA.x ) > HalfExtents.x
				|| std::abs( PointA.y ) > HalfExtents.y
				|| std::abs( PointA.z ) > HalfExtents.z )
			{
				bSameAndInside = false;
				continue;
			}

			SumX += PointA.x;
			SumY += PointA.y;
			SumZ += PointA.z;
			const f64 NormalizedX = static_cast<f64>( PointA.x )
				/ HalfExtents.x;
			const f64 NormalizedY = static_cast<f64>( PointA.y )
				/ HalfExtents.y;
			const f64 NormalizedZ = static_cast<f64>( PointA.z )
				/ HalfExtents.z;
			SumNormalizedX2 += NormalizedX * NormalizedX;
			SumNormalizedY2 += NormalizedY * NormalizedY;
			SumNormalizedZ2 += NormalizedZ * NormalizedZ;
			SumNormalizedXY += NormalizedX * NormalizedY;
			SumNormalizedXZ += NormalizedX * NormalizedZ;
			SumNormalizedYZ += NormalizedY * NormalizedZ;
			/** 端点1を末尾へ収めるX軸の度数位置。 */
			usize BinX = static_cast<usize>( ( NormalizedX + 1.0 ) * 4.0 );
			/** 端点1を末尾へ収めるY軸の度数位置。 */
			usize BinY = static_cast<usize>( ( NormalizedY + 1.0 ) * 4.0 );
			/** 端点1を末尾へ収めるZ軸の度数位置。 */
			usize BinZ = static_cast<usize>( ( NormalizedZ + 1.0 ) * 4.0 );
			if ( BinX >= 8u ) BinX = 7u;
			if ( BinY >= 8u ) BinY = 7u;
			if ( BinZ >= 8u ) BinZ = 7u;
			++AxisBinCounts[0][BinX];
			++AxisBinCounts[1][BinY];
			++AxisBinCounts[2][BinZ];
		}

		const f64 InverseCount = 1.0 / static_cast<f64>( kSampleCount );
		/** 各軸の各binが期待値から十分近いか。 */
		bool bBinsBalanced = true;
		constexpr u32 kExpectedBinCount =
			static_cast<u32>( kSampleCount / 8u );
		constexpr u32 kBinTolerance = 96u;
		for ( usize AxisIndex = 0u; AxisIndex < 3u; ++AxisIndex )
		{
			for ( usize BinIndex = 0u; BinIndex < 8u; ++BinIndex )
			{
				const u32 BinCount = AxisBinCounts[AxisIndex][BinIndex];
				if ( BinCount < kExpectedBinCount - kBinTolerance
					|| BinCount > kExpectedBinCount + kBinTolerance )
				{
					bBinsBalanced = false;
				}
			}
		}
		Harness.Check( bSameAndInside,
			"同じ種で同じ箱内部位置を各軸範囲内に選ぶ" );
		Harness.Check( std::abs( SumX * InverseCount ) < 0.08
			&& std::abs( SumY * InverseCount ) < 0.10
			&& std::abs( SumZ * InverseCount ) < 0.12,
			"箱内部位置の平均が中心付近へ戻る" );
		Harness.Check( std::abs(
				SumNormalizedX2 * InverseCount - ( 1.0 / 3.0 ) ) < 0.03
			&& std::abs(
				SumNormalizedY2 * InverseCount - ( 1.0 / 3.0 ) ) < 0.03
			&& std::abs(
				SumNormalizedZ2 * InverseCount - ( 1.0 / 3.0 ) ) < 0.03,
			"3軸を独立した一様範囲として選ぶ" );
		Harness.Check( std::abs( SumNormalizedXY * InverseCount ) < 0.03
			&& std::abs( SumNormalizedXZ * InverseCount ) < 0.03
			&& std::abs( SumNormalizedYZ * InverseCount ) < 0.03,
			"3軸の組へ相関を残さない" );
		Harness.Check( bBinsBalanced,
			"各軸の8区間へ過度な偏りなく分布する" );
		Harness.CheckEqualU64( A.GetDrawCount(), kSampleCount * 3u,
			"箱内部1点につき32bit乱数を3個進める" );
	}

	Harness.BeginSuite( "CDeterministicRandom / 3D箱抽選を安全に復元する" );

	{
		CDeterministicRandom Random;
		Random.Reseed( 271828u );
		FVec3 Warmup{};
		Random.TryPointInBox3D( FVec3{ 1.0f, 0.0f, 2.0f }, Warmup );
		FRandomSnapshot Snapshot{};
		u64 SnapshotDrawCount = 0u;
		Random.CaptureSnapshot( Snapshot, SnapshotDrawCount );
		constexpr usize kReplayCount = 64u;
		FVec3 ExpectedPoints[kReplayCount]{};
		for ( usize Index = 0u; Index < kReplayCount; ++Index )
		{
			Random.TryPointInBox3D(
				FVec3{ 2.0f, 3.0f, 4.0f }, ExpectedPoints[Index] );
		}

		const bool bRestored = Random.TryRestoreSnapshot(
			Snapshot, SnapshotDrawCount );
		bool bReplayed = bRestored;
		for ( usize Index = 0u; Index < kReplayCount; ++Index )
		{
			FVec3 Point{};
			if ( !Random.TryPointInBox3D(
					FVec3{ 2.0f, 3.0f, 4.0f }, Point )
				|| Point.x != ExpectedPoints[Index].x
				|| Point.y != ExpectedPoints[Index].y
				|| Point.z != ExpectedPoints[Index].z )
			{
				bReplayed = false;
			}
		}

		Harness.CheckEqualU64( SnapshotDrawCount, 3u,
			"退化軸を含む箱抽選後も3個の消費数を写す" );
		Harness.Check( bReplayed,
			"途中状態へ戻すと同じ箱内部位置列を再生する" );
		Harness.CheckEqualU64( Random.GetDrawCount(),
			SnapshotDrawCount + kReplayCount * 3u,
			"復元後の箱抽選でも消費数が一致する" );
	}

	{
		/** 不正値で出力を変えないことを確認する乱数。 */
		CDeterministicRandom Random;
		/** 拒否後の乱数位置を比較する同じ種の乱数。 */
		CDeterministicRandom Expected;
		Random.Reseed( 42u );
		Expected.Reseed( 42u );
		/** 失敗時に保持する出力値。 */
		FVec3 Point{ 7.0f, 8.0f, 9.0f };
		/** 各成分へ混ぜる非数値。 */
		const f32 NotANumber =
			std::numeric_limits<f32>::quiet_NaN();
		/** 各成分へ混ぜる無限値。 */
		const f32 Infinity = std::numeric_limits<f32>::infinity();
		const bool bRejected =
			!Random.TryPointInBox3D( FVec3{ -1.0f, 2.0f, 3.0f }, Point )
			&& !Random.TryPointInBox3D(
				FVec3{ 1.0f, NotANumber, 3.0f }, Point )
			&& !Random.TryPointInBox3D(
				FVec3{ 1.0f, 2.0f, Infinity }, Point );
		Harness.Check( bRejected && Point.x == 7.0f
			&& Point.y == 8.0f && Point.z == 9.0f,
			"負または非有限の半寸法を出力変更なしで拒否する" );
		Harness.CheckEqualU64( Random.GetDrawCount(), 0u,
			"不正半寸法では乱数を進めない" );

		/** 全辺0から原点へ正規化する出力値。 */
		FVec3 ZeroPoint{ 1.0f, 2.0f, 3.0f };
		Harness.Check( Random.TryPointInBox3D( FVec3{}, ZeroPoint )
			&& ZeroPoint.x == 0.0f && ZeroPoint.y == 0.0f
			&& ZeroPoint.z == 0.0f,
			"全辺0は乱数なしで原点を返す" );
		Harness.CheckEqualU64( Random.GetDrawCount(), 0u,
			"全辺0でも乱数を進めない" );
		Harness.CheckEqualU64( Random.NextU32(), Expected.NextU32(),
			"拒否と全辺0の後も次の乱数値が変わらない" );
	}

	{
		/** 面へ退化した箱と最大有限値を確認する乱数。 */
		CDeterministicRandom Random;
		Random.Reseed( 314159u );
		/** Y軸の幅だけ0にした箱内部位置。 */
		FVec3 DegeneratePoint{};
		const bool bDegenerate = Random.TryPointInBox3D(
			FVec3{ 2.0f, 0.0f, 4.0f }, DegeneratePoint );
		Harness.Check( bDegenerate && std::abs( DegeneratePoint.x ) <= 2.0f
			&& DegeneratePoint.y == 0.0f
			&& std::abs( DegeneratePoint.z ) <= 4.0f,
			"退化した軸を0へ固定して残り2軸へ散らす" );
		Harness.CheckEqualU64( Random.GetDrawCount(), 3u,
			"退化軸を含んでも箱抽選の消費数を固定する" );

		/** 最大有限半寸法の箱内部位置。 */
		FVec3 HugePoint{};
		const f32 HugeExtent = std::numeric_limits<f32>::max();
		const bool bHuge = Random.TryPointInBox3D(
			FVec3{ HugeExtent, HugeExtent, HugeExtent }, HugePoint );
		Harness.Check( bHuge && std::isfinite( HugePoint.x )
			&& std::isfinite( HugePoint.y ) && std::isfinite( HugePoint.z )
			&& std::abs( HugePoint.x ) <= HugeExtent
			&& std::abs( HugePoint.y ) <= HugeExtent
			&& std::abs( HugePoint.z ) <= HugeExtent,
			"最大有限半寸法でもoverflowせず箱内部へ収める" );
		Harness.CheckEqualU64( Random.GetDrawCount(), 6u,
			"連続した2回の箱抽選で6個の乱数を進める" );
	}

	Harness.BeginSuite( "CDeterministicRandom / 3D球面を均等に選ぶ" );

	{
		constexpr usize kSampleCount = 4096u;
		constexpr f32 kRadius = 3.0f;
		constexpr f64 kRadiusSquared = 9.0;
		CDeterministicRandom A;
		CDeterministicRandom B;
		A.Reseed( 20260831u );
		B.Reseed( 20260831u );
		bool bSameAndOnSurface = true;
		f64 SumX = 0.0;
		f64 SumY = 0.0;
		f64 SumZ = 0.0;
		f64 SumNormalizedY2 = 0.0;
		for ( usize SampleIndex = 0u; SampleIndex < kSampleCount;
			++SampleIndex )
		{
			FVec3 PointA{};
			FVec3 PointB{};
			if ( !A.TryPointOnSphere3D( kRadius, PointA )
				|| !B.TryPointOnSphere3D( kRadius, PointB )
				|| PointA.x != PointB.x || PointA.y != PointB.y
				|| PointA.z != PointB.z ) bSameAndOnSurface = false;

			const f64 LengthSquared = static_cast<f64>( PointA.x ) * PointA.x
				+ static_cast<f64>( PointA.y ) * PointA.y
				+ static_cast<f64>( PointA.z ) * PointA.z;
			if ( std::abs( LengthSquared - kRadiusSquared ) > 0.00001 )
			{
				bSameAndOnSurface = false;
			}
			SumX += PointA.x;
			SumY += PointA.y;
			SumZ += PointA.z;
			const f64 NormalizedY = static_cast<f64>( PointA.y ) / kRadius;
			SumNormalizedY2 += NormalizedY * NormalizedY;
		}

		const f64 InverseCount = 1.0 / static_cast<f64>( kSampleCount );
		Harness.Check( bSameAndOnSurface,
			"同じ種で同じ球面位置を長さを保って選ぶ" );
		Harness.Check( std::abs( SumX * InverseCount ) < 0.08
			&& std::abs( SumY * InverseCount ) < 0.08
			&& std::abs( SumZ * InverseCount ) < 0.08,
			"球面位置の平均が中心付近へ戻る" );
		Harness.Check( std::abs(
			SumNormalizedY2 * InverseCount - ( 1.0 / 3.0 ) ) < 0.03,
			"上下軸も球面面積に比例して分布する" );
		Harness.CheckEqualU64( A.GetDrawCount(), kSampleCount * 2u,
			"球面1点につき32bit乱数を2個進める" );
	}

	Harness.BeginSuite( "CDeterministicRandom / 3D球内部を体積で選ぶ" );

	{
		constexpr usize kSampleCount = 4096u;
		constexpr f32 kRadius = 4.0f;
		constexpr f64 kRadiusSquared = 16.0;
		CDeterministicRandom Random;
		Random.Reseed( 314159u );
		bool bInside = true;
		f64 SumX = 0.0;
		f64 SumY = 0.0;
		f64 SumZ = 0.0;
		f64 SumNormalizedRadiusCubed = 0.0;
		for ( usize SampleIndex = 0u; SampleIndex < kSampleCount;
			++SampleIndex )
		{
			FVec3 Point{};
			if ( !Random.TryPointInSphere3D( kRadius, Point ) ) bInside = false;
			const f64 LengthSquared = static_cast<f64>( Point.x ) * Point.x
				+ static_cast<f64>( Point.y ) * Point.y
				+ static_cast<f64>( Point.z ) * Point.z;
			if ( LengthSquared > kRadiusSquared + 0.00001 ) bInside = false;
			const f64 NormalizedRadius = std::sqrt( LengthSquared ) / kRadius;
			SumNormalizedRadiusCubed += NormalizedRadius
				* NormalizedRadius * NormalizedRadius;
			SumX += Point.x;
			SumY += Point.y;
			SumZ += Point.z;
		}

		const f64 InverseCount = 1.0 / static_cast<f64>( kSampleCount );
		Harness.Check( bInside, "全ての点が指定した球内部に収まる" );
		Harness.Check( std::abs( SumX * InverseCount ) < 0.10
			&& std::abs( SumY * InverseCount ) < 0.10
			&& std::abs( SumZ * InverseCount ) < 0.10,
			"球内部位置の平均が中心付近へ戻る" );
		Harness.Check( std::abs(
			SumNormalizedRadiusCubed * InverseCount - 0.5 ) < 0.03,
			"半径の3乗が一様になり体積へ比例する" );
		Harness.CheckEqualU64( Random.GetDrawCount(), kSampleCount * 3u,
			"球内部1点につき32bit乱数を3個進める" );
	}

	Harness.BeginSuite( "CDeterministicRandom / 3D球抽選を復元する" );

	{
		CDeterministicRandom Random;
		Random.Reseed( 271828u );
		FVec3 Warmup{};
		Random.TryPointInSphere3D( 2.0f, Warmup );
		FRandomSnapshot Snapshot{};
		u64 SnapshotDrawCount = 0u;
		Random.CaptureSnapshot( Snapshot, SnapshotDrawCount );

		FVec3 ExpectedSurface{};
		FVec3 ExpectedVolume{};
		const bool bExpected = Random.TryPointOnSphere3D(
			3.0f, ExpectedSurface )
			&& Random.TryPointInSphere3D( 5.0f, ExpectedVolume );
		const u64 ExpectedDrawCount = Random.GetDrawCount();
		const bool bRestored = Random.TryRestoreSnapshot(
			Snapshot, SnapshotDrawCount );
		FVec3 ReplayedSurface{};
		FVec3 ReplayedVolume{};
		const bool bReplayed = Random.TryPointOnSphere3D(
			3.0f, ReplayedSurface )
			&& Random.TryPointInSphere3D( 5.0f, ReplayedVolume );

		Harness.Check( bExpected && bRestored && bReplayed
			&& ExpectedSurface.x == ReplayedSurface.x
			&& ExpectedSurface.y == ReplayedSurface.y
			&& ExpectedSurface.z == ReplayedSurface.z
			&& ExpectedVolume.x == ReplayedVolume.x
			&& ExpectedVolume.y == ReplayedVolume.y
			&& ExpectedVolume.z == ReplayedVolume.z,
			"途中状態へ戻すと球面と球内部の点を再生する" );
		Harness.CheckEqualU64( SnapshotDrawCount, 3u,
			"球内部抽選後の消費数を写す" );
		Harness.CheckEqualU64( Random.GetDrawCount(), ExpectedDrawCount,
			"復元後の球抽選でも消費数が一致する" );
	}

	{
		CDeterministicRandom Random;
		CDeterministicRandom Expected;
		Random.Reseed( 42u );
		Expected.Reseed( 42u );
		FVec3 Point{ 7.0f, 8.0f, 9.0f };
		const bool bRejected =
			!Random.TryPointOnSphere3D( -1.0f, Point )
			&& !Random.TryPointInSphere3D(
				std::numeric_limits<f32>::infinity(), Point )
			&& !Random.TryPointOnSphere3D(
				std::numeric_limits<f32>::quiet_NaN(), Point );
		Harness.Check( bRejected && Point.x == 7.0f
			&& Point.y == 8.0f && Point.z == 9.0f,
			"負または非有限の半径を出力変更なしで拒否する" );
		Harness.CheckEqualU64( Random.GetDrawCount(), 0u,
			"不正半径では乱数を進めない" );

		FVec3 SurfaceZero{ 1.0f, 2.0f, 3.0f };
		FVec3 VolumeZero{ 4.0f, 5.0f, 6.0f };
		Harness.Check( Random.TryPointOnSphere3D( 0.0f, SurfaceZero )
			&& Random.TryPointInSphere3D( 0.0f, VolumeZero )
			&& SurfaceZero.x == 0.0f && SurfaceZero.y == 0.0f
			&& SurfaceZero.z == 0.0f && VolumeZero.x == 0.0f
			&& VolumeZero.y == 0.0f && VolumeZero.z == 0.0f,
			"半径0は乱数なしで原点を返す" );
		Harness.CheckEqualU64( Random.GetDrawCount(), 0u,
			"半径0でも乱数を進めない" );
		Harness.CheckEqualU64( Random.NextU32(), Expected.NextU32(),
			"拒否と半径0の後も次の乱数値が変わらない" );
	}

	Harness.BeginSuite( "CDeterministicRandom / 3D円錐内を立体角で選ぶ" );

	{
		constexpr usize kSampleCount = 4096u;
		constexpr f32 kHalfAngleDegrees = 40.0f;
		constexpr f64 kDegreesToRadians =
			3.14159265358979323846 / 180.0;
		const FVec3 Axis = Normalize( FVec3{ 1.0f, 2.0f, 3.0f } );
		const f64 MinimumCosine = std::cos(
			static_cast<f64>( kHalfAngleDegrees ) * kDegreesToRadians );
		const f64 ExpectedMeanCosine = ( 1.0 + MinimumCosine ) * 0.5;
		CDeterministicRandom A;
		CDeterministicRandom B;
		A.Reseed( 20260831u );
		B.Reseed( 20260831u );
		bool bSameAndInside = true;
		FVec3 DirectionSum{};
		f64 AxisCosineSum = 0.0;

		for ( usize SampleIndex = 0u; SampleIndex < kSampleCount;
			++SampleIndex )
		{
			FVec3 DirectionA{};
			FVec3 DirectionB{};
			const bool bChosenA = A.TryDirectionInCone3D(
				FVec3{ 1.0f, 2.0f, 3.0f }, kHalfAngleDegrees,
				DirectionA );
			const bool bChosenB = B.TryDirectionInCone3D(
				FVec3{ 2.0f, 4.0f, 6.0f }, kHalfAngleDegrees,
				DirectionB );
			const f64 AxisCosine = static_cast<f64>(
				Dot( Axis, DirectionA ) );
			const f64 LengthSquared = static_cast<f64>(
				LengthSq( DirectionA ) );
			if ( !bChosenA || !bChosenB
				|| DirectionA.x != DirectionB.x
				|| DirectionA.y != DirectionB.y
				|| DirectionA.z != DirectionB.z
				|| AxisCosine < MinimumCosine - 0.00001
				|| std::abs( LengthSquared - 1.0 ) > 0.00001 )
			{
				bSameAndInside = false;
			}

			DirectionSum = DirectionSum + DirectionA;
			AxisCosineSum += AxisCosine;
		}

		const f64 InverseCount = 1.0 / static_cast<f64>( kSampleCount );
		Harness.Check( bSameAndInside,
			"軸の長さに依らず同じ円錐内の単位方向を再生する" );
		Harness.Check( std::abs(
			AxisCosineSum * InverseCount - ExpectedMeanCosine ) < 0.02,
			"中心軸との内積を均等にして立体角へ比例させる" );
		Harness.Check(
			std::abs( static_cast<f64>( DirectionSum.x ) * InverseCount
				- static_cast<f64>( Axis.x ) * ExpectedMeanCosine ) < 0.02
			&& std::abs( static_cast<f64>( DirectionSum.y ) * InverseCount
				- static_cast<f64>( Axis.y ) * ExpectedMeanCosine ) < 0.02
			&& std::abs( static_cast<f64>( DirectionSum.z ) * InverseCount
				- static_cast<f64>( Axis.z ) * ExpectedMeanCosine ) < 0.02,
			"方位角の偏りを残さず平均方向を中心軸へ戻す" );
		Harness.CheckEqualU64( A.GetDrawCount(), kSampleCount * 2u,
			"円錐方向1個につき32bit乱数を2個進める" );
	}

	{
		const f32 NotANumber =
			std::numeric_limits<f32>::quiet_NaN();
		const f32 Infinity = std::numeric_limits<f32>::infinity();
		CDeterministicRandom Random;
		CDeterministicRandom Expected;
		Random.Reseed( 42u );
		Expected.Reseed( 42u );
		FVec3 Direction{ 7.0f, 8.0f, 9.0f };
		const bool bRejected =
			!Random.TryDirectionInCone3D(
				FVec3{}, 30.0f, Direction )
			&& !Random.TryDirectionInCone3D(
				FVec3{ NotANumber, 0.0f, 1.0f }, 30.0f, Direction )
			&& !Random.TryDirectionInCone3D(
				FVec3{ Infinity, 0.0f, 1.0f }, 30.0f, Direction )
			&& !Random.TryDirectionInCone3D(
				FVec3::Forward(), -0.01f, Direction )
			&& !Random.TryDirectionInCone3D(
				FVec3::Forward(), 180.01f, Direction )
			&& !Random.TryDirectionInCone3D(
				FVec3::Forward(), NotANumber, Direction );
		Harness.Check( bRejected && Direction.x == 7.0f
			&& Direction.y == 8.0f && Direction.z == 9.0f,
			"無効な軸と角度を出力変更なしで拒否する" );

		FVec3 ZeroAngle{};
		FVec3 LargeAxis{};
		const bool bZeroAngles = Random.TryDirectionInCone3D(
			FVec3{ 2.0f, 0.0f, 0.0f }, 0.0f, ZeroAngle )
			&& Random.TryDirectionInCone3D(
				FVec3{ std::numeric_limits<f32>::max(), 0.0f, 0.0f },
				0.0f, LargeAxis )
			&& ZeroAngle.x == 1.0f && ZeroAngle.y == 0.0f
			&& ZeroAngle.z == 0.0f && LargeAxis.x == 1.0f
			&& LargeAxis.y == 0.0f && LargeAxis.z == 0.0f;
		Harness.Check( bZeroAngles,
			"0度は極端に大きい有限軸も安全に正規化して返す" );
		Harness.CheckEqualU64( Random.GetDrawCount(), 0u,
			"拒否と0度では乱数を進めない" );
		Harness.CheckEqualU64( Random.NextU32(), Expected.NextU32(),
			"拒否と0度の後も次の乱数値が変わらない" );
	}

	{
		CDeterministicRandom Random;
		Random.Reseed( 314159u );
		FVec3 FullSphereDirection{};
		const bool bFullSphere = Random.TryDirectionInCone3D(
			FVec3::Up(), 180.0f, FullSphereDirection );
		Harness.Check( bFullSphere && std::abs(
			static_cast<f64>( LengthSq( FullSphereDirection ) ) - 1.0 )
			< 0.00001,
			"180度を球面全体として扱う" );
		Harness.CheckEqualU64( Random.GetDrawCount(), 2u,
			"180度でも方向抽選の消費数を固定する" );
	}

	{
		constexpr usize kReplayCount = 64u;
		CDeterministicRandom Random;
		Random.Reseed( 271828u );
		FVec3 Warmup{};
		Random.TryDirectionInCone3D(
			FVec3::Forward(), 25.0f, Warmup );
		FRandomSnapshot Snapshot{};
		u64 SnapshotDrawCount = 0u;
		Random.CaptureSnapshot( Snapshot, SnapshotDrawCount );
		FVec3 ExpectedDirections[kReplayCount]{};
		for ( usize Index = 0u; Index < kReplayCount; ++Index )
		{
			Random.TryDirectionInCone3D(
				FVec3::Forward(), 25.0f, ExpectedDirections[Index] );
		}

		const bool bRestored = Random.TryRestoreSnapshot(
			Snapshot, SnapshotDrawCount );
		bool bReplayed = bRestored;
		for ( usize Index = 0u; Index < kReplayCount; ++Index )
		{
			FVec3 Direction{};
			if ( !Random.TryDirectionInCone3D(
				FVec3::Forward(), 25.0f, Direction )
				|| Direction.x != ExpectedDirections[Index].x
				|| Direction.y != ExpectedDirections[Index].y
				|| Direction.z != ExpectedDirections[Index].z )
			{
				bReplayed = false;
			}
		}

		Harness.Check( SnapshotDrawCount == 2u,
			"円錐方向抽選後の乱数位置を写す" );
		Harness.Check( bReplayed,
			"復元後に同じ円錐方向列を再生する" );
		Harness.CheckEqualU64( Random.GetDrawCount(),
			SnapshotDrawCount + kReplayCount * 2u,
			"復元後の円錐方向でも消費数が一致する" );
	}
}
