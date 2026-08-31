// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/GameplayResourceState.h"

using namespace acs;

/**
 * 上限、現在値、消費と回復だけを扱う局所的なゲーム資源。
 *
 * @details
 * 体力、スタミナ、魔力、シールドなどを表すゲーム規則のfieldとして持つ。
 * ダメージ計算、死亡、再生条件、UI、時間は所有せず、利用側が決めた量だけを安全に反映する。
 * `HealthSystem`など意味を固定する仕組みではなく、保存復元できる上限付き数値に留める。
 */
class FGameplayResource
{
public:
	/** 上限1、現在値1の満杯状態を構築する。 */
	FGameplayResource() noexcept = default;

	/**
	 * 指定上限と同じ現在値の満杯状態を構築する。
	 *
	 * @param MaximumValue 有限かつ0より大きい上限。
	 * @details 不正値なら既定の1/1を使う。
	 */
	explicit FGameplayResource( f32 MaximumValue ) noexcept;

	/**
	 * 指定上限と現在値で構築する。
	 *
	 * @param MaximumValue 有限かつ0より大きい上限。
	 * @param CurrentValue 有限かつ0以上MaximumValue以下の現在値。
	 * @details 不正な組なら既定の1/1を使う。
	 */
	FGameplayResource( f32 MaximumValue, f32 CurrentValue ) noexcept;

	/**
	 * 上限と現在値を同時に設定する。
	 *
	 * @param MaximumValue 有限かつ0より大きい上限。
	 * @param CurrentValue 有限かつ0以上MaximumValue以下の現在値。
	 * @return 設定できたらtrue。不正値では従来状態を変えずfalse。
	 */
	bool TryConfigure( f32 MaximumValue, f32 CurrentValue ) noexcept;

	/**
	 * 上限を変更し、超えた現在値だけ新しい上限へ収める。
	 *
	 * @param MaximumValue 有限かつ0より大きい新しい上限。
	 * @return 変更できたらtrue。不正値では従来状態を変えずfalse。
	 */
	bool TrySetMaximumValue( f32 MaximumValue ) noexcept;

	/**
	 * 現在値を上限内で直接設定する。
	 *
	 * @param CurrentValue 有限かつ0以上MaximumValue以下の現在値。
	 * @return 設定できたらtrue。不正値では従来状態を変えずfalse。
	 */
	bool TrySetCurrentValue( f32 CurrentValue ) noexcept;

	/**
	 * 指定量を全て消費できる場合だけ現在値から引く。
	 *
	 * @param Amount 有限かつ0以上の消費量。
	 * @return 全量を消費できたらtrue。不正量または不足では状態を変えずfalse。
	 */
	bool TrySpend( f32 Amount ) noexcept;

	/**
	 * 指定量を上限まで回復する。
	 *
	 * @param Amount 有限かつ0以上の回復要求量。
	 * @return 要求を検証して反映できたらtrue。満杯では変更なしでtrue。
	 */
	bool TryRestore( f32 Amount ) noexcept;

	/**
	 * 指定量を上限まで回復し、実際に増えた量も返す。
	 *
	 * @param Amount 有限かつ0以上の回復要求量。
	 * @param OutAppliedAmount 丸めと上限を反映した実増加量。失敗時は変更しない。
	 * @return 要求を検証して反映できたらtrue。満杯では0を返してtrue。
	 */
	bool TryRestore( f32 Amount, f32& OutAppliedAmount ) noexcept;

	/** 現在値を上限と同じ満杯状態にする。 */
	void Fill() noexcept { m_CurrentValue = m_MaximumValue; }

	/** 現在値を0の空状態にする。 */
	void Empty() noexcept { m_CurrentValue = 0.0f; }

	/** 上限と現在値を保存可能な値として返す。 */
	FGameplayResourceState CaptureState() const noexcept;

	/**
	 * 保存した上限と現在値を復元する。
	 *
	 * @param State 有限かつ矛盾のない保存値。
	 * @return 復元できたらtrue。不正な状態では従来状態を変えずfalse。
	 */
	bool RestoreState( const FGameplayResourceState& State ) noexcept;

	/** 現在の上限を返す。 */
	f32 GetMaximumValue() const noexcept { return m_MaximumValue; }

	/** 現在値を返す。 */
	f32 GetCurrentValue() const noexcept { return m_CurrentValue; }

	/** 上限まで不足している量を返す。 */
	f32 GetMissingValue() const noexcept
	{
		return m_MaximumValue - m_CurrentValue;
	}

	/** 現在値を上限で割った0以上1以下の割合を返す。 */
	f32 GetRatio() const noexcept;

	/** 現在値が0ならtrue。 */
	bool IsEmpty() const noexcept { return m_CurrentValue == 0.0f; }

	/** 現在値が上限と同じならtrue。 */
	bool IsFull() const noexcept
	{
		return m_CurrentValue == m_MaximumValue;
	}

private:
	/** 0より大きい資源の上限。 */
	f32 m_MaximumValue = 1.0f;

	/** 0以上m_MaximumValue以下の現在値。 */
	f32 m_CurrentValue = 1.0f;
};
