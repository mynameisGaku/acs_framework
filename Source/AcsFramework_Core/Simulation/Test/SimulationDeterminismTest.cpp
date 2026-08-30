// SPDX-License-Identifier: Apache-2.0
// Simulation モジュールの決定性を、ゲームを起動せずに確かめる自己テスト。
//
//   1. 記録しながら N ステップ回し、最終状態とイベント列を控える
//   2. テープをバイト列へ書き出し、別のテープへ読み込む
//   3. そのテープで再生し、同じ N ステップを回す
//   4. 最終状態とイベント列が完全に一致するかを見る
//
// Actor も描画も音も使わない。ここが通れば「同じ入力から同じ結果」が実際に成り立っている。

#include <cstdio>

#include "AcsFramework_Core/Simulation/ActionInputTape.h"
#include "AcsFramework_Core/Simulation/DeterministicRandom.h"
#include "AcsFramework_Core/Simulation/FixedStepDriver.h"
#include "AcsFramework_Core/Simulation/IActionInputSource.h"
#include "AcsFramework_Core/Simulation/ISimulationRule.h"
#include "AcsFramework_Core/Simulation/Input/ActionBindingTable.h"
#include "AcsFramework_Core/Simulation/ReplayFile.h"
#include "AcsFramework_Core/Simulation/SimulationEventQueue.h"
#include "AcsFramework_Core/Simulation/SimulationSnapshot.h"
#include "AcsFramework_Core/Simulation/SimulationSnapshotFile.h"

namespace
{
	/** 押されていることにするキーを直接指定できる、装置の代わり。 */
	class CFakeDevice final : public IActionDeviceReader
	{
	public:
		void SetKeyDown( EKey Key, bool bDown ) noexcept
		{
			if ( bDown ) { if ( !IsKeyDown( Key ) ) m_DownKeys.TryAdd( Key ); }
			else         { m_DownKeys.RemoveSingleSwap( Key ); }
		}

		void SetAxis( f32 Value ) noexcept { m_AxisValue = Value; }

		bool IsKeyDown( EKey Key ) const noexcept override
		{
			for ( usize Index = 0u; Index < m_DownKeys.Num(); ++Index )
			{
				if ( m_DownKeys[Index] == Key ) return true;
			}
			return false;
		}

		bool IsGamepadButtonDown( u32, EGamepadButton ) const noexcept override { return false; }

		f32 GetGamepadAxis( u32, EGamepadAxis ) const noexcept override { return m_AxisValue; }

	private:
		TArray<EKey> m_DownKeys;
		f32 m_AxisValue = 0.0f;
	};
	constexpr u32 kActionMove = 0u;
	constexpr u32 kActionFire = 1u;
	constexpr u32 kEventFired = 100u;
	constexpr u32 kStepCount = 600u;

	/** 台本どおりに動く入力元。人でも AI でもない、試験用の 3 つ目の入力元。 */
	class CScriptedInput final : public IActionInputSource
	{
	public:
		bool TryGetActionInput( FActionInput& OutInput ) noexcept override
		{
			// 周期の違う 2 つの波で、押しっぱなし・連打・無入力が混ざるようにする。
			const u32 Tick = m_Tick++;

			OutInput.SetAxis( 0u, ( ( Tick / 7u ) % 3u == 0u ) ? 1.0f : -0.5f );
			OutInput.SetDown( kActionMove, ( Tick % 5u ) != 0u );
			OutInput.SetDown( kActionFire, ( Tick % 11u ) == 0u );

			return true;
		}

		void Rewind() noexcept { m_Tick = 0u; }

	private:
		u32 m_Tick = 0u;
	};

	/** 位置と発射回数だけを持つ試験用の規則。乱数を引いてブレを足す。 */
	class CTestRule final : public ISimulationRule
	{
	public:
		void AdvanceStep( const FSimulationContext& Context ) noexcept override
		{
			const f32 Jitter = ( Context.Random != nullptr ) ? Context.Random->NextRangeFloat( -0.1f, 0.1f ) : 0.0f;

			if ( Context.IsDown( kActionMove ) )
			{
				m_PositionX += ( Context.GetAxis( 0u ) + Jitter ) * 4.0f * Context.StepSeconds;
			}

			if ( Context.WasPressed( kActionFire ) )
			{
				++m_FireCount;
				Context.Raise( kEventFired, m_FireCount, m_PositionX );
			}
		}

