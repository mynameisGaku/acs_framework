// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Audio/Music/MusicStateRequest.h"

using namespace acs;

/**
 * 集まった申告から 1 つを選ぶ係。
 *
 * @details
 * 溜める (AddRequest) のと、選ぶ (ResolveWinner) のを分けてある。フレームの途中でどこから
 * 申告が来ても、実際に曲が変わるのは選んだ 1 か所だけになる。
 *
 * 選ぶ処理は溜めた値だけで決まるので、状態を変えない。曲も音も知らない。
 */
class CMusicStateArbiter
{
public:
	/**
	 * 申告を溜める。
	 *
	 * @details ここでは選ばない。
	 * @param Request 申告 1 件。
	 * @return 溜められたら true。
	 */
	bool AddRequest( const FMusicStateRequest& Request ) noexcept;

	/**
	 * 溜まった中から 1 つを選ぶ。
	 *
	 * @details 強さが同じなら、後から来たほうを採る (直近の状況を優先する)。
	 * @param OutWinner 選ばれた申告の入れ先。
	 * @return 1 件でも溜まっていれば true。
	 */
	bool ResolveWinner( FMusicStateRequest& OutWinner ) const noexcept;

	/** 溜めたものを捨てる (フレームの終わりに呼ぶ)。 */
	void ClearFrame() noexcept { m_Requests.Reset(); }

	/** 溜まっている数を返す。 */
	usize Num() const noexcept { return m_Requests.Num(); }

private:
	/** このフレームに溜まった申告。 */
	TArray<FMusicStateRequest> m_Requests;
};
