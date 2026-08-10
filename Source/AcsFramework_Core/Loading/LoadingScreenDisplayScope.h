// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

class CLoadingScreenSubsystem;

/** 手動ロード表示を局所的に所有し、現在の表示世代だけを変更する通常型。 */
class CLoadingScreenDisplayScope final
{
public:
	/** LoadingScreenを非所有で参照する。LoadingScreenはこの型より長く生存させる。 */
	explicit CLoadingScreenDisplayScope( CLoadingScreenSubsystem& Loading ) noexcept;

	/** 現在の表示世代なら表示を解除する。LoadingScreenは破棄まで生存させる。 */
	~CLoadingScreenDisplayScope() noexcept;

	/** 表示解除の所有を一つに保つため複製を禁止する。 */
	CLoadingScreenDisplayScope( const CLoadingScreenDisplayScope& ) = delete;

	/** 表示解除の所有を一つに保つため移動構築を禁止する。 */
	CLoadingScreenDisplayScope( CLoadingScreenDisplayScope&& ) = delete;

	/** 表示解除の所有を一つに保つため複製代入を禁止する。 */
	CLoadingScreenDisplayScope& operator=( const CLoadingScreenDisplayScope& ) = delete;

	/** 表示解除の所有を一つに保つため移動代入を禁止する。 */
	CLoadingScreenDisplayScope& operator=( CLoadingScreenDisplayScope&& ) = delete;

	/** 追従中でなければ表示を取得する。失敗時は既存の追従と表示を変えない。 */
	bool Show( const FString& Message = FString() );

	/** 現在の表示世代なら文言を差し替える。古い世代や追従中はfalseを返す。 */
	bool SetMessage( const FString& Message );

	/** 現在の表示世代なら進捗を範囲へ収めて差し替える。古い世代はfalseを返す。 */
	bool SetProgress( f32 Ratio ) noexcept;

	/** 現在の表示世代だけフォントを差し替える。古い世代や追従中はfalseを返し、FontはReset・失効・再設定まで生存させる。 */
	bool SetFont( const FFont* Font ) noexcept;

	/** 現在の表示世代なら表示を解除する。外部交代後はfalseを返し状態を変えない。 */
	bool Reset() noexcept;

	/** この型が現在の表示世代を所有しているか返す。追従中や外部交代後はfalseを返す。 */
	bool IsActive() const noexcept;

private:
	/** LoadingScreenへの非所有参照。破棄まで参照可能であることを呼び出し側が保証する。 */
	CLoadingScreenSubsystem* m_Loading = nullptr;

	/** 手動表示を所有した世代。0は所有していない状態を示す。 */
	u64 m_Revision = 0u;
};
