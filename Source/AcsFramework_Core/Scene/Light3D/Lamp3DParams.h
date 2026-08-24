// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Light3D/Light3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawnParams.h"

/**
 * 見える発光球と周囲を照らす点光源を、同じ位置へ置く3Dランプの指定。
 *
 * @details
 * 自己発光だけでは周囲を照らさず、点光源だけでは光源本体が見えない。その2つを1組にし、
 * 色と位置を二重に書かずに済ませる。1個につきACSの点光源枠を1灯使用する。
 */
struct FLamp3DParams
{
	/** 配置先親から見た発光球と点光源の中心。 */
	FVec3 Position{ 0.0f, 2.0f, 0.0f };

	/** 発光球の半径。ACSの直径1の球へ2倍して適用する。 */
	f32 Radius = 0.16f;

	/** 発光球と点光源で共有する0から1の線形RGB。 */
	FVec3 Color{ 1.0f, 0.62f, 0.30f };

	/** 発光球からbloomへ渡す0から10のHDR強度。 */
	f32 EmissiveStrength = 4.0f;

	/** 点光源の色へ掛ける強さ。0なら発光球だけを見せる。 */
	f32 LightIntensity = 2.0f;

	/** 点光源が届く距離。 */
	f32 Range = 5.0f;

	/** 発光球ノードに付ける名前。 */
	FStringView BulbName = FStringView( "LampBulb" );

	/** 点光源ノードに付ける名前。 */
	FStringView LightName = FStringView( "LampLight" );

	/**
	 * 位置だけを指定し、暖色の見える点光源を作る。
	 *
	 * @param InPosition 配置先親から見た中心。
	 * @return 既定の半径、色、発光強度、照明強度、到達距離を持つ指定。
	 */
	static FLamp3DParams At( FVec3 InPosition ) noexcept;

	/**
	 * 発光球と点光源の低水準配置指定を組み立てる。
	 *
	 * @details 失敗時は2つの出力を変更しない。半径から作る直径が溢れる指定も拒否する。
	 * @param OutBulb 成功時に受け取る自己発光球の配置指定。
	 * @param OutLight 成功時に受け取る点光源の配置指定。
	 * @return 両方を安全に配置できる値へ変換できた場合だけtrue。
	 */
	bool TryBuildParts( FModel3DSpawnParams& OutBulb,
		FLight3DSpawnParams& OutLight ) const noexcept;

	/** 位置、半径、色と2種類の強度から安全なランプを作れるか返す。 */
	bool IsValid() const noexcept;
};
