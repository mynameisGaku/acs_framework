// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * コンソールの記録から、末尾の数行だけを写し取ったもの。
 *
 * @details
 * CDevConsole は行数を教えてくれず、`LogLine(i)` が範囲外で nullptr を返すだけなので、
 * 何行あるかは端から当たって数えるしかない。それを表示のたびに行ごとにやると、
 * 行数ぶん走査が走る。ここで 1 度だけ数えて写し取る。
 *
 * 写した文字列は自前で持つので、コンソールが後から書き換えても表示中の行は動かない。
 */
class CConsoleLogTail
{
public:
	/**
	 * 末尾から数行を写し取る。
	 *
	 * @param Console 写し元。
	 * @param MaxLines 写す最大行数。
	 */
	void CaptureFrom( const CDevConsole& Console, usize MaxLines ) noexcept;

	/** 写した行数を返す。 */
	usize Num() const noexcept { return m_Lines.Num(); }

	/**
	 * 写した行を返す。
	 *
	 * @param Index 0 が最も古い。
	 * @return 行の文字列 (範囲外なら空)。
	 */
	const FString& Get( usize Index ) const noexcept;

	/** 記録全体の行数を返す (写した数ではない)。 */
	usize GetTotalCount() const noexcept { return m_TotalCount; }

private:
	/**
	 * 記録が何行あるかを数える。
	 *
	 * @param Console 数える相手。
	 * @return 行数。
	 */
	static usize CountLines( const CDevConsole& Console ) noexcept;

	/** 写した行。 */
	TArray<FString> m_Lines;

	/** 記録全体の行数。 */
	usize m_TotalCount = 0u;

	/** 範囲外を返すときの空文字列。 */
	FString m_Empty;
};
