// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Common/Text/InternedNamePool.h"

using namespace acs;
using namespace acs::game;

/**
 * コンソールへコマンドを登録する窓口。
 *
 * @details
 * CDevConsole は名前と説明文を**複製せずポインタで持つ**。呼び出し側の FString をそのまま
 * 渡すと、その FString が消えた時点でコンソールの一覧が壊れる。ここが名前プールへ写してから
 * 渡すので、申告する側は普通に FString を組み立ててよい。
 *
 * 登録だけを引き受ける。コマンドの中身も、コンソールの表示も持たない。
 */
class CConsoleCommandRegistrar
{
public:
	/**
	 * 登録先を受け取る。
	 *
	 * @param Console 登録先のコンソール。
	 * @param Names 名前を写す先。コンソールより長く生きること。
	 */
	CConsoleCommandRegistrar( CDevConsole& Console, CInternedNamePool& Names ) noexcept
		: m_Console( &Console )
		, m_Names( &Names )
	{
	}

	/**
	 * コマンドを 1 つ登録する。
	 *
	 * @details 名前と説明文はプールへ写してから渡す。同じ名前を二度登録しないこと。
	 * @param Name コマンド名 (打ち込む文字列)。
	 * @param HelpText 説明文。
	 * @param Function 呼ばれる関数。
	 * @param UserData 関数へ渡すもの。コンソールより長く生きること。
	 * @return 登録できたら true。
	 */
	bool Add( const FString& Name, const FString& HelpText, CommandFn Function, void* UserData ) noexcept;

	/** 登録した数を返す。 */
	usize GetAddedCount() const noexcept { return m_AddedCount; }

private:
	/** 登録先。所有はしない。 */
	CDevConsole* m_Console = nullptr;

	/** 名前を写す先。所有はしない。 */
	CInternedNamePool* m_Names = nullptr;

	/** 登録した数。 */
	usize m_AddedCount = 0u;
};
