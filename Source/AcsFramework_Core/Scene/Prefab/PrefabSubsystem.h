// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Prefab/IPrefabProvider.h"
#include "AcsFramework_Core/Scene/Prefab/PrefabSpawnParams.h"
#include "Common/Text/InternedNamePool.h"

using namespace acs;
using namespace acs::game;

/**
 * 名前で ANode を出せるようにするサブシステム。
 *
 * @details
 * 対応表と生成はエンジン (CPrefabSystem) が持っている。ただし**誰も持っておらず、
 * 何も登録されていない**ので、ゲームごとに次を書くことになる。ここが引き受ける。
 *
 * 1. 対応表の実体を持つ
 * 2. 名前を、消えない場所へ写してから登録する
 * 3. 「これを作れる」と申告してくるもの (IPrefabProvider) を回す
 *
 * **何を作るかは持たない。** ここが作り方を知ってしまうと、枠組みがゲームの中身を抱える。
 * 申告する側が自分の作り方を登録する。
 *
 * シーンを跨いで同じ対応表を使う。シーンごとに変えたい場合は、名前を分けて登録すること。
 *
 * @code
 * Prefabs->AddProvider( MakeUnique<CEnemyPrefabs>() );
 *
 * FPrefabSpawnParams Params;
 * Params.bApplyTransform = true;
 * Params.LocalTransform.position = FVec3{ 3.0f, 0.0f, 0.0f };
 * ANode* const Slime = Prefabs->SpawnAttached( FString( "Enemy/Slime" ), RootNode, Params );
 * @endcode
 */
class CPrefabSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CPrefabSubsystem )

	/**
	 * 作り方を申告するものを受け取り、その場で登録する。
	 *
	 * @details 渡したものの寿命はここが持つ (作り方は後から何度も呼ばれるため)。
	 * @param Provider 申告するもの。
	 * @return 登録できたら true。
	 */
	bool AddProvider( TUniquePtr<IPrefabProvider> Provider ) noexcept;

	/**
	 * 名前から識別子を引く。
	 *
	 * @details 同じものを何度も出すなら、識別子を控えておくほうが速い。
	 * @param Name 登録した名前。
	 * @return 見つかった識別子 (無ければ無効な識別子)。
	 */
	FPrefabId FindId( const FString& Name ) const noexcept;

	/**
	 * 名前で出して、親へ付ける。
	 *
	 * @param Name 登録した名前。
	 * @param Parent 付ける先。
	 * @param Params 置き方。
	 * @return 出したもの (出せなければ nullptr)。所有は親にある。
	 */
	ANode* SpawnAttached( const FString& Name, ANode& Parent, const FPrefabSpawnParams& Params = FPrefabSpawnParams() ) noexcept;

	/**
	 * 識別子で出して、親へ付ける。
	 *
	 * @param Id 控えておいた識別子。
	 * @param Parent 付ける先。
	 * @param Params 置き方。
	 * @return 出したもの (出せなければ nullptr)。所有は親にある。
	 */
	ANode* SpawnAttached( FPrefabId Id, ANode& Parent, const FPrefabSpawnParams& Params = FPrefabSpawnParams() ) noexcept;

	/**
	 * 名前で出して、所有ごと受け取る。
	 *
	 * @param Name 登録した名前。
	 * @param Params 置き方。
	 * @return 出したもの (出せなければ空)。
	 */
	TObjectPtr<ANode> SpawnDetached( const FString& Name, const FPrefabSpawnParams& Params = FPrefabSpawnParams() ) noexcept;

	/** 登録されている作り方の数を返す。 */
	usize GetRegisteredCount() const noexcept { return m_RegisteredCount; }

	/** 申告するものを受け取った数を返す。 */
	usize GetProviderCount() const noexcept { return m_Providers.Num(); }

	/** これまでに出した数を返す。 */
	u64 GetSpawnedCount() const noexcept { return m_SpawnedCount; }

private:
	/** 対応表の本体。 */
	CPrefabSystem m_Prefabs;

	/** 対応表へ渡した名前の実体。 */
	CInternedNamePool m_Names;

	/** 申告するもの。寿命をここで持つ。 */
	TArray<TUniquePtr<IPrefabProvider>> m_Providers;

	/** 登録された作り方の数。 */
	usize m_RegisteredCount = 0u;

	/** これまでに出した数。 */
	u64 m_SpawnedCount = 0u;
};
