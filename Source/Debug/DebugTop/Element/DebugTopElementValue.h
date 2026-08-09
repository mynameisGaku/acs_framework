#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElementTypes.h"

using namespace acs;

class CDebugTopElement;

/**
 * 行 1 つぶんの値を、型を問わず持ち運べる形にしたもの。
 *
 * @details
 * 「いまの値を控えて、後で元へ戻す」ために使う。行そのものは指さないので、控えた後に行が
 * 消えても安全。
 *
 * 保存形式 (Settings/FDebugTopSetting) と形は似ているが別物にしてある。あちらはファイルへ
 * 書く都合を持ち、こちらは行の値そのものを表す。混ぜると、保存形式を増やすたびに行の側が
 * 引きずられる。
 */
struct FDebugTopElementValue
{
	/** 値の種類 (None なら値を持たない行)。 */
	EDebugTopValueKind Kind = EDebugTopValueKind::None;

	/** Kind が Int / Enum のときの値。 */
	i32 IntValue = 0;

	/** Kind が Float のときの値。 */
	f32 FloatValue = 0.0f;

	/** Kind が Bool のときの値。 */
	bool bBoolValue = false;

	/** Kind が String のときの値。 */
	FString StringValue;

	/** 値を持っているかを返す。 */
	bool IsValid() const noexcept { return Kind != EDebugTopValueKind::None; }
};


/**
 * 行の現在値を取り出す。
 *
 * @details
 * どの取り出し口 (TryGetInt / TryGetFloat / ...) を使うかは種類ごとに決まっている。その
 * 対応をここ 1 か所に置き、控える側 (取り消し) と書き出す側 (保存) で食い違わないようにする。
 * @param Element 対象の行。
 * @param OutValue 取り出せた場合に値を書き込む先。
 * @return 取り出せたら true (値を持たない行は false)。
 */
bool DebugTopCaptureValue( const CDebugTopElement& Element, FDebugTopElementValue& OutValue );

/**
 * 控えておいた値を行へ書き戻す。
 *
 * @details
 * 種類が食い違う場合は書き戻さない (行が作り直されて別物になっている可能性があるため)。
 * @param Element 書き戻す先の行。
 * @param Value 書き戻す値。
 * @return 書き戻せたら true。
 */
bool DebugTopApplyValue( CDebugTopElement& Element, const FDebugTopElementValue& Value );

/**
 * 2 つの値が同じかを返す。
 *
 * @details 変化していないものを取り消しの控えへ積まないために使う。
 * @param Left 比べる値。
 * @param Right 比べる値。
 * @return 種類も中身も同じなら true。
 */
bool DebugTopValuesEqual( const FDebugTopElementValue& Left, const FDebugTopElementValue& Right ) noexcept;
