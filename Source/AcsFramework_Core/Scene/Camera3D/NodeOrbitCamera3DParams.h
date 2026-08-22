// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * ノードを追う3D軌道カメラの初期状態、操作速度、安全範囲。
 *
 * @details 既定値は人物の足元ノードから少し上を、斜め上の6m後方から見る設定になる。
 */
struct FNodeOrbitCamera3DParams
{
	/** 追従ノードから見た注視点。 */
	FVec3 LocalTargetOffset{ 0.0f, 1.4f, 0.0f };

	/** 初期の水平角度。正で右へ回る度数。 */
	f32 InitialYawDegrees = 0.0f;

	/** 初期の上下角度。正で見下ろす度数。 */
	f32 InitialPitchDegrees = 20.0f;

	/** 注視点からカメラまでの初期距離。 */
	f32 InitialDistance = 6.0f;

	/** 水平操作量1.0で1秒間に回る度数。 */
	f32 YawDegreesPerSecond = 180.0f;

	/** 上下操作量1.0で1秒間に回る度数。 */
	f32 PitchDegreesPerSecond = 120.0f;

	/** 距離変更量1.0で1秒間に変える現在距離の倍率。 */
	f32 ZoomDistanceScalePerSecond = 1.0f;

	/** 上下反転を防ぐ上下角度の絶対値上限。 */
	f32 PitchLimitDegrees = 80.0f;

	/** 注視点へ近づける最小距離。 */
	f32 MinimumDistance = 1.5f;

	/** 注視点から離れられる最大距離。 */
	f32 MaximumDistance = 12.0f;

	/** 場面の描画形状が間に入ったときカメラを手前へ寄せるならtrue。 */
	bool bAvoidObstructions = true;

	/** 注視点からこの距離未満の形状を追従対象として除外する。 */
	f32 TargetClearance = 0.75f;

	/** カメラを遮蔽物の手前へ離す距離。 */
	f32 CameraClearance = 0.25f;

	/** 点ではなく球で遮蔽物を調べる世界半径。 */
	f32 ProbeRadius = 0.2f;

	/** 遮蔽物が消えた後に元の距離へ戻る1秒あたりの速さ。 */
	f32 RecoverySharpness = 12.0f;
};
