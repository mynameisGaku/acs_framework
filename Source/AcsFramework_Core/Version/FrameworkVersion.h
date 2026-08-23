// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * ACS Frameworkの公開版を比較できる値。
 *
 * @details 数値部が同じ場合、正式版は同じ数値部の開発版より新しいものとして扱う。
 */
struct FFrameworkVersion
{
	/** 互換でない公開API変更で上げる主版。 */
	u32 Major = 0u;

	/** 後方互換な機能追加、またはv1.0.0前の契約変更で上げる副版。 */
	u32 Minor = 0u;

	/** 公開契約を保つ修正で上げる修正版。 */
	u32 Patch = 0u;

	/** 正式公開前の版ならtrue。 */
	bool bPreRelease = false;

	/**
	 * 指定版以上かを数値部と開発版・正式版の順で返す。
	 *
	 * @details 同じ数値部を持つ`dev`と`rc`など、開発版識別子どうしの順序は区別しない。
	 * 厳密な識別が必要な場合は`kAcsFrameworkVersionText`を使う。
	 * @param Required 利用側が必要とする最小版。
	 * @return 数値部が新しいか、同じ数値部で公開段階も満たす場合true。
	 */
	constexpr bool IsAtLeast( const FFrameworkVersion& Required ) const noexcept
	{
		if ( Major != Required.Major ) return Major > Required.Major;
		if ( Minor != Required.Minor ) return Minor > Required.Minor;
		if ( Patch != Required.Patch ) return Patch > Required.Patch;
		return bPreRelease == Required.bPreRelease || ( !bPreRelease && Required.bPreRelease );
	}

	/**
	 * v1.0.0以降の正式なAPI安定版か返す。
	 *
	 * @return 主版が1以上で、開発版でなければtrue。
	 */
	constexpr bool HasStableApi() const noexcept { return Major >= 1u && !bPreRelease; }

	/**
	 * 同じ数値部と公開段階を持つか比較する。
	 *
	 * @param Other 比較する版。
	 * @return 4つの値が全て同じならtrue。
	 */
	constexpr bool operator==( const FFrameworkVersion& Other ) const noexcept = default;
};

/** 現在のFramework版。rootのVERSIONと同じ値を保つ。 */
inline constexpr FFrameworkVersion kAcsFrameworkVersion{ 0u, 5u, 0u, true };

/** 現在版をSemantic Versioning文字列で表した値。 */
inline constexpr char kAcsFrameworkVersionText[] = "0.5.0-dev";
