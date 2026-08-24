// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3DParams.h"
#include "AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3DUpdateResult.h"
#include "AcsFramework_Core/Scene/Trigger3D/ProximityTrigger3D.h"

class CSceneCollision3D;

/**
 * 指定した1つの3D衝突形状が球または箱へ進入したとき、チェックポイントを発火する。
 *
 * @details 呼出側が所有し、ノードグラフと衝突集合は所有しない。時間、入力、描画、
 * コールバックは持たず、呼出側が明示的に`Update`して結果を受け取る。
 */
class CCheckpoint3D
{
public:
	/** 接続状態の重複を防ぐためコピーを禁止する。 */
	CCheckpoint3D( const CCheckpoint3D& ) = delete;

	/** 接続状態の重複を防ぐためコピー代入を禁止する。 */
	CCheckpoint3D& operator=( const CCheckpoint3D& ) = delete;

	/** 未接続の空のチェックポイントを作る。 */
	CCheckpoint3D() noexcept = default;

	/**
	 * 範囲基準ノードと追跡対象形状へ接続し、未発火状態から開始する。
	 *
	 * @details 接続だけを行い、ノードや衝突登録は変更しない。失敗時は既存状態を変えない。
	 * @param Graph 基準ノードと対象ノードを所有する場面グラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Origin チェックポイント範囲の位置、回転、拡縮を決めるノード。
	 * @param TargetShape 進入だけを追跡する登録済み衝突形状。
	 * @param Params ローカル範囲、対象レイヤー、一度限りか再進入可能かの設定。
	 * @return 所属、形状、設定を全て確認して接続できたらtrue。
	 */
	bool Bind( CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		ANode& Origin, FCollisionShapeId3D TargetShape,
		const FCheckpoint3DParams& Params = FCheckpoint3DParams{} ) noexcept;

	/** ノードと衝突集合を変更せず、接続、範囲内状態、発火状態を解除する。 */
	void Unbind() noexcept;

	/**
	 * 対象形状の現在位置から、進入と発火状態を1回求める。
	 *
	 * @param OutResult 今回発火、現在範囲内、発火済み状態の受け取り先。失敗時は変更しない。
	 * @return 接続と対象形状が有効で、近接判定を完了できたらtrue。
	 */
	bool Update( FCheckpoint3DUpdateResult& OutResult ) noexcept;

	/** 次回更新から使う範囲と発火方法を置き換える。失敗時は以前の設定を保つ。 */
	bool SetParams( const FCheckpoint3DParams& Params ) noexcept;

	/** 発火済みと範囲内の記録を消し、現在範囲内でも次回更新を新しい進入として扱う。 */
	void ResetActivation() noexcept;

	/** グラフ、衝突集合、基準、対象の識別子を保持していればtrue。 */
	bool IsBound() const noexcept;

	/** 指定した場面グラフへの接続情報を保持していればtrue。 */
	bool IsBoundToGraph( const CSceneNodeGraph& Graph ) const noexcept
	{
		return IsBound() && m_Graph == &Graph;
	}

	/** 成功した接続ごとに変わる所有確認用の世代。未接続の初期値は0。 */
	u64 BindingRevision() const noexcept { return m_BindingRevision; }

	/** 現在保持する場面グラフと基準番号が指定した組と同じならtrue。 */
	bool HasBindingOrigin( const CSceneNodeGraph& Graph,
		FNodeId Origin ) const noexcept;

	/** 現在の接続先と基準番号が、指定した接続世代と同じならtrue。 */
	bool HasBindingIdentity( const CSceneNodeGraph& Graph,
		const CSceneCollision3D& Collision, FNodeId Origin,
		u64 BindingRevision ) const noexcept;

	/** 指定した場面へ接続し、基準ノードと対象形状が現在も対になっていればtrue。 */
	bool IsBoundTo( const CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision ) const noexcept;

	/** 現在も生存する範囲基準ノードを返す。未接続または破棄済みならnullptr。 */
	ANode* Origin() const noexcept;

	/** 現在も生存する追跡対象ノードを返す。未接続または破棄済みならnullptr。 */
	ANode* Target() const noexcept;

	/** 追跡対象として接続した衝突形状番号を返す。 */
	FCollisionShapeId3D TargetShape() const noexcept { return m_TargetShape; }

	/** 追跡対象として接続したノード番号を返す。未接続なら無効値。 */
	FNodeId TargetNodeId() const noexcept { return m_Target; }

	/** 接続または最後の初期化後に1回以上発火していればtrue。 */
	bool HasActivated() const noexcept { return m_bHasActivated; }

	/** 直前の成功更新で対象形状が範囲内ならtrue。 */
	bool IsTargetInside() const noexcept { return m_bTargetInside; }

	/** 現在の範囲と発火方法を返す。 */
	const FCheckpoint3DParams& Params() const noexcept { return m_Params; }

	/** 判定と同じ範囲をデバッグ描画などへ読み取り専用で公開する。 */
	const CProximityTrigger3D& Range() const noexcept { return m_Range; }

private:
	/** グラフ全差し替え、ノード破棄、対象形状の付け替えを検出する。 */
	bool RefreshBinding_Internal() noexcept;

	/** 呼出側が所有する場面ノードグラフ。 */
	CSceneNodeGraph* m_Graph = nullptr;

	/** 呼出側が所有する場面衝突集合。 */
	CSceneCollision3D* m_Collision = nullptr;

	/** 接続時のrootポインタ。場面内容の全差し替え検出だけに使う。 */
	ANode* m_RootIdentity = nullptr;

	/** チェックポイント範囲の位置、回転、拡縮を決めるノード番号。 */
	FNodeId m_Origin;

	/** 追跡する衝突形状を持つノード番号。 */
	FNodeId m_Target;

	/** 進入だけを追跡する登録済み衝突形状番号。 */
	FCollisionShapeId3D m_TargetShape;

	/** 範囲内ノードの前回状態を計算する既存近接トリガー。 */
	CProximityTrigger3D m_Range;

	/** 現在の範囲と発火方法。 */
	FCheckpoint3DParams m_Params;

	/** 直前の成功更新で対象形状が範囲内ならtrue。 */
	bool m_bTargetInside = false;

	/** 接続または最後の初期化後に1回以上発火していればtrue。 */
	bool m_bHasActivated = false;

	/** 成功した接続を生成結果と照合する世代。解除後も次の接続まで保持する。 */
	u64 m_BindingRevision = 0u;
};
