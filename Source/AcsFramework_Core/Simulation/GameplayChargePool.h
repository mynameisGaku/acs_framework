// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/GameplayChargePoolState.h"

using namespace acs;

/**
 * 上限付きの整数チャージを、明示したゲーム時間だけで自動回復する局所状態。
 *
 * @details
 * 回数制の能力、回避、道具、弾薬などを表すゲーム規則のfieldとして持つ。
 * callback、入力、場面、時計と利用目的は所有せず、消費と回復時に行う処理も利用側へ残す。
 * 固定ステップで決定論的に進め、途中状態を保存復元できる。
 */
class FGameplayChargePool
{
public:
	/** 呼出側が上限を省略したとき、1更新で回復する最大チャージ数。 */
	static constexpr u32 kDefaultMaximumCatchUpCount = 64u;

	/** 上限1、現在数1、1秒回復の満杯状態を構築する。 */
	FGameplayChargePool() noexcept = default;

	/**
	 * 指定上限と回復秒で満杯状態を構築する。
	 *
	 * @param MaximumCharges 1以上のチャージ上限。
	 * @param RechargeSeconds 1チャージを回復する有限かつ0より大きい秒数。
	 * @details 不正な組なら既定の1個、1秒を使う。
	 */
	FGameplayChargePool( u32 MaximumCharges, f32 RechargeSeconds ) noexcept;

	/**
	 * 指定上限、現在数、回復秒で構築する。
	 *
	 * @param MaximumCharges 1以上のチャージ上限。
	 * @param CurrentCharges 0以上MaximumCharges以下の現在数。
	 * @param RechargeSeconds 1チャージを回復する有限かつ0より大きい秒数。
	 * @details 不正な組なら既定の1個、1秒の満杯状態を使う。
	 */
	FGameplayChargePool( u32 MaximumCharges, u32 CurrentCharges,
		f32 RechargeSeconds ) noexcept;

	/**
	 * 上限、現在数、回復秒を同時に設定し、回復進行を0秒から始め直す。
	 *
	 * @return 設定できたらtrue。不正値では従来状態を一切変えずfalse。
	 */
	bool TryConfigure( u32 MaximumCharges, u32 CurrentCharges,
		f32 RechargeSeconds ) noexcept;

	/**
	 * 上限を変更し、超えた現在数だけ新しい上限へ収める。
	 *
	 * @details 満杯から上限を増やすと、現在設定の0秒地点から不足分の回復を始める。
	 * @param MaximumCharges 1以上の新しい上限。
	 * @return 変更できたらtrue。0では従来状態を変えずfalse。
	 */
	bool TrySetMaximumCharges( u32 MaximumCharges ) noexcept;

	/**
	 * 今後の回復開始時に使う1チャージ当たりの秒数を変更する。
	 *
	 * @details 現在進行中の1回は開始時の秒数を保ち、次の回復から新設定を使う。
	 * @return 変更できたらtrue。不正値では従来状態を一切変えずfalse。
	 */
	bool TrySetRechargeSeconds( f32 RechargeSeconds ) noexcept;

	/**
	 * 指定数を全て消費できる場合だけ現在数から引く。
	 *
	 * @details 満杯から1個以上消費した場合は現在設定の0秒地点から回復を始める。
	 * 既に回復中または一時停止中なら、その進行状態を保つ。0個は変更なしで受理する。
	 * @param ChargeCount 消費するチャージ数。
	 * @return 全数を消費できたらtrue。不足では状態を変えずfalse。
	 */
	bool TryConsume( u32 ChargeCount = 1u ) noexcept;

	/**
	 * 指定数を上限まで手動回復し、実際に増えた数を返す。
	 *
	 * @details 満杯になれば時間持越しと一時停止を消す。満杯では0を返す。
	 */
	u32 RestoreCharges( u32 ChargeCount ) noexcept;

	/**
	 * 自動回復を明示したゲーム時間だけ進め、今回回復した数を返す。
	 *
	 * @details 上限を超えた回復回数ぶんの時間は捨てず、次回以降へ持ち越す。
	 * 満杯、一時停止中、または`DeltaSeconds`が0でも有効な更新として扱う。
	 * @param DeltaSeconds 前回更新から進んだ有限かつ0以上の秒数。
	 * @param OutRestoredChargeCount 今回回復した数。失敗時は変更しない。
	 * @param MaximumCatchUpCount 1更新で回復する1以上の最大数。
	 * @return 入力を受理できたらtrue。不正入力や時間加算overflowでは全状態と出力を変えずfalse。
	 */
	bool Update( f32 DeltaSeconds, u32& OutRestoredChargeCount,
		u32 MaximumCatchUpCount = kDefaultMaximumCatchUpCount ) noexcept;

