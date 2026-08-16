// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DevConsole/IConsoleCommandProvider.h"

using namespace acs;
using namespace acs::game;

class CAppSubsystem;
class CConsoleArgumentReader;
class CDevConsoleSubsystem;

/**
 * アプリそのものへの用事を申告する既製のコマンド。
 *
 * @details
 * `app.quit` で終わらせ、`app.fps` で速さを見る。どちらも CAppSubsystem が答えられることを
 * 打ち込みから呼べるようにしただけで、ここが何かを計算することはない。
 *
 * これはモジュールの**利用者**であって、コンソール本体の一部ではない。同じ形でゲーム側も
 * 自分のコマンドを申告できる。
 */
class CConsoleCommandsApp : public IConsoleCommandProvider
{
public:
	/**
	 * 相手を受け取る。
	 *
	 * @param Console 結果を書き出す先。
	 * @param App 用事を頼む相手。
	 */
	CConsoleCommandsApp( CDevConsoleSubsystem& Console, CAppSubsystem& App ) noexcept
		: m_Console( &Console )
		, m_App( &App )
	{
	}

	/** `app.quit` と `app.fps` を登録する。 */
	void ProvideConsoleCommands( CConsoleCommandRegistrar& Registrar ) noexcept override;

private:
	/** エンジンから呼ばれる入口 (自分自身へ渡し直すだけ)。 */
	static void OnQuit( void* User, u32 ArgumentCount, const FConsoleArg* Arguments ) noexcept;

	/** エンジンから呼ばれる入口 (自分自身へ渡し直すだけ)。 */
	static void OnFps( void* User, u32 ArgumentCount, const FConsoleArg* Arguments ) noexcept;

	/** 終了を頼む。 */
	void Quit() noexcept;

	/** 速さを記録へ書く。 */
	void ReportFps() noexcept;

	/** 結果を書き出す先。所有はしない。 */
	CDevConsoleSubsystem* m_Console = nullptr;

	/** 用事を頼む相手。所有はしない。 */
	CAppSubsystem* m_App = nullptr;
};
