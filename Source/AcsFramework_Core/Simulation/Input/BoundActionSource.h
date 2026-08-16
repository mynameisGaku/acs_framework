// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/IActionInputSource.h"
#include "AcsFramework_Core/Simulation/Input/ActionBindingTable.h"
#include "AcsFramework_Core/Simulation/Input/DeviceActionReader.h"

using namespace acs;

/**
 * 割り当て表を通して、人の操作を入力へ変える入力元。
 *
 * @details
 * 人が操るときの既定の入力元。AI に差し替えるときは、これを別の
 * `IActionInputSource` に置き換えるだけでよい。**規則は 1 文字も変わらない。**
 *
 * 読む相手は差し替えられる。何も渡さなければ実機の装置 (`acs::CInput`) を読む。
 *
 * @code
 * TUniquePtr<CBoundActionSource> Source = MakeUnique<CBoundActionSource>();
 * Source->GetTable().BindAxisKeys( 0u, EKey::A, EKey::D );
 * Source->GetTable().BindKey( kActionFire, EKey::Space );
 * Simulation->SetInputSource( Move( Source ) );
 * @endcode
 */
class CBoundActionSource final : public IActionInputSource
{
public:
	/** 実機の装置を読む形で構築する。 */
	CBoundActionSource() noexcept = default;

	/**
	 * 読む相手を指定して構築する。
	 *
	 * @param Reader 読む相手。この入力元より長く生きること。
	 */
	explicit CBoundActionSource( const IActionDeviceReader& Reader ) noexcept
		: m_Reader( &Reader )
	{
	}

	/** 割り当て表を返す。ここへ Bind* して組み立てる。 */
	CActionBindingTable& GetTable() noexcept { return m_Table; }

	/** 割り当て表を const で返す。 */
	const CActionBindingTable& GetTable() const noexcept { return m_Table; }

	/**
	 * 読む相手を差し替える。
	 *
	 * @param Reader 読む相手。nullptr を渡すと実機の装置へ戻る。
	 */
	void SetReader( const IActionDeviceReader* Reader ) noexcept { m_Reader = Reader; }

	/** 装置を読んで、このティックの入力を作る。 */
	bool TryGetActionInput( FActionInput& OutInput ) noexcept override;

private:
	/** 割り当て表。 */
	CActionBindingTable m_Table;

	/** 読む相手 (nullptr なら実機)。所有はしない。 */
	const IActionDeviceReader* m_Reader = nullptr;

	/** 実機を読むときに使う実装。 */
	CDeviceActionReader m_DeviceReader;
};
