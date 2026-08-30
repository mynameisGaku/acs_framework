// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** アクションの最大数。1 つの u32 へ収める。 */
inline constexpr u32 kActionButtonCount = 32u;

/** 軸の最大数。移動 2 軸 + 視点 2 軸を想定する。 */
inline constexpr u32 kActionAxisCount = 4u;

/**
 * 1 ティックぶんの「やりたいこと」。
 *
 * @details
 * キーやパッドではなく**アクション**で持つ。こうしておくと、同じロジックを人が操っても
 * AI が操っても動かせる。記録して後から流し直すこともできる (装置の状態を記録すると、
 * キー割り当てを変えただけで再生が壊れる)。
 *
 * 何が押されているかだけを持ち、「今フレーム押された」といった差分は持たない。
 * 差分は前ティックまたは前フレームと突き合わせて初めて決まるので、固定ステップでは
 * `FSimulationContext`、通常の場面更新では`CActionInputTracker`が答える。
 */
struct FActionInput
{
	/** 軸の値 (-1..1 を想定するが、範囲は利用側の取り決め)。 */
	f32 Axes[kActionAxisCount] = {};

	/** 押されているアクション (bit ごと)。 */
	u32 Buttons = 0u;

	/**
	 * そのアクションが押されているかを返す。
	 *
	 * @param ActionIndex 0 以上 kActionButtonCount 未満。
	 * @return 押されていれば true。
	 */
	bool IsDown( u32 ActionIndex ) const noexcept
	{
		if ( ActionIndex >= kActionButtonCount ) return false;

		return ( Buttons & ( 1u << ActionIndex ) ) != 0u;
	}

	/**
	 * アクションの押下を設定する。
	 *
	 * @param ActionIndex 0 以上 kActionButtonCount 未満。
	 * @param bDown 押されているなら true。
	 */
	void SetDown( u32 ActionIndex, bool bDown ) noexcept
	{
		if ( ActionIndex >= kActionButtonCount ) return;

		const u32 Mask = 1u << ActionIndex;
		Buttons = bDown ? ( Buttons | Mask ) : ( Buttons & ~Mask );
	}

	/**
	 * 軸の値を返す。
	 *
	 * @param AxisIndex 0 以上 kActionAxisCount 未満。
	 * @return 軸の値 (範囲外なら 0)。
	 */
	f32 GetAxis( u32 AxisIndex ) const noexcept
	{
		if ( AxisIndex >= kActionAxisCount ) return 0.0f;

		return Axes[AxisIndex];
	}

	/**
	 * 軸の値を設定する。
	 *
	 * @param AxisIndex 0 以上 kActionAxisCount 未満。
	 * @param Value 設定する値。
	 */
	void SetAxis( u32 AxisIndex, f32 Value ) noexcept
	{
		if ( AxisIndex >= kActionAxisCount ) return;

		Axes[AxisIndex] = Value;
	}

	/** 何も入力されていないかを返す。 */
	bool IsNeutral() const noexcept
	{
		if ( Buttons != 0u ) return false;

		for ( u32 Index = 0u; Index < kActionAxisCount; ++Index )
		{
			if ( Axes[Index] != 0.0f ) return false;
		}

		return true;
	}

	/**
	 * 同じ入力かを返す。
	 *
	 * @details 記録の間引き (前と同じなら書かない) に使う。
	 * @param Other 比べる相手。
	 * @return 完全に一致すれば true。
	 */
	bool Equals( const FActionInput& Other ) const noexcept
	{
		if ( Buttons != Other.Buttons ) return false;

		for ( u32 Index = 0u; Index < kActionAxisCount; ++Index )
		{
			if ( Axes[Index] != Other.Axes[Index] ) return false;
		}

		return true;
	}
};
