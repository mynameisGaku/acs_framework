// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3DParams.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3DState.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3DUpdateResult.h"
#include "AcsFramework_Core/UI/WorldLabel3D/WorldLabel3DLayer.h"

using namespace acs;
using namespace acs::game;

/**
 * 画面上の1点から最前面の実形状を調べ、登録した3D対象の出入りと決定を返す場面アダプター。
 *
 * @details レイ判定は`CScenePicker`、表示は`CWorldLabel3DLayer`へ委ねる。対象文字列だけを所有し、
 * 場面、ノード、ラベルレイヤー、GPU資源は所有しない。命中した子から登録済み祖先を探すため、
 * 見た目を子ノードへ分けた扉や人物は親1件を登録すればよい。未登録の最前面形状は遮蔽物になる。
 */
class CInteractionFocus3D
{
public:
	/** 接続中の非所有参照と一時ラベルを安全に外す。 */
	~CInteractionFocus3D() noexcept;

	CInteractionFocus3D() noexcept = default;
	CInteractionFocus3D( const CInteractionFocus3D& ) = delete;
	CInteractionFocus3D& operator=( const CInteractionFocus3D& ) = delete;
	CInteractionFocus3D( CInteractionFocus3D&& ) = delete;
	CInteractionFocus3D& operator=( CInteractionFocus3D&& ) = delete;

	/**
	 * 同じ場面へ接続済みのノードグラフとワールドラベルレイヤーを借りる。
	 *
	 * @param Graph 実形状判定と世代付きノード解決を行う場面グラフ。
	 * @param Labels フォーカス中だけ操作案内を表示する同一グラフのレイヤー。
	 * @param Params 画面上の視線位置と届く距離。
	 * @return 未接続で、両者のグラフと設定が有効ならtrue。失敗時は状態を変えない。
	 */
	bool Bind( CSceneNodeGraph& Graph, CWorldLabel3DLayer& Labels, const FInteractionFocus3DParams& Params = FInteractionFocus3DParams{} ) noexcept;

	/** 操作案内と対象を消し、場面への全接続を外す。 */
	void Unbind() noexcept;

	/** ノードグラフとラベルレイヤーへ接続済みならtrue。 */
	bool IsBound() const noexcept { return m_Graph != nullptr && m_Labels != nullptr; }

	/**
	 * 次回更新から使う画面位置と距離を差し替える。
	 *
	 * @param Params 検証する新設定。
	 * @return 有効な値を反映できたらtrue。失敗時は以前の設定を保つ。
	 */
	bool SetParams( const FInteractionFocus3DParams& Params ) noexcept;

	/** 現在の画面位置と距離設定を返す。 */
	const FInteractionFocus3DParams& Params() const noexcept { return m_Params; }

	/**
	 * 既定見た目の操作案内を持つ対象として、場面ノードを登録する。
	 *
	 * @param Node 命中した自身または子をこの対象へまとめる場面所有ノード。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @return 所属、文字、確保が全て有効で新規登録できたらtrue。
	 */
	bool RegisterTarget( ANode& Node, FStringView Prompt, FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f } ) noexcept;

	/**
	 * 見た目を指定した操作案内を持つ対象として、場面ノードを登録する。
	 *
	 * @param Node 命中した自身または子をこの対象へまとめる場面所有ノード。
	 * @param PromptLabel 内部へ複製する文字と、操作案内の見た目。
	 * @return 所属、設定、確保が全て有効で新規登録できたらtrue。
	 */
	bool RegisterTarget( ANode& Node, const FWorldLabel3DParams& PromptLabel ) noexcept;

	/**
	 * 1対象を登録から外し、その対象の操作案内を直ちに消す。
	 *
	 * @param Node 外す場面ノード。
	 * @return 登録中の同じ世代付きノードを外せたらtrue。
	 */
	bool UnregisterTarget( ANode& Node ) noexcept;

	/** 全対象、現在状態、操作案内を消し、場面接続は維持する。 */
	void ClearTargets() noexcept;

	/**
	 * 現カメラから最前面の実形状を調べ、対象出入りと決定を1回進める。
	 *
	 * @param Camera 今回の表示に使う3Dカメラ。
	 * @param bActivateRequested 今回捉えた対象へ決定操作を要求するならtrue。
	 * @return 更新前後の対象と、成立した決定対象。未接続なら全て無効。
	 */
	FInteractionFocus3DUpdateResult Update( const CCamera& Camera, bool bActivateRequested = false ) noexcept;

	/** 現在捉えている登録対象を安全に解決する。無い、破棄済み、登録解除済みならnullptr。 */
	ANode* FocusedNode() const noexcept;

	/** 現在の決定論的なフォーカス状態を返す。 */
	const FInteractionFocus3DState& State() const noexcept { return m_State; }

	/** 登録中の対象数を返す。 */
	u32 TargetCount() const noexcept { return static_cast<u32>( m_Targets.Num() ); }

private:
	/** 対象1件の世代付き識別子、所有文字列、ラベル見た目。 */
	struct FEntry
	{
		/** 登録した場面ノード。 */
		FNodeId Node;

		/** NUL終端で所有する操作案内文字列。 */
		FString Prompt;

		/** Textを除く操作案内の見た目。使用時に所有文字列viewを入れる。 */
		FWorldLabel3DParams LabelParams;
	};

	/** 1から4096byteのUTF-8文字列を、失敗時に出力を変えず複製する。 */
	static bool TryCopyPrompt_Internal( FStringView Prompt, FString& OutPrompt ) noexcept;

	/** 登録対象の世代付き識別子に一致する配列位置を返す。 */
	usize FindTargetIndex_Internal( FNodeId Node ) const noexcept;

	/** 命中ノードから祖先へ進み、最初に見つかる登録対象を返す。 */
	FNodeId FindRegisteredAncestor_Internal( ANode* HitNode ) const noexcept;

	/** 現在対象の操作案内が無ければ、所有文字列から1件作る。 */
	bool EnsurePrompt_Internal() noexcept;

	/** 現在作っている操作案内をラベルレイヤーから外す。 */
	void RemovePrompt_Internal() noexcept;

	/** グラフroot差し替え時に古い対象と案内を消し、現在rootを記録する。 */
	bool RefreshGraphIdentity_Internal() noexcept;

	/** 実形状判定と識別子解決に使う場面グラフ。所有しない。 */
	CSceneNodeGraph* m_Graph = nullptr;

	/** フォーカス中の操作案内を描くレイヤー。所有しない。 */
	CWorldLabel3DLayer* m_Labels = nullptr;

	/** 接続時または直近更新時のrootポインタ。scene内容差し替えの検出に使う。 */
	ANode* m_RootIdentity = nullptr;

	/** 登録順の対象と所有文字列。 */
	TArray<FEntry> m_Targets;

	/** 入力と候補から次回へ持ち越す値状態。 */
	FInteractionFocus3DState m_State;

	/** 現在対象だけに作った操作案内ラベル。 */
	FWorldLabel3DHandle m_PromptLabel;

	/** 画面上の視線位置と届く距離。 */
	FInteractionFocus3DParams m_Params;
};
