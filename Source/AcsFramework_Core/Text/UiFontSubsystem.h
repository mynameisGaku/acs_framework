#pragma once

#include <acs.h>

using namespace acs;

/**
 * 画面へ文字を出すための共有フォント。
 *
 * @details
 * エンジンにも共有の UI フォントはあるが、2 つの理由でそのままでは使えない。
 *
 * 1. **漢字が焼かれていない。** 収録が ASCII と仮名だけなので、漢字は例外も警告も無く
 *    消える (「メニューを重ねる」が「メニューをねる」になる)。
 * 2. **シーンの外から取れない。** FRenderContext のフォントはシーンを描いている間だけ
 *    配線されていて、幕を重ねる段 (シーンを描き終えた後) では HasFont() が false になる。
 *
 * そこでこの層で 1 つだけ焼いて持ち、幕もシーンも同じものを使う。焼くのは最初に required
 * された時だけなので、文字を一度も出さなければ GPU 資源を作らない。
 *
 * @code
 * // アプリの描画の頭で 1 度 (ここで必要なら焼かれる)
 * FFont* Font = UiFont->Acquire( GetRenderer() );
 *
 * // シーンや幕からは、焼けているものをそのまま貰う
 * if ( const FFont* Font = UiFont->Peek() ) Batch.DrawString( *Font, "重ねる", X, Y, Color );
 * @endcode
 */
class CUiFontSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CUiFontSubsystem )

	/**
	 * 文字の大きさを設定する。
	 *
	 * @details 変えると次に Acquire したときに焼き直す。
	 * @param SizePixels 文字のピクセルサイズ。
	 */
	void SetSize( f32 SizePixels ) noexcept;

	/** 文字の大きさを返す。 */
	f32 GetSize() const noexcept { return m_SizePixels; }

	/**
	 * 漢字を焼き込むかを設定する。
	 *
	 * @details
	 * 焼くとアトラスが 4096 四方まで大きくなり、最初の 1 回に数百 ms かかる。日本語を出すなら
	 * 必要。既定は true (黙って文字が消えるより、起動が少し重い方がましなため)。
	 * @param bIncludeCjk 焼き込むなら true。
	 */
	void SetIncludeCjk( bool bIncludeCjk ) noexcept;

	/** 漢字を焼き込む設定かを返す。 */
	bool IsIncludeCjk() const noexcept { return m_bIncludeCjk; }

	/**
	 * フォントを用意して返す (必要なら焼く)。
	 *
	 * @details
	 * 焼くのに描画資源が要るので、これを呼べるのは描画側だけ。毎フレーム呼んでよい
	 * (設定が変わっていなければ焼き直さない)。
	 * @param Renderer 描画資源の取得元。
	 * @return 使えるフォント (用意できなければ nullptr)。
	 */
	FFont* Acquire( CRenderer& Renderer ) noexcept;

	/**
	 * 既に焼けているフォントを返す (焼かない)。
	 *
	 * @details
	 * 描画資源を持たない場所 (シーンの HUD 描画など) から使う。まだ焼けていなければ nullptr。
	 * @return 焼けているフォント (無ければ nullptr)。
	 */
	FFont* Peek() noexcept { return m_bReady ? &m_Font : nullptr; }

	/** 焼けているかを返す。 */
	bool IsReady() const noexcept { return m_bReady; }

private:
	/** 焼いたフォント。 */
	FFont m_Font;

	/** 文字のピクセルサイズ。 */
	f32 m_SizePixels = 20.0f;

	/** いま焼けているものの大きさ (焼き直しが要るかの判定に使う)。 */
	f32 m_BakedSize = 0.0f;

	/** 漢字を焼き込むか。 */
	bool m_bIncludeCjk = true;

	/** いま焼けているものが漢字を含むか。 */
	bool m_bBakedCjk = false;

	/** 焼けているか。 */
	bool m_bReady = false;

	/** 焼くのに失敗したか (毎フレーム焼き直そうとしないため)。 */
	bool m_bFailed = false;
};
