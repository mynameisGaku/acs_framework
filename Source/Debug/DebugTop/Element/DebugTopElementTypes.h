#pragma once

#include <acs.h>

using namespace acs;

// 行が使う小さな型 (種別・見せ方・色・デリゲート)。
// 行そのもの (CDebugTopElement) と分けてあるのは、これらを使うだけの側が
// 基底クラス全体を取り込まずに済むようにするため。

// 行はページを指すことがあるが、中身までは要らない (実体は Page/ 側にある)。
class ADebugTopEntity;

/**
 * 表示文字列をその場で作り直すためのデリゲート。
 *
 * @details
 * 毎フレーム呼ばれるので、重い処理を書かないこと。バインド先の寿命は行より長く保つこと。
 */
using FDebugTopTextDelegate = TDelegate<FString()>;

/**
 * 展開マーカーを出すかどうかの方針。
 *
 * @details
 * マーカーの形は描画側が決める。決定キーやクリックで実際に開閉できる行 (開閉が許可されていて
 * 子行を持つ行) だけが三角形になり、それ以外は「押しても何も起きない」ことを示す薄い横棒になる。
 */
enum class EDebugTopMarkerVisibility : u8
{
	/** 子行を持つときだけ出す (既定)。 */
	Auto,

	/** 子行の有無にかかわらず必ず出す。 */
	Always,

	/** 子行を持っていても出さない。 */
	Never,
};

/**
 * 行が持つ値の種類。
 *
 * @details
 * RTTI を切っているため dynamic_cast が使えない。カーソル行から具体型へ落とすときは
 * この種別で判別してから static_cast する。
 */
enum class EDebugTopValueKind : u8
{
	/** 値を持たない (カテゴリ行・Action 行・遷移行)。 */
	None,

	/** i32 の値を 1 つ持つ。 */
	Int,

	/** f32 の値を 1 つ持つ。 */
	Float,

	/** bool の値を 1 つ持つ。 */
	Bool,

	/** 選択肢から 1 つ選ぶ (TryGetInt で選択位置が取れる)。 */
	Enum,

	/** 文字列を 1 つ持つ (GetValueText で取れる)。 */
	String,

	/** i32 の配列を持つ (CDebugTopElementIntArray へ static_cast できる)。 */
	IntArray,

	/** f32 の配列を持つ (CDebugTopElementFloatArray へ static_cast できる)。 */
	FloatArray,
};

/**
 * 明示指定できる色。
 *
 * @details bSet が false の間は描画側が状況 (選択中かどうか) に応じた既定色を使う。
 * 行ごとに色を変えたいときだけ設定する。
 */
struct FDebugTopColor
{
	/** 指定された色。bSet が false の間は使われない。 */
	FVec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

	/** 色が明示指定されているか。 */
	bool bSet = false;
};
