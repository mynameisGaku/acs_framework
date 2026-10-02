// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * ノードへworld移動量を加えた後のローカル位置を計算する。
 *
 * @details 親の移動、回転、拡縮を逆変換し、ノード自体は変更しない。
 * @param Node 現在のworld位置と親Transformを読む対象。
 * @param WorldTranslation 加えたいworld座標系の移動量。
 * @param OutLocalPosition 成功時だけ書き換える親座標系の位置。
 * @return 対象が有効で、有限なローカル位置を計算できたらtrue。
 */
bool TryCalculateLocalPositionAfterWorldTranslation3D( const ANode& Node, FVec3 WorldTranslation, FVec3& OutLocalPosition ) noexcept;

/**
 * 親Transformに左右されず、ノードをworld座標系の量だけ移動する。
 *
 * @param Node 成功時だけローカル位置を書き換える対象。
 * @param WorldTranslation 加えたいworld座標系の移動量。
 * @return 対象が有効で移動を適用できたらtrue。失敗時はノードを変更しない。
 */
bool TryTranslateNodeWorld3D( ANode& Node, FVec3 WorldTranslation ) noexcept;
