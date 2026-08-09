// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

class CEventSubsystem;

/**
 * 購読を所有し、破棄時に対応する購読を外す値。
 *
 * 空の値は購読を持たず、コピーはできない。購読を引き渡すと元の値は空になる。
 */
class FEventSubscription
{
public:
	/** 購読を持たない値を作る。 */
	FEventSubscription() noexcept = default;

	/** 保持中の購読を外して破棄する。 */
	~FEventSubscription() noexcept { Reset(); }

	/** 二重解除を防ぐため、コピーを禁止する。 */
	FEventSubscription( const FEventSubscription& ) = delete;

	/** 二重解除を防ぐため、コピー代入を禁止する。 */
	FEventSubscription& operator=( const FEventSubscription& ) = delete;

	/**
	 * 購読を引き取り、引き取り元を空にする。
	 * @param Other 引き取り元の値。
	 */
	FEventSubscription( FEventSubscription&& Other ) noexcept : m_Owner( Other.m_Owner ), m_Handle( Other.m_Handle )
	{
		Other.m_Owner = nullptr;
		Other.m_Handle = FSubscriptionHandle{};
	}

	/**
	 * 自分の購読を外してから、Other の購読を引き取る。
	 * @param Other 引き取り元の値。
	 * @return 自分自身。
	 */
	FEventSubscription& operator=( FEventSubscription&& Other ) noexcept;

	/** 保持中の購読を外し、空の値にする。 */
	void Reset() noexcept;

	/** 有効な購読を保持しているかを返す。 */
	bool IsValid() const noexcept { return m_Owner != nullptr && m_Handle.IsValid(); }

private:
	friend class CEventSubsystem;

	/**
	 * 購読の解除先とエンジン側の購読識別子を記録する。
	 * @param Owner 購読解除を行うサブシステム。
	 * @param Handle エンジン側の購読識別子。
	 */
	FEventSubscription( CEventSubsystem& Owner, FSubscriptionHandle Handle ) noexcept : m_Owner( &Owner ), m_Handle( Handle ) {}

	/** 購読解除を行うFramework subsystem。所有はせず、GameInstance寿命が値より長い前提で参照する。 */
	CEventSubsystem* m_Owner = nullptr;

	/** Engine側の購読識別子。 */
	FSubscriptionHandle m_Handle{};
};
