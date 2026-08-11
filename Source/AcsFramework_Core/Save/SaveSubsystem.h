// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Save/SaveSlotStore.h"

using namespace acs;

/** GameInstanceが共有するセーブ枠の所有者と公開窓口。 */
class CSaveSubsystem : public ASubsystem
{
public:
	/** サブシステムの型IDと診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CSaveSubsystem )

	/** 保存先と枠数を設定する。正でない枠数は未設定として扱う。 */
	void Configure( const FString& Directory, const FString& BaseName, i32 SlotCount );

	/** 設定済みの枠数を返す。未設定なら0を返す。 */
	i32 GetSlotCount() const noexcept { return m_Store.GetSlotCount(); }

	/** 指定枠のUTF-8パスを返す。範囲外なら空を返す。 */
	FString GetSlotPath( i32 Slot ) const;

	/** 指定枠のファイルが存在するかを返す。無効な枠はfalseを返す。 */
	bool Exists( i32 Slot ) const;

	/** 指定枠のヘッダー情報を返す。無効または破損時は未存在の値を返す。 */
	FSaveSlotInfo GetSlotInfo( i32 Slot ) const;

	/** 指定枠へ値を書き込む。パス準備またはEngine保存に失敗するとfalseを返す。 */
	template<typename T>
	bool Write( i32 Slot, const T& Data, u32 Version = 1u )
	{
		return m_Store.Write( Slot, Data, Version );
	}

	/** 指定枠から値を読む。失敗時はOutDataを変更しない。 */
	template<typename T>
	bool Read( i32 Slot, T& OutData, u32 Version = 1u )
	{
		return m_Store.Read( Slot, OutData, Version );
	}

	/** 指定枠を削除する。無効な枠または削除失敗時はfalseを返す。 */
	bool Erase( i32 Slot );

private:
	/** GameInstanceの寿命で所有するセーブ枠の保存実体。 */
	FSaveSlotStore m_Store;
};
