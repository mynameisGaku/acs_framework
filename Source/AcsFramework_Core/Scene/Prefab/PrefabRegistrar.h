// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Common/Text/InternedNamePool.h"

using namespace acs;
using namespace acs::game;

/**
 * 名前と作り方の対応を登録する窓口。
 *
 * @details
 * CPrefabSystem は名前を**複製せずポインタで持つ**。呼び出し側の FString をそのまま渡すと、
 * その FString が消えた時点で対応表が壊れる。ここが名前プールへ写してから渡す。
 *
 * 登録だけを引き受ける。作り方も、作ったものの置き場所も持たない。
 */
class CPrefabRegistrar
{
public:
	/**
	 * 登録先を受け取る。
	 *
	 * @param Prefabs 登録先。
	 * @param Names 名前を写す先。登録先より長く生きること。
	 */
	CPrefabRegistrar( CPrefabSystem& Prefabs, CInternedNamePool& Names ) noexcept
		: m_Prefabs( &Prefabs )
		, m_Names( &Names )
	{
	}

	/**
	 * 作り方を 1 つ登録する。
	 *
	 * @param Name 呼び出すときの名前。
	 * @param Factory 呼ばれると 1 つ作って返すもの。
	 * @param UserData Factory へ渡すもの。登録先より長く生きること。
	 * @return 登録できたら true。
	 */
	bool Add( const FString& Name, PrefabFactoryFn Factory, void* UserData = nullptr ) noexcept;

	/** 登録した数を返す。 */
	usize GetAddedCount() const noexcept { return m_AddedCount; }

private:
	/** 登録先。所有はしない。 */
	CPrefabSystem* m_Prefabs = nullptr;

	/** 名前を写す先。所有はしない。 */
	CInternedNamePool* m_Names = nullptr;

	/** 登録した数。 */
	usize m_AddedCount = 0u;
};
