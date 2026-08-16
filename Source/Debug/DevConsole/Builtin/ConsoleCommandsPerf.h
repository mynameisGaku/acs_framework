// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DevConsole/IConsoleCommandProvider.h"

using namespace acs;
using namespace acs::game;

class CDevConsoleSubsystem;
class CPerfBudgetSubsystem;

/**
 * 予算の様子を打ち込みから見るための既製のコマンド。
 *
 * @details
 * `perf.frame` で平均と目標、`perf.list` でカテゴリごとの使われ方を記録へ書く。
 * デバッグメニューを開けない場面 (画面を占有したくないとき) でも数字を確かめられる。
 */
class CConsoleCommandsPerf : public IConsoleCommandProvider
{
public:
	/**
	 * 相手を受け取る。
	 *
	 * @param Console 結果を書き出す先。
	 * @param Perf 数字の出どころ。
	 */
	CConsoleCommandsPerf( CDevConsoleSubsystem& Console, CPerfBudgetSubsystem& Perf ) noexcept
		: m_Console( &Console )
		, m_Perf( &Perf )
	{
	}

	/** `perf.frame` と `perf.list` を登録する。 */
	void ProvideConsoleCommands( CConsoleCommandRegistrar& Registrar ) noexcept override;

private:
	/** エンジンから呼ばれる入口 (自分自身へ渡し直すだけ)。 */
	static void OnFrame( void* User, u32 ArgumentCount, const FConsoleArg* Arguments ) noexcept;

	/** エンジンから呼ばれる入口 (自分自身へ渡し直すだけ)。 */
	static void OnList( void* User, u32 ArgumentCount, const FConsoleArg* Arguments ) noexcept;

	/** 平均と目標を記録へ書く。 */
	void ReportFrame() noexcept;

	/** カテゴリごとの使われ方を記録へ書く。 */
	void ReportCategories() noexcept;

	/** 結果を書き出す先。所有はしない。 */
	CDevConsoleSubsystem* m_Console = nullptr;

	/** 数字の出どころ。所有はしない。 */
	CPerfBudgetSubsystem* m_Perf = nullptr;
};
