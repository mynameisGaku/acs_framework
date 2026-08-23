// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Trigger3D/ProximityTrigger3DParams.h"
#include "AcsFramework_Core/Scene/Trigger3D/ProximityTrigger3DUpdateResult.h"

class CSceneCollision3D;
class CDebugDraw3DQueue;

/**
 * 1ノードへ追従する球または箱の範囲から、衝突ノードの進入、滞在、退出を求める。
 *
 * @details 場面またはゲーム機能が所有し、ノードグラフと衝突集合は所有しない。
 * 前回の世代付きノード識別子だけを保持し、コールバック、入力、描画、時間進行は所有しない。
 */
class CProximityTrigger3D
{
public:
	/**
	 * 場面衝突と範囲基準ノードへ接続し、空の前回状態から開始する。
	 *
	 * @details 接続だけを行い、ノードや衝突登録は変更しない。失敗時は既存接続を保つ。
	 * @param Graph 基準ノードを所有する場面グラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Origin 近接範囲の位置、回転、拡縮を決める自場面ノード。
	 * @param Params ローカル形状と検出する衝突レイヤー。
	 * @return 未接続で、所属と設定を全て確認できたらtrue。
	 */
	bool Bind( CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		ANode& Origin,
		const FProximityTrigger3DParams& Params = FProximityTrigger3DParams{} ) noexcept;

	/** ノードと衝突集合を変更せず、接続と前回の範囲内状態を解除する。 */
	void Unbind() noexcept;

	/**
	 * 現在のworld近接範囲と衝突集合から、前回との差を1回求める。
	 *
	 * @details 最初の成功更新では、範囲内の全対象を進入として返す。基準ノード自身は含めない。
	 * @param OutResult 進入、滞在、退出の受け取り先。失敗時は変更しない。
	 * @return 接続、範囲変換、衝突同期、結果確保を全て完了できたらtrue。
	 */
	bool Update( FProximityTrigger3DUpdateResult& OutResult ) noexcept;

	/** 次回更新から使う近接範囲と対象レイヤーを置き換える。失敗時は以前の設定を保つ。 */
	bool SetParams( const FProximityTrigger3DParams& Params ) noexcept;

	/** 前回の範囲内状態だけを空にし、次の成功更新で全対象を進入として返す。 */
	void ResetState() noexcept;

	/** 場面グラフ、衝突集合、基準ノードへ接続済みならtrue。 */
	bool IsBound() const noexcept;

	/**
	 * 指定した場面グラフと衝突集合へ接続中か確認する。
	 *
	 * @param Graph 確認する場面グラフ。
	 * @param Collision `Graph`へ接続した確認対象の衝突集合。
	 * @return 両方が接続先と一致し、基準ノードが現在も生存するならtrue。
	 */
	bool IsBoundTo( const CSceneNodeGraph& Graph,
		const CSceneCollision3D& Collision ) const noexcept;

	/** 現在も生存する基準ノードを返す。未接続、場面差し替え、破棄済みならnullptr。 */
	ANode* Origin() const noexcept;

	/** 指定ノードが直前の成功更新で範囲内ならtrue。 */
	bool IsInside( FNodeId Node ) const noexcept;

	/** 直前の成功更新で範囲内だったノード数。 */
	u32 InsideCount() const noexcept { return static_cast<u32>( m_InsideNodes.Num() ); }

	/** 現在の近接範囲と対象レイヤーを返す。 */
	const FProximityTrigger3DParams& Params() const noexcept { return m_Params; }

	/**
	 * 選択中の箱範囲を現在Transformでworld軸平行箱へ変換する。
	 *
	 * @param OutBox 変換結果。未接続、別形状、無効化、不正Transformでは変更しない。
	 * @return 有効な箱トリガーの現在範囲を取得できたらtrue。
	 */
	bool TryGetWorldBox( FAabb3& OutBox ) const noexcept;

	/**
	 * 選択中の球範囲を現在Transformでworld球へ変換する。
	 *
	 * @param OutSphere 変換結果。未接続、別形状、無効化、不正Transformでは変更しない。
	 * @return 有効な球トリガーの現在範囲を取得できたらtrue。
	 */
	bool TryGetWorldSphere( FSphere& OutSphere ) const noexcept;

private:
	/** 現在の衝突結果を重複のない世代付きノード識別子へ変換する。 */
	bool TryCollectInsideNodes_Internal( const ANode& Origin,
		TArray<FNodeId>& OutNodes ) noexcept;

	/** 前回と今回の識別子から、出力候補を確保失敗時に元を変えず作る。 */
	bool TryBuildUpdateResult_Internal( const TArray<FNodeId>& CurrentNodes,
		FProximityTrigger3DUpdateResult& OutResult ) const noexcept;

	/** 基準ノード自身と全祖先が有効ならtrue。 */
	static bool IsNodeActive_Internal( const ANode& Node ) noexcept;

	/** 配列に同じ世代付きノード識別子があればtrue。 */
	static bool ContainsNode_Internal( const TArray<FNodeId>& Nodes,
		FNodeId Node ) noexcept;

	/** グラフと衝突集合の接続、root差し替えを確認する。 */
	bool RefreshGraphIdentity_Internal() noexcept;

	/** 呼出側が所有する場面ノードグラフ。 */
	CSceneNodeGraph* m_Graph = nullptr;

	/** 呼出側が所有する場面衝突集合。 */
	CSceneCollision3D* m_Collision = nullptr;

	/** 接続時のrootポインタ。場面内容の全差し替え検出だけに使う。 */
	ANode* m_RootIdentity = nullptr;

	/** 近接範囲の位置、回転、拡縮を決める世代付きノード識別子。 */
	FNodeId m_Origin;

	/** 次回更新へ使う近接範囲と対象レイヤー。 */
	FProximityTrigger3DParams m_Params;

	/** 直前の成功更新で範囲内だった世代付きノード識別子。 */
	TArray<FNodeId> m_InsideNodes;
};

/**
 * 接続済み近接トリガーの現在world範囲を、GPU非依存の1フレーム線キューへ追加する。
 *
 * @param Trigger 表示する球または箱の近接トリガー。
 * @param Queue 検証済み線の追加先。
 * @param Color 全ての線へ使う色。
 * @param SphereSegments 球を構成する各円の分割数。箱では使わない。
 * @return world形状を取得し、必要な線を全て追加できたらtrue。
 */
bool TryQueueProximityTrigger3D( const CProximityTrigger3D& Trigger,
	CDebugDraw3DQueue& Queue, FVec4 Color, u32 SphereSegments ) noexcept;
