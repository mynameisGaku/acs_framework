// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 再生中の 3D エフェクトを指す値。
 *
 * @details 0 は常に無効。描画ライブラリ固有の番号を公開しないため、backend を作り直しても
 * 呼び出し側の型は変わらない。
 */
class FEffect3DHandle
{
public:
	/** 無効な handle を作る。 */
	constexpr FEffect3DHandle() noexcept = default;

	/**
	 * framework が発行した値から handle を作る。
	 *
	 * @param Value 0 以外の発行値。
	 * @return 指定値を持つ handle。0 なら無効。
	 */
	static constexpr FEffect3DHandle FromValue( u32 Value ) noexcept
	{
		return FEffect3DHandle( Value );
	}

	/**
	 * 有効な発行値を持つか返す。
	 *
	 * @return 0 以外なら true。
	 */
	constexpr bool IsValid() const noexcept { return m_Value != 0u; }

	/**
	 * 比較や保存に使う発行値を返す。
	 *
	 * @return 0 は無効、それ以外は player 内で一意な値。
	 */
	constexpr u32 Value() const noexcept { return m_Value; }

	/** handle を無効へ戻す。 */
	constexpr void Reset() noexcept { m_Value = 0u; }

	/**
	 * 2 つの handle が同じ再生を指すか返す。
	 *
	 * @param Other 比べる handle。
	 * @return 発行値が同じなら true。
	 */
	constexpr bool operator==( const FEffect3DHandle& Other ) const noexcept = default;

private:
	/** player が発行する値。0 は無効。 */
	u32 m_Value = 0u;

	/** 発行値を直接受け取る。 */
	constexpr explicit FEffect3DHandle( u32 Value ) noexcept : m_Value( Value ) {}
};
