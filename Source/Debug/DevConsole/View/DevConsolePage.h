// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Page/DebugTopEntity.h"
#include "Debug/DevConsole/ConsoleLogTail.h"

using namespace acs;

class CDebugTopElementString;
class CDevConsoleSubsystem;

/**
 * コンソールを打ち込むデバッグメニューのページ。
 *
 * @details
 * **専用の画面は作らない。** 文字の打ち込みも、描画も、キーの取り回しも DebugTop が既に
 * 持っている。同じものをもう一組作れば、フォントの用意から入力の奪い合いまで二重になる。
 * ここはそのページとして振る舞い、コンソールへ橋を架けるだけにしてある。
 *
 * 上から順に「打ち込む欄」「実行」「消す」「登録数」、その下に記録の末尾が並ぶ。
 * 欄で Enter を押すとそのまま実行される (実行の行は、同じものを もう一度流したいとき用)。
 *
 * @code
 * Overlay->GetHUD().AddEntity( NewObject<ADevConsolePage>( FString( "Console" ), *Console ) );
 * @endcode
 */
class ADevConsolePage : public ADebugTopEntity
{
public:
	/**
	 * ページを構築する。
	 *
	 * @param Name パンくずへ出すページ名。
	 * @param Console 打ち込む先。ページより長く生きること。
	 */
	ADevConsolePage( const FString& Name, CDevConsoleSubsystem& Console );

	/** 記録を写し直してから 1 フレーム進める。 */
	void Update( f32 DeltaSeconds ) noexcept override;

	/** 直近に写し取った記録を返す (行が読む)。 */
	const CConsoleLogTail& GetLogTail() const noexcept { return m_LogTail; }

protected:
	/** 打ち込む欄・操作・記録の行を並べる。 */
	void OnBuild() noexcept override;

private:
	/** 記録を写し直す。 */
	void RefreshLogTail() noexcept;

	/** 打ち込む欄と操作の行を足す。 */
	void BuildCommandRows();

	/** 記録を映す行を足す。 */
	void BuildLogRows();

	/** 欄の中身を実行する。 */
	void ExecuteTypedCommand();

	/** 記録を消す。 */
	void ClearLog();

	/** 登録されているコマンド数の文字列を作る。 */
	FString MakeCommandCountText() const;

	/** 打ち込む先。所有はしない。 */
	CDevConsoleSubsystem* m_Console = nullptr;

	/** 打ち込む欄。所有はページの行配列。 */
	CDebugTopElementString* m_CommandField = nullptr;

	/** 直近に写し取った記録。 */
	CConsoleLogTail m_LogTail;
};
