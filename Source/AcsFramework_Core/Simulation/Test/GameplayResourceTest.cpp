// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/GameplayResource.h"
#include "Common/Test/TestHarness.h"

#include <cmath>
#include <limits>


namespace
{
	/** ゲーム資源の保存値が全項目で一致するか返す。 */
	bool GameplayResourceStatesEqual_Internal(
		const FGameplayResourceState& Left,
		const FGameplayResourceState& Right ) noexcept
	{
		return Left.MaximumValue == Right.MaximumValue
			&& Left.CurrentValue == Right.CurrentValue;
	}
}


void RunGameplayResourceTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FGameplayResource / 上限と現在値を構成する" );

	{
		/** 設定なしで安全に使える既定資源。 */
		const FGameplayResource Defaults;
		/** 上限を指定して満杯から始める資源。 */
		const FGameplayResource Full{ 100.0f };
		/** 上限と現在値を同時指定した資源。 */
		FGameplayResource Partial{ 100.0f, 75.0f };
		Harness.Check( Defaults.GetMaximumValue() == 1.0f
			&& Defaults.GetCurrentValue() == 1.0f
			&& Defaults.GetRatio() == 1.0f && Defaults.IsFull(),
			"既定は1/1の満杯状態になる" );
		Harness.Check( Full.GetMaximumValue() == 100.0f
			&& Full.GetCurrentValue() == 100.0f && Full.IsFull(),
			"上限だけなら同じ現在値で満杯になる" );
		Harness.Check( Partial.GetMaximumValue() == 100.0f
			&& Partial.GetCurrentValue() == 75.0f
			&& Partial.GetMissingValue() == 25.0f
			&& Partial.GetRatio() == 0.75f,
			"上限、現在値、不足量と割合を同じ状態から返す" );
		Harness.Check( Partial.TryConfigure( 80.0f, 20.0f )
			&& Partial.GetMaximumValue() == 80.0f
			&& Partial.GetCurrentValue() == 20.0f,
			"有効な上限と現在値を原子的に設定する" );
	}

	Harness.BeginSuite( "FGameplayResource / 全量を消費する" );

	{
		/** 消費境界を確認する資源。 */
		FGameplayResource Resource{ 100.0f, 75.0f };
		Harness.Check( Resource.TrySpend( 25.0f )
			&& Resource.GetCurrentValue() == 50.0f
			&& Resource.GetRatio() == 0.5f,
			"足りる量を全て現在値から引く" );
		/** 不足時に変化しないことを比べる直前状態。 */
		const FGameplayResourceState BeforeInsufficient =
			Resource.CaptureState();
		Harness.Check( !Resource.TrySpend( 50.01f )
			&& GameplayResourceStatesEqual_Internal(
				Resource.CaptureState(), BeforeInsufficient ),
			"不足している消費要求を一切反映しない" );
		Harness.Check( Resource.TrySpend( 0.0f )
			&& Resource.GetCurrentValue() == 50.0f,
			"0の消費を有効な変更なしとして受理する" );
		Harness.Check( Resource.TrySpend( 50.0f )
			&& Resource.IsEmpty() && Resource.GetCurrentValue() == 0.0f
			&& Resource.GetRatio() == 0.0f,
			"残量ちょうどを消費して正確に空にする" );
	}

	Harness.BeginSuite( "FGameplayResource / 上限まで回復する" );

	{
		/** 回復と上限変更を確認する資源。 */
		FGameplayResource Resource{ 100.0f, 40.0f };
		/** 上限で切られた実回復量。 */
		f32 AppliedAmount = -1.0f;
		Harness.Check( Resource.TryRestore( 80.0f, AppliedAmount )
			&& AppliedAmount == 60.0f && Resource.IsFull()
			&& Resource.GetMissingValue() == 0.0f,
			"回復要求を上限で止めて実増加量を返す" );
		AppliedAmount = -1.0f;
		Harness.Check( Resource.TryRestore( 10.0f, AppliedAmount )
			&& AppliedAmount == 0.0f && Resource.IsFull(),
			"満杯への回復は0増加の成功として扱う" );

		Resource.Empty();
		Harness.Check( Resource.TryRestore( 25.0f )
			&& Resource.GetCurrentValue() == 25.0f,
			"増加量が不要な呼出でも上限内で回復する" );
		Harness.Check( Resource.TrySetMaximumValue( 20.0f )
			&& Resource.GetMaximumValue() == 20.0f
			&& Resource.GetCurrentValue() == 20.0f,
			"上限を下げたときだけ現在値を新上限へ収める" );
		Harness.Check( Resource.TrySetMaximumValue( 40.0f )
			&& Resource.GetMaximumValue() == 40.0f
			&& Resource.GetCurrentValue() == 20.0f,
			"上限を上げても現在値を勝手に増やさない" );
		Harness.Check( Resource.TrySetCurrentValue( 10.0f )
			&& Resource.GetCurrentValue() == 10.0f,
			"上限内の現在値を直接設定する" );
		Resource.Fill();
		Harness.Check( Resource.IsFull()
			&& Resource.GetCurrentValue() == 40.0f,
			"満杯操作で現在値を上限へ揃える" );
	}

	Harness.BeginSuite( "FGameplayResource / 有限値全域で境界を守る" );

	{
		/** 単精度で表せる最大の上限。 */
		const f32 HugeValue = std::numeric_limits<f32>::max();
		/** 最大上限へ回復する資源。 */
		FGameplayResource Huge{ HugeValue, 0.0f };
		/** 最大値まで実際に増えた量。 */
		f32 HugeApplied = 0.0f;
		Harness.Check( Huge.TryRestore( HugeValue, HugeApplied )
			&& HugeApplied == HugeValue && Huge.IsFull()
			&& std::isfinite( Huge.GetCurrentValue() ),
			"最大有限値までoverflowさせず回復する" );

		/** 単精度で表せる最小の正の上限。 */
		const f32 TinyValue = std::numeric_limits<f32>::denorm_min();
		/** 最小上限へ回復する資源。 */
		FGameplayResource Tiny{ TinyValue, 0.0f };
		/** 最小値まで実際に増えた量。 */
		f32 TinyApplied = 0.0f;
		Harness.Check( Tiny.TryRestore( TinyValue, TinyApplied )
			&& TinyApplied == TinyValue && Tiny.IsFull()
			&& Tiny.GetRatio() == 1.0f,
			"最小正数も0へ潰さず上限として扱う" );
	}

	Harness.BeginSuite( "FGameplayResource / 不正入力を原子的に扱う" );

	{
		/** 不正操作で変化しないことを確認する資源。 */
		FGameplayResource Resource{ 100.0f, 40.0f };
		/** 全不正操作より前の保存値。 */
		const FGameplayResourceState BeforeInvalid = Resource.CaptureState();
		/** 失敗時に保持される回復量出力。 */
		f32 AppliedAmount = 7.0f;
		/** 非数入力。 */
		const f32 NotANumber =
			std::numeric_limits<f32>::quiet_NaN();
		/** 無限入力。 */
		const f32 Infinity = std::numeric_limits<f32>::infinity();
		const bool bRejected =
			!Resource.TryConfigure( 0.0f, 0.0f )
			&& !Resource.TryConfigure( 100.0f, 100.01f )
			&& !Resource.TryConfigure( NotANumber, 0.0f )
			&& !Resource.TrySetMaximumValue( -1.0f )
			&& !Resource.TrySetMaximumValue( Infinity )
			&& !Resource.TrySetCurrentValue( -0.01f )
			&& !Resource.TrySetCurrentValue( 100.01f )
			&& !Resource.TrySetCurrentValue( NotANumber )
			&& !Resource.TrySpend( -0.01f )
			&& !Resource.TrySpend( 40.01f )
			&& !Resource.TrySpend( Infinity )
			&& !Resource.TryRestore( -0.01f, AppliedAmount )
			&& !Resource.TryRestore( NotANumber, AppliedAmount );
		Harness.Check( bRejected && AppliedAmount == 7.0f,
			"範囲外と非有限の設定・増減を拒否する" );
		Harness.Check( GameplayResourceStatesEqual_Internal(
				Resource.CaptureState(), BeforeInvalid ),
			"不正入力では上限と現在値を一切変えない" );

		/** 不正な上限だけを渡した構築結果。 */
		const FGameplayResource InvalidMaximum{ -1.0f };
		/** 上限を超える現在値を渡した構築結果。 */
		const FGameplayResource InvalidPair{ 10.0f, 11.0f };
		Harness.Check( InvalidMaximum.GetMaximumValue() == 1.0f
			&& InvalidMaximum.IsFull()
			&& InvalidPair.GetMaximumValue() == 1.0f
			&& InvalidPair.IsFull(),
			"不正な構築値では既定の1/1を維持する" );
	}

	Harness.BeginSuite( "FGameplayResource / 保存して原子的に復元する" );

	{
		/** 保存元になる途中資源。 */
		FGameplayResource Source{ 250.0f, 125.0f };
		Source.TrySpend( 25.0f );
		/** 上限と現在値を含む保存値。 */
		const FGameplayResourceState Saved = Source.CaptureState();
		/** 保存値を受け取る別の資源。 */
		FGameplayResource Restored;
		Harness.Check( Saved.IsValid() && Restored.RestoreState( Saved )
			&& GameplayResourceStatesEqual_Internal(
				Restored.CaptureState(), Saved ),
			"上限と途中の現在値を別の資源へ復元する" );

		/** 不正復元より前の受取側状態。 */
		const FGameplayResourceState BeforeFailure = Restored.CaptureState();
		/** 0上限で無効にした保存値。 */
		FGameplayResourceState InvalidMaximum = Saved;
		InvalidMaximum.MaximumValue = 0.0f;
		/** 上限を超えて無効にした保存値。 */
		FGameplayResourceState InvalidCurrent = Saved;
		InvalidCurrent.CurrentValue = 251.0f;
		/** 非数で無効にした保存値。 */
		FGameplayResourceState InvalidFinite = Saved;
		InvalidFinite.CurrentValue =
			std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !InvalidMaximum.IsValid()
			&& !InvalidCurrent.IsValid() && !InvalidFinite.IsValid()
			&& !Restored.RestoreState( InvalidMaximum )
			&& !Restored.RestoreState( InvalidCurrent )
			&& !Restored.RestoreState( InvalidFinite ),
			"矛盾した保存値を復元前に拒否する" );
		Harness.Check( GameplayResourceStatesEqual_Internal(
				Restored.CaptureState(), BeforeFailure ),
			"不正な保存値では現在状態を一切変えない" );
	}
}
