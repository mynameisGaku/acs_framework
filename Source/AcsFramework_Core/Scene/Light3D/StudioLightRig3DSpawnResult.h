// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

struct FStudioLightRig3DParams;

/** 3点照明の3灯と生成時の場面所有関係を保持する結果。 */
class FStudioLightRig3DSpawnResult
{
public:
	/** 生成失敗を表す空の結果を作る。 */
	FStudioLightRig3DSpawnResult() noexcept = default;

	/**
	 * 3灯を生成し、全て成功した場合だけ非公開の所有情報を持つ結果を返す。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Params 被写体中心、見る方向、半径と3灯の見た目。
	 * @param Parent 3灯を繋ぐ親。nullptrなら場面のルート。
	 * @return 3灯を全て配置した結果。失敗時は空で、途中までの光を破棄予定へ戻す。
	 */
	static FStudioLightRig3DSpawnResult TrySpawnInto(
		CSceneNodeGraph& Graph, const FStudioLightRig3DParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/** 現在も生成時の場面で生存するキーライト。失効または破棄予定ならnullptr。 */
	ANode* KeyLight() const noexcept;

	/** 現在も生成時の場面で生存するフィルライト。失効または破棄予定ならnullptr。 */
	ANode* FillLight() const noexcept;

	/** 現在も生成時の場面で生存するリムライト。失効または破棄予定ならnullptr。 */
	ANode* RimLight() const noexcept;

	/** キーライトの生成時の世代付き番号。 */
	FNodeId KeyLightId() const noexcept { return m_KeyLightId; }

	/** フィルライトの生成時の世代付き番号。 */
	FNodeId FillLightId() const noexcept { return m_FillLightId; }

	/** リムライトの生成時の世代付き番号。 */
	FNodeId RimLightId() const noexcept { return m_RimLightId; }

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

	/** 保持する3灯と所有情報を空にする。 */
	void Reset() noexcept { *this = FStudioLightRig3DSpawnResult{}; }

	/** 生成時に3灯を全て配置できた結果ならtrue。 */
	bool Succeeded() const noexcept
	{
		return m_KeyLightId.IsValid() && m_FillLightId.IsValid()
			&& m_RimLightId.IsValid() && m_OwnerGraph != nullptr
			&& m_RootIdentity != nullptr;
	}

	/** 3灯も所有情報も保持していない空の結果ならtrue。 */
	bool IsEmpty() const noexcept
	{
		return !m_KeyLightId.IsValid() && !m_FillLightId.IsValid()
			&& !m_RimLightId.IsValid() && m_OwnerGraph == nullptr
			&& m_RootIdentity == nullptr;
	}

	/** 生成成功結果を条件式で直接調べられるようにする。 */
	explicit operator bool() const noexcept { return Succeeded(); }

private:
	/** 指定した生成番号の光が現在も同じ場面で生存する場合だけ返す。 */
	ANode* ResolveLight_Internal( FNodeId LightId ) const noexcept;

	/** ノード自身と全祖先が破棄予定でなければtrue。 */
	static bool IsNodeAlive_Internal( const ANode& Node ) noexcept;

	/** 途中まで生成した光を逆順で破棄予定へ戻す。 */
	static void Rollback_Internal( CSceneNodeGraph& Graph,
		const FNodeId* LightIds, usize LightCount ) noexcept;

	/** キーライトの生成時の世代付き番号。 */
	FNodeId m_KeyLightId;

	/** フィルライトの生成時の世代付き番号。 */
	FNodeId m_FillLightId;

	/** リムライトの生成時の世代付き番号。 */
	FNodeId m_RimLightId;

	/** 3灯を生成した場面グラフ。 */
	CSceneNodeGraph* m_OwnerGraph = nullptr;

	/** 生成時の場面内容を識別するrootノード。 */
	const ANode* m_RootIdentity = nullptr;
};
