// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Assets/AssetLoadRequest.h"
#include "AcsFramework_Core/Scene/SceneTravelSubsystem.h"
#include "Debug/DebugTop/DebugTopHUD.h"
#include "Debug/DebugTop/Element/DebugTopElements.h"
#include "Debug/DebugTop/Page/DebugTopEntity.h"

using namespace acs;

/**
 * デバッグビルドの起点になるデバッグメニューのシーン。
 *
 * @details
 * メニューの表示とページ管理は ADebugTopHUD が持ち、中身は ADebugTopEntity を派生した
 * ページが自分の OnBuild で組み立てる。本シーンはページの登録と、行から要求された
 * シーン遷移の実行だけを担当する。
 */
class ADebugTopScene : public AScene
{
public:
	/** シーンが top に積まれたときに 1 回呼ばれる。 */
	void OnEnter() noexcept override;

	/** シーン終了前にアセット読み込みの観測と表示追従を解除する。 */
	void OnExit() noexcept override;

	/** 2D の標準サービス構成 (Default2D | Camera2D | Physics2D) を要求する。 */
	ESvc WantedServices() const noexcept override { return kScene2DServices; }

	/**
	 * 毎フレームの update。メニューの入力処理を進める。
	 *
	 * @param DeltaSeconds 経過秒。
	 */
	void OnTick( f32 DeltaSeconds ) noexcept override;

	/** HUD view (画面座標、カメラ非依存) のカスタム描画。 */
	void OnDrawHud( FRenderContext& RenderContext, CSpriteBatch& Batch ) noexcept override;

private:
	/** 暗転しながらサンプルシーンへ遷移する (Travel ページの行から呼ばれる)。 */
	void TravelToSampleWithFade() noexcept;

	/** 幕なしでサンプルシーンへ遷移する (Travel ページの行から呼ばれる)。 */
	void TravelToSampleWithCut() noexcept;

	/**
	 * サンプルシーンへ遷移する。設定の保存も併せて行う。
	 *
	 * @param Transition 見せ方 (暗転して切り替えるか、その場で切り替えるか)。
	 */
	void TravelToSample( ESceneTransition Transition ) noexcept;

	/** メニューの現在値を保管庫へ吸い出し、ファイルへ保存する。 */
	void SaveSettings() noexcept;

	/** ファイルから設定を読み込み、メニューへ反映する。 */
	void LoadSettings() noexcept;

	/** 値が変わっていれば、メニューの現在値を保管庫へ吸い上げる。 */
	void SyncSettingsIfChanged() noexcept;

	/** メニューの現在値をクリップボードへ写す (不具合の報告へ貼るため)。 */
	void CopySnapshot() noexcept;

	/** 保存先をエクスプローラーで開く (保存の通知へ置いたボタンから呼ばれる)。 */
	void OpenSaveFolder() noexcept;

	/** ロード画面を出す見本 (進捗なし)。 */
	void ShowLoadingDemo() noexcept;

	/** ロード画面を出す見本 (進捗つき)。 */
	void ShowLoadingProgressDemo() noexcept;

	/**
	 * ロード画面の見本を始める。
	 *
	 * @param bWithProgress 進捗のバーを出すなら true。
	 */
	void StartLoadingDemo( bool bWithProgress ) noexcept;

	/** フェードの見本を出す。 */
	void ShowFadeDemo() noexcept;

	/** ロード画面の出し入れを入れ替える。 */
	void ToggleLoading() noexcept;

	/** アセットの一覧を渡して非同期に読ませる見本 (バーは自動で進む)。 */
	void LoadAssetsDemo() noexcept;

	/** アセットの読み込みが終わったときに呼ばれる。 */
	void OnAssetsLoaded() noexcept;

	/** メニュー本体。ページの木を所有する。 */
	TObjectPtr<ADebugTopHUD> m_HUD;

	/** 直近で保管庫へ吸い上げたときの値の版。 */
	u32 m_SyncedValueVersion = 0;

	/** ロード画面の見本の残り秒 (0 なら出していない)。 */
	f32 m_LoadingDemoLeft = 0.0f;

	/** ロード画面の見本で進捗のバーを出すか。 */
	bool m_bLoadingDemoProgress = false;

	/** アセット読み込み完了通知を待っているか。 */
	bool m_bAssetLoadPending = false;

	/** 待機中のアセット読み込み要求。完了または終了時に無効化する。 */
	FAssetLoadRequest m_AssetLoadRequest;
};
