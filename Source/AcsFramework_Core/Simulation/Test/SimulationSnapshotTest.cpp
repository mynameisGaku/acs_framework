// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/SimulationSnapshot.h"
#include "AcsFramework_Core/Simulation/SimulationSubsystem.h"
#include "Common/Test/TestHarness.h"

namespace
{
	/** v1実装で生成した固定fixture。型配置が変わっても旧ファイル互換を検出する。 */
	constexpr u8 kLegacySnapshotV1[] = {
		0x00, 0x5A, 0x57, 0xAC, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
		0xF1, 0xF9, 0x53, 0xCA, 0x42, 0xEB, 0x8F, 0x1A, 0xB2, 0x7E, 0xE0, 0x92,
		0x25, 0x97, 0x54, 0x84, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x9D, 0x35, 0xD0,
		0x64, 0xB8, 0xD6, 0xEA, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x81, 0x3F,
		0x04, 0x00, 0x00, 0x00, 0xF8, 0x7F, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11,
		0x11, 0x11, 0xA1, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x00, 0x00,
	};

	/** v1fixtureの復元位置から次に得られる乱数値。 */
	constexpr u32 kLegacySnapshotNextRandom = 2787987282u;

	/** v2実装でfield単位に生成した固定fixture。版を上げないwire変更を検出する。 */
	constexpr u8 kSnapshotV2[] = {
		0x00, 0x5A, 0x57, 0xAC, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
		0xCD, 0x7B, 0xAE, 0x06, 0x71, 0xAB, 0x29, 0x89, 0x47, 0x6E, 0x88, 0x2C,
		0x57, 0x72, 0x81, 0x48, 0x00, 0x00, 0x00, 0x00, 0xB7, 0x6E, 0x6F, 0x43,
		0xB4, 0xBD, 0x6B, 0x7F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x81, 0x3F,
		0x04, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0xA1, 0x3F,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x3F, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBF, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
		0x07, 0x00, 0x00, 0x00,
	};

	static_assert( sizeof( kSnapshotV2 ) == 148u, "v2固定部144 byteと盤面4 byteを維持する" );

	/** v2fixtureの復元位置から次に得られる乱数値。 */
	constexpr u32 kSnapshotV2NextRandom = 697397638u;

	/** 現在形式で入力履歴フラグが置かれる位置。 */
	constexpr usize kInputHistoryFlagOffset =
		sizeof( u32 ) * 3u
		+ sizeof( u64 )
		+ sizeof( u32 ) * 6u + sizeof( u64 )
		+ sizeof( f64 ) * 4u + sizeof( u32 ) + sizeof( u64 );

	/** 乱数の保存fieldがすべて一致するかを返す。 */
	bool RandomSnapshotsEqual( const FRandomSnapshot& Left, const FRandomSnapshot& Right ) noexcept
	{
		return Left.version == Right.version
			&& Left.state0 == Right.state0
			&& Left.state1 == Right.state1
			&& Left.state2 == Right.state2
			&& Left.state3 == Right.state3
			&& Left.reserved == Right.reserved
			&& Left.signature == Right.signature;
	}

	/** 固定時計の保存fieldがすべて一致するかを返す。 */
	bool ClockSnapshotsEqual( const FFixedStepClockSnapshot& Left, const FFixedStepClockSnapshot& Right ) noexcept
	{
		return Left.options.step_seconds == Right.options.step_seconds
			&& Left.options.maximum_steps_per_advance == Right.options.maximum_steps_per_advance
			&& Left.options.maximum_accumulated_seconds == Right.options.maximum_accumulated_seconds
			&& Left.accumulated_seconds == Right.accumulated_seconds
			&& Left.total_dropped_seconds == Right.total_dropped_seconds
			&& Left.total_step_count == Right.total_step_count;
	}

	/** 盤面を出し入れできる、試験用のいちばん小さい規則。 */
	class CCountingRule final : public ISimulationRule
	{
	public:
		void AdvanceStep( const FSimulationContext& Context ) noexcept override
		{
			(void)Context;
			++m_Counter;
		}

		void ResetState() noexcept override { m_Counter = 0u; }

		bool TrySaveState( TArray<u8>& OutBytes ) const noexcept override
		{
			OutBytes.Reset();
			OutBytes.SetNum( sizeof( u32 ) );
			MemCopy( OutBytes.GetData(), &m_Counter, sizeof( u32 ) );
			return true;
		}

