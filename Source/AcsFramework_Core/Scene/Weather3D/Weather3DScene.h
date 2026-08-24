// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Effects/Effect3D/Effect3DScene.h"
#include "AcsFramework_Core/Scene/Weather3D/TimeOfDay3DAppearance.h"
#include "AcsFramework_Core/Scene/Weather3D/Weather3DAppearance.h"

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * ACSの天候遷移を雲、霧、空、環境光へ自動で接続する3D場面。
 *
 * @details
 * 派生側は`SetWeather( EWeatherKind::Storm )`のように指定するだけでよい。派生場面が
 * `OnEnter`で調整した雲・霧・空を最初の更新時に基準として記録し、そこから相対的に変える。
 * 雨雪そのものは作品ごとの素材が必要なので自動生成せず、`WeatherAppearance()`の
 * `ParticleDensity`を3Dエフェクトの発生量として利用できる。
 */
class AWeather3DScene : public AEffect3DScene
{
public:
	/** 晴天状態の天候を持つ場面を作る。 */
	AWeather3DScene() noexcept = default;

	/** 場面固有の天候状態を破棄する。 */
	~AWeather3DScene() noexcept override = default;

	/** 場面状態を重複所有しないためコピーを禁止する。 */
	AWeather3DScene( const AWeather3DScene& ) = delete;

	/** 場面状態を重複所有しないためコピー代入を禁止する。 */
	AWeather3DScene& operator=( const AWeather3DScene& ) = delete;

	/**
	 * 指定した天候への遷移を始める。
	 *
	 * @param Kind 8種類のACS天候。
	 * @param TransitionSeconds 遷移に掛ける有限な0秒以上の時間。0なら即時。
	 * @return 指定を受け付けたらtrue。不正値なら状態を変えずfalse。
	 */
	bool SetWeather( EWeatherKind Kind, f32 TransitionSeconds = 3.0f ) noexcept;

	/**
	 * 降水エフェクトへ渡す風向きを設定する。
	 *
	 * @param Direction XZ平面の有限な非0方向。長さは内部で1に揃える。
	 * @return 設定できたらtrue。不正値なら状態を変えずfalse。
	 */
	bool SetWeatherWindDirection( FVec2 Direction ) noexcept;

	/**
	 * ACSが保持する現在・目標天候と遷移率を読む。
	 *
	 * @return この場面が所有する読み取り専用の天候状態。
	 */
	const CWeatherSystem& Weather() const noexcept { return m_Weather; }

	/**
	 * 現在フレームの描画値と降水量を読む。
	 *
	 * @return 最後に評価できた天候の見た目。
	 */
	const FWeather3DAppearance& WeatherAppearance() const noexcept { return m_Appearance; }

	/**
	 * 時刻に合わせて動く太陽を1灯置き、空とIBL環境光の自動更新を始める。
	 *
	 * @details 既定では無効。呼出しに成功すると、この場面が専用の太陽ノードを所有する。
	 * 手動の太陽を併用すると平行光が増えるため、通常はどちらか一方だけを使う。
	 * @param InitialHour 開始時刻。有限な値なら24時間へ折り返す。
	 * @param GameHoursPerRealSecond 実時間1秒で進めるゲーム内時間。有限な0以上。
	 * @param SunAzimuthDegrees 太陽軌道を+Y軸回りへ回す有限な角度。
	 * @return `OnEnter`後の場面へ太陽を配置し、全指定を反映できた場合だけtrue。
	 */
	bool EnableTimeOfDay3D( f32 InitialHour = 12.0f,
		f32 GameHoursPerRealSecond = 1.0f / 60.0f,
		f32 SunAzimuthDegrees = 0.0f ) noexcept;

	/** 専用の太陽を破棄し、空と環境光を時刻制御前の天候へ戻す。 */
	void DisableTimeOfDay3D() noexcept;

	/**
	 * 現在時刻を変更し、太陽、空、環境光へ即座に反映する。
	 *
	 * @param Hours 有限な時刻。24時間へ折り返す。
	 * @return 時刻制御中で、描画値と太陽を更新できた場合だけtrue。
	 */
	bool SetTimeOfDay3D( f32 Hours ) noexcept;

	/**
	 * 時刻の進行速度を変更する。
	 *
	 * @param GameHoursPerRealSecond 実時間1秒で進めるゲーム内時間。有限な0以上。
	 * @return 時刻制御中で値を受け付けた場合だけtrue。
	 */
	bool SetTimeOfDayRate3D( f32 GameHoursPerRealSecond ) noexcept;

