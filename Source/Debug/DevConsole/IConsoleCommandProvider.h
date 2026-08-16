// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

class CConsoleCommandRegistrar;

/**
 * 自分のコマンドを申告する差込口。
 *
 * @details
 * コマンドの中身をコンソール側へ集めると、そこへ全機能への参照が集まってしまう。向きを逆にして、
 * **機能の側が「自分にはこれができる」と申告する**形にしてある。コンソールは文字列を受けて
 * 登録済みの関数を呼ぶだけで、何が登録されているかを知らない。
 *
 * 実装はコンソールより長く生きること (登録した関数はコンソールから呼ばれ続ける)。
 * CDevConsoleSubsystem::AddProvider へ渡せば寿命はそちらが持つ。
 *
 * @code
 * class CMyCommands : public IConsoleCommandProvider
 * {
 *     void ProvideConsoleCommands( CConsoleCommandRegistrar& Registrar ) noexcept override
 *     {
 *         Registrar.Add( FString( "my.hello" ), FString( "挨拶する" ), &CMyCommands::Hello, this );
 *     }
 * };
 * @endcode
 */
class IConsoleCommandProvider
{
public:
	/** 派生を正しく破棄するための仮想デストラクタ。 */
	virtual ~IConsoleCommandProvider() noexcept = default;

	/**
	 * 自分のコマンドを登録する。
	 *
	 * @details 起動時に 1 度だけ呼ばれる。ここで登録した関数は解除できない。
	 * @param Registrar 登録の窓口。
	 */
	virtual void ProvideConsoleCommands( CConsoleCommandRegistrar& Registrar ) noexcept = 0;
};
