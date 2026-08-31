// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"
#include "AcsFramework_Core/Simulation/Input/ActionDirectionTrackerState.h"

using namespace acs;

class CActionInputTracker;

/**
 * 2軸入力の離散方向を保持し、今回だけの開始・方向変更・解除を返す局所状態。
 *
 * @details
 * メニュー、格子移動、キャラクターの向き、方向別アニメーションなどに使う。入力装置や時計を
 * 所有せず、通常フレーム、固定ステップ、AI、再生入力から同じ方向遷移を作る。
 */
class FActionDirectionTracker
{
public:
	/** 既定の8方向量子化設定とNone方向で構築する。 */
	FActionDirectionTracker() noexcept = default;

	/**
	 * 指定した量子化設定とNone方向で構築する。
	 *
	 * @details 不正設定なら既定設定を使う。
	 * @param Quantizer 2軸の開始・解除閾値と4/8方向設定。
	 */
	explicit FActionDirectionTracker(
		const FActionDirectionQuantizer& Quantizer ) noexcept;

	/**
	 * 今後の更新に使う量子化設定を変更する。
	 *
	 * @param Quantizer 有限で通常の最大入力を利用できる設定。
	 * @return 反映できたらtrue。不正設定では設定と方向状態を変えずfalse。
	 */
	bool Configure( const FActionDirectionQuantizer& Quantizer ) noexcept;

	/** 現在使っている量子化設定を返す。 */
	const FActionDirectionQuantizer& GetQuantizer() const noexcept
	{
		return m_Quantizer;
	}

	/**
	 * 明示した2軸入力から方向状態を1回進める。
	 *
	 * @param Axes X正を右、Y正を上とする有限な入力。
	 * @return 更新できたらtrue。不正な設定または入力では全状態を変えずfalse。
	 */
	bool Update( FVec2 Axes ) noexcept;

	/**
	 * 通常フレームの入力から指定した2軸を読み、方向状態を1回進める。
	 *
	 * @param Input 現在入力を保持する通常フレーム用トラッカー。
	 * @param XAxisIndex 右を正とする範囲内の軸番号。
	 * @param YAxisIndex 上を正とする、Xとは異なる範囲内の軸番号。
	 * @return 更新できたらtrue。不正な設定、軸番号、入力では全状態を変えずfalse。
	 */
	bool Update( const CActionInputTracker& Input,
		u32 XAxisIndex, u32 YAxisIndex ) noexcept;

	/**
	 * 固定ステップ、AI、再生入力から指定した2軸を読み、方向状態を1回進める。
	 *
	 * @param Input 変換元の汎用アクション入力。
	 * @param XAxisIndex 右を正とする範囲内の軸番号。
	 * @param YAxisIndex 上を正とする、Xとは異なる範囲内の軸番号。
	 * @return 更新できたらtrue。不正な設定、軸番号、入力では全状態を変えずfalse。
	 */
	bool Update( const FActionInput& Input,
		u32 XAxisIndex, u32 YAxisIndex ) noexcept;

	/**
	 * 現在方向を明示値へ変更し、今回だけの変化を空にする。
	 *
	 * @param Direction Noneまたは8方向の既知値。
	 * @return 反映できたらtrue。未知値では全状態を変えずfalse。
	 */
	bool SetDirection( EActionDirection2D Direction ) noexcept;

	/** 現在と前回の方向をNoneへ戻す。量子化設定は維持する。 */
	void Reset() noexcept;

	/** 量子化設定と現在・前回方向を保存可能な値として返す。 */
	FActionDirectionTrackerState CaptureState() const noexcept;

	/**
	 * 保存した方向追跡状態を復元する。
	 *
	 * @param State `CaptureState`で取得した矛盾のない状態。
	 * @return 復元できたらtrue。不正状態では現在値を一切変えずfalse。
	 */
	bool RestoreState( const FActionDirectionTrackerState& State ) noexcept;

	/** 現在の離散方向を返す。 */
	EActionDirection2D GetDirection() const noexcept { return m_Direction; }

	/** 最後に成功した更新より前の方向を返す。 */
	EActionDirection2D GetPreviousDirection() const noexcept
	{
		return m_PreviousDirection;
	}

	/** 現在方向がNoneでなければtrue。 */
	bool IsActive() const noexcept
	{
		return m_Direction != EActionDirection2D::None;
	}

	/** 今回の成功した更新で方向が変わったならtrue。開始と解除も含む。 */
	bool WasChanged() const noexcept
	{
		return m_Direction != m_PreviousDirection;
	}

	/** 今回の成功した更新でNoneから方向入力を始めたならtrue。 */
	bool WasStarted() const noexcept
	{
		return m_PreviousDirection == EActionDirection2D::None
			&& m_Direction != EActionDirection2D::None;
	}

	/** 今回の成功した更新で方向入力をNoneへ戻したならtrue。 */
	bool WasReleased() const noexcept
	{
		return m_PreviousDirection != EActionDirection2D::None
			&& m_Direction == EActionDirection2D::None;
	}

private:
	/** 2軸入力へ適用する開始・解除閾値と4/8方向設定。 */
	FActionDirectionQuantizer m_Quantizer;

	/** 最後に成功した更新で決まった現在方向。 */
	EActionDirection2D m_Direction = EActionDirection2D::None;

	/** 最後に成功した更新より前の方向。 */
	EActionDirection2D m_PreviousDirection = EActionDirection2D::None;
};
