// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"

using namespace acs;
using namespace acs::game;

/**
 * 球型3Dキャラクターの移動計算をシーンノードへ反映する。
 *
 * @details
 * 場面またはキャラクターが所有し、衝突集合とノードは所有しない。入力、保持状態、経過秒、
 * 調整値からACSの決定的な次状態を求め、成功した場合だけノード位置と保持状態を更新する。
 * 固定更新、入力寿命、描画、アニメーションは所有しない。
 */
class CCharacterMover3D
{
public:
	/**
	 * 衝突集合と移動対象ノードへ接続し、停止した空中状態から開始する。
	 *
	 * @details 接続だけを行い、ノード位置や衝突形状は変更しない。失敗時は既存接続を保つ。
	 * @param Collision ノードグラフへ接続済みの衝突集合。この型より長く生存すること。
	 * @param Node 移動するノード。この型より長く生存すること。
	 * @param LocalCenter ノードから見たキャラクター球の中心。足元原点ならYへ半径を指定する。
	 * @param Params 球半径、重力、ジャンプ初速、接触調整値。
	 * @return 有限な球中心と有効な調整値を確定できたらtrue。
	 */
	bool Bind( CSceneCollision3D& Collision, ANode& Node, FVec3 LocalCenter = {}, const FKinematicCharacterMovementParams3D& Params = FKinematicCharacterMovementParams3D{} ) noexcept;

	/** ノードを変更せず、この接続と保持状態だけを解除する。 */
	void Unbind() noexcept;

	/**
	 * 希望する世界X/Z速度で1回進め、成功した場合だけノードへ移動を反映する。
	 *
	 * @param DesiredWorldXZVelocity xを世界X、yを世界Zとする希望水平速度。
	 * @param bJumpRequested 接地中なら上向き初速を与える要求。
	 * @param DeltaSeconds 進める有限かつ0以上の秒数。
	 * @return 衝突同期、次状態計算、親座標への位置変換を全て完了できたらtrue。
	 */
	bool Move( FVec2 DesiredWorldXZVelocity, bool bJumpRequested, f32 DeltaSeconds ) noexcept;

	/**
	 * 画面上の左右・前後入力を現在カメラ基準の世界速度へ変換して1回進める。
	 *
	 * @details カメラの上下角は移動へ含めず、入力の長さは1以下へ制限する。
	 * @param Camera 移動方向の基準にする現在カメラ。
	 * @param MoveAxes xを画面右、yを画面奥とする操作量。
	 * @param MaximumSpeed 入力の長さが1のときの有限かつ0以上の世界速度。
	 * @param bJumpRequested 接地中なら上向き初速を与える要求。
	 * @param DeltaSeconds 進める有限かつ0以上の秒数。
	 * @return 方向変換と移動を完了できたらtrue。失敗時はノードと保持状態を変更しない。
	 */
	bool MoveFromCamera( const CCamera& Camera, FVec2 MoveAxes, f32 MaximumSpeed, bool bJumpRequested, f32 DeltaSeconds ) noexcept;

	/**
	 * 直前に確定した水平速度へ、ノードの前方向を世界Y軸回りで滑らかに向ける。
	 *
	 * @details 停止中は向きを保つ。親ノードがある場合は確定した世界回転を親座標へ戻す。
	 * @param MaximumDegreesPerSecond 1秒間に回せる有限かつ0以上の最大角度。
	 * @param DeltaSeconds 進める有限かつ0以上の秒数。
	 * @return 接続と回転が有効で向きの更新または維持を完了できたらtrue。
	 */
	bool TurnTowardMovement( f32 MaximumDegreesPerSecond, f32 DeltaSeconds ) noexcept;

	/**
	 * 現在のノード位置を球中心へ読み直し、速度と接地状態を初期化する。
	 *
	 * @return 接続中のノードから有限な球中心を作れたらtrue。失敗時は状態を変更しない。
	 */
	bool ResetMotion() noexcept;