		void ResetState() noexcept override
		{
			m_PositionX = 0.0f;
			m_FireCount = 0u;
		}

		/** 盤面は 2 つだけ。結果へ影響する値をここへ漏らさず入れる。 */
		bool TrySaveState( TArray<u8>& OutBytes ) const noexcept override
		{
			OutBytes.Reset();
			OutBytes.SetNum( sizeof( f32 ) + sizeof( u32 ) );

			u8* Cursor = OutBytes.GetData();
			MemCopy( Cursor, &m_PositionX, sizeof( f32 ) );
			MemCopy( Cursor + sizeof( f32 ), &m_FireCount, sizeof( u32 ) );

			return true;
		}

		bool TryRestoreState( const u8* Bytes, usize Size ) noexcept override
		{
			if ( Bytes == nullptr || Size != sizeof( f32 ) + sizeof( u32 ) ) return false;

			MemCopy( &m_PositionX, Bytes, sizeof( f32 ) );
			MemCopy( &m_FireCount, Bytes + sizeof( f32 ), sizeof( u32 ) );

			return true;
		}

		f32 GetPositionX() const noexcept { return m_PositionX; }
		u32 GetFireCount() const noexcept { return m_FireCount; }

	private:
		f32 m_PositionX = 0.0f;
		u32 m_FireCount = 0u;
	};

	/** 1 回ぶんの走行結果。 */
	struct FRunResult
	{
		f32 PositionX = 0.0f;
		u32 FireCount = 0u;
		u64 DrawCount = 0u;
		TArray<FSimulationEvent> Events;
	};

	/**
	 * N ステップ回す。Tape が nullptr でなく bRecord なら記録し、bRecord でなければ再生する。
	 */
	void RunSteps( CTestRule& Rule, CScriptedInput* Input, CActionInputTape& Tape, bool bRecord, u64 Seed, FRunResult& OutResult )
	{
		CFixedStepDriver Driver;
		CDeterministicRandom Random;
		CSimulationEventQueue Events;

		Driver.Configure( 1.0 / 60.0, 8u );
		Random.Reseed( Seed );
		Rule.ResetState();
		if ( Input != nullptr ) Input->Rewind();

		FActionInput Current;
		FActionInput Previous;

		for ( u32 Step = 0u; Step < kStepCount; ++Step )
		{
			const u32 Tick = Driver.GetTick();

			Previous = Current;
			Current = FActionInput();

			if ( bRecord )
			{
				if ( Input != nullptr ) Input->TryGetActionInput( Current );
				Tape.Record( Tick, Current );
			}
			else
			{
				Tape.TryGet( Tick, Current );
			}

			FSimulationContext Context;
			Context.Input = Current;
			Context.PreviousInput = Previous;
			Context.Tick = Tick;
			Context.StepSeconds = Driver.GetStepSeconds();
			Context.Random = &Random;
			Context.Events = &Events;

			Rule.AdvanceStep( Context );
			Driver.AdvanceTick();
		}

		OutResult.PositionX = Rule.GetPositionX();
		OutResult.FireCount = Rule.GetFireCount();
		OutResult.DrawCount = Random.GetDrawCount();

		for ( usize Index = 0u; Index < Events.Num(); ++Index ) OutResult.Events.TryAdd( Events.Get( Index ) );
	}

	/** 与えられた部品のまま N ステップだけ進める (途中から再開する試験に使う)。 */
	void StepRange( CTestRule& Rule, const CActionInputTape& Tape, CFixedStepDriver& Driver,
		CDeterministicRandom& Random, CSimulationEventQueue& Events,
		FActionInput& Current, FActionInput& Previous, u32 StepCount )
	{
		for ( u32 Step = 0u; Step < StepCount; ++Step )
		{
			const u32 Tick = Driver.GetTick();

			Previous = Current;
			Current = FActionInput();
			Tape.TryGet( Tick, Current );

			FSimulationContext Context;
			Context.Input = Current;
			Context.PreviousInput = Previous;
			Context.Tick = Tick;
			Context.StepSeconds = Driver.GetStepSeconds();
			Context.Random = &Random;
			Context.Events = &Events;

			Rule.AdvanceStep( Context );
			Driver.AdvanceTick();
		}
	}

