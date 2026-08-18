// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 保存する値の種類。
 */
enum class EDebugTopSettingKind : u8
{
	/** i32 (Int 行と Enum 行の選択位置)。 */
	Int,

	/** f32。 */
	Float,

	/** bool。 */
	Bool,

	/** 文字列。 */
	String,
};


/**
 * 設定 1 件分。
 *
 * @details
 * キーと値だけの入れ物で、デバッグメニューのことは知らない。保存形式の各層はこの型の
 * 配列だけを相手にする。
 */
struct FDebugTopSetting
{
	/** 値を引くキー。 */
	FString Key;

	/** 値の種類。 */
	EDebugTopSettingKind Kind = EDebugTopSettingKind::Int;

	/** Kind が Int のときの値。 */
	i32 IntValue = 0;

	/** Kind が Float のときの値。 */
	f32 FloatValue = 0.0f;

	/** Kind が Bool のときの値。 */
	bool bBoolValue = false;

	/** Kind が String のときの値。 */
	FString StringValue;
};
