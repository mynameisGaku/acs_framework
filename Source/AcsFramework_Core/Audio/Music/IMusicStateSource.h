// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Audio/Music/MusicStateRequest.h"

using namespace acs;

/**
 * 毎フレーム「いまの状態」を申告する差込口。
 *
 * @details
 * 呼ばれたときだけ答える形にしてあるので、実装側はイベントの取りこぼしを気にしなくてよい
 * (状態を持っていて、聞かれたら今の値を返すだけ)。
 *
 * 実装はサブシステムより長く生きること。CMusicSubsystem::AddSource へ渡せば寿命はそちらが持つ。
 *
 * @code
 * bool CBattleMusic::TryGetMusicState( FMusicStateRequest& OutRequest ) noexcept override
 * {
 *     if ( !m_bInBattle ) return false;
 *
 *     OutRequest.State = EMusicState::Combat;
 *     OutRequest.Intensity = m_Danger;
 *     return true;
 * }
 * @endcode
 */
class IMusicStateSource
{
public:
	/** 派生を正しく破棄するための仮想デストラクタ。 */
	virtual ~IMusicStateSource() noexcept = default;

	/**
	 * いまの状態を申告する。
	 *
	 * @details 毎フレーム呼ばれる。重い処理を書かないこと。
	 * @param OutRequest 申告の入れ先。
	 * @return 申告するなら true。何も言うことがなければ false。
	 */
	virtual bool TryGetMusicState( FMusicStateRequest& OutRequest ) noexcept = 0;
};
