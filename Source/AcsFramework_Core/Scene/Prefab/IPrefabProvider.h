// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

class CPrefabRegistrar;

/**
 * 「この名前でこれを作れる」と申告する差込口。
 *
 * @details
 * 何をどう組み立てるかはゲーム側にしか分からない。枠組みは名前と作り方の対応表を預かるだけに
 * して、中身は申告した側が持つ。
 *
 * 実装はサブシステムより長く生きること (作り方は後から何度も呼ばれる)。
 * CPrefabSubsystem::AddProvider へ渡せば寿命はそちらが持つ。
 *
 * @code
 * class CEnemyPrefabs : public IPrefabProvider
 * {
 *     void ProvidePrefabs( CPrefabRegistrar& Registrar ) noexcept override
 *     {
 *         Registrar.Add( FString( "Enemy/Slime" ), &CEnemyPrefabs::MakeSlime, this );
 *     }
 * };
 * @endcode
 */
class IPrefabProvider
{
public:
	/** 派生を正しく破棄するための仮想デストラクタ。 */
	virtual ~IPrefabProvider() noexcept = default;

	/**
	 * 自分が作れるものを登録する。
	 *
	 * @details 起動時に 1 度だけ呼ばれる。
	 * @param Registrar 登録の窓口。
	 */
	virtual void ProvidePrefabs( CPrefabRegistrar& Registrar ) noexcept = 0;
};
