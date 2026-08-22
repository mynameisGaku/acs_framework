// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Animation3D/CharacterAnimator3D.h"
#include "AcsFramework_Core/Scene/Camera3D/NodeOrbitCamera3D.h"
#include "AcsFramework_Core/Scene/Character3D/CharacterMover3D.h"
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DActionSet.h"
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DInput.h"
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DParams.h"
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DUpdateResult.h"

using namespace acs;
using namespace acs::game;

/**
 * 移動、向き、追従カメラ、任意の骨格アニメーションを1回の入力へまとめる。
 *
 * @details
 * 場面、衝突集合、ノード、骨付き部品は所有しない。既存の小さな機能を順番に呼ぶ薄い統合層で、
 * 入力装置、時刻、固定更新、描画を内部取得しない。各機能は公開アダプターから個別にも操作できる。
 */
class CThirdPersonCharacter3D
{
public:
	/** 接続中なら、場面のカメラ設定を復元して全接続を解除する。 */
	~CThirdPersonCharacter3D() noexcept;

	CThirdPersonCharacter3D() noexcept = default;
	CThirdPersonCharacter3D( const CThirdPersonCharacter3D& ) = delete;
	CThirdPersonCharacter3D& operator=( const CThirdPersonCharacter3D& ) = delete;
	CThirdPersonCharacter3D( CThirdPersonCharacter3D&& ) = delete;
	CThirdPersonCharacter3D& operator=( CThirdPersonCharacter3D&& ) = delete;

	/**
	 * 衝突集合、3D場面、操作対象ノードへ移動と追従カメラを接続する。
	 *
	 * @details 再接続は明示的な`Unbind()`後だけ受け付ける。失敗時は接続を残さず、場面設定を保つ。
	 * @param Collision Sceneのノードグラフへ接続した衝突集合。
	 * @param Scene カメラを表示する3D場面。
	 * @param Character 移動と追従の対象にするScene所有ノード。
	 * @param Params 移動、向き、衝突、追従カメラ設定。
	 * @return 全設定を検証し、移動とカメラの両方へ接続できたらtrue。
	 */
	bool Bind( CSceneCollision3D& Collision, ALegacyScene3DAdapter& Scene, ANode& Character, const FThirdPersonCharacter3DParams& Params = FThirdPersonCharacter3DParams{} ) noexcept;

	/** 場面のカメラ設定を復元し、アニメーションを含む全接続を解除する。 */
	void Unbind() noexcept;

	/**
	 * 操作対象ノード自身の骨付き部品へ任意のアニメーションを接続する。
	 *
	 * @param Profile 待機・歩き・走り・ジャンプの規則。
	 * @return 本体接続中で、対象ノードの部品へ待機再生を開始できたらtrue。
	 */
	bool TryBindAnimation( const FCharacterAnimation3DProfile& Profile = FCharacterAnimation3DProfile{} ) noexcept;

	/**
	 * 子ノードなどにある任意の骨付き部品へアニメーションを接続する。
	 *
	 * @param Component 接続する骨付きモデル部品。
	 * @param Profile 待機・歩き・走り・ジャンプの規則。
	 * @return 本体接続中で、部品へ待機再生を開始できたらtrue。
	 */
	bool TryBindAnimation( ASkinnedMeshComponent3D& Component, const FCharacterAnimation3DProfile& Profile = FCharacterAnimation3DProfile{} ) noexcept;

	/** 骨格の再生状態を変えず、任意のアニメーション接続だけを解除する。 */
	void UnbindAnimation() noexcept { m_Animator.Unbind(); }

	/**
	 * 明示入力と経過秒から、視点、移動、向き、追従点、任意アニメーションを順に更新する。
	 *
	 * @details 入力と時刻は処理前に検証する。途中失敗時は後続処理を可能な範囲で省略し、
	 * 戻り値の段階別フラグで実際に反映済みの処理を知らせる。
	 * @param Input 移動、視点、距離、ジャンプ、走行の操作量。
	 * @param DeltaSeconds 進める有限かつ0以上の秒数。
	 * @return 各処理を完了できたか表す結果。
	 */
	FThirdPersonCharacter3DUpdateResult Update( const FThirdPersonCharacter3DInput& Input, f32 DeltaSeconds ) noexcept;

	/**
	 * 汎用アクション入力を変換し、視点、移動、向き、追従点、任意アニメーションを更新する。
	 *
	 * @details 現在と前回の入力を明示してジャンプの押した瞬間を判定する。入力変換に失敗した場合は
	 * どの処理も実行せず、全段階が未完了の結果を返す。
	 * @param CurrentInput 今回の汎用アクション入力。
	 * @param PreviousInput 前回の汎用アクション入力。
	 * @param DeltaSeconds 進める有限かつ0以上の秒数。
	 * @param Actions 軸とボタンを第三者視点操作へ割り当てる値。
	 * @return 各処理を完了できたか表す結果。
	 */
	FThirdPersonCharacter3DUpdateResult Update( const FActionInput& CurrentInput, const FActionInput& PreviousInput, f32 DeltaSeconds, const FThirdPersonCharacter3DActionSet& Actions = FThirdPersonCharacter3DActionSet{} ) noexcept;

	/** 移動と追従カメラの両方へ接続中ならtrue。 */
	bool IsBound() const noexcept { return m_Scene != nullptr && m_Mover.IsBound() && m_Camera.IsBound(); }

	/** 操作対象ノードを返す。未接続ならnullptr。 */
	ANode* Character() const noexcept { return m_Camera.Target(); }

	/** 詳細な移動状態を操作する公開アダプターを返す。 */
	CCharacterMover3D& Mover() noexcept { return m_Mover; }

	/** 詳細な移動状態を読む公開アダプターを返す。 */
	const CCharacterMover3D& Mover() const noexcept { return m_Mover; }

	/** 揺れを含む追従カメラを操作する公開アダプターを返す。 */
	CNodeOrbitCamera3D& OrbitCamera() noexcept { return m_Camera; }

	/** 追従カメラ状態を読む公開アダプターを返す。 */
	const CNodeOrbitCamera3D& OrbitCamera() const noexcept { return m_Camera; }

	/** 任意の骨格アニメーションを操作する公開アダプターを返す。 */
	CCharacterAnimator3D& Animator() noexcept { return m_Animator; }

	/** 任意の骨格アニメーション状態を読む公開アダプターを返す。 */
	const CCharacterAnimator3D& Animator() const noexcept { return m_Animator; }

	/** 接続時に確定した設定を返す。 */
	const FThirdPersonCharacter3DParams& Params() const noexcept { return m_Params; }

private:
	/** 統合前に検証できる速度とローカル中心が有効ならtrue。 */
	static bool IsValidParams_Internal( const FThirdPersonCharacter3DParams& Params ) noexcept;

	/** 3次元値の全成分が有限ならtrue。 */
	static bool IsFinite_Internal( FVec3 Value ) noexcept;

	/** 衝突とノード移動を接続する既存アダプター。 */
	CCharacterMover3D m_Mover;

	/** ノード追従と視点操作を接続する既存アダプター。 */
	CNodeOrbitCamera3D m_Camera;

	/** 移動状態から任意の骨格再生を選ぶ既存アダプター。 */
	CCharacterAnimator3D m_Animator;

	/** 呼出側が所有する表示先3D場面。 */
	ALegacyScene3DAdapter* m_Scene = nullptr;

	/** 接続時に確定した移動、向き、衝突、カメラ設定。 */
	FThirdPersonCharacter3DParams m_Params;
};
