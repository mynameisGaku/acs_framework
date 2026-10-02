// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 画面上の左右・前後入力を、カメラ基準の水平な世界X/Z速度へ変換する。
 *
 * @details カメラの上下角を除き、長さ1を超える入力は方向を保って制限する。
 * @param Camera 移動方向の基準にする現在カメラ。
 * @param MoveAxes xを画面右、yを画面奥とする操作量。
 * @param MaximumSpeed 入力の長さが1のときの有限かつ0以上の世界速度。
 * @param OutWorldXZVelocity 成功時だけ書き換える世界X/Z速度。
 * @return 入力とカメラから水平な速度を計算できたらtrue。
 */
bool TryCalculateCameraRelativeVelocity3D( const CCamera& Camera, FVec2 MoveAxes, f32 MaximumSpeed, FVec2& OutWorldXZVelocity ) noexcept;
