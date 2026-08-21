// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Animation3D/CharacterAnimation3DProfile.h"

using namespace acs;
using namespace acs::game;

/**
 * 3Dキャラクターの移動入力を、骨格アニメーションの滑らかな切替へ接続する。
 *
 * @details
 * 部品は所有せず、利用側の場面またはキャラクターがこの型を所有する。
 * 状態選択は`FCharacterAnimation3DProfile`へ任せ、この型はACSの再生APIへ結果を渡すだけにする。
 */
class CCharacterAnimator3D
{
public:
	/**
	 * 骨付きモデル部品と4状態の規則を接続し、待機クリップを先頭から再生する。
	 *
	 * @details 失敗時は部品の再生と、この型が保持している既存の接続を変更しない。
	 * @param Component 接続する骨付きモデル部品。呼出側がこの型より長く所有する。
	 * @param Profile 待機・歩き・走り・ジャンプのクリップ名と速度境界。
	 * @return 描画可能なモデルに全クリップがあり、待機再生を開始できたらtrue。
	 */
	bool Bind( ASkinnedMeshComponent3D& Component,
		const FCharacterAnimation3DProfile& Profile = FCharacterAnimation3DProfile{} ) noexcept;

	/**
	 * ノードから骨付きモデル部品を探して接続する。
	 *
	 * @param Node 接続先の部品を持つノード。
	 * @param Profile 待機・歩き・走り・ジャンプのクリップ名と速度境界。
	 * @return 部品を見つけて接続できたらtrue。
	 */
	bool Bind( ANode& Node,
		const FCharacterAnimation3DProfile& Profile = FCharacterAnimation3DProfile{} ) noexcept;

	/** 部品の再生を変えず、この接続だけを解除する。 */
	void Unbind() noexcept;

	/**
	 * 1フレームの移動入力から状態を選び、必要な場合だけ姿勢遷移を要求する。
	 *
	 * @details 既存の姿勢遷移が進行中ならACSが要求を拒否するため、状態を変えずfalseを返す。
	 * 次のフレームでも同じ入力を渡せば、既存遷移の完了後に自動で再試行できる。
	 * @param Input 地面と平行な速度と接地状態。
	 * @return 入力を処理できたらtrue。未接続、不正値、遷移中の再要求ではfalse。
	 */
	bool Update( const FCharacterAnimation3DInput& Input ) noexcept;

	/** 接続中ならtrueを返す。 */
	bool IsBound() const noexcept { return m_Component != nullptr; }

	/** 現在選択済みの待機・歩き・走り・ジャンプ状態を返す。 */
	EAnimationGraphState CurrentState() const noexcept { return m_CurrentState; }

	/** 接続中の状態選択規則を返す。 */
	const FCharacterAnimation3DProfile& Profile() const noexcept { return m_Profile; }

private:
	/** モデルに指定名のクリップがあるか返す。 */
	static bool HasAnimation_Internal( const ASkinnedMeshAsset& Mesh, FStringView Name ) noexcept;

	/** 部品と規則を、再生状態を変える前にまとめて検証する。 */
	static bool CanBind_Internal( const ASkinnedMeshComponent3D& Component,
		const FCharacterAnimation3DProfile& Profile ) noexcept;

	/** 呼出側が所有する骨付きモデル部品。 */
	ASkinnedMeshComponent3D* m_Component = nullptr;

	/** 接続時に複製した状態選択規則。 */
	FCharacterAnimation3DProfile m_Profile;

	/** ACSへ受理された最後の状態。 */
	EAnimationGraphState m_CurrentState = EAnimationGraphState::Idle;
};
