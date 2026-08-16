// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * ロジックが「起きた」と言い切ったこと 1 件。
 *
 * @details
 * ロジック側が音を鳴らしたり絵を出したりし始めると、その瞬間からロジックは
 * ゲームを起動しないと試せなくなる。**何が起きたかだけ**を残し、実際に鳴らす・出すのは
 * 受け取った側の仕事にする。
 *
 * 種類ごとにクラスを作らない。id はゲーム側が決める列挙をそのまま入れる。
 * 数値 3 つと対象 1 つで足りない情報は、id から引ける形でゲーム側が持つこと
 * (ここへ可変長の中身を持たせると、記録も比較も一気に難しくなる)。
 */
struct FSimulationEvent
{
	/** 何が起きたか。値の意味はゲーム側が決める。 */
	u32 Id = 0u;

	/** 起きたティック。 */
	u32 Tick = 0u;

	/** 誰に起きたか。値の意味はゲーム側が決める (0 は「特定の相手なし」)。 */
	u32 Target = 0u;

	/** 付随する数値。意味はゲーム側が決める。 */
	f32 ValueA = 0.0f;

	/** 付随する数値。 */
	f32 ValueB = 0.0f;

	/** 付随する数値。 */
	f32 ValueC = 0.0f;

	/**
	 * 同じ内容かを返す。
	 *
	 * @details 再生した結果が元と一致するかを比べるために使う。
	 * @param Other 比べる相手。
	 * @return 完全に一致すれば true。
	 */
	bool Equals( const FSimulationEvent& Other ) const noexcept
	{
		return Id == Other.Id
			&& Tick == Other.Tick
			&& Target == Other.Target
			&& ValueA == Other.ValueA
			&& ValueB == Other.ValueB
			&& ValueC == Other.ValueC;
	}
};
