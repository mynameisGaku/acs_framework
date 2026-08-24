// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

struct FLamp3DParams;

/** 発光球と点光源、および生成時の場面所有関係を保持する結果。 */
class FLamp3DSpawnResult
{
public:
	/** 生成失敗を表す空の結果を作る。 */
	FLamp3DSpawnResult() noexcept = default;

	/**
	 * 発光球と点光源を生成し、両方が成功した場合だけ所有情報を持つ結果を返す。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Params 位置、半径、共有色、発光と照明の強さ。
	 * @param Parent 2ノードを繋ぐ親。nullptrなら場面のルート。
	 * @return 2ノードを配置した結果。失敗時は空で、途中までのノードを破棄予定へ戻す。
	 */
	static FLamp3DSpawnResult TrySpawnInto( CSceneNodeGraph& Graph,
		const FLamp3DParams& Params, ANode* Parent = nullptr ) noexcept;

	/** 現在も生成時の場面で生存する発光球。失効または破棄予定ならnullptr。 */
	ANode* Bulb() const noexcept;

	/** 現在も生成時の場面で生存する点光源。失効または破棄予定ならnullptr。 */
	ANode* Light() const noexcept;

	/** 発光球の生成時の世代付き番号。 */
	FNodeId BulbId() const noexcept { return m_BulbId; }

	/** 点光源の生成時の世代付き番号。 */
	FNodeId LightId() const noexcept { return m_LightId; }

	/** 指定した場面グラフから生成した結果ならtrue。 */
	bool IsOwnedBy( const CSceneNodeGraph& Graph ) const noexcept
	{
		return m_OwnerGraph == &Graph;
	}

	/** 指定したrootを持つ場面内容から生成した結果ならtrue。 */
	bool IsFromRoot( const ANode& Root ) const noexcept
	{
		return m_RootIdentity == &Root;
	}

	/** 保持する2ノードと所有情報を空にする。 */
	void Reset() noexcept { *this = FLamp3DSpawnResult{}; }

	/** 生成時に発光球と点光源を両方配置できた結果ならtrue。 */
	bool Succeeded() const noexcept
	{
		return m_BulbId.IsValid() && m_LightId.IsValid()
			&& m_OwnerGraph != nullptr && m_RootIdentity != nullptr;
	}

	/** 2ノードも所有情報も保持していない空の結果ならtrue。 */
	bool IsEmpty() const noexcept
	{
		return !m_BulbId.IsValid() && !m_LightId.IsValid()
			&& m_OwnerGraph == nullptr && m_RootIdentity == nullptr;
	}

	/** 生成成功結果を条件式で直接調べられるようにする。 */
	explicit operator bool() const noexcept { return Succeeded(); }

private:
	/** 指定した生成番号のノードが現在も同じ場面で生存する場合だけ返す。 */
	ANode* ResolveNode_Internal( FNodeId NodeId ) const noexcept;

	/** ノード自身と全祖先が破棄予定でなければtrue。 */
	static bool IsNodeAlive_Internal( const ANode& Node ) noexcept;

	/** 途中まで生成した点光源と発光球を逆順で破棄予定へ戻す。 */
	static void Rollback_Internal( CSceneNodeGraph& Graph,
		FNodeId BulbId, FNodeId LightId ) noexcept;

	/** 発光球の生成時の世代付き番号。 */
	FNodeId m_BulbId;

	/** 点光源の生成時の世代付き番号。 */
	FNodeId m_LightId;

	/** 2ノードを生成した場面グラフ。 */
	CSceneNodeGraph* m_OwnerGraph = nullptr;

	/** 生成時の場面内容を識別するrootノード。 */
	const ANode* m_RootIdentity = nullptr;
};
