// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElement.h"
#include "Debug/DebugTop/Element/DebugTopElementNumber.h"
#include "Debug/DebugTop/Render/DebugTopColorField.h"

using namespace acs;

// 色の行。R/G/B/A を子行に持ち、見本を押すと色を選ぶパネルが出る。

/**
 * 色 (RGBA) を編集する行。
 *
 * @details
 * 展開すると R / G / B / A が個別のスライダーになり、閉じていても右カラムの色見本で今の色が
 * 分かる。値は子行が持つので、保存も 4 本の数値としてそのまま乗る。
 */
class CDebugTopElementColor : public CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @param Label 左カラムへ出す表示名。
	 * @param Value 初期の色 (各成分 0..1)。
	 */
	CDebugTopElementColor( const FString& Label, const FVec4& Value );

	/** 現在の色を返す。 */
	FVec4 GetValue() const noexcept;

	/**
	 * 色を設定する (各成分は 0..1 へ丸める)。
	 *
	 * @param Value 設定する色。
	 */
	void SetValue( const FVec4& Value );

	/** 右カラムへ現在の色を見本として出す。 */
	bool TryGetColorSwatch( FVec4& OutColor ) const noexcept override;

	/** 右カラムの文字は色見本に譲る。 */
	FString GetValueText() const override { return FString(); }

	/** 色の行であることを呼び出し側へ伝える。 */
	CDebugTopElementColor* AsColor() noexcept override { return this; }

	/** 色を選ぶ面を出しているかを返す。 */
	bool IsPickerOpen() const noexcept { return m_bPickerOpen; }

	/**
	 * 色を選ぶ面の出し入れを設定する。
	 *
	 * @details
	 * 面は行としてではなく、色見本の近くへ浮かせて出す。常に置いておくと縦に場所を取り、
	 * 段差の中に大きな四角が挟まって読みにくい。
	 * @param bOpen 出すなら true。
	 */
	void SetPickerOpen( bool bOpen ) noexcept { m_bPickerOpen = bOpen; }

	/** いま選んでいる色相 (0..1) を返す。 */
	f32 GetHue() const noexcept { return m_Hue; }

	/**
	 * 面の中で押された位置から色を決める。
	 *
	 * @details 上側が彩度 (横) と明度 (縦) の面、下側が色相の帯。
	 * @param LocalX 面の左端からの距離 (ピクセル)。
	 * @param LocalY 面の上端からの距離 (ピクセル)。
	 * @param Width 面の幅 (ピクセル)。
	 * @param Height 面の高さ (ピクセル)。
	 */
	void PickAt( f32 LocalX, f32 LocalY, f32 Width, f32 Height );

	/**
	 * いま選んでいる色相・彩度・明度を返す。
	 *
	 * @param OutHue 色相の書き込み先。
	 * @param OutSaturation 彩度の書き込み先。
	 * @param OutValue 明度の書き込み先。
	 */
	void GetPickerState( f32& OutHue, f32& OutSaturation, f32& OutValue ) const noexcept;

private:
	/** R / G / B / A を持つ子行。所有はこの行 (子行配列) が持つ。 */
	CDebugTopElementFloat* m_Components[4] {};

	/**
	 * いま選んでいる色相 (0..1)。
	 *
	 * @details 明度や彩度が 0 になると RGB から色相を復元できないため、こちらで覚えておく。
	 */
	f32 m_Hue = 0.0f;

	/** 色を選ぶ面を出しているか。 */
	bool m_bPickerOpen = false;
};