	/**
	 * 次回以降に使う移動調整値を置き換える。
	 *
	 * @param Params 球半径、重力、ジャンプ初速、接触調整値。
	 * @return ACSの移動契約を満たす値なら置き換えてtrue。失敗時は既存値を保つ。
	 */
	bool SetMovementParams( const FKinematicCharacterMovementParams3D& Params ) noexcept;

	/**
	 * 次回以降の自己除外形状と対象レイヤーを設定する。
	 *
	 * @param SelfShape 自身として全問い合わせから除外する形状。無効値なら除外なし。
	 * @param CollisionMask 登録形状のレイヤーとのANDが0でない形状だけを対象にする値。
	 */
	void SetCollisionFilter( FCollisionShapeId3D SelfShape = {}, u32 CollisionMask = CSceneCollision3D::kAllLayers ) noexcept;

	/** 接続中ならtrueを返す。 */
	bool IsBound() const noexcept { return m_Collision != nullptr && m_Node != nullptr; }

	/** 直前に確定した状態が接地中ならtrueを返す。 */
	bool IsGrounded() const noexcept { return m_State.Grounded; }

	/** 重力と接触面投影を反映した直前の世界速度を返す。 */
	FVec3 Velocity() const noexcept { return m_State.Velocity; }

	/** 次回計算へ渡す現在状態を返す。 */
	const FKinematicCharacterState3D& State() const noexcept { return m_State; }

	/** 直前に成功した移動結果を返す。接続直後と初期化直後は空の結果。 */
	const FKinematicCharacterMovementResult3D& LastResult() const noexcept { return m_LastResult; }

	/** 現在の移動調整値を返す。 */
	const FKinematicCharacterMovementParams3D& MovementParams() const noexcept { return m_Params; }

private:
	/** ノードの現在変形からキャラクター球の世界中心を作る。 */
	static bool TryWorldCenter_Internal( const ANode& Node, FVec3 LocalCenter, FVec3& OutWorldCenter ) noexcept;

	/** 世界移動量を親座標へ変換し、適用後のローカル位置を作る。 */
	static bool TryLocalPositionAfterWorldTranslation_Internal( const ANode& Node, FVec3 WorldTranslation, FVec3& OutLocalPosition ) noexcept;

	/** ACSの移動処理へ安全に渡せる調整値ならtrueを返す。 */
	static bool IsValidParams_Internal( const FKinematicCharacterMovementParams3D& Params ) noexcept;

	/** 画面上の操作量を水平なカメラ基準の世界X/Z速度へ変換する。 */
	static bool TryCameraRelativeVelocity_Internal( const CCamera& Camera, FVec2 MoveAxes, f32 MaximumSpeed, FVec2& OutVelocity ) noexcept;

	/** 全成分が有限ならtrueを返す。 */
	static bool IsFinite_Internal( FVec3 Value ) noexcept;

	/** 回転の全成分が有限で長さを持つならtrueを返す。 */
	static bool IsUsableRotation_Internal( FQuat Value ) noexcept;

	/** 呼出側が所有するシーン衝突集合。 */
	CSceneCollision3D* m_Collision = nullptr;

	/** 呼出側のノードグラフが所有する移動対象。 */
	ANode* m_Node = nullptr;

	/** ノード座標から見たキャラクター球の中心。 */
	FVec3 m_LocalCenter;

	/** 球形状と移動感覚を決める調整値。 */
	FKinematicCharacterMovementParams3D m_Params;

	/** 次回計算へ渡す速度と接地状態。位置は毎回ノードから読み直す。 */
	FKinematicCharacterState3D m_State;

	/** 直前に成功した移動の次状態と接触事象。 */
	FKinematicCharacterMovementResult3D m_LastResult;

	/** 衝突集合へ自身も登録した場合に除外する形状。 */
	FCollisionShapeId3D m_SelfShape;

	/** 移動を妨げる形状レイヤーのビット列。 */
	u32 m_CollisionMask = CSceneCollision3D::kAllLayers;
};
