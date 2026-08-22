// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3D.h"
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DControlPreset.h"
#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/Weather3D/Weather3DScene.h"
#include "AcsFramework_Core/Simulation/Input/ActionBindingTable.h"
#include "AcsFramework_Core/Simulation/Input/ActionKeyRebindState.h"
#include "AcsFramework_Core/Simulation/Input/DeviceActionReader.h"

using namespace acs;
using namespace acs::game;

/**
 * 3D が映ることを実際に見て確かめるための場面。
 *
 * @details
 * この枠組みの目当ては「3D を、少ない手数で、綺麗に」なので、**何も設定していない状態で
 * どう見えるか**が肝になる。それを目で確かめられる最小の場面。
 *
 * 描くのはエンジン (`ALegacyScene3DAdapter`) が全部やる。ここがするのは、
 * 木に物と光を置くことだけ。
 *
 * ここに置く物は「既定のままどう見えるか」を見るためのものなので、
 * **素材ファイルを要らない形 (立方体・球・板) だけで組む**。素材が無い環境でも必ず映る。
 */
class ADemo3DScene : public AWeather3DScene
{
public:
	/** 物と光を置き、カメラを引く。 */
	void OnEnter() noexcept override;

	/** 第三者視点接続と衝突集合を場面ノードより先に解除する。 */
	void OnExit() noexcept override;

	/**
	 * 見え方の変化が分かるように、置いた物をゆっくり回す。
	 *
	 * @param DeltaSeconds 経過秒。
	 */
	void OnUpdate( f32 DeltaSeconds ) noexcept override;

	/**
	 * キーボード割り当て待ちへ、押下開始のキーを1件ずつ渡す。
	 *
	 * @param Event 配送された入力イベント。
	 */
	void OnEvent( const FEvent& Event ) noexcept override;

protected:
	/**
	 * 3Dの仕上げ後へ、操作できるプレイヤーUIの見本を重ねる。
	 *
	 * @param Context 現フレームの描画先と共有フォント。
	 * @param Sprites 開かれているHUD用スプライトバッチ。
	 */
	void OnDrawHud( FRenderContext& Context, CSpriteBatch& Sprites ) noexcept override;

private:
	/**
	 * 1 秒ぶんためて、平均のフレーム時間を 1 行だけ出す。
	 *
	 * @details
	 * **雲は «どこを向いているか» で値段が変わる。** 上を向けば画面全部が雲になり、
	 * 地平線を見れば 1 本のレイが数十 km を貫く。重さの話をするには数字が要る。
	 *
	 * @param DeltaSeconds 経過秒。
	 */
	void ReportFrameTime( f32 DeltaSeconds ) noexcept;

	/**
	 * FXAAの有効状態と表示を同時に更新する。
	 *
	 * @param bEnabled 新しい有効状態。
	 */
	void SetFxaaEnabled( bool bEnabled ) noexcept;

	/** 現在のキーまたは入力待ちをUIへ反映する。 */
	void RefreshFxaaKeyText() noexcept;

	/**
	 * 現在の3Dカメラから、空間音響が使う聴く位置と向きを作る。
	 *
	 * @param OutListener 作れた聴く位置と向き。
	 * @return 有効なカメラ姿勢を作れたらtrue。
	 */
	bool TryMakeCameraAudioListener( FAudioListener& OutListener ) const noexcept;

	/** 現在の3Dカメラを空間音響の聴く位置へ反映する。 */
	void RefreshSpatialAudioListener() noexcept;

	/** UIに表示中の左右どちらかから、短い3D効果音を鳴らす。 */
	void PlaySpatialDemoSound() noexcept;

	/** 固定された次の位置へ、ACSの動的な水面波紋を1つ追加する。 */
	void AddDemoWaterRipple() noexcept;

	/** デモ用の次天候へ遷移し、ボタンの行き先を進める。 */
	void AdvanceDemoWeather() noexcept;

	/** 現在と次の天候をプレイヤーUIへ反映する。 */
	void RefreshWeatherText() noexcept;

	/** カメラから回転立方体の実表面へ線を当て、結果をUIと3D印へ反映する。 */
	void PickSpinnerGeometry() noexcept;

	/**
	 * 素材不要の操作キャラクターを置き、衝突、追従カメラ、既定操作へ接続する。
	 *
	 * @return 必須の接続を全て完了できたらtrue。
	 */
	bool TryInitializeThirdPersonCharacter() noexcept;

