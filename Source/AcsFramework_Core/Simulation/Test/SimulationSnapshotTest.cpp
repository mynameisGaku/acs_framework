// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/SimulationSnapshot.h"
#include "Common/Test/TestHarness.h"

namespace
{
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

	Harness.BeginSuite( "CSimulationSnapshot / 戻せないときは何も変えない" );

	{
		// 一部だけ戻った状態から進むと、原因の分からないずれ方をする。
		CFixedStepDriver Driver;
		CDeterministicRandom Random;
		CCountingRule Rule;

		Driver.Configure( 1.0 / 60.0, 8u );
		Random.Reseed( 5u );
		Rule.SetCounter( 3u );
		Driver.AdvanceTick();
		Driver.AdvanceTick();

		CSimulationSnapshot Snapshot;
		Snapshot.TryCaptureFrom( Driver, Random, Rule );

		// 盤面を受け取らない規則へ戻そうとする。
		COpaqueRule Other;
		const u32 TickBefore = Driver.GetTick();

		Harness.Check( !Snapshot.TryRestoreTo( Driver, Random, Other ), "受け取れない規則へは戻さない" );
		Harness.CheckEqualU64( Driver.GetTick(), TickBefore, "時計は動いていない" );
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

		Harness.Check( !Loaded.TryLoadFromBuffer( Bytes.GetData(), Written - 1u ), "途中で切れたものは弾く" );
		Harness.Check( !Loaded.IsValid(), "失敗したら無効になる" );
	}
}
