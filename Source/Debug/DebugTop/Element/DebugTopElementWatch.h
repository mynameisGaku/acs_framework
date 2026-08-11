// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElement.h"

using namespace acs;

// 値を見るだけの行。いじれないので、触ってはいけないものを安心して並べられる。

/**
 * 右カラムへ毎フレーム値を出すだけの行。
 *
 * @details
 * 値は自分で持たず、渡されたデリゲートから毎フレーム取り直す。ゲーム側の変数をそのまま
 * 覗きたいときに使う (状態名・残数・座標など、文字列にできるものなら何でもよい)。
 *
 * 左カラムの表示名を差し替えたいだけなら CDebugTopElement::SetLabelProvider を使う。
 * こちらは右カラムを差し替える。折れ線で推移も見たいなら CDebugTopElementGraph。
 */
class CDebugTopElementWatch : public CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @details デリゲートは毎フレーム呼ばれるので、重い処理を書かないこと。
	 * @param Label 左カラムへ出す表示名。
	 * @param Provider 右カラムへ出す文字列を作るもの。
	 */
	CDebugTopElementWatch( const FString& Label, FDebugTopTextDelegate Provider )
		: CDebugTopElement( Label )
		, m_Provider( Provider )
	{
	}

	/**
	 * 見に行く先を差し替える。
	 *
	 * @param Provider 右カラムへ出す文字列を作るもの。
	 */
	void SetProvider( FDebugTopTextDelegate Provider ) noexcept { m_Provider = Provider; }

	/** 右カラムへ、いまの値を出す。 */
	FString GetValueText() const override
	{
		FString Text;
		if ( m_Provider.IsBound() ) m_Provider.TryExecute( Text );
		return Text;
	}

private:
	/** 右カラムへ出す文字列を作るもの。 */
	FDebugTopTextDelegate m_Provider;
};