	/** 明示秒と実機入力から操作キャラクターを1回更新する。 */
	void UpdateThirdPersonCharacter( f32 DeltaSeconds ) noexcept;

	/** 回す対象。所有はしない (木が持っている)。 */
	ANode* m_Spinner = nullptr;

	/** 往復させる取り込みモデル。所有はしない (木が持っている)。 */
	ANode* m_Mover = nullptr;

	/** 場面グラフより短く所有する、操作キャラクター用の衝突集合。 */
	TUniquePtr<CSceneCollision3D> m_CharacterCollision;

	/** 移動、向き、追従カメラをまとめる操作キャラクター。 */
	CThirdPersonCharacter3D m_ThirdPersonCharacter;

	/** 操作対象の足元ノード。所有はしない。 */
	ANode* m_CharacterNode = nullptr;

	/** WASDとゲームパッドを第三者視点操作へ変換する専用表。 */
	CActionBindingTable m_CharacterActionBindings;

	/** ジャンプの押した瞬間を判定する前フレームの操作入力。 */
	FActionInput m_PreviousCharacterInput;

	/** 波紋を追加する水面の世代付き識別子。 */
	FNodeId m_WaterSurfaceId;

	/** いま向かっている先。着いたら z の符号を反転して折り返す。 */
	FVec3 m_MoveTarget{ -3.4f, 1.0f, -2.4f };

	/** 直近 1 秒ぶんの経過秒の合計。 */
	f32 m_FrameTimeAccum = 0.0f;

	/** 直近 1 秒ぶんのフレーム数。 */
	u32 m_FrameCount = 0u;

	/** 次の3Dエフェクトまでに経過した秒。 */
	f32 m_EffectElapsedSeconds = 0.0f;

	/** 次の水面波紋までに経過した秒。 */
	f32 m_WaterRippleElapsedSeconds = 0.0f;

	/** 次に使う固定波紋位置の番号。 */
	usize m_WaterRippleIndex = 0u;

	/** FXAAの切り替えを受け付けるボタン。 */
	u32 m_FxaaToggleButton = 0u;

	/** 現在のFXAA状態を表示し、切り替え時に安全に差し替える文字。 */
	u32 m_FxaaStatusText = 0u;

	/** FXAA操作のキーボード割り当てを変更するボタン。 */
	u32 m_FxaaRebindButton = 0u;

	/** 現在のキーボード割り当てまたは入力待ちを示す文字。 */
	u32 m_FxaaKeyText = 0u;

	/** 左右定位を交互に試すボタン。 */
	u32 m_SpatialSoundButton = 0u;

	/** 直前に鳴らした側または失敗理由を示す文字。 */
	u32 m_SpatialSoundStatusText = 0u;

	/** 嵐、霧、晴天を順に試すボタン。 */
	u32 m_WeatherButton = 0u;

	/** 現在または遷移中の天候を示す文字。 */
	u32 m_WeatherStatusText = 0u;

	/** 次に適用するデモ天候の番号。 */
	usize m_NextWeatherIndex = 0u;

	/** 回転立方体の実表面へ線を当てるボタン。 */
	u32 m_GeometryPickButton = 0u;

	/** 実形状判定の直近結果を示す文字。 */
	u32 m_GeometryPickStatusText = 0u;

	/** 実機キーをFXAA操作へ変換する割り当て表。 */
	CActionBindingTable m_ActionBindings;

	/** 実機キー状態を割り当て表へ渡す読み口。 */
	CDeviceActionReader m_ActionReader;

	/** 押下開始を判定するために保持する前フレームのアクション入力。 */
	FActionInput m_PreviousActionInput;

	/** FXAA操作の現在キーと、次のキーを待つ状態。 */
	FActionKeyRebindState m_FxaaKeyRebind;

	/** 割り当て確定に使った押下をFXAA切り替えへ重ねて使わないための印。 */
	bool m_bSuppressBoundActionPress = false;

	/** 取消キーを基底場面が処理し終えた後に自由カメラを戻すための印。 */
	bool m_bRestoreFreeCameraAfterUpdate = false;

	/** 入力待ち前に自由カメラが有効だったか。 */
	bool m_bFreeCameraWasEnabledBeforeCapture = false;

	/** 次の3D効果音を右から鳴らすか。falseなら左。 */
	bool m_bNextSpatialSoundRight = false;
};
