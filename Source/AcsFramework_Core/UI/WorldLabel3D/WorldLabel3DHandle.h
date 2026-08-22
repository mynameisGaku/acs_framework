// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 3Dワールドラベルを指す場面内の値。
 *
 * @details 0は常に無効。値は`CWorldLabel3DLayer`が単調増加で発行し、削除後も再利用しない。
 */
class FWorldLabel3DHandle
{
public:
	/** 無効なhandleを作る。 */
	constexpr FWorldLabel3DHandle() noexcept = default;

	/**
	 * 発行値からhandleを作る。
	 *
	 * @param Value 0以外の場面内発行値。
	 * @return 指定値を持つhandle。0なら無効。
	 */
	static constexpr FWorldLabel3DHandle FromValue( u32 Value ) noexcept
	{
		return FWorldLabel3DHandle( Value );
	}

	/** 有効な発行値を持つならtrueを返す。 */
	constexpr bool IsValid() const noexcept { return m_Value != 0u; }

	/** 比較や記録に使う発行値を返す。 */
	constexpr u32 Value() const noexcept { return m_Value; }

	/** handleを無効へ戻す。 */
	constexpr void Reset() noexcept { m_Value = 0u; }

	/** 同じ場面内ラベルを指すならtrueを返す。 */
	constexpr bool operator==( const FWorldLabel3DHandle& Other ) const noexcept = default;

private:
	/** レイヤーが発行する値。0は無効。 */
	u32 m_Value = 0u;

	/** 発行値を直接受け取る。 */
	constexpr explicit FWorldLabel3DHandle( u32 Value ) noexcept : m_Value( Value ) {}
};
