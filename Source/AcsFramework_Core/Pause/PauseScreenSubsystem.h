// SPDX-License-Identifier: MIT
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Pause/PauseScreenRenderer.h"
#include "AcsFramework_Core/Time/TimeSubsystem.h"

using namespace acs;

/**
 * 指定した時間停止理由または直接設定した状態に従い、ポーズ幕と濃さを管理する。
 * 追従先が無い場合は自動判定せず、描画資源を取得できない場合は無描画で継続する。
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

	/** 表示する状態かを返す (薄くなっている最中は false)。 */
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

	/** GPU 描画資源とポーズ幕の見た目を保持する通常型。 */
	FPauseScreenRenderer m_Renderer;

	/** 見に行っている時間の担当。所有はしない (nullptr なら誰も見ていない)。 */
	const CTimeSubsystem* m_Followed = nullptr;

	/** 表示を求めているか。 */
	bool m_bVisible = false;

};
