// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * ACSの時刻状態から3D場面へ反映する太陽、空、環境光をまとめた値。
 *
 * @details
 * `CAmbientDirector`が所有する時刻補間と太陽高度を再利用し、Framework側では
 * 3D場面へ必要な色と強さだけを補う。GPU資源や更新時刻は所有しない。
 */
struct FTimeOfDay3DAppearance
{
	/** 天頂方向の線形RGB。 */
	FVec3 SkyZenith{ 0.15f, 0.35f, 0.78f };

	/** 地平線方向の線形RGB。 */
	FVec3 SkyHorizon{ 0.70f, 0.83f, 0.95f };

	/** 地面方向の線形RGB。 */
	FVec3 SkyGround{ 0.18f, 0.20f, 0.20f };

	/** 面から太陽へ向かう単位方向。 */
	FVec3 SunDirectionToLight{ 0.0f, 1.0f, 0.0f };

	/** 太陽の線形RGB。 */
	FVec3 SunColor{ 1.0f, 0.95f, 0.85f };

	/** 太陽色へ掛ける正の強さ。夜も既定光への切替を防ぐ微小値を保つ。 */
	f32 SunIntensity = 1.6f;

	/** 場面が設定したIBL環境光へ掛ける0から1の倍率。 */
	f32 EnvironmentLightMultiplier = 1.0f;

	/**
	 * ACSの現在時刻から3D環境の見た目を評価する。
	 *
	 * @param Director 正規化済み時刻、空色、環境色、太陽高度を持つACSの計算器。
	 * @param SunAzimuthDegrees 太陽軌道を+Y軸回りへ回す有限な角度。正で+Xから-Zへ回す。
	 * @param OutAppearance 成功時だけ更新する出力。
	 * @return 全入力と計算結果を描画へ安全に渡せる場合だけtrue。
	 */
	static bool TryEvaluate( const CAmbientDirector& Director, f32 SunAzimuthDegrees,
		FTimeOfDay3DAppearance& OutAppearance ) noexcept;

	/**
	 * 空に描く太陽円盤を、方向光と同じ方向・色へ揃える。
	 *
	 * @param Sky 更新するACSの空。
	 * @return 全ての値が有効で、太陽方向と色を反映できた場合だけtrue。
	 */
	bool TryApplySunTo( CSky& Sky ) const noexcept;

	/** 全ての色、方向、倍率を3D描画へ安全に渡せるか返す。 */
	bool IsValid() const noexcept;
};
