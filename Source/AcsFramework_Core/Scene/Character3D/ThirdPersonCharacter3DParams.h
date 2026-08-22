// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Camera3D/NodeOrbitCamera3DParams.h"
#include "AcsFramework_Core/Scene/Character3D/CharacterMover3D.h"

using namespace acs;
using namespace acs::game;

/**
 * 第三者視点キャラクターの移動、向き、衝突、追従カメラ設定。
 */
struct FThirdPersonCharacter3DParams
{
	/** ノードから見たキャラクター球の中心。既定は足元原点の半径0.5m。 */
	FVec3 LocalCollisionCenter{ 0.0f, 0.5f, 0.0f };

	/** 球半径、重力、ジャンプ初速、接触調整値。 */
	FKinematicCharacterMovementParams3D Movement;

	/** 入力の長さが1のときに出す世界単位毎秒の水平速度。 */
	f32 MaximumMoveSpeed = 4.0f;

	/** 1秒間に移動方向へ回せる最大角度。0なら現在の向きを保つ。 */
	f32 MaximumTurnDegreesPerSecond = 540.0f;

	/** 衝突集合へ自身も登録した場合に問い合わせから除外する形状。 */
	FCollisionShapeId3D SelfShape;

	/** 移動を妨げる形状レイヤーのビット列。 */
	u32 CollisionMask = CSceneCollision3D::kAllLayers;

	/** 追従点、初期角度、距離、操作速度、遮蔽物回避設定。 */
	FNodeOrbitCamera3DParams Camera;

	/** 走行要求中に基本速度へ掛ける倍率。1以上を指定する。 */
	f32 RunSpeedMultiplier = 1.75f;
};
