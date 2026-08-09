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
     * 起動時に最初に積む AScene を返す (CGame の純粋仮想)。
     *
     * @details CGame::OnStart() がこの戻り値を push して即時適用する。
     * @return 最初のシーン (所有権が CSceneManager へ移る)。
     */
    TUniquePtr<AScene> InitialScene() noexcept override;

    /**
     * 起動時に 1 回呼ばれる初期化フック。
     *
     * @details CGame::OnStart() が InitialScene() を push するため、基底呼び出しは必須。
     */
    void OnStart() noexcept override;

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
