// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Common/Text/InternedNamePool.h"
#include "Debug/DevConsole/ConsoleLogTail.h"
#include "Debug/DevConsole/IConsoleCommandProvider.h"

using namespace acs;
using namespace acs::game;

/**
 * 打ち込んだ 1 行でゲームを操るコンソールを持って配線するサブシステム。
 *
 * @details
 * 解釈と実行はエンジン (CDevConsole) が持っている。ただし**誰も持っておらず、
 * 何も登録されていない**ので、ゲームごとに次を書くことになる。ここが引き受ける。
 *
 * 1. コンソールの実体を持つ
 * 2. 名前と説明文を、消えない場所へ写してから登録する
 * 3. 「自分にはこれができる」と申告してくる機能 (IConsoleCommandProvider) を回す
 *
 * **コマンドの中身は持たない。** ここが各機能を知ってしまうと、コンソールが全部への
 * 参照置き場になる。申告する側が自分の関数を登録する。
 *
 * 画面はこのサブシステムの担当ではない。デバッグメニューのページ (ADevConsolePage) が
 * 表示と打ち込みを受け持つ。
 *
 * @code
 * Console->AddProvider( MakeUnique<CConsoleCommandsAudio>( *Audio ) );
 * Console->Execute( FString( "audio.bgm Assets/Bgm/Field.wav" ) );
 * @endcode
 */
class CDevConsoleSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CDevConsoleSubsystem )

	/**
	 * コマンドを申告するものを受け取り、その場で登録する。
	 *
	 * @details
	 * 渡したものの寿命はここが持つ (登録した関数はコンソールから呼ばれ続けるため)。
	 * 登録は取り消せない (エンジン側に解除の口がない)。
	 * @param Provider 申告するもの。
	 * @return 登録できたら true。
	 */
	bool AddProvider( TUniquePtr<IConsoleCommandProvider> Provider ) noexcept;

	/**
	 * 1 行を実行する。
	 *
	 * @details 見つからないコマンドは、コンソールの記録にその旨が残る。
	 * @param CommandLine 「名前 引数 引数…」の 1 行。
	 */
	void Execute( const FString& CommandLine ) noexcept;

	/**
	 * 記録へ 1 行書く。
	 *
	 * @param Message 書く内容。
	 */
	void Log( const FString& Message ) noexcept;

	/**
	 * 記録の末尾を写し取る。
	 *
	 * @details 表示側はこれを 1 フレームに 1 度だけ呼び、写した値を読むこと。
	 * @param OutTail 写し先。
	 * @param MaxLines 写す最大行数。
	 */
	void CaptureLogTail( CConsoleLogTail& OutTail, usize MaxLines ) const noexcept;

	/** 記録を消す。 */
	void ClearLog() noexcept;

	/** 登録されているコマンドの数を返す。 */
	u32 GetCommandCount() const noexcept { return m_Console.CommandCount(); }

	/** 申告するものを受け取った数を返す。 */
	usize GetProviderCount() const noexcept { return m_Providers.Num(); }

private:
	/** コンソール本体。 */
	CDevConsole m_Console;

	/** コンソールへ渡した名前と説明文の実体。 */
	CInternedNamePool m_Names;

	/** 申告するもの。寿命をここで持つ。 */
	TArray<TUniquePtr<IConsoleCommandProvider>> m_Providers;
};
