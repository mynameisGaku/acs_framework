// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 1フレームの天候が3D場面へ与える見た目の値。
 *
 * @details
 * `CWeatherSystem`の現在状態だけから作る純粋な値で、場面や描画機器を所有しない。
 * 降水素材はゲームごとに異なるため、`ParticleDensity`を公開して再生量の判断に使わせる。
 */
struct FWeather3DAppearance
{
	/** IBLによる環境光へ掛ける倍率。 */
	f32 EnvironmentLightMultiplier = 1.0f;

	/** 雨、雪、砂などの粒子を出す量の目安。 */
	f32 ParticleDensity = 0.0f;

	/** 天頂、地平線、地面の空色へ掛ける色。 */
	FVec3 SkyTint{ 1.0f, 1.0f, 1.0f };

	/** 風の強さ。0から1の範囲。 */
	f32 WindStrength = 0.10f;

	/** 場面が元から持つ霧の濃さへ掛ける倍率。 */
	f32 FogDensityMultiplier = 1.0f;

	/** 場面の雲量がこれ未満にならないようにする下限。 */
	f32 MinimumCloudCoverage = 0.0f;

	/** 場面が元から持つ雲の濃さへ掛ける倍率。 */
	f32 CloudDensityMultiplier = 1.0f;

	/** 元の霧が0でも悪天候を見せるために足す濃さ。 */
	f32 ExtraFogDensity = 0.0f;

	/** 霧の色を寄せる先。 */
	FVec3 FogColorTarget{ 0.0f, 0.0f, 0.0f };

	/** 元の霧色から`FogColorTarget`へ寄せる割合。 */
	f32 FogColorBlend = 0.0f;

	/**
	 * Frameworkが見た目を定義している天候種別か返す。
	 *
	 * @param Kind 確認するACSの天候種別。
	 * @return 8種類の既知天候ならtrue。
	 */
	static bool IsSupportedKind( EWeatherKind Kind ) noexcept;

	/**
	 * ACSの天候状態から見た目を評価する。
	 *
	 * @param Weather 評価する現在・目標・遷移率。
	 * @param OutAppearance 成功時だけ更新する出力先。
	 * @return 種別と数値が有効で評価できたらtrue。
	 */
	static bool TryEvaluate( const CWeatherSystem& Weather, FWeather3DAppearance& OutAppearance ) noexcept;

	/**
	 * 描画へ渡せる有限な範囲に収まっているか返す。
	 *
	 * @return 全項目が有効ならtrue。
	 */
	bool IsValid() const noexcept;
};
