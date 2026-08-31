// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/DeterministicRandom.h"
#include "AcsFramework_Core/Simulation/FixedStepDriver.h"
#include "Common/Test/TestHarness.h"

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
}