	/**
	 * 不足中の自動回復を一時停止する。
	 *
	 * @return 停止できたらtrue。満杯または停止済みでは状態を変えずfalse。
	 */
	bool Pause() noexcept;

	/**
	 * 一時停止した自動回復を再開する。
	 *
	 * @return 再開できたらtrue。満杯または実行中なら状態を変えずfalse。
	 */
	bool Resume() noexcept;

	/** 現在数を上限へ満たし、時間持越しと一時停止を消す。 */
	void Fill() noexcept;

	/** 現在数を0にし、現在設定の0秒地点から自動回復を始める。 */
	void Empty() noexcept;

	/** 上限、現在数、秒数、持越しと停止状態を保存可能な値として返す。 */
	FGameplayChargePoolState CaptureState() const noexcept;

	/**
	 * 保存したチャージ状態を復元する。
	 *
	 * @return 復元できたらtrue。不正な状態では現在値を一切変えずfalse。
	 */
	bool RestoreState( const FGameplayChargePoolState& State ) noexcept;

	/** 1以上の現在上限を返す。 */
	u32 GetMaximumCharges() const noexcept { return m_MaximumCharges; }

	/** 現在利用できるチャージ数を返す。 */
	u32 GetCurrentCharges() const noexcept { return m_CurrentCharges; }

	/** 上限まで不足しているチャージ数を返す。 */
	u32 GetMissingCharges() const noexcept
	{
		return m_MaximumCharges - m_CurrentCharges;
	}

	/** 今後の回復開始時に使う1チャージ当たりの秒数を返す。 */
	f32 GetRechargeSeconds() const noexcept { return m_RechargeSeconds; }

	/** 現在進行中の1回に固定した秒数を返す。満杯なら今後の設定値。 */
	f32 GetActiveRechargeSeconds() const noexcept
	{
		return m_ActiveRechargeSeconds;
	}

	/** 次の1チャージまで持ち越している秒数を返す。未処理の複数回ぶんを含む。 */
	f64 GetAccumulatedSeconds() const noexcept { return m_AccumulatedSeconds; }

	/** 次の1チャージまでの秒数を返す。満杯または未処理ぶんが在れば0。 */
	f32 GetSecondsUntilNextCharge() const noexcept;

	/** 次の1チャージへ進んだ割合を0から1で返す。満杯または未処理ぶんが在れば1。 */
	f32 GetRechargeProgress() const noexcept;

	/** 指定数を現在消費できるならtrue。0は常にtrue。 */
	bool CanConsume( u32 ChargeCount = 1u ) const noexcept
	{
		return ChargeCount <= m_CurrentCharges;
	}

	/** 現在数が0ならtrue。 */
	bool IsEmpty() const noexcept { return m_CurrentCharges == 0u; }

	/** 現在数が上限と同じならtrue。 */
	bool IsFull() const noexcept
	{
		return m_CurrentCharges == m_MaximumCharges;
	}

	/** 不足中かつ明示時間で自動回復が進むならtrue。 */
	bool IsRecharging() const noexcept { return !IsFull() && !m_bIsPaused; }

	/** 不足中の自動回復を一時停止しているならtrue。 */
	bool IsPaused() const noexcept { return !IsFull() && m_bIsPaused; }

private:
	/** 満杯状態へ正規化し、時間持越しと一時停止を消す。 */
	void NormalizeFull_Internal() noexcept;

	/** 1以上のチャージ上限。 */
	u32 m_MaximumCharges = 1u;

	/** 0以上m_MaximumCharges以下の現在チャージ数。 */
	u32 m_CurrentCharges = 1u;

	/** 今後の回復開始時に使う1チャージ当たりの秒数。 */
	f32 m_RechargeSeconds = 1.0f;

	/** 現在進行中の1回を始めた時点で固定した秒数。 */
	f32 m_ActiveRechargeSeconds = 1.0f;

	/** 次の回復判定へ持ち越している秒数。未処理の複数回ぶんを含む。 */
	f64 m_AccumulatedSeconds = 0.0;

	/** 不足中の自動回復を一時停止しているならtrue。 */
	bool m_bIsPaused = false;
};
