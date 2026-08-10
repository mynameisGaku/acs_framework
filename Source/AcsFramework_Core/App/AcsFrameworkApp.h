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
     * 毎フレーム呼ばれる更新フック。
     *
     * @details CGame::OnUpdate() が現 top シーンを駆動するため、基底呼び出しは必須。
     * @param DeltaSeconds 前フレームからの経過秒。
     */
    void OnUpdate( f32 DeltaSeconds ) noexcept override;

    /**
     * 毎フレーム呼ばれる描画フック (BeginFrame/EndFrame は基底が囲む)。
     *
     * @details CGame::OnRender() が現 top シーンを描画するため、基底呼び出しは必須。
     */
    void OnRender() noexcept override;

    /**
     * 終了時に 1 回呼ばれる後始末フック。
     *
     * @details CGame::OnShutdown() が残ったシーンへ OnExit を流すため、基底呼び出しは必須。
     */
    void OnShutdown() noexcept override;

    /**
     * ウィンドウ/入力イベントを受信したときに呼ばれるフック。
     *
     * @details CGame::OnEvent() が現 top シーンへ配送するため、基底呼び出しは必須。
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
