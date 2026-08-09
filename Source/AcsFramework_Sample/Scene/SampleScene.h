#pragma once
#include <acs.h>

#include "AcsFramework_Core/Event/EventSubsystem.h"

using namespace acs;

/**
 * ゲーム側の都合で時間を止めるときの理由。
 *
 * @details
 * 止める側 (メニューの行) と見せる側 (ポーズの幕) が同じ文字列を使う。デバッグメニューの
 * 理由と分けてあるので、メニューを開いただけではポーズの幕は出ない。
 */
constexpr const char* kGamePauseReason = "PauseMenu";


/**
 * シーンを跨いで残す状態。
 *
 * @details
 * 置けるのは 1 つだけなので、持ち回りたいものはこの型へまとめる。シーンを作り直しても
 * 消えないことを目で見るために、入った回数を数えている。
 */
struct FSampleAppState
{
	/** このシーンへ入った回数。 */
	i32 EnterCount = 0;
};


/**
 * セーブへ書き込む中身。
 *
 * @details
 * そのままメモリを写せる形だけを置く決まりなので、文字列や配列は持たせない (要るなら
 * 固定長の配列にしてここへ埋める)。
 */
struct FSampleSave
{
	/** 何面まで進んだか。 */
	i32 Stage = 1;

	/** 通しの得点。 */
	i32 Score = 0;

	/** 書き込んだ時点で何回シーンへ入っていたか。 */
	i32 EnterCount = 0;
};


/**
 * 時計が刻んだことを知らせる。
 *
 * @details
 * 出す側 (時計を仕掛けた所) と受け取る側 (画面) が互いを知らずに済むよう、型そのものを
 * 宛先にして配る。継承も登録も要らない、ただのデータ。
 */
struct FSampleTick
{
	/** ゲーム時間の時計から出たものか (false なら実時間)。 */
	bool bGameTime = false;
};

#if _DEBUG
class ASampleDebugPage;
#endif

class ASampleScene : public AScene
{
public:
	/** 2D の標準サービス構成 (Default2D | Camera2D | Physics2D) を要求する。 */
	ESvc WantedServices() const noexcept override { return kScene2DServices; }

	/** シーンが top に積まれたときに 1 回呼ばれる。 */
	void OnEnter() noexcept override;

	/** 重ねていたものが下ろされて、また top に戻ったときに呼ばれる。 */
	void OnResume() noexcept override;

	/**
	 * 毎フレーム呼ばれる。
	 *
	 * @details
	 * 渡ってくる経過秒には時間の倍率が乗っている (CTimeSubsystem で変えられる)。ここでは
	 * それをただ足していくので、表示された秒数の進み方が倍率の効き目そのものになる。
	 * @param DeltaSeconds 前フレームからの経過秒 (倍率が乗っている)。
	 */
	void OnUpdate( f32 DeltaSeconds ) noexcept override;

private:
	/** ゲーム時間の時計から呼ばれる。数えるのは自分ではなく、知らせを受けた側がやる。 */
	void OnGameTick() noexcept;

	/** 実時間の時計から呼ばれる。 */
	void OnRealTick() noexcept;

	/**
	 * 時計の知らせを受け取る。
	 *
	 * @param Event 刻んだ時計。
	 */
	void OnTickEvent( const FSampleTick& Event ) noexcept;

public:

	/** HUD view (画面座標、カメラ非依存) のカスタム描画。 */
	void OnDrawHud( FRenderContext& RenderContext, CSpriteBatch& Batch ) noexcept override;

private:
#if _DEBUG
	/** 重ねたメニューの値を、このシーンの変数へ取り込む (行が変わるたびに呼ばれる)。 */
	void PullDebugValues() noexcept;

	/** このシーンを畳んで積み直す (購読が自動で外れることの確認用)。 */
	void ReloadScene() noexcept;

	/** いまの様子を 0 番の枠へ書き込む。 */
	void WriteSave() noexcept;

	/** 0 番の枠から読み出す。 */
	void ReadSave() noexcept;

	/** 0 番の枠を消す。 */
	void EraseSave() noexcept;

	/** 枠の一覧を組み立て直す。 */
	void RefreshSaveSummary() noexcept;

	/** 重ねて触るページ。所有はしない (メニューが所有している)。 */
	ASampleDebugPage* m_DebugPage = nullptr;
#endif

	/** デバッグメニューから受け取った移動速度 (Gameplay/MoveSpeed)。 */
	f32 m_MoveSpeed = 1.0f;

	/** デバッグメニューから受け取った開始レベル (Gameplay/StartLevel)。 */
	i32 m_StartLevel = 0;

	/** 見本の文字色 (重ねたメニューから変えられる)。 */
	FVec4 m_TextColor{ 0.95f, 0.95f, 0.75f, 1.0f };

	/** このシーンが動いていた秒数 (時間の倍率が乗った経過秒の合計)。 */
	f32 m_SceneSeconds = 0.0f;

	/** ゲーム時間の時計が刻んだ回数 (止めている間は増えない)。 */
	i32 m_GameTicks = 0;

	/** 実時間の時計が刻んだ回数 (止めていても増える)。 */
	i32 m_RealTicks = 0;

	/** 重ねた画面から返ってきた答え (まだ受け取っていなければ空)。 */
	FString m_ModalResult;

	/** 重ねた画面を開いた回数。 */
	i32 m_ModalOpenCount = 0;

	/** シーンを跨いで残っている、入った回数。 */
	i32 m_EnterCount = 0;

	/** セーブの様子 (一覧の見出しから組み立てた文字列)。 */
	FString m_SaveSummary;

	/**
	 * 時計の知らせの購読。
	 *
	 * @details メンバとして持つので、このシーンが畳まれれば購読も自動で外れる。
	 */
	FEventSubscription m_TickSubscription;
};
