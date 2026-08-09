#include "DebugTopCursor.h"

#include <windows.h>

namespace
{
	/** 差し替える対象のウィンドウ (最初に使うときに探して覚える)。 */
	HWND g_Window = nullptr;

	/** 対象を探し終えたか (見つからなかった場合も繰り返さないため)。 */
	bool g_bSearched = false;

	/** いま設定してある形 (同じ形なら何もしないため)。 */
	EDebugTopCursor g_Current = EDebugTopCursor::Arrow;

	/**
	 * 列挙で見つけたウィンドウを控える。
	 *
	 * @param Window 見つかったウィンドウ。
	 * @param Param 使わない。
	 * @return 見える最上位のウィンドウを見つけたら FALSE (列挙を止める)。
	 */
	BOOL CALLBACK OnThreadWindow( HWND Window, LPARAM Param )
	{
		(void)Param;

		// 見えないウィンドウ (メッセージ専用など) は対象にしない。
		if ( !::IsWindowVisible( Window ) ) return TRUE;
		if ( ::GetWindow( Window, GW_OWNER ) != nullptr ) return TRUE;

		g_Window = Window;
		return FALSE;
	}

	/**
	 * 差し替える対象のウィンドウを返す。
	 *
	 * @return ウィンドウ (見つからなければ nullptr)。
	 */
	HWND FindTargetWindow() noexcept
	{
		if ( g_bSearched ) return g_Window;

		g_bSearched = true;
		::EnumThreadWindows( ::GetCurrentThreadId(), &OnThreadWindow, 0 );
		return g_Window;
	}
}


void DebugTopSetCursor( EDebugTopCursor Shape ) noexcept
{
	if ( Shape == g_Current ) return;

	HWND const Window = FindTargetWindow();
	if ( Window == nullptr ) return;

	HCURSOR const Cursor = ::LoadCursorW( nullptr, Shape == EDebugTopCursor::Text ? IDC_IBEAM : IDC_ARROW );
	if ( Cursor == nullptr ) return;

	::SetClassLongPtrW( Window, GCLP_HCURSOR, reinterpret_cast<LONG_PTR>( Cursor ) );

	// いま乗っている場所にも即座に反映させる (次にマウスが動くまで待たせない)。
	::SetCursor( Cursor );
	g_Current = Shape;
}
