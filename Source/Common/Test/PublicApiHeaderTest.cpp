// SPDX-License-Identifier: Apache-2.0

#include "AcsFramework_Core/AcsFramework.h"

#include "Common/Test/TestHarness.h"

#include <cmath>

/**
 * 利用側が共通ヘッダーだけを読み込んでも、3D公開入口の型が解決できることを確かめる。
 *
 * @param Harness 単体テストの結果を集める土台。
 */
void RunPublicApiHeaderTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "AcsFramework.h / 利用側の共通includeだけで公開APIを解決する" );

	const FVec3 Position{};
	const FCollidableModel3DSpawnResult EmptyResult{};
	const FCheckpoint3DParams CheckpointParams = FCheckpoint3DParams::Around( 2.0f );
	const FCheckpoint3DSpawnResult EmptyCheckpoint{};
	const FCheckpointRoute3DParams RouteParams =
		FCheckpointRoute3DParams::ForCheckpoints( 3u, 2u );
	const FCheckpointRoute3DProgress RouteProgress =
		FCheckpointRoute3D{}.CaptureProgress();
	FCheckpointRoute3DTimer RouteTimer;
	const FCheckpointRoute3DTimerState RouteTimerState = RouteTimer.CaptureState();
	const FCheckpointRoute3DTimingResult RouteTiming{};
	const EVisualPreset3D Preset = EVisualPreset3D::Balanced;
	const FActionAxisResponse AxisResponse;
	const FActionDirectionQuantizer ActionDirectionQuantizer;
	EActionDirection2D PublicActionDirection = EActionDirection2D::None;
	FVec2 PublicActionDirectionVector{};
	const FActionDirectionTracker ActionDirectionTracker;
	const FActionDirectionTrackerState ActionDirectionTrackerState =
		ActionDirectionTracker.CaptureState();
	const FActionChord ActionChord{ 0u };
	const FActionCommandSequenceTracker ActionCommandSequence;
	const FActionCommandSequenceTrackerState ActionCommandSequenceState =
		ActionCommandSequence.CaptureState();
	const FActionHoldTracker ActionHold;
	const FActionHoldTrackerState ActionHoldState = ActionHold.CaptureState();
	const FActionRepeatTracker ActionRepeat;
	const FActionRepeatTrackerState ActionRepeatState = ActionRepeat.CaptureState();
	const FActionInputBuffer ActionBuffer;
	const FActionInputBufferState ActionBufferState = ActionBuffer.CaptureState();
	const FActionInputMask ActionMask;
	const FActionInputMaskStack ActionMaskStack;
	const FActionInputMaskStackState ActionMaskStackState =
		ActionMaskStack.CaptureState();
	const CActionInputTracker ActionInput;
	const FActionTapSequenceTracker ActionTapSequence;
	const FActionTapSequenceTrackerState ActionTapSequenceState =
		ActionTapSequence.CaptureState();
	const FActionToggle ActionToggle;
	const FGameplayCooldown GameplayCooldown;
	const FGameplayCooldownState GameplayCooldownState =
		GameplayCooldown.CaptureState();
	FGameplayChargePool GameplayCharges{ 2u, 1u, 0.5f };
	const FGameplayChargePoolState GameplayChargeState =
		GameplayCharges.CaptureState();
	FGameplayInterval GameplayInterval{ 0.5f };
	const FGameplayIntervalState GameplayIntervalState =
		GameplayInterval.CaptureState();
	FGameplayResource GameplayResource{ 100.0f, 40.0f };
	const FGameplayResourceState GameplayResourceState =
		GameplayResource.CaptureState();
	FGameplayTimer GameplayTimer{ 2.0f };
	const FGameplayTimerState GameplayTimerState =
		GameplayTimer.CaptureState();
	CDeterministicRandom WeightedRandom;
	constexpr f32 PublicWeights[] = { 1.0f };
	usize WeightedIndex = 9u;
	u32 PublicOrder[] = { 0u, 1u, 2u };
	usize PublicRandomIndex = 9u;
	FVec3 PublicBoxPoint{};
	FVec3 PublicTrianglePoint{};
	FVec3 PublicDiskPoint{};
	FVec3 PublicCylinderPoint{};
	FVec3 PublicCapsulePoint{};
	FVec3 PublicSpherePoint{};
	FVec3 PublicConeDirection{};
	u32 PublicRestoredChargeCount = 0u;
	u32 PublicOccurrenceCount = 0u;
	bool bPublicChanceOccurred = false;
	WeightedRandom.Reseed( 1u );

	Harness.Check( Position.x == 0.0f, "ACSの基本型が解決できる" );
	Harness.Check( !EmptyResult.Succeeded(), "3D生成結果が解決できる" );
	Harness.Check( CheckpointParams.IsValid() && !EmptyCheckpoint.Succeeded(),
		"3Dチェックポイントの設定と生成結果が解決できる" );
	Harness.Check( RouteParams.IsValid() && RouteProgress.IsValid(),
		"3Dチェックポイント順序ルートの設定と進行値が解決できる" );
	Harness.Check( !RouteTimer.IsRunning()
		&& RouteTimerState.IsValid()
		&& RouteTiming.TotalElapsedSeconds == 0.0,
		"3Dチェックポイント順序ルートの計測型が解決できる" );
	Harness.Check( Preset == EVisualPreset3D::Balanced, "3D見た目設定が解決できる" );
	f32 AxisValue = 9.0f;
	Harness.Check( AxisResponse.TryApply( 0.0f, AxisValue ) && AxisValue == 0.0f,
		"アナログ軸応答の公開型が解決できる" );
	Harness.Check( ActionDirectionQuantizer.TryResolve( FVec2{ 1.0f, 0.0f },
			EActionDirection2D::None, PublicActionDirection )
		&& PublicActionDirection == EActionDirection2D::Right
		&& TryGetActionDirection2DVector(
			PublicActionDirection, PublicActionDirectionVector )
		&& PublicActionDirectionVector.x == 1.0f,
		"2軸の離散方向変換を共通ヘッダーから使える" );
	Harness.Check( !ActionDirectionTracker.IsActive()
		&& ActionDirectionTrackerState.IsValid(),
		"離散方向追跡と保存状態の公開型が解決できる" );
	Harness.Check( ActionChord.IsValid() && ActionChord.IsActionRequired( 0u ),
		"アクション同時押しの公開型が解決できる" );
	Harness.Check( !ActionCommandSequence.IsConfigured()
		&& ActionCommandSequenceState.IsValid(),
		"アクション順序入力と保存状態の公開型が解決できる" );
	Harness.Check( !ActionHold.IsHolding() && ActionHoldState.IsValid(),
		"長押し判定と保存状態の公開型が解決できる" );
	Harness.Check( !ActionRepeat.IsTracking() && ActionRepeatState.IsValid(),
		"長押しrepeat判定と保存状態の公開型が解決できる" );
	Harness.Check( !ActionBuffer.IsBuffered( 0u ) && ActionBufferState.IsValid(),
		"入力猶予と保存状態の公開型が解決できる" );
	Harness.Check( ActionMask.IsActionEnabled( 0u )
		&& ActionMask.IsAxisEnabled( 0u ),
		"入力許可マスクの公開型が解決できる" );
	Harness.Check( ActionMaskStack.IsEmpty() && ActionMaskStackState.IsValid(),
		"入力マスクstackと保存状態の公開型が解決できる" );
	Harness.Check( ActionInput.GetCurrentInput().IsNeutral(), "通常フレームのアクション入力が解決できる" );
	Harness.Check( !ActionTapSequence.IsWaitingForNextTap()
		&& ActionTapSequenceState.IsValid(),
		"複数回タップ判定と保存状態の公開型が解決できる" );
	Harness.Check( !ActionToggle.IsEnabled(),
		"押下トグルの公開型が解決できる" );
	Harness.Check( GameplayCooldown.IsReady()
		&& GameplayCooldownState.IsValid(),
		"再使用待ちと保存状態の公開型が解決できる" );
	Harness.Check( GameplayChargeState.IsValid()
		&& GameplayCharges.Update( 0.5f, PublicRestoredChargeCount )
		&& PublicRestoredChargeCount == 1u && GameplayCharges.IsFull(),
		"自動回復チャージと保存状態の公開型が解決できる" );
	Harness.Check( GameplayIntervalState.IsValid()
		&& GameplayInterval.Start()
		&& GameplayInterval.Update( 0.5f, PublicOccurrenceCount )
		&& PublicOccurrenceCount == 1u,
		"周期到達回数と保存状態の公開型が解決できる" );
	Harness.Check( GameplayResourceState.IsValid()
		&& GameplayResource.TrySpend( 10.0f )
		&& GameplayResource.GetCurrentValue() == 30.0f,
		"上限付きゲーム資源と保存状態の公開型が解決できる" );
	Harness.Check( GameplayTimerState.IsValid()
		&& GameplayTimer.Start() && GameplayTimer.IsRunning(),
		"局所ゲームプレイタイマーと保存状態の公開型が解決できる" );
	Harness.Check( WeightedRandom.TryChance( 1.0f, bPublicChanceOccurred )
		&& bPublicChanceOccurred,
		"決定論的な成功確率判定の公開APIが解決できる" );
	Harness.Check( WeightedRandom.TryChooseWeightedIndex(
			PublicWeights, 1u, WeightedIndex )
		&& WeightedIndex == 0u,
		"決定論的な重み付き抽選の公開APIが解決できる" );
	Harness.Check( WeightedRandom.TryShuffle( PublicOrder, 3u ),
		"決定論的な配列シャッフルの公開APIが解決できる" );
	Harness.Check( WeightedRandom.TryChooseIndex( 3u, PublicRandomIndex )
		&& PublicRandomIndex < 3u,
		"決定論的な均等index抽選の公開APIが解決できる" );
	Harness.Check( WeightedRandom.TryPointInBox3D(
			FVec3{ 1.0f, 2.0f, 3.0f }, PublicBoxPoint )
		&& std::abs( PublicBoxPoint.x ) <= 1.0f
		&& std::abs( PublicBoxPoint.y ) <= 2.0f
		&& std::abs( PublicBoxPoint.z ) <= 3.0f,
		"決定論的な3D箱内部抽選の公開APIが解決できる" );
	Harness.Check( WeightedRandom.TryPointInTriangle3D(
			FVec3{}, FVec3::Right(), FVec3::Up(), PublicTrianglePoint )
		&& PublicTrianglePoint.x >= 0.0f
		&& PublicTrianglePoint.y >= 0.0f
		&& PublicTrianglePoint.x + PublicTrianglePoint.y <= 1.000001f
		&& PublicTrianglePoint.z == 0.0f,
		"決定論的な3D三角形抽選の公開APIが解決できる" );
	Harness.Check( WeightedRandom.TryPointInDisk3D(
			FVec3::Up(), 2.0f, PublicDiskPoint )
		&& PublicDiskPoint.y == 0.0f
		&& LengthSq( PublicDiskPoint ) <= 4.0f,
		"決定論的な3D円盤内部抽選の公開APIが解決できる" );
	Harness.Check( WeightedRandom.TryPointInCylinder3D(
			FVec3::Up(), 2.0f, 3.0f, PublicCylinderPoint )
		&& PublicCylinderPoint.x * PublicCylinderPoint.x
			+ PublicCylinderPoint.z * PublicCylinderPoint.z <= 4.0f
		&& std::abs( PublicCylinderPoint.y ) <= 3.0f,
		"決定論的な3D円柱内部抽選の公開APIが解決できる" );
	Harness.Check( WeightedRandom.TryPointInCapsule3D(
			FVec3::Up(), 2.0f, 3.0f, PublicCapsulePoint )
		&& std::abs( PublicCapsulePoint.y ) <= 5.0f,
		"決定論的な3Dカプセル内部抽選の公開APIが解決できる" );
	Harness.Check( WeightedRandom.TryPointOnSphere3D(
			2.0f, PublicSpherePoint )
		&& LengthSq( PublicSpherePoint ) > 3.99f,
		"決定論的な3D球面抽選の公開APIが解決できる" );
	Harness.Check( WeightedRandom.TryPointInSphere3D(
			2.0f, PublicSpherePoint )
		&& LengthSq( PublicSpherePoint ) <= 4.0f,
		"決定論的な3D球内部抽選の公開APIが解決できる" );
	Harness.Check( WeightedRandom.TryDirectionInCone3D(
			FVec3::Forward(), 15.0f, PublicConeDirection )
		&& Dot( FVec3::Forward(), PublicConeDirection ) > 0.9f,
		"決定論的な3D円錐方向抽選の公開APIが解決できる" );
}
