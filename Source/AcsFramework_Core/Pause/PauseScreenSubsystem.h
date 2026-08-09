#pragma once

#include <acs.h>

#include "AcsFramework_Core/Time/TimeSubsystem.h"

using namespace acs;

/**
 * 止まっていることを見せる幕。
 *
 * @details
 * ロード画面と同じ作りで、シーンを差し替えず今の画面の上へ重ねる。時間を止める側
 * (CTimeSubsystem) はこの幕を知らないので、止め方を変えても見せ方は影響を受けない。
 *
 * **見に行くのは名前を決めた理由 1 つだけ。** 時間はいろいろな理由で止まる (デバッグメニューを
 * 開いた、ムービーに入った) が、そのたびにポーズ画面が出ては困る。「この理由で止まっている
 * ときだけ出す」と決めておく。
 *
 * 出し入れは滑らかに繋ぐので、一瞬だけ止めてもちらつかない。
 *
 * @code
 * // 止める側 (ゲーム)
 * Time->Pause( "PauseMenu" );
 *
 * // 見せる側 (シーンの入り口で 1 度だけ)
 * Pause->Follow( *Time, FString( "PauseMenu" ), FString( "PAUSED" ) );
 *
 * // 自分で出し入れしたいときは Follow を使わずに
 * Pause->Show( FString( "PAUSED" ) );
 * Pause->Disable();
 * @endcode
 */
class CPauseScreenSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CPauseScreenSubsystem )

	/**
	 * 幕を出す。
	 *
	 * @details 既に出ている場合は文言だけ差し替える (出し直しでちらつかせない)。
	 * @param Message 中央へ出す文言 (空なら文字を出さない)。
	 */
	void Show( const FString& Message = FString() );

	/** 幕を出す (文言はそのまま)。 */
	void Enable() noexcept { SetEnabled( true ); }

	/** 幕を消す (すぐには消えず、薄くなってから消える)。 */
	void Disable() noexcept { SetEnabled( false ); }

	/** 出ていれば消し、消えていれば出す。 */
	void Toggle() noexcept { SetEnabled( !m_bVisible ); }

	/**
	 * 出す / 消すを切り替える。
	 *
	 * @param bEnabled 出すなら true。
	 */
	void SetEnabled( bool bEnabled ) noexcept { m_bVisible = bEnabled; }

	/**
	 * 時間の担当を見に行き、決めた理由で止まっている間だけ出す。
	 *
	 * @details
	 * 出るのも消えるのも自動になる。他の理由 (デバッグメニュー等) で止まっていても出さない。
	 * @param Time 見に行く時間の担当。
	 * @param Reason 反応する理由 (Pause へ渡したものと同じ文字列)。
	 * @param Message 出している間に中央へ出す文言。
	 */
	void Follow( const CTimeSubsystem& Time, const FString& Reason, const FString& Message = FString() );

	/** 見に行くのをやめる (出したままにはしない)。 */
	void Unfollow() noexcept;

	/**
	 * 文言だけを差し替える。
	 *
	 * @param Message 中央へ出す文言。
	 */
	void SetMessage( const FString& Message );

	/**
	 * 文言に使うフォントを差し替える。
	 *
	 * @details
	 * 既定はエンジン共有の UI フォントだが、これは ASCII と仮名しか焼いていないので漢字が
	 * 無言で消える。漢字を出すなら、漢字を含めて焼いたフォントをここへ渡すこと。
	 * 渡したフォントは呼び出し側が生かし続けること。
	 * @param Font 使うフォント (nullptr で共有フォントへ戻す)。
	 */
	void SetFont( const FFont* Font ) noexcept { m_Font = Font; }

	/** 出す指示が生きているかを返す (薄くなっている最中は false)。 */
	bool IsVisible() const noexcept { return m_bVisible; }

	/** 画面に何か出ているかを返す (消えかけも含む)。 */
	bool IsOnScreen() const noexcept { return m_bVisible || m_Alpha > 0.0f; }

	/**
	 * 1 フレーム進める。
	 *
	 * @details
	 * アプリの更新から毎フレーム呼ぶ。止まっている間も呼ばれること (止まっていることを
	 * 見せる幕なので、止まると動かなくなっては困る)。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 */
	void Update( f32 DeltaSeconds ) noexcept;

	/**
	 * 幕を描く。
	 *
	 * @details
	 * シーンを描き終えた後に呼ぶこと。描画資源は最初に出すときだけ用意するので、
	 * 一度も出さなければ GPU 資源を 1 つも作らない。
	 * @param Renderer 描画資源の取得元。
	 * @param SharedFont エンジン共有の UI フォント (SetFont の指定が無いときに使う)。
	 */
	void Draw( CRenderer& Renderer, const FFont* SharedFont ) noexcept;

private:
	/** 見に行っている理由が立っているかを、出し入れへ反映する。 */
	void UpdateFollow() noexcept;

	/** 中央へ出す文言。 */
	FString m_Message;

	/** 反応する理由。 */
	FString m_Reason;

	/** 文言に使うフォント (nullptr なら共有フォント)。所有はしない。 */
	const FFont* m_Font = nullptr;

	/** 幕の濃さ (0 で透明、1 で出し切り)。 */
	f32 m_Alpha = 0.0f;

	/** 幕を描くための SpriteBatch (最初に出すときだけ用意する)。 */
	CSpriteBatch m_Overlay;

	/** 見に行っている時間の担当。所有はしない (nullptr なら誰も見ていない)。 */
	const CTimeSubsystem* m_Followed = nullptr;

	/** 出す指示が生きているか。 */
	bool m_bVisible = false;

	/** 描画資源を用意しようとしたか (失敗を毎フレーム繰り返さないため)。 */
	bool m_bOverlayTried = false;

	/** 描画資源が使える状態か。 */
	bool m_bOverlayReady = false;

	/** フォントが無いことを既に知らせたか (毎フレーム出さないため)。 */
	bool m_bFontWarned = false;
};
