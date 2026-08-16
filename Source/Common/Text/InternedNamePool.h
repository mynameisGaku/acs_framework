// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * `const char*` を受け取るエンジン API へ渡す名前を、寿命ごと引き受ける入れ物。
 *
 * @details
 * エンジンの多くの部品 (CPerfBudget、CDevConsole、CPrefabSystem など) は名前を
 * `const char*` で受け取り、**その文字列を自分では複製しない**。呼び出し側が持っている
 * FString をそのまま渡すと、FString が伸び縮みした時点で中身が別の場所へ移り、
 * エンジン側は解放済みの領域を指したままになる。
 *
 * ここへ写しておけば、プールが生きている間ポインタは動かない。同じ名前を二度渡しても
 * 同じポインタが返るので、ポインタ比較で速く済ませているエンジン側とも噛み合う。
 *
 * 中身は TUniquePtr<FString> の配列なので、配列が伸びても FString の実体は動かない。
 *
 * @code
 * const char* const StableName = Names.Intern( FString( "Scene/Update" ) );
 * if ( StableName != nullptr ) Budget.DefineCategory( StableName, 4.0f, 0u );
 * @endcode
 */
class CInternedNamePool
{
public:
	/**
	 * 名前を写して、動かないポインタを返す。
	 *
	 * @details 既に写してある名前なら、前と同じポインタを返す (二重に持たない)。
	 * @param Name 写したい名前。
	 * @return プールが生きている間有効なポインタ (確保に失敗したら nullptr)。
	 */
	const char* Intern( const FString& Name ) noexcept;

	/**
	 * 既に写してある名前を探す。
	 *
	 * @param Name 探す名前。
	 * @return 見つかればそのポインタ、無ければ nullptr。
	 */
	const char* Find( const FString& Name ) const noexcept;

	/** 写してある名前の数を返す。 */
	usize Num() const noexcept { return m_Names.Num(); }

	/**
	 * 全て捨てる。
	 *
	 * @details
	 * 返したポインタは全て無効になる。エンジン側へ渡したものが残っている間は呼ばないこと。
	 */
	void Clear() noexcept { m_Names.Reset(); }

private:
	/** 写した名前。TUniquePtr なので配列が伸びても実体は動かない。 */
	TArray<TUniquePtr<FString>> m_Names;
};
