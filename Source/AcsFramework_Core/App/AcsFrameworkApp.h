// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>
using namespace acs;

class CAcsFrameworkApp : public CGame
{
public:
    CAcsFrameworkApp();
    ~CAcsFrameworkApp();

private:
    /**
     * サービス配線を完了して初期シーンを返す。
     *
     * @details 配線後に CreateInitialScene() を呼び、空の TUniquePtr<AScene> は CGame の終了契約へ渡す。
     * @return ゲーム固有の初期シーン。
     */
    TUniquePtr<AScene> InitialScene() noexcept final override;

    /**
     * 起動時の初期シーン適用を基底へ委譲する。
     *
     * @details InitialScene() の配線と初期シーン適用を CGame::OnStart() へ渡す。
     */
    void OnStart() noexcept final override;

protected:
    /**
     * ゲーム固有の初期シーンを生成する。
     *
     * @return 初期シーン。空の TUniquePtr<AScene> は CGame の終了契約へ渡る。
     */
    virtual TUniquePtr<AScene> CreateInitialScene() noexcept;

private:

    /**
     * 時間制御に従って現 top シーンとシーン外の各機能を更新する。
     *
     * @details 時間制御がシーン進行を許可したフレームだけ CGame::OnUpdate() が現 top シーンを駆動する。
     * @param DeltaSeconds 前フレームからの経過秒。
     */
    void OnUpdate( f32 DeltaSeconds ) noexcept override;

    /**
     * 現 top シーンとポーズ、デバッグ、ロードの表示を定めた順で描画する。
     *
     * @details CGame::OnRender() が現 top シーンを描画し、その後に各表示を順番に重ねる。
     */
    void OnRender() noexcept override;

    /**
     * 未保存の設定を保存し、残ったシーンを終了処理する。
     *
     * @details 設定を保存した後、CGame::OnShutdown() が残ったシーンを終了処理する。
     */
    void OnShutdown() noexcept override;

    /**
     * 受信したウィンドウ/入力イベントを現 top シーンへ配送する。
     *
     * @details CGame::OnEvent() が現 top シーンへの配送を担う。
     * @param Event 受信したイベント。
     */
    void OnEvent( const FEvent& Event ) noexcept override;

    /**
     * 直近に取れた UI フォント。
     *
     * @details
     * CUiFontSubsystem が焼いたもの。エンジン共有の UI フォントは漢字を持たず、しかも
     * シーンを描き終えると FRenderContext から取れなくなるので、そちらは使わない。
     * 実体はサブシステムが持つので、所有はしない。
     */
    FFont* m_UiFont = nullptr;
};
