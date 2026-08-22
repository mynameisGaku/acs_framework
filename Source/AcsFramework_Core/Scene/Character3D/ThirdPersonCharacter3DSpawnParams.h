// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Animation3D/CharacterAnimation3DProfile.h"
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DParams.h"
#include "AcsFramework_Core/Scene/Collision3D/CollisionShape3DParams.h"

/** 第三者視点キャラクターの衝突登録、操作接続、任意アニメーション接続の指定。 */
struct FThirdPersonCharacter3DSpawnParams
{
	/** 移動、向き、追従カメラの設定。自己形状番号は生成結果で上書きする。 */
	FThirdPersonCharacter3DParams Control;

	/** キャラクターノード自身へ登録する衝突形状。 */
	FCollisionShape3DParams Collision;

	/** 骨格モデルの待機、歩き、走り、ジャンプを選ぶ規則。 */
	FCharacterAnimation3DProfile Animation;

	/** 骨格モデルなら移動連動アニメーションも接続するか。 */
	bool bBindAnimation = true;
};