	/** 進んだ結果を控える。 */
	void Collect( const CTestRule& Rule, const CDeterministicRandom& Random, const CSimulationEventQueue& Events, FRunResult& OutResult )
	{
		OutResult.PositionX = Rule.GetPositionX();
		OutResult.FireCount = Rule.GetFireCount();
		OutResult.DrawCount = Random.GetDrawCount();
		OutResult.Events.Reset();

		for ( usize Index = 0u; Index < Events.Num(); ++Index ) OutResult.Events.TryAdd( Events.Get( Index ) );
	}

	/**
	 * 盤面と乱数だけを比べる。
	 *
	 * @details
	 * イベントは「どの区間ぶんを溜めたか」で数が変わるので、区間の違う 2 つを比べるときは
	 * こちらを使う。区間が同じなら CheckEqual でイベント列まで比べる。
	 */
	bool CheckScalarEqual( const FRunResult& A, const FRunResult& B, const char* Label )
	{
		bool bOk = true;

		if ( A.PositionX != B.PositionX ) { std::printf( "  [NG] %s: PositionX %.9f vs %.9f\n", Label, A.PositionX, B.PositionX ); bOk = false; }
		if ( A.FireCount != B.FireCount ) { std::printf( "  [NG] %s: FireCount %u vs %u\n", Label, A.FireCount, B.FireCount ); bOk = false; }
		if ( A.DrawCount != B.DrawCount ) { std::printf( "  [NG] %s: DrawCount %llu vs %llu\n", Label, (unsigned long long)A.DrawCount, (unsigned long long)B.DrawCount ); bOk = false; }

		if ( bOk ) std::printf( "  [OK] %s\n", Label );

		return bOk;
	}

	bool CheckEqual( const FRunResult& A, const FRunResult& B, const char* Label )
	{
		bool bOk = true;

		if ( A.PositionX != B.PositionX ) { std::printf( "  [NG] %s: PositionX %.9f vs %.9f\n", Label, A.PositionX, B.PositionX ); bOk = false; }
		if ( A.FireCount != B.FireCount ) { std::printf( "  [NG] %s: FireCount %u vs %u\n", Label, A.FireCount, B.FireCount ); bOk = false; }
		if ( A.DrawCount != B.DrawCount ) { std::printf( "  [NG] %s: DrawCount %llu vs %llu\n", Label, (unsigned long long)A.DrawCount, (unsigned long long)B.DrawCount ); bOk = false; }
		if ( A.Events.Num() != B.Events.Num() ) { std::printf( "  [NG] %s: Events %zu vs %zu\n", Label, A.Events.Num(), B.Events.Num() ); bOk = false; }
		else
		{
			for ( usize Index = 0u; Index < A.Events.Num(); ++Index )
			{
				if ( !A.Events[Index].Equals( B.Events[Index] ) )
				{
					std::printf( "  [NG] %s: Event[%zu] が一致しません (tick %u vs %u)\n", Label, Index, A.Events[Index].Tick, B.Events[Index].Tick );
					bOk = false;
					break;
				}
			}
		}

		if ( bOk ) std::printf( "  [OK] %s\n", Label );

		return bOk;
	}
}

