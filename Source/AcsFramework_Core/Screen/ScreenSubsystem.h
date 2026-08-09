#pragma once

#include <acs.h>

using namespace acs;

/**
 * 窓の見え方を、どこからでも変えられるようにするサブシステム。
 *
 * @details
 * 窓そのものはエンジン (CApplication::GetWindow) が持っている。ただし CApplication への参照は
 * 普通のゲームコードからは辿れないので、この層で受け取って GetSubsystem<CScreenSubsystem>()
 * から頼めるようにする。
 *
 * 扱うのは「どう見えるか」だけ。アプリを終わらせる・フレームの様子を知るといった話は
 * CAppSubsystem が持つ。
 *
 * 画面の大きさは窓から毎回取り直す。フルスクリーンの切り替えや窓のリサイズで変わるので、
 * 値を控えて持ち回らないこと。
 *
 * @code
 * if ( CScreenSubsystem* Screen = GetSubsystem<CScreenSubsystem>() )
 * {
 *     Screen->ToggleFullscreen();
 *     Screen->SetTitle( FString( "拙作ゲーム" ) );
 *     const f32 Aspect = Screen->GetAspect();
 * }
 * @endcode
 */
class CScreenSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CScreenSubsystem )

	/**
	 * 窓を持っているものを受け取る。
	 *
	 * @details アプリの起動時に 1 度だけ呼ぶ。渡さない間は何を頼まれても何も起きない。
	 * @param Application 窓を持っているもの。
	 */
	void Bind( CApplication& Application ) noexcept { m_Application = &Application; }

	/**
	 * 全画面にするかを設定する。
	 *
	 * @details 切り替えると画面の大きさが変わる。控えた大きさを使い回さないこと。
	 * @param bFullscreen 全画面にするなら true。
	 */
	void SetFullscreen( bool bFullscreen ) noexcept;

	/** 全画面と窓を入れ替える。 */
	void ToggleFullscreen() noexcept { SetFullscreen( !IsFullscreen() ); }

	/** 全画面かを返す。 */
	bool IsFullscreen() const noexcept;

	/** 画面の幅 (ピクセル) を返す。 */
	u32 GetWidth() const noexcept;

	/** 画面の高さ (ピクセル) を返す。 */
	u32 GetHeight() const noexcept;

	/**
	 * 画面の縦横比を返す。
	 *
	 * @return 幅 / 高さ (高さが 0 なら 1)。
	 */
	f32 GetAspect() const noexcept;

	/** 最小化されているかを返す。 */
	bool IsMinimized() const noexcept;

	/**
	 * 窓の見出しを設定する。
	 *
	 * @param Title 見出し (UTF-8)。
	 */
	void SetTitle( const FString& Title ) noexcept;

private:
	/** 窓を持っているもの。所有はしない (アプリが持っている)。 */
	CApplication* m_Application = nullptr;
};
