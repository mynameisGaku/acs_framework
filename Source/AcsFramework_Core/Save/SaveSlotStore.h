// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Save/SaveSlotInfo.h"
#include "AcsFramework_Core/Text/StringConvert.h"

using namespace acs;
using namespace acs::game;

/** セーブ枠の設定、パス、同期入出力をまとめて保持する。 */
class FSaveSlotStore
{
public:
	/** 保存先と枠数を置き換える。枠数が正でなければ未設定に戻す。 */
	void Configure( const FString& Directory, const FString& BaseName, i32 SlotCount );

	/** 設定済みの枠数を返す。未設定なら0を返す。 */
	i32 GetSlotCount() const noexcept { return m_SlotCount; }

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
		// 指定枠のEngine保存処理を受け持つ実体。
		TSaveSlot<T> SlotFile;
		if ( !PrepareSlot( Slot, SlotFile, true ) ) return false;

		return SlotFile.Save( Data, Version ).IsOk();
	}

	/** 指定枠から値を読む。失敗時はOutDataを変更しない。 */
	template<typename T>
	bool Read( i32 Slot, T& OutData, u32 Version = 1u )
	{
		// 指定枠のEngine読込処理を受け持つ実体。
		TSaveSlot<T> SlotFile;
		if ( !PrepareSlot( Slot, SlotFile, false ) ) return false;

		// 読込成功時だけOutDataへ反映する結果。
		auto Result = SlotFile.Load( Version );
		if ( !Result.IsOk() ) return false;

		OutData = Result.Value();
		return true;
	}

	/** 指定枠を削除する。無効な枠または削除失敗時はfalseを返す。 */
	bool Erase( i32 Slot );

private:
	/** 指定枠のnativeパスを構築する。範囲外または文字変換失敗時はfalseを返す。 */
	bool BuildSlotPath( i32 Slot, TArray<wchar_t>& OutWide ) const;

	/** 保存先が未作成なら作成を試みる。失敗は後続の保存結果へ委ねる。 */
	void EnsureDirectory() const;

	/** 指定枠をEngineの保存実体へ配線する。書込時は枠判定より先に保存先を準備する。 */
	template<typename T>
	bool PrepareSlot( i32 Slot, TSaveSlot<T>& OutSlot, bool bForWrite ) const
	{
		if ( bForWrite ) EnsureDirectory();

		// Engineへ渡す指定枠のnativeパス。
		TArray<wchar_t> Wide;
		if ( !BuildSlotPath( Slot, Wide ) ) return false;

		return OutSlot.TryInit( Wide.GetData() ).IsOk();
	}

	/** 保存先のUTF-8ディレクトリ。 */
	FString m_Directory;

	/** 枠番号の前へ付けるファイル名。 */
	FString m_BaseName;

	/** 有効な枠数。未設定時は0。 */
	i32 m_SlotCount = 0;
};