int main()
{
	const u64 Seed = 20260816u;
	bool bAllOk = true;

	std::printf( "== Simulation determinism test (%u steps) ==\n", kStepCount );

	// 1. 記録しながら 1 回目を回す。
	CTestRule RuleA;
	CScriptedInput Input;
	CActionInputTape RecordedTape;
	RecordedTape.SetSeed( Seed );

	FRunResult RunA;
	RunSteps( RuleA, &Input, RecordedTape, true, Seed, RunA );
	std::printf( "run A: x=%.6f fire=%u draws=%llu events=%zu tape=%zu entries\n",
		RunA.PositionX, RunA.FireCount, (unsigned long long)RunA.DrawCount, RunA.Events.Num(), RecordedTape.Num() );

	// 2. テープをバイト列へ落として、別のテープへ戻す。
	TArray<u8> Buffer;
	Buffer.SetNum( RecordedTape.GetRequiredBytes() );

	usize Written = 0u;
	if ( !RecordedTape.TrySaveToBuffer( Buffer.GetData(), Buffer.Num(), Written ) )
	{
		std::printf( "  [NG] テープを書き出せませんでした\n" );
		return 1;
	}

	CActionInputTape LoadedTape;
	if ( !LoadedTape.TryLoadFromBuffer( Buffer.GetData(), Written ) )
	{
		std::printf( "  [NG] テープを読み込めませんでした\n" );
		return 1;
	}

	std::printf( "tape: %zu bytes / seed=%llu / entries=%zu\n",
		Written, (unsigned long long)LoadedTape.GetSeed(), LoadedTape.Num() );

	// 3. 読み込んだテープで再生する (入力元は渡さない)。
	CTestRule RuleB;
	FRunResult RunB;
	RunSteps( RuleB, nullptr, LoadedTape, false, LoadedTape.GetSeed(), RunB );
	std::printf( "run B: x=%.6f fire=%u draws=%llu events=%zu\n",
		RunB.PositionX, RunB.FireCount, (unsigned long long)RunB.DrawCount, RunB.Events.Num() );

	bAllOk &= CheckEqual( RunA, RunB, "record -> save -> load -> replay" );

	// 4. 種を変えたら違う結果になること (テストが «常に一致» を見ているだけでないことの確認)。
	CTestRule RuleC;
	FRunResult RunC;
	RunSteps( RuleC, nullptr, LoadedTape, false, LoadedTape.GetSeed() + 1u, RunC );

	if ( RunC.PositionX == RunA.PositionX )
	{
		std::printf( "  [NG] 種を変えても同じ結果になりました (乱数が効いていません)\n" );
		bAllOk = false;
	}
	else
	{
		std::printf( "  [OK] 種を変えると結果が変わる (x=%.6f)\n", RunC.PositionX );
	}

	// 5. 割り当て表を、装置なしで確かめる。偽の装置を差せることがここで効く。
	{
		CFakeDevice Device;
		CActionBindingTable Table;
		Table.BindKey( kActionFire, EKey::Space );
		Table.BindAxisKeys( 0u, EKey::A, EKey::D );

		const FActionInput Neutral = Table.Resolve( Device );

		Device.SetKeyDown( EKey::D, true );
		Device.SetKeyDown( EKey::Space, true );
		const FActionInput Pressed = Table.Resolve( Device );

		Device.SetKeyDown( EKey::A, true );
		const FActionInput Both = Table.Resolve( Device );

		const bool bBindingOk = Neutral.IsNeutral()
			&& Pressed.IsDown( kActionFire ) && Pressed.GetAxis( 0u ) == 1.0f
			&& Both.GetAxis( 0u ) == 0.0f;

		if ( bBindingOk ) std::printf( "  [OK] 割り当て表: 装置なしで解決できる (両押しは 0)\n" );
		else
		{
			std::printf( "  [NG] 割り当て表: neutral=%d fire=%d axis=%.1f both=%.1f\n",
				Neutral.IsNeutral() ? 1 : 0, Pressed.IsDown( kActionFire ) ? 1 : 0,
				Pressed.GetAxis( 0u ), Both.GetAxis( 0u ) );
			bAllOk = false;
		}
	}

	// 6. テープをファイルへ置いて戻す。再現の «保存» が実際に効くかを見る。
	{
		// 実行ディレクトリへ置く。掘っていないフォルダを指すと書き込みが失敗する。
		const FString Path( "replay_roundtrip.acssave" );

		CActionInputTape FromFile;
		const bool bSaved = CReplayFile::Save( RecordedTape, Path );
		const bool bLoaded = bSaved && CReplayFile::Load( Path, FromFile );

		if ( !bSaved || !bLoaded )
		{
			std::printf( "  [NG] リプレイファイル: save=%d load=%d\n", bSaved ? 1 : 0, bLoaded ? 1 : 0 );
			bAllOk = false;
		}
		else
		{
			CTestRule RuleD;
			FRunResult RunD;
			RunSteps( RuleD, nullptr, FromFile, false, FromFile.GetSeed(), RunD );

			bAllOk &= CheckEqual( RunA, RunD, "record -> file -> load -> replay" );
		}
	}

	// 7. 途中で写して、そこへ戻って続きを回す。同じ道を進むはず。
	{
		const u32 kHalf = kStepCount / 2u;

		CTestRule Rule;
		CFixedStepDriver Driver;
		CDeterministicRandom Random;
		CSimulationEventQueue Events;
		FActionInput Current;
		FActionInput Previous;

		Driver.Configure( 1.0 / 60.0, 8u );
		Random.Reseed( LoadedTape.GetSeed() );
		Rule.ResetState();

		StepRange( Rule, LoadedTape, Driver, Random, Events, Current, Previous, kHalf );

		CSimulationSnapshot Snapshot;
		const bool bCaptured = Snapshot.TryCaptureFrom( Driver, Random, Rule, Current, Previous );

		// 写した地点から後だけを比べたいので、ここで一度捨てる。
		// 前半ぶんを残したまま比べると «区間の違う 2 つ» を比べることになる。
		Events.Clear();

		// 写した地点から最後まで進める (1 回目)。
		StepRange( Rule, LoadedTape, Driver, Random, Events, Current, Previous, kStepCount - kHalf );

		FRunResult FirstHalfEnd;
		Collect( Rule, Random, Events, FirstHalfEnd );

		// 戻して、もう一度同じ区間を進める (2 回目)。
		Events.Clear();
		Current = FActionInput();
		Previous = FActionInput();

		const bool bRestored = bCaptured && Snapshot.TryRestoreTo( Driver, Random, Rule, Current, Previous );

		StepRange( Rule, LoadedTape, Driver, Random, Events, Current, Previous, kStepCount - kHalf );

		FRunResult SecondHalfEnd;
		Collect( Rule, Random, Events, SecondHalfEnd );

		if ( !bCaptured || !bRestored )
		{
			std::printf( "  [NG] スナップショット: capture=%d restore=%d\n", bCaptured ? 1 : 0, bRestored ? 1 : 0 );
			bAllOk = false;
		}
		else
		{
			std::printf( "snapshot: tick=%u draws=%llu rule=%zu bytes\n",
				Snapshot.GetTick(), (unsigned long long)Snapshot.GetDrawCount(), Snapshot.GetRuleByteCount() );

			bAllOk &= CheckEqual( FirstHalfEnd, SecondHalfEnd, "途中で写す -> 戻す -> 続きが同じ" );

			// 最後まで通しで回した結果とも一致するはず (途中で写しても道は変わらない)。
			// イベントは溜めた区間が違う (通しは 600 步ぶん、こちらは後半 300 步ぶん) ので比べない。
			bAllOk &= CheckScalarEqual( RunA, SecondHalfEnd, "通し と 途中から の盤面が同じ" );
		}

		// 8. 写したものをファイルへ置いて戻す。
		if ( bCaptured )
		{
			const FString Path( "snapshot_roundtrip.acssave" );

			CSimulationSnapshot FromFile;
			const bool bSaved = CSimulationSnapshotFile::Save( Snapshot, Path );
			const bool bLoaded = bSaved && CSimulationSnapshotFile::Load( Path, FromFile );

			if ( !bSaved || !bLoaded )
			{
				std::printf( "  [NG] スナップショットのファイル: save=%d load=%d\n", bSaved ? 1 : 0, bLoaded ? 1 : 0 );
				bAllOk = false;
			}
			else
			{
				CTestRule FileRule;
				CFixedStepDriver FileDriver;
				CDeterministicRandom FileRandom;
				CSimulationEventQueue FileEvents;
				FActionInput FileCurrent;
				FActionInput FilePrevious;

				FileDriver.Configure( 1.0 / 60.0, 8u );

				if ( !FromFile.TryRestoreTo( FileDriver, FileRandom, FileRule, FileCurrent, FilePrevious ) )
				{
					std::printf( "  [NG] ファイルから戻せません\n" );
					bAllOk = false;
				}
				else
				{
					StepRange( FileRule, LoadedTape, FileDriver, FileRandom, FileEvents, FileCurrent, FilePrevious, kStepCount - kHalf );

					FRunResult FromFileEnd;
					Collect( FileRule, FileRandom, FileEvents, FromFileEnd );

					// 区間が同じ (どちらも写した地点から後半 300 步) のでイベント列まで比べる。
					bAllOk &= CheckEqual( FirstHalfEnd, FromFileEnd, "snapshot -> file -> load -> 続き" );
				}
			}
		}
	}

	std::printf( "== %s ==\n", bAllOk ? "PASS" : "FAIL" );

	return bAllOk ? 0 : 1;
}