	/**
	 * 太陽軌道の方位を変更し、現在時刻の太陽へ即座に反映する。
	 *
	 * @param SunAzimuthDegrees +Y軸回りの有限な角度。
	 * @return 時刻制御中で、描画値と太陽を更新できた場合だけtrue。
	 */
	bool SetTimeOfDaySunAzimuth3D( f32 SunAzimuthDegrees ) noexcept;

	/** 時刻連動の環境を使用中ならtrue。 */
	bool TimeOfDay3DEnabled() const noexcept { return m_bTimeOfDayEnabled; }

	/** ACSが所有する現在時刻と進行速度を読む。 */
	const CAmbientDirector& TimeOfDay3D() const noexcept { return m_TimeOfDay; }

	/** 最後に完全反映できた時刻連動の見た目を読む。 */
	const FTimeOfDay3DAppearance& TimeOfDay3DAppearance() const noexcept { return m_TimeOfDayAppearance; }

	/** 通常の3D場面を開始し、晴天から使える状態へ戻す。 */
	void OnEnter() noexcept override;

	/** 元の環境設定へ戻してから通常の3D場面を終了する。 */
	void OnExit() noexcept override;

	/** 通常の場面更新と同じ明示秒で天候を進め、描画設定へ反映する。 */
	void OnUpdate( f32 DeltaSeconds ) noexcept override;

private:
	/** 派生場面が作った晴天時の見た目。 */
	struct FEnvironmentBaseline
	{
		/** 晴天時の雲量。 */
		f32 CloudCoverage = 0.0f;

		/** 晴天時の雲の濃さ。 */
		f32 CloudDensity = 1.0f;

		/** 晴天時の雲の流速。 */
		f32 CloudWind = 1.0f;

		/** 晴天時の霧色。 */
		FVec3 FogColor{ 0.0f, 0.0f, 0.0f };

		/** 晴天時の霧の濃さ。 */
		f32 FogDensity = 0.0f;

		/** 晴天時の天頂色。 */
		FVec3 SkyZenith{ 0.0f, 0.0f, 0.0f };

		/** 晴天時の地平線色。 */
		FVec3 SkyHorizon{ 0.0f, 0.0f, 0.0f };

		/** 晴天時の地面方向の空色。 */
		FVec3 SkyGround{ 0.0f, 0.0f, 0.0f };

		/** 晴天時に空へ描いていた太陽の方向。 */
		FVec3 SkySunDirection{ 0.0f, 1.0f, 0.0f };

		/** 晴天時に空へ描いていた太陽の色。 */
		FVec3 SkySunColor{ 1.0f, 1.0f, 1.0f };

		/** 晴天時のIBL環境光倍率。 */
		f32 EnvironmentLightMultiplier = 1.0f;

		/** 描画へ戻せる有限な値か返す。 */
		bool IsValid() const noexcept;
	};

	/** 派生場面が設定し終えた環境を晴天時の基準として記録する。 */
	bool CaptureEnvironment_Internal() noexcept;

	/** 現在の天候を記録済みの基準へ相対適用する。 */
	void ApplyWeather_Internal() noexcept;

	/** 評価済みの時刻環境を、この場面が所有する太陽へ反映する。 */
	bool ApplyTimeOfDaySun_Internal( const FTimeOfDay3DAppearance& Appearance ) noexcept;

	/** 時刻制御が所有する太陽を、存在する場合だけ破棄する。 */
	void DestroyTimeOfDaySun_Internal() noexcept;

	/** 記録済みの基準を描画設定へそのまま戻す。 */
	void RestoreEnvironment_Internal() noexcept;

	/** ACSが所有する場面ローカルな天候遷移。 */
	CWeatherSystem m_Weather;

	/** 現在フレームで評価済みの見た目。 */
	FWeather3DAppearance m_Appearance;

	/** ACSの決定的な24時間補間と進行速度。 */
	CAmbientDirector m_TimeOfDay;

	/** 最後に太陽と環境へ完全反映できた時刻の見た目。 */
	FTimeOfDay3DAppearance m_TimeOfDayAppearance;

	/** 派生場面が作った晴天時の環境。 */
	FEnvironmentBaseline m_Baseline;

	/** 晴天時の環境を記録できたか。 */
	bool m_bEnvironmentCaptured = false;

	/** `OnEnter`後かつ`OnExit`前ならtrue。 */
	bool m_bSceneEntered = false;

	/** 時刻連動の太陽、空、環境光を更新するならtrue。 */
	bool m_bTimeOfDayEnabled = false;

	/** この場面が生成して所有する時刻連動太陽の世代付き識別子。 */
	FNodeId m_TimeOfDaySun;

	/** 太陽軌道を+Y軸回りへ回す角度。 */
	f32 m_TimeOfDaySunAzimuthDegrees = 0.0f;
};
