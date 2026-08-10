// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 非所有の読み込み要求を識別する値。無効値は読み込みを指し示さない。 */
struct FAssetLoadRequest
{
public:
	/** 無効な読み込み要求を構築する。 */
	constexpr FAssetLoadRequest() noexcept = default;

	/** 発行元識別子と世代が有効な読み込み要求かを返す。 */
	constexpr bool IsValid() const noexcept { return m_OwnerId != 0u && m_Generation != 0u; }

	/** 2つの読み込み要求が同じ発行元と世代を指すかを返す。 */
	constexpr bool operator==( const FAssetLoadRequest& Other ) const noexcept
	{
		return m_OwnerId == Other.m_OwnerId && m_Generation == Other.m_Generation;
	}

	/** 2つの読み込み要求が異なる発行元または世代を持つかを返す。 */
	constexpr bool operator!=( const FAssetLoadRequest& Other ) const noexcept { return !( *this == Other ); }

private:
	/** 同じ読み込み窓口を識別する非所有のプロセス全体の値。0は無効値と枯渇状態に予約する。 */
	u64 m_OwnerId = 0u;

	/** 読み込み窓口内で要求を識別する世代。0は無効値と枯渇状態に予約する。 */
	u64 m_Generation = 0u;

	/** 読み込み窓口だけが有効な要求を発行する。 */
	friend class CAssetLoaderSubsystem;

	/** 発行済みの発行元識別子と世代から要求を構築する。0を含む値は有効な要求にならない。 */
	constexpr FAssetLoadRequest( u64 OwnerId, u64 Generation ) noexcept
		: m_OwnerId( OwnerId ), m_Generation( Generation ) {}
};
