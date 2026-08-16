// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * コマンドへ渡ってきた引数を読む係。
 *
 * @details
 * エンジンが渡してくるのは切り分けた文字列だけで、数値への変換も個数の確認も呼ばれた側の仕事に
 * なっている。コマンドごとに書くと、同じ確認が何度も並ぶうえ、確認を忘れたコマンドが混ざる。
 *
 * 読むだけで、コマンドが何をするかは知らない。
 *
 * @code
 * const CConsoleArgumentReader Args( ArgCount, ArgValues );
 *
 * f32 Volume = 0.0f;
 * if ( !Args.TryGetFloat( 0u, Volume ) ) return;
 * @endcode
 */
class CConsoleArgumentReader
{
public:
	/**
	 * 引数の並びを受け取る。
	 *
	 * @param ArgumentCount 引数の数。
	 * @param Arguments 引数の並び (呼び出しの間だけ有効)。
	 */
	CConsoleArgumentReader( u32 ArgumentCount, const FConsoleArg* Arguments ) noexcept
		: m_Count( Arguments != nullptr ? ArgumentCount : 0u )
		, m_Arguments( Arguments )
	{
	}

	/** 引数の数を返す。 */
	u32 Num() const noexcept { return m_Count; }

	/**
	 * 引数を文字列として返す。
	 *
	 * @param Index 0 起点。
	 * @return 引数の文字列 (無ければ nullptr)。
	 */
	const char* GetString( u32 Index ) const noexcept;

	/**
	 * 引数を 1 つの文字列として繋げて返す。
	 *
	 * @details パスのように空白で切れてほしくないものを受けるときに使う。
	 * @param FirstIndex 繋げ始める位置。
	 * @return 空白で繋いだ文字列。
	 */
	FString JoinFrom( u32 FirstIndex ) const;

	/**
	 * 引数を小数として読む。
	 *
	 * @details 読めるのは `[-+]?digits[.digits]` の形だけ。指数表記は受け付けない。
	 * @param Index 0 起点。
	 * @param OutValue 読めた値の入れ先。
	 * @return 読めたら true。
	 */
	bool TryGetFloat( u32 Index, f32& OutValue ) const noexcept;

	/**
	 * 引数を整数として読む。
	 *
	 * @param Index 0 起点。
	 * @param OutValue 読めた値の入れ先。
	 * @return 読めたら true。
	 */
	bool TryGetInt( u32 Index, i32& OutValue ) const noexcept;

private:
	/** 引数の数。 */
	u32 m_Count = 0u;

	/** 引数の並び。所有はしない。 */
	const FConsoleArg* m_Arguments = nullptr;
};
