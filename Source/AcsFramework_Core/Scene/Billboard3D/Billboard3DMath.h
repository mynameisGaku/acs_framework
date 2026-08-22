// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Billboard3D/Billboard3DMode.h"

using namespace acs;

/**
 * 画像板の正面がカメラ位置へ向くローカル回転を計算する。
 *
 * @param WorldPosition 画像板中心のworld位置。
 * @param CameraPosition 現在カメラのworld位置。
 * @param ParentWorldRotation 親ノードのworld回転。親が無ければ単位回転。
 * @param Mode 上下も追うか、worldのY軸を保つか。
 * @param RollDegrees 正面軸まわりへ加える度数。
 * @param OutLocalRotation 成功時だけ書き換える親座標内の回転。
 * @return 入力が有限で向きを一意に決められたらtrue。
 */
bool TryCalculateBillboard3DRotation( FVec3 WorldPosition, FVec3 CameraPosition,
	FQuat ParentWorldRotation, EBillboard3DMode Mode, f32 RollDegrees,
	FQuat& OutLocalRotation ) noexcept;