		bool TryRestoreState( const u8* Bytes, usize Size ) noexcept override
		{
			if ( Bytes == nullptr || Size != sizeof( u32 ) ) return false;

			MemCopy( &m_Counter, Bytes, sizeof( u32 ) );
			return true;
		}

		void SetCounter( u32 Value ) noexcept { m_Counter = Value; }
		u32 GetCounter() const noexcept { return m_Counter; }

	private:
		u32 m_Counter = 0u;
	};

	/** 盤面を出せない規則 (既定のまま)。 */
	class COpaqueRule final : public ISimulationRule
	{
	public:
		void AdvanceStep( const FSimulationContext& Context ) noexcept override { (void)Context; }
	};

	/** 候補をすべて検証してからだけ盤面へ反映する、復元失敗試験用の規則。 */
	class CAtomicRejectingRule final : public ISimulationRule
	{
	public:
		/** 指定値を持つ失敗時不変の規則を作る。 */
		explicit CAtomicRejectingRule( u32 Value = 77u ) noexcept : m_Value( Value ) {}

		void AdvanceStep( const FSimulationContext& Context ) noexcept override { (void)Context; }

		bool TrySaveState( TArray<u8>& OutBytes ) const noexcept override
		{
			constexpr u32 InvalidGuard = 0xDEADBEEFu;
			OutBytes.SetNum( sizeof( u32 ) * 2u );
			MemCopy( OutBytes.GetData(), &m_Value, sizeof( u32 ) );
			MemCopy( OutBytes.GetData() + sizeof( u32 ), &InvalidGuard, sizeof( u32 ) );
			return true;
		}

		bool TryRestoreState( const u8* Bytes, usize Size ) noexcept override
		{
			if ( Bytes == nullptr || Size != sizeof( u32 ) * 2u ) return false;

			u32 CandidateValue = 0u;
			u32 CandidateGuard = 0u;
			MemCopy( &CandidateValue, Bytes, sizeof( u32 ) );
			MemCopy( &CandidateGuard, Bytes + sizeof( u32 ), sizeof( u32 ) );
			if ( CandidateGuard != 0xA55A5AA5u ) return false;

			m_Value = CandidateValue;
			return true;
		}

		/** 復元失敗前後で比較する盤面値を返す。 */
		u32 GetValue() const noexcept { return m_Value; }

	private:
		/** 成功した復元だけが変更できる盤面値。 */
		u32 m_Value = 77u;
	};

	/** 押下開始を数え、スナップショットへ状態を出し入れできる規則。 */
	class CPressedCountingRule final : public ISimulationRule
	{
	public:
		void AdvanceStep( const FSimulationContext& Context ) noexcept override
		{
			if ( Context.WasPressed( 0u ) ) ++m_PressedCount;
		}

		void ResetState() noexcept override { m_PressedCount = 0u; }

		bool TrySaveState( TArray<u8>& OutBytes ) const noexcept override
		{
			OutBytes.Reset();
			OutBytes.SetNum( sizeof( u32 ) );
			MemCopy( OutBytes.GetData(), &m_PressedCount, sizeof( u32 ) );
			return true;
		}

		bool TryRestoreState( const u8* Bytes, usize Size ) noexcept override
		{
			if ( Bytes == nullptr || Size != sizeof( u32 ) ) return false;

			MemCopy( &m_PressedCount, Bytes, sizeof( u32 ) );
			return true;
		}

		u32 GetPressedCount() const noexcept { return m_PressedCount; }

	private:
		/** 検出した押下開始の回数。 */
		u32 m_PressedCount = 0u;
	};

	/** 明示したアクション入力を毎ステップ返す入力元。 */
	class CFixedActionInputSource final : public IActionInputSource
	{
	public:
		bool TryGetActionInput( FActionInput& OutInput ) noexcept override
		{
			OutInput = m_Input;
			return true;
		}

		/** 返す入力の押下状態を変更する。 */
		void SetDown( u32 ActionIndex, bool bDown ) noexcept { m_Input.SetDown( ActionIndex, bDown ); }

	private:
		/** 毎ステップ返す入力。 */
		FActionInput m_Input;
	};

}


void RunSimulationSnapshotTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CSimulationSnapshot / 盤面を出せない規則は写さない" );

	{
		// 時計と乱数だけ写しても «続きから» にはならない。半端に写せてしまうほうが危ない。
		CFixedStepDriver Driver;
		CDeterministicRandom Random;
		COpaqueRule Rule;

		CSimulationSnapshot Snapshot;
		Harness.Check( !Snapshot.TryCaptureFrom( Driver, Random, Rule ), "写せない" );
		Harness.Check( !Snapshot.IsValid(), "無効のまま" );

		CFixedStepDriver OtherDriver;
		CDeterministicRandom OtherRandom;
		Harness.Check( !Snapshot.TryRestoreTo( OtherDriver, OtherRandom, Rule ), "無効なものは戻せない" );
	}

	Harness.BeginSuite( "CSimulationSnapshot / 写して戻す" );

	{
		CFixedStepDriver Driver;
		CDeterministicRandom Random;
		CCountingRule Rule;

		Driver.Configure( 1.0 / 60.0, 8u );
		Random.Reseed( 1234u );

		for ( u32 Step = 0u; Step < 10u; ++Step )
		{
			Random.NextU32();
			Rule.SetCounter( Rule.GetCounter() + 1u );
			Driver.AdvanceTick();
		}

		CSimulationSnapshot Snapshot;
		Harness.Check( Snapshot.TryCaptureFrom( Driver, Random, Rule ), "写せる" );
		Harness.Check( !Snapshot.HasInputHistory(), "低レベルAPIは入力履歴を暗黙に作らない" );
		Harness.CheckEqualU64( Snapshot.GetTick(), 10u, "ティックを覚えている" );
		Harness.CheckEqualU64( Snapshot.GetDrawCount(), 10u, "引いた回数を覚えている" );
		Harness.CheckEqualU64( Snapshot.GetRuleByteCount(), sizeof( u32 ), "盤面の大きさ" );

		// 進めてから戻す。
		for ( u32 Step = 0u; Step < 5u; ++Step )
		{
			Random.NextU32();
			Rule.SetCounter( Rule.GetCounter() + 1u );
			Driver.AdvanceTick();
		}

		Harness.CheckEqualU64( Driver.GetTick(), 15u, "進んでいる" );

		Harness.Check( Snapshot.TryRestoreTo( Driver, Random, Rule ), "戻せる" );
		Harness.CheckEqualU64( Driver.GetTick(), 10u, "ティックが戻る" );
		Harness.CheckEqualU64( Random.GetDrawCount(), 10u, "引いた回数が戻る" );
		Harness.CheckEqualU64( Rule.GetCounter(), 10u, "盤面が戻る" );
	}

	Harness.BeginSuite( "CSimulationSnapshot / 入力履歴を写して戻す" );

	{
		CFixedStepDriver Driver;
		CDeterministicRandom Random;
		CCountingRule Rule;

		Driver.Configure( 1.0 / 120.0, 4u );
		Driver.AdvanceTick();
		Random.Reseed( 99u );
		Random.NextU32();
		Rule.SetCounter( 7u );

		FActionInput LastInput;
		LastInput.SetDown( 0u, true );
		LastInput.SetAxis( 0u, 0.75f );

		FActionInput PreviousInput;
		PreviousInput.SetDown( 2u, true );
		PreviousInput.SetAxis( 1u, -0.5f );

		CSimulationSnapshot Snapshot;
		Harness.Check( Snapshot.TryCaptureFrom( Driver, Random, Rule, LastInput, PreviousInput ), "入力履歴込みで写せる" );
		Harness.Check( Snapshot.HasInputHistory(), "入力履歴があると分かる" );

		FActionInput RestoredLastInput;
		FActionInput RestoredPreviousInput;
		RestoredLastInput.SetDown( 5u, true );
		RestoredPreviousInput.SetDown( 6u, true );

		Rule.SetCounter( 99u );
		Driver.AdvanceTick();
		Random.NextU32();

		Harness.Check( Snapshot.TryRestoreTo( Driver, Random, Rule, RestoredLastInput, RestoredPreviousInput ), "入力履歴込みで戻せる" );
		Harness.Check( RestoredLastInput.Equals( LastInput ), "直近の入力が戻る" );
		Harness.Check( RestoredPreviousInput.Equals( PreviousInput ), "1つ前の入力が戻る" );
		Harness.CheckEqualU64( Driver.GetTick(), 1u, "入力と同じ時点へ時計が戻る" );
		Harness.CheckEqualU64( Random.GetDrawCount(), 1u, "入力と同じ時点へ乱数が戻る" );
		Harness.CheckEqualU64( Rule.GetCounter(), 7u, "入力と同じ時点へ盤面が戻る" );

		FSimulationContext NextContext;
		NextContext.Input = RestoredLastInput;
		NextContext.PreviousInput = RestoredLastInput;
		Harness.Check( !NextContext.WasPressed( 0u ), "復元後の長押しを新しい押下にしない" );

		TArray<u8> Bytes;
		Bytes.SetNum( Snapshot.GetRequiredBytes() );
		usize Written = 0u;
		Harness.Check( Snapshot.TrySaveToBuffer( Bytes.GetData(), Bytes.Num(), Written ), "入力履歴をバイト列へ書ける" );
		Harness.CheckEqualU64( Written, sizeof( kSnapshotV2 ), "v2の固定wire大きさを保つ" );

		bool bWireMatches = Written == sizeof( kSnapshotV2 );
		for ( usize Index = 0u; bWireMatches && Index < Written; ++Index ) bWireMatches = Bytes[Index] == kSnapshotV2[Index];
		Harness.Check( bWireMatches, "v2のfield順とbyte列を保つ" );

		CSimulationSnapshot Loaded;
		Harness.Check( Loaded.TryLoadFromBuffer( kSnapshotV2, sizeof( kSnapshotV2 ) ), "固定v2fixtureを読める" );
		Harness.Check( Loaded.HasInputHistory(), "往復後も入力履歴がある" );

		CFixedStepDriver LoadedDriver;
		LoadedDriver.Configure( 1.0 / 30.0, 2u );
		LoadedDriver.AdvanceTick();
		LoadedDriver.AdvanceTick();
		CDeterministicRandom LoadedRandom;
		LoadedRandom.Reseed( 123u );
		LoadedRandom.NextU32();
		LoadedRandom.NextU32();
		CCountingRule LoadedRule;
		LoadedRule.SetCounter( 88u );

		FActionInput LoadedLastInput;
		FActionInput LoadedPreviousInput;
		LoadedLastInput.SetDown( 8u, true );
		LoadedPreviousInput.SetDown( 9u, true );
		Harness.Check( Loaded.TryRestoreTo( LoadedDriver, LoadedRandom, LoadedRule, LoadedLastInput, LoadedPreviousInput ), "異なる状態へ固定v2fixtureを戻せる" );
		Harness.CheckEqualU64( LoadedDriver.GetTick(), 1u, "固定v2fixtureのティックが戻る" );
		Harness.CheckNearF32( LoadedDriver.GetStepSeconds(), 1.0f / 120.0f, 0.0001f, "固定v2fixtureの時計設定が戻る" );
		Harness.CheckEqualU64( LoadedRule.GetCounter(), 7u, "固定v2fixtureの盤面が戻る" );
		Harness.Check( LoadedLastInput.Equals( LastInput ) && LoadedPreviousInput.Equals( PreviousInput ), "往復後の入力履歴が一致する" );
		Harness.CheckEqualU64( LoadedRandom.NextU32(), kSnapshotV2NextRandom, "固定v2fixtureの次の乱数値が一致する" );
	}

	Harness.BeginSuite( "CSimulationSnapshot / 戻せないときは何も変えない" );

	{
		// 一部だけ戻った状態から進むと、原因の分からないずれ方をする。
		CFixedStepDriver Driver;
		CDeterministicRandom Random;
		Driver.Configure( 1.0 / 60.0, 8u );
		Random.Reseed( 5u );
		Driver.AdvanceTick();
		Driver.AdvanceTick();

		CAtomicRejectingRule SourceRule( 123u );
		FActionInput SnapshotLastInput;
		FActionInput SnapshotPreviousInput;
		SnapshotLastInput.SetDown( 0u, true );
		SnapshotPreviousInput.SetDown( 1u, true );

		CSimulationSnapshot Snapshot;
		Harness.Check( Snapshot.TryCaptureFrom( Driver, Random, SourceRule, SnapshotLastInput, SnapshotPreviousInput ), "不正guardを持つ8 byte候補を写せる" );

		Driver.Configure( 1.0 / 75.0, 5u );
		const u32 AdvancedSteps = Driver.Advance( 1.0 );
		for ( u32 Step = 0u; Step < AdvancedSteps; ++Step ) Driver.AdvanceTick();
		Random.NextU32();

		// 候補を最後まで読んでguardを拒否しても、実際の盤面は変えない。
		CAtomicRejectingRule Other( 77u );
		FFixedStepClockSnapshot ClockBefore;
		u32 TickBefore = 0u;
		Harness.Check( Driver.TryCaptureSnapshot( ClockBefore, TickBefore ), "失敗前の時計全体を控えられる" );
		const bool bWasClampedBefore = Driver.WasClamped();
		FRandomSnapshot RandomBefore;
		u64 DrawCountBefore = 0u;
		Random.CaptureSnapshot( RandomBefore, DrawCountBefore );
		const u64 SeedBefore = Random.GetSeed();
		const u32 RuleValueBefore = Other.GetValue();
		FActionInput RestoredLastInput;
		FActionInput RestoredPreviousInput;
		RestoredLastInput.SetDown( 5u, true );
		RestoredLastInput.SetAxis( 0u, 0.25f );
		RestoredPreviousInput.SetDown( 6u, true );
		RestoredPreviousInput.SetAxis( 1u, -0.75f );
		const FActionInput LastInputBefore = RestoredLastInput;
		const FActionInput PreviousInputBefore = RestoredPreviousInput;

		Harness.Check( !Snapshot.TryRestoreTo( Driver, Random, Other, RestoredLastInput, RestoredPreviousInput ), "不正guardを持つ候補は戻さない" );

		FFixedStepClockSnapshot ClockAfter;
		u32 TickAfter = 0u;
		Harness.Check( Driver.TryCaptureSnapshot( ClockAfter, TickAfter ), "失敗後の時計全体を控えられる" );
		Harness.Check( TickAfter == TickBefore && ClockSnapshotsEqual( ClockAfter, ClockBefore )
			&& Driver.WasClamped() == bWasClampedBefore, "時計の全状態は動いていない" );

		FRandomSnapshot RandomAfter;
		u64 DrawCountAfter = 0u;
		Random.CaptureSnapshot( RandomAfter, DrawCountAfter );
		Harness.Check( DrawCountAfter == DrawCountBefore && Random.GetSeed() == SeedBefore
			&& RandomSnapshotsEqual( RandomAfter, RandomBefore ), "乱数の全状態は動いていない" );
		Harness.CheckEqualU64( Other.GetValue(), RuleValueBefore, "規則は失敗時に盤面を変えない" );
		Harness.Check( RestoredLastInput.Equals( LastInputBefore ) && RestoredPreviousInput.Equals( PreviousInputBefore ), "入力出力の全fieldは変わらない" );

		CDeterministicRandom ExpectedRandom;
		Harness.Check( ExpectedRandom.TryRestoreSnapshot( RandomBefore, DrawCountBefore ), "次値比較用の乱数を戻せる" );
		Harness.CheckEqualU64( Random.NextU32(), ExpectedRandom.NextU32(), "失敗後の乱数内部位置も変わらない" );
	}

	Harness.BeginSuite( "CSimulationSnapshot / バイト列の往復と、壊れたもの" );

	{
		CFixedStepDriver Driver;
		CDeterministicRandom Random;
		CCountingRule Rule;

		Driver.Configure( 1.0 / 120.0, 4u );
		Random.Reseed( 99u );
		Random.NextU32();
		Rule.SetCounter( 7u );
		Driver.AdvanceTick();

		CSimulationSnapshot Snapshot;
		Snapshot.TryCaptureFrom( Driver, Random, Rule );

		TArray<u8> Bytes;
		Bytes.SetNum( Snapshot.GetRequiredBytes() );

		usize Written = 0u;
		Harness.Check( Snapshot.TrySaveToBuffer( Bytes.GetData(), Bytes.Num(), Written ), "書き出せる" );
		Harness.CheckEqualU64( Written, Snapshot.GetRequiredBytes(), "書けた大きさ" );

		CSimulationSnapshot Loaded;
		Harness.Check( Loaded.TryLoadFromBuffer( Bytes.GetData(), Written ), "読み込める" );
		Harness.CheckEqualU64( Loaded.GetTick(), Snapshot.GetTick(), "ティックが一致" );
		Harness.CheckEqualU64( Loaded.GetDrawCount(), Snapshot.GetDrawCount(), "引いた回数が一致" );
		Harness.CheckEqualU64( Loaded.GetRuleByteCount(), Snapshot.GetRuleByteCount(), "盤面の大きさが一致" );

		// 読み込んだものから戻せること (別の部品一式へ)。
		CFixedStepDriver OtherDriver;
		CDeterministicRandom OtherRandom;
		CCountingRule OtherRule;
		OtherDriver.Configure( 1.0 / 60.0, 8u );

		Harness.Check( Loaded.TryRestoreTo( OtherDriver, OtherRandom, OtherRule ), "読み込んだものから戻せる" );
		Harness.CheckEqualU64( OtherRule.GetCounter(), 7u, "盤面が移る" );
		Harness.CheckEqualU64( OtherDriver.GetTick(), 1u, "ティックが移る" );
		Harness.CheckNearF32( OtherDriver.GetStepSeconds(), 1.0f / 120.0f, 0.0001f, "ステップ幅も移る" );

		// 壊れたものを弾く。
		Harness.Check( !Loaded.TryLoadFromBuffer( nullptr, 0u ), "nullptr は弾く" );
		Harness.Check( !Loaded.TryLoadFromBuffer( Bytes.GetData(), 8u ), "短すぎるものは弾く" );

		TArray<u8> Broken;
		Broken.SetNum( Written );
		MemCopy( Broken.GetData(), Bytes.GetData(), Written );
		Broken[0] = static_cast<u8>( Broken[0] ^ 0xFFu );
		Harness.Check( !Loaded.TryLoadFromBuffer( Broken.GetData(), Written ), "目印が違うものは弾く" );

		MemCopy( Broken.GetData(), Bytes.GetData(), Written );
		const u32 InvalidInputHistory = 2u;
		MemCopy( Broken.GetData() + kInputHistoryFlagOffset, &InvalidInputHistory, sizeof( u32 ) );
		Harness.Check( !Loaded.TryLoadFromBuffer( Broken.GetData(), Written ), "不正な入力履歴フラグは弾く" );

		TArray<u8> WithTrailingByte;
		WithTrailingByte.SetNum( Written + 1u );
		MemCopy( WithTrailingByte.GetData(), Bytes.GetData(), Written );
		WithTrailingByte[Written] = 0xCCu;
		Harness.Check( !Loaded.TryLoadFromBuffer( WithTrailingByte.GetData(), WithTrailingByte.Num() ), "v2末尾の余剰データは弾く" );

		Harness.Check( !Loaded.TryLoadFromBuffer( Bytes.GetData(), Written - 1u ), "途中で切れたものは弾く" );
		Harness.Check( !Loaded.IsValid(), "失敗したら無効になる" );
	}

	Harness.BeginSuite( "CSimulationSnapshot / 旧v1形式を互換読込する" );

	{
		CSimulationSnapshot Loaded;
		Harness.Check( Loaded.TryLoadFromBuffer( kLegacySnapshotV1, sizeof( kLegacySnapshotV1 ) ), "固定した旧v1形式を読める" );
		Harness.Check( Loaded.IsValid(), "旧v1形式も有効になる" );
		Harness.Check( !Loaded.HasInputHistory(), "旧v1形式には入力履歴がない" );
		Harness.CheckEqualU64( Loaded.GetTick(), 2u, "旧v1fixtureのティックを読める" );
		Harness.CheckEqualU64( Loaded.GetDrawCount(), 1u, "旧v1fixtureの乱数位置を読める" );

		TArray<u8> LegacyWithTrailingByte;
		LegacyWithTrailingByte.SetNum( sizeof( kLegacySnapshotV1 ) + 1u );
		MemCopy( LegacyWithTrailingByte.GetData(), kLegacySnapshotV1, sizeof( kLegacySnapshotV1 ) );
		LegacyWithTrailingByte[sizeof( kLegacySnapshotV1 )] = 0xCCu;
		CSimulationSnapshot LegacyLoadedWithTrailingByte;
		Harness.Check( LegacyLoadedWithTrailingByte.TryLoadFromBuffer( LegacyWithTrailingByte.GetData(), LegacyWithTrailingByte.Num() ), "旧v1の余剰許容は互換維持する" );

		CFixedStepDriver TargetDriver;
		CDeterministicRandom TargetRandom;
		CCountingRule TargetRule;
		TargetDriver.AdvanceTick();
		TargetRandom.Reseed( 99u );
		TargetRandom.NextU32();
		TargetRandom.NextU32();
		TargetRule.SetCounter( 77u );

		FActionInput TargetLastInput;
		FActionInput TargetPreviousInput;
		TargetLastInput.SetDown( 5u, true );
		TargetPreviousInput.SetDown( 6u, true );

		Harness.Check( !Loaded.TryRestoreTo( TargetDriver, TargetRandom, TargetRule, TargetLastInput, TargetPreviousInput ), "入力履歴込み復元では旧v1形式を拒否する" );
		Harness.CheckEqualU64( TargetDriver.GetTick(), 1u, "拒否時に時計を変えない" );
		Harness.CheckEqualU64( TargetRandom.GetDrawCount(), 2u, "拒否時に乱数を変えない" );
		Harness.CheckEqualU64( TargetRule.GetCounter(), 77u, "拒否時に盤面を変えない" );
		Harness.Check( TargetLastInput.IsDown( 5u ) && TargetPreviousInput.IsDown( 6u ), "拒否時に入力出力を変えない" );

		Harness.Check( Loaded.TryRestoreTo( TargetDriver, TargetRandom, TargetRule ), "従来の低レベル復元では旧v1形式を使える" );
		Harness.CheckEqualU64( TargetDriver.GetTick(), 2u, "旧v1の時計が戻る" );
		Harness.CheckEqualU64( TargetRandom.GetDrawCount(), 1u, "旧v1の乱数が戻る" );
		Harness.CheckEqualU64( TargetRule.GetCounter(), 42u, "旧v1の盤面が戻る" );
		Harness.CheckNearF32( TargetDriver.GetStepSeconds(), 1.0f / 120.0f, 0.0001f, "旧v1の時計設定が戻る" );
		Harness.CheckEqualU64( TargetRandom.NextU32(), kLegacySnapshotNextRandom, "旧v1の次の乱数値が一致する" );
	}

	Harness.BeginSuite( "CSimulationSubsystem / 復元後の長押しを再発火しない" );

	{
		CSimulationSubsystem Simulation;

		TUniquePtr<CPressedCountingRule> Rule = MakeUnique<CPressedCountingRule>();
		CPressedCountingRule* const RuleView = Rule.Get();
		Harness.Check( Simulation.SetRule( Move( Rule ) ), "規則を設定できる" );

		TUniquePtr<CFixedActionInputSource> Input = MakeUnique<CFixedActionInputSource>();
		CFixedActionInputSource* const InputView = Input.Get();
		InputView->SetDown( 0u, true );
		Harness.Check( Simulation.SetInputSource( Move( Input ) ), "入力元を設定できる" );

		Simulation.StartLive( 123u );
		Simulation.AdvanceSteps( 1u );
		Harness.CheckEqualU64( RuleView->GetPressedCount(), 1u, "最初の押下だけを数える" );

		CSimulationSnapshot Snapshot;
		Harness.Check( Simulation.TryCaptureSnapshot( Snapshot ), "サブシステムから入力履歴込みで写せる" );
		Harness.Check( Snapshot.HasInputHistory(), "サブシステムの写しに入力履歴がある" );

		Simulation.AdvanceSteps( 1u );
		Harness.CheckEqualU64( RuleView->GetPressedCount(), 1u, "長押し中は増えない" );
		Harness.Check( Simulation.TryRestoreSnapshot( Snapshot ), "サブシステムへ入力履歴込みで戻せる" );
		Harness.CheckEqualU64( Simulation.GetTick(), 1u, "写したティックへ戻る" );

		Simulation.AdvanceSteps( 1u );
		Harness.CheckEqualU64( RuleView->GetPressedCount(), 1u, "復元直後も長押しを押下開始にしない" );

		CFixedStepDriver LegacyDriver;
		CDeterministicRandom LegacyRandom;
		CSimulationSnapshot WithoutInputHistory;
		Harness.Check( WithoutInputHistory.TryCaptureFrom( LegacyDriver, LegacyRandom, *RuleView ), "入力履歴のない低レベル写しを作れる" );

		const u32 TickBeforeRejectedRestore = Simulation.GetTick();
		const u32 CountBeforeRejectedRestore = RuleView->GetPressedCount();
		Harness.Check( !Simulation.TryRestoreSnapshot( WithoutInputHistory ), "サブシステムは入力履歴のない写しを拒否する" );
		Harness.CheckEqualU64( Simulation.GetTick(), TickBeforeRejectedRestore, "拒否時にサブシステムの時計を変えない" );
		Harness.CheckEqualU64( RuleView->GetPressedCount(), CountBeforeRejectedRestore, "拒否時にサブシステムの盤面を変えない" );
	}
}
