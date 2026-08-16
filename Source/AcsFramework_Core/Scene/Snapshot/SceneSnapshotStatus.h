// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 書き出し・読み込みの結果を読み解く係。
 *
 * @details
 * エンジンが返すのは列挙値だけで、それが「やり直せば通るのか」「データが壊れているのか」
 * までは呼ぶ側が判断することになっている。判断の基準が呼び出し箇所ごとにばらけると、
 * 入れ物が小さいだけなのに壊れた扱いにする、といった食い違いが出る。
 *
 * 値を見るだけで、書き出しも読み込みもしない。
 */
class CSceneSnapshotStatus
{
public:
	/**
	 * うまくいったかを返す。
	 *
	 * @param Error 結果の値。
	 * @return 問題がなければ true。
	 */
	static bool IsSuccess( ESceneSerializeError Error ) noexcept { return Error == ESceneSerializeError::None; }

	/**
	 * 入れ物を大きくすればやり直せるかを返す。
	 *
	 * @param Error 結果の値。
	 * @return やり直す価値があれば true。
	 */
	static bool IsBufferTooSmall( ESceneSerializeError Error ) noexcept { return Error == ESceneSerializeError::BufferTooSmall; }

	/**
	 * データそのものが壊れているかを返す。
	 *
	 * @details やり直しても同じ結果になるので、呼ぶ側は諦める判断ができる。
	 * @param Error 結果の値。
	 * @return 壊れていれば true。
	 */
	static bool IsCorruptData( ESceneSerializeError Error ) noexcept;

	/**
	 * エンジンが付けている名前を返す。
	 *
	 * @param Error 結果の値。
	 * @return 英字の名前。
	 */
	static const char* GetName( ESceneSerializeError Error ) noexcept { return SceneSerializeErrorName( Error ); }

	/**
	 * 何が起きたかを日本語で返す。
	 *
	 * @param Error 結果の値。
	 * @return 説明文。
	 */
	static FString MakeMessage( ESceneSerializeError Error );
};
