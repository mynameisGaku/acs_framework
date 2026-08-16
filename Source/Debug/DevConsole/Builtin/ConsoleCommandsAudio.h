// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DevConsole/IConsoleCommandProvider.h"

using namespace acs;
using namespace acs::game;

class CAudioSubsystem;
class CConsoleArgumentReader;
class CDevConsoleSubsystem;

/**
 * 音への用事を申告する既製のコマンド。
 *
 * @details
 * `audio.bgm <パス>` で鳴らし、`audio.bgmstop` で止め、`audio.volume <0..1>` で全体の音量を変える。
 * 鳴らす仕組みは CAudioSubsystem が持っているので、ここは引数を読んで渡すだけ。
 */
class CConsoleCommandsAudio : public IConsoleCommandProvider
{
public:
	/**
	 * 相手を受け取る。
	 *
	 * @param Console 結果を書き出す先。
	 * @param Audio 用事を頼む相手。
	 */
	CConsoleCommandsAudio( CDevConsoleSubsystem& Console, CAudioSubsystem& Audio ) noexcept
		: m_Console( &Console )
		, m_Audio( &Audio )
	{
	}

	/** `audio.bgm` / `audio.bgmstop` / `audio.volume` を登録する。 */
	void ProvideConsoleCommands( CConsoleCommandRegistrar& Registrar ) noexcept override;

private:
	/** エンジンから呼ばれる入口 (自分自身へ渡し直すだけ)。 */
	static void OnPlayBgm( void* User, u32 ArgumentCount, const FConsoleArg* Arguments ) noexcept;

	/** エンジンから呼ばれる入口 (自分自身へ渡し直すだけ)。 */
	static void OnStopBgm( void* User, u32 ArgumentCount, const FConsoleArg* Arguments ) noexcept;

	/** エンジンから呼ばれる入口 (自分自身へ渡し直すだけ)。 */
	static void OnVolume( void* User, u32 ArgumentCount, const FConsoleArg* Arguments ) noexcept;

	/** BGM を鳴らす。 */
	void PlayBgm( const CConsoleArgumentReader& Arguments ) noexcept;

	/** BGM を止める。 */
	void StopBgm() noexcept;

	/** 全体の音量を変える。 */
	void SetMasterVolume( const CConsoleArgumentReader& Arguments ) noexcept;

	/** 結果を書き出す先。所有はしない。 */
	CDevConsoleSubsystem* m_Console = nullptr;

	/** 用事を頼む相手。所有はしない。 */
	CAudioSubsystem* m_Audio = nullptr;
};
