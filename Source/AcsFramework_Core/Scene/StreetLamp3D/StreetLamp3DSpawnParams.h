// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Block3D/Block3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Light3D/Lamp3DParams.h"

/**
 * 床位置から、衝突付きポストと見える点光源を組み立てる街灯の指定。
 *
 * @details 直立した箱型ポストの上へ`Lamp3D`を載せる。回転箱を使わないため、表示と
 * 箱型衝突は同じ寸法になる。1基につきACSの点光源枠を1灯使用する。
 */
struct FStreetLamp3DSpawnParams
{
	/** 配置先親から見たポスト底面中央。 */
	FVec3 BasePosition{ 0.0f, 0.0f, 0.0f };

	/** 底面から発光球の下端までのポスト高さ。 */
	f32 PostHeight = 2.4f;

	/** 正方形ポストのX・Z方向の全幅。 */
	f32 PostWidth = 0.12f;

	/** ポスト上端へ載せる発光球の半径。 */
	f32 BulbRadius = 0.18f;

	/** ポスト表面の0から1のRGBA。 */
	FVec4 PostColor{ 0.14f, 0.16f, 0.20f, 1.0f };

	/** ポストの金属らしさ。 */
	f32 PostMetallic = 0.78f;

	/** ポスト表面の粗さ。 */
	f32 PostRoughness = 0.32f;

	/** 発光球と点光源で共有する0から1の線形RGB。 */
	FVec3 LampColor{ 1.0f, 0.62f, 0.30f };

	/** 発光球からbloomへ渡す0から10のHDR強度。 */
	f32 EmissiveStrength = 4.0f;

	/** 点光源の色へ掛ける強さ。 */
	f32 LightIntensity = 2.2f;

	/** 点光源が届く距離。 */
	f32 LightRange = 6.0f;

	/** ポストが属する非0の衝突レイヤー。 */
	u32 CollisionLayer = CCollisionWorld3D::kAllLayers;

	/** ポストノードに付ける名前。 */
	FStringView PostName = FStringView( "StreetLampPost" );

	/** 発光球ノードに付ける名前。 */
	FStringView BulbName = FStringView( "StreetLampBulb" );

	/** 点光源ノードに付ける名前。 */
	FStringView LightName = FStringView( "StreetLampLight" );

	/**
	 * 床位置だけを指定した既定の街灯を作る。
	 *
	 * @param InBasePosition 配置先親から見たポスト底面中央。
	 * @return 既定の高さ、金属ポスト、暖色ランプを持つ指定。
	 */
	static FStreetLamp3DSpawnParams At( FVec3 InBasePosition ) noexcept;

	/**
	 * 衝突付きポストとランプの低水準配置指定を組み立てる。
	 *
	 * @details 失敗時は2つの出力を変更しない。派生位置が溢れる指定も拒否する。
	 * @param OutPost 成功時に受け取るポストの表示・箱衝突指定。
	 * @param OutLamp 成功時に受け取る発光球・点光源指定。
	 * @return 両方を安全に配置できる値へ変換できた場合だけtrue。
	 */
	bool TryBuildParts( FBlock3DSpawnParams& OutPost,
		FLamp3DParams& OutLamp ) const noexcept;

	/** 床位置、寸法、材質、光と衝突レイヤーから安全な街灯を作れるか返す。 */
	bool IsValid() const noexcept;
};
