#pragma once

#include <acs.h>

#include "AcsFramework_Core/Save/SaveSlotInfo.h"
#include "AcsFramework_Core/Text/StringConvert.h"

using namespace acs;
using namespace acs::game;

/**
 * 複数枠のセーブを取りまとめるサブシステム。
 *
 * @details
 * 1 枠ぶんの読み書きはエンジン (TSaveSlot) が持っている。ただしエンジンが面倒を見るのは
 * 「1 つのファイル」までで、ゲームが実際に要る次のものは無い。
 *
 * - 枠が何個あって、どれが埋まっているか
 * - 枠の番号からファイルのパスを組み立てること (置き場所を各所で書かない)
 * - 保存先のフォルダが無いときに作ること
 * - 中身を読まずに一覧へ出す見出し
 *
 * ここはそれを持つ。**中身が何であるかは知らない** (型は呼び出し側が決める)。
 *
 * 置ける型はエンジンの決まりに従い、そのままメモリを写せるもの (trivially copyable) だけ。
 * 文字列や配列を持たせたいなら、固定長の配列にして型の中へ埋めること。
 *
 * @code
 * struct FMySave { i32 Stage = 1; i32 Score = 0; };   // そのまま写せる型
 *
 * Save->Configure( FString( "Saved/Save" ), FString( "Slot" ), 3 );
 *
 * Save->Write( 0, FMySave{ 3, 12000 } );
 *
 * FMySave Loaded;
 * if ( Save->Read( 0, Loaded ) ) m_Stage = Loaded.Stage;
 *
 * for ( i32 Index = 0; Index < Save->GetSlotCount(); ++Index )
 * {
 *     const FSaveSlotInfo Info = Save->GetSlotInfo( Index );
 *     // Info.bExists で「つづきから」を出し分ける
 * }
 * @endcode
 */
class CSaveSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CSaveSubsystem )

	/**
	 * 置き場所と枠の数を決める。
	 *
	 * @details
	 * アプリの起動時に 1 度だけ呼ぶ。決める前に読み書きを頼まれても何も起きない。
	 * フォルダは書き込むときに無ければ作る。
	 * @param Directory 置き場所 (実行時の作業フォルダからの相対でよい)。
	 * @param BaseName ファイル名の頭 (これに枠の番号と拡張子が付く)。
	 * @param SlotCount 枠の数 (1 以上)。
	 */
	void Configure( const FString& Directory, const FString& BaseName, i32 SlotCount );

	/** 枠の数を返す (決めていなければ 0)。 */
	i32 GetSlotCount() const noexcept { return m_SlotCount; }

	/**
	 * 枠のファイルのパスを返す。
	 *
	 * @param Slot 何番目の枠か。
	 * @return パス (UTF-8。枠が範囲外なら空文字列)。
	 */
	FString GetSlotPath( i32 Slot ) const;

	/**
	 * その枠に中身があるかを返す。
	 *
	 * @param Slot 何番目の枠か。
	 * @return あれば true。
	 */
	bool Exists( i32 Slot ) const;

	/**
	 * 枠の見出しを返す。
	 *
	 * @details 中身は読まない (header だけ見る) ので、一覧のために全枠を呼んでよい。
	 * @param Slot 何番目の枠か。
	 * @return 見出し。
	 */
	FSaveSlotInfo GetSlotInfo( i32 Slot ) const;

	/**
	 * 枠へ書き込む。
	 *
	 * @details 置き場所のフォルダが無ければ作る。書き込みは途中で壊れない形で行われる。
	 * @tparam T 書き込む型 (そのままメモリを写せるもの)。
	 * @param Slot 何番目の枠か。
	 * @param Data 書き込む中身。
	 * @param Version 書き込む版 (読むときに同じ値を渡すこと)。
	 * @return 書き込めたら true。
	 */
	template<typename T>
	bool Write( i32 Slot, const T& Data, u32 Version = 1u )
	{
		TSaveSlot<T> SlotFile;
		if ( !PrepareSlot( Slot, SlotFile, true ) ) return false;

		return SlotFile.Save( Data, Version ).IsOk();
	}

	/**
	 * 枠から読み出す。
	 *
	 * @details
	 * 中身が壊れている・版が違う場合は読まずに false を返す (OutData は触らない)。
	 * @tparam T 読み出す型 (書いたときと同じ型)。
	 * @param Slot 何番目の枠か。
	 * @param OutData 読み出した中身の書き込み先。
	 * @param Version 読み出す版 (書いたときと同じ値)。
	 * @return 読み出せたら true。
	 */
	template<typename T>
	bool Read( i32 Slot, T& OutData, u32 Version = 1u )
	{
		TSaveSlot<T> SlotFile;
		if ( !PrepareSlot( Slot, SlotFile, false ) ) return false;

		auto Result = SlotFile.Load( Version );
		if ( !Result.IsOk() ) return false;

		OutData = Result.Value();
		return true;
	}

	/**
	 * 枠を消す。
	 *
	 * @param Slot 何番目の枠か。
	 * @return 消せたら true (元から無ければ true)。
	 */
	bool Erase( i32 Slot );

private:
	/**
	 * 枠のファイルを開ける状態にする。
	 *
	 * @param Slot 何番目の枠か。
	 * @param OutWide 組み立てたパスの置き場 (呼び出し側が生かし続ける)。
	 * @return 用意できたら true。
	 */
	bool BuildSlotPath( i32 Slot, TArray<wchar_t>& OutWide ) const;

	/** 置き場所のフォルダが無ければ作る。 */
	void EnsureDirectory() const;

	/**
	 * 枠を指す TSaveSlot を用意する。
	 *
	 * @details テンプレートから呼ぶので、パスの組み立てとフォルダ作りだけをここへ抜き出す。
	 * @tparam T 枠が扱う型。
	 * @param Slot 何番目の枠か。
	 * @param OutSlot 用意する枠。
	 * @param bForWrite 書き込みのために用意するなら true (フォルダを作る)。
	 * @return 用意できたら true。
	 */
	template<typename T>
	bool PrepareSlot( i32 Slot, TSaveSlot<T>& OutSlot, bool bForWrite ) const
	{
		if ( bForWrite ) EnsureDirectory();

		TArray<wchar_t> Wide;
		if ( !BuildSlotPath( Slot, Wide ) ) return false;

		// TSaveSlot はパスを非所有で持つので、自前で確保させる方の口を使う。
		return OutSlot.TryInit( Wide.GetData() ).IsOk();
	}

	/** 置き場所。 */
	FString m_Directory;

	/** ファイル名の頭。 */
	FString m_BaseName;

	/** 枠の数 (0 なら決めていない)。 */
	i32 m_SlotCount = 0;
};
