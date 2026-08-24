// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Bridge3D/Bridge3DDirection.h"
#include "AcsFramework_Core/Scene/Fence3D/Fence3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Ground3D/Ground3DSpawnParams.h"

using namespace acs;
using namespace acs::game;

/** 歩ける床板と両側の柵を持つ、軸方向の衝突付き3D橋を置くときの指定。 */
struct FBridge3DSpawnParams
{
	/** 配置先親から見た、入口境界の床板上中心。root直下ではworld位置になる。 */
	FVec3 EntranceCenter{ 0.0f, 0.0f, 0.0f };

	/** 入口から出口へ橋を伸ばすローカル軸方向。 */
	EBridge3DDirection Direction = EBridge3DDirection::PositiveZ;

	/** 両側の床板端を結ぶ全幅。柵支柱より十分広くする。 */
	f32 Width = 3.0f;

	/** 入口境界から出口境界までの床板全長。 */
	f32 Length = 8.0f;

	/** 床板上面から下へ持たせる表示尺度と箱型衝突の厚さ。 */
	f32 DeckThickness = 0.45f;

	/** 床板上面から支柱上端までの柵高。 */
	f32 RailingHeight = 1.15f;

	/** 隣り合う支柱中心の間に許す最大距離。 */
	f32 MaximumPostSpacing = 2.0f;

	/** XZの両方向へ使う正方形支柱の全幅。 */
	f32 PostThickness = 0.14f;

	/** 各側へ置く、1から`FFence3DSpawnParams::kMaximumRailCount`までの横桟数。 */
	u32 RailCount = 2u;

	/** 各横桟のY方向全寸法。 */
	f32 RailHeight = 0.10f;

	/** 柵面に直交する方向にある各横桟の全厚。 */
	f32 RailThickness = 0.08f;

	/** 床板の表面色。各成分は0から1。 */
	FVec4 DeckColor{ 0.34f, 0.38f, 0.44f, 1.0f };

	/** 両側柵の表面色。各成分は0から1。 */
	FVec4 RailingColor{ 0.20f, 0.23f, 0.28f, 1.0f };

	/** 床板の金属らしさ。0で非金属、1で金属。 */
	f32 DeckMetallic = 0.0f;

	/** 床板の表面の粗さ。0で鏡面、1で完全に拡散する。 */
	f32 DeckRoughness = 0.66f;

	/** 両側柵の金属らしさ。0で非金属、1で金属。 */
	f32 RailingMetallic = 0.35f;

	/** 両側柵の表面の粗さ。0で鏡面、1で完全に拡散する。 */
	f32 RailingRoughness = 0.42f;

	/** 床板自身が影を落とすならtrue。 */
	bool bDeckCastsShadow = true;

	/** 両側柵が影を落とすならtrue。 */
	bool bRailingsCastShadow = true;

	/** 床板、支柱、横桟が属する非0の衝突レイヤー。 */
	u32 CollisionLayer = CCollisionWorld3D::kAllLayers;

	/** 床板ノードへ付ける名前。 */
	FStringView DeckName = FStringView( "BridgeDeck" );

	/** 両側の各支柱ノードへ共通で付ける名前。 */
	FStringView RailingPostName = FStringView( "BridgeRailingPost" );

	/** 両側の各横桟ノードへ共通で付ける名前。 */
	FStringView RailingRailName = FStringView( "BridgeRailingRail" );

	/**
	 * 幅、長さ、柵高、入口位置、方向だけを指定した橋設定を作る。
	 *
	 * @param InWidth 床板の全幅。
	 * @param InLength 入口境界から出口境界までの全長。
	 * @param InRailingHeight 床板上面からの柵高。
	 * @param InEntranceCenter 入口境界の床板上中心。
	 * @param InDirection 入口から出口へ伸びる軸方向。
	 * @return そのまま配置へ渡せる橋設定。不正値は`IsValid`で拒否される。
	 */
	static FBridge3DSpawnParams FromDimensions( f32 InWidth, f32 InLength,
		f32 InRailingHeight, FVec3 InEntranceCenter = FVec3{},
		EBridge3DDirection InDirection = EBridge3DDirection::PositiveZ ) noexcept;

	/**
	 * 床板1枚と幅軸の負側・正側にある柵2組の既存設定を計算する。
	 *
	 * @details 支柱は床板の四隅から半幅内側へ置き、入口と出口からはみ出さない。
	 * 失敗時は3つの出力を変更しない。
	 * @param OutDeck 成功時に受け取る床板設定。
	 * @param OutNegativeRailing 成功時に受け取る幅軸負側の柵設定。
	 * @param OutPositiveRailing 成功時に受け取る幅軸正側の柵設定。
	 * @return 表示と衝突が一致する既存設定3組へ安全に変換できた場合だけtrue。
	 */
	bool TryBuildParts( FGround3DSpawnParams& OutDeck,
		FFence3DSpawnParams& OutNegativeRailing,
		FFence3DSpawnParams& OutPositiveRailing ) const noexcept;

	/** 床板と両側柵を半端なく生成できる値か返す。 */
	bool IsValid() const noexcept;
};
