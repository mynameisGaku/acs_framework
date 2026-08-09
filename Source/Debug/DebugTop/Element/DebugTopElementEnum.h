// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElement.h"

using namespace acs;

// 選択肢から 1 つ選ぶ行と、その選択肢 1 つ分の子行。
// 末尾の DebugTopAddEnumRow / DebugTopGetEnumValue は、列挙型から選択肢を自動で作る橋渡し。

/**
 * あらかじめ与えた選択肢から 1 つ選ぶ行。
 */
class CDebugTopElementEnum : public CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @param Label 左カラムへ出す表示名。
	 * @param Options 選択肢 (空配列を渡した場合は右カラムが空になる)。
	 * @param SelectedIndex 初期の選択位置。
	 */
	CDebugTopElementEnum( const FString& Label, TArray<FString> Options, i32 SelectedIndex = 0 );

	/** 現在の選択位置を返す。 */
	i32 GetSelectedIndex() const noexcept { return m_SelectedIndex; }

	/**
	 * 選択位置を設定する (範囲外は無視し、変化したときだけ通知する)。
	 *
	 * @param SelectedIndex 設定する選択位置。
	 */
	void SetSelectedIndex( i32 SelectedIndex );

	/** 右カラムへ選択中の選択肢を出す。 */
	FString GetValueText() const override;

	/** 左右キーで選択位置を 1 つずつ動かす (端で止まる)。 */
	void OnLeftRight( i32 Delta ) override;

	/** 左右キーに反応する。 */
	bool IsLeftRightAdjustable() const noexcept override { return true; }

	/** 選択位置と選択肢の数を返す。 */
	bool TryGetSelection( i32& OutIndex, i32& OutCount ) const noexcept override;

	/** 値の種類として Enum を返す。 */
	EDebugTopValueKind GetValueKind() const noexcept override { return EDebugTopValueKind::Enum; }

	/** 現在の選択位置を i32 として取り出す (選択肢の文字列は GetValueText で取る)。 */
	bool TryGetInt( i32& OutValue ) const noexcept override;

	/** 選択位置を書き込む (範囲外は無視する)。 */
	bool TrySetInt( i32 Value ) override;

	/** 構築時の選択位置と違っていれば true。 */
	bool IsModified() const noexcept override { return m_SelectedIndex != m_DefaultIndex; }

	/** 構築時の選択位置へ戻す。 */
	void ResetToDefault() override { SetSelectedIndex( m_DefaultIndex ); }

	/** 選択肢の数を返す。 */
	usize GetOptionCount() const noexcept { return m_Options.Num(); }

	/**
	 * 選択肢の表示名を返す。
	 *
	 * @param Index 取り出す位置。
	 * @return 表示名 (範囲外なら空)。
	 */
	const FString& GetOptionText( usize Index ) const noexcept;

private:
	/** 選択肢。 */
	TArray<FString> m_Options;

	/** 現在の選択位置。 */
	i32 m_SelectedIndex;

	/** 構築時の選択位置 (既定値へ戻すときと、変更されたかの判定に使う)。 */
	i32 m_DefaultIndex = 0;
};



/**
 * コンボボックスを開いたときに出る、選択肢 1 つ分の行。
 *
 * @details
 * CDebugTopElementEnum が選択肢の数だけ自動で作る。値は自分では持たず、選択中かどうかを
 * 親へ問い合わせる。チェックボックスとして描かれるので、どれが選ばれているか一目で分かる。
 */
class CDebugTopElementEnumOption : public CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @param Label 選択肢の表示名。
	 * @param Owner 選択位置を持っている親の行。
	 * @param Index 親の選択肢の中でこの行が指す位置。
	 */
	CDebugTopElementEnumOption( const FString& Label, CDebugTopElementEnum& Owner, i32 Index );

	/** 決定キーでこの選択肢を選び、一覧を畳む。 */
	void OnDecide() override;

	/** 選択中かどうかをチェックボックスとして見せるため、Bool を返す。 */
	EDebugTopValueKind GetValueKind() const noexcept override { return EDebugTopValueKind::Bool; }

	/** この選択肢が選ばれていれば true。 */
	bool TryGetBool( bool& bOutValue ) const noexcept override;

	/** チェックを入れるとこの選択肢を選ぶ (外す操作は何もしない)。 */
	bool TrySetBool( bool bValue ) override;

	/** 親の状態を映しているだけなので、保存の対象にはしない。 */
	bool IsSaveable() const noexcept override { return false; }

	/** 右カラムはチェックボックスだけにする。 */
	FString GetValueText() const override { return FString(); }

private:
	/** 選択位置を持っている親の行。所有はしない (親が自分を所有している)。 */
	CDebugTopElementEnum* m_Owner;

	/** 親の選択肢の中でこの行が指す位置。 */
	i32 m_Index;
};




namespace DebugTopDetail
{
	/**
	 * 列挙型の名前を選択肢へ変換する。
	 *
	 * @param Names 変換元の名前表。
	 * @param NameCount 名前表の個数。
	 * @param ExpectedCount 必要な選択肢の個数。
	 * @param OutOptions 変換結果。失敗時は呼出し前の内容を保つ。
	 * @return 個数一致と確保に成功すれば true。
	 */
	inline bool TryBuildEnumOptions( const acs::FEnumName* Names, usize NameCount, usize ExpectedCount, TArray<FString>& OutOptions )
	{
		if ( NameCount != ExpectedCount ) return false;
		if ( Names == nullptr && NameCount != 0u ) return false;

		TArray<FString> StagedOptions( *OutOptions.GetAllocator() );
		if ( !StagedOptions.TryReserve( ExpectedCount ) ) return false;

		for ( usize Index = 0; Index < NameCount; ++Index )
		{
			const acs::FEnumName& Name = Names[Index];
			if ( Name.Data == nullptr || Name.IsEmpty() ) return false;

			FString Option( *OutOptions.GetAllocator() );
			if ( !Option.TryAppend( FStringView( Name.Data, Name.Size ) ) ) return false;
			if ( !StagedOptions.TryAdd( Move( Option ) ) ) return false;
		}

		if ( StagedOptions.Num() != ExpectedCount ) return false;
		OutOptions = Move( StagedOptions );
		return true;
	}
}

/**
 * 列挙型の名前から選択肢を作る。
 *
 * @tparam TEnum 対象の列挙型。
 * @param OutOptions 変換結果。失敗時は呼出し前の内容を保つ。
 * @return 列挙子の個数と名前表が一致し、確保に成功すれば true。
 */
template<typename TEnum>
bool DebugTopMakeEnumOptions( TArray<FString>& OutOptions )
{
	const auto Names = acs::EnumNames<TEnum>();
	return DebugTopDetail::TryBuildEnumOptions( Names.Items, Names.Size(), acs::EnumCount<TEnum>, OutOptions );
}

/**
 * 列挙型の値から選択肢配列を返す。
 *
 * @tparam TEnum 対象の列挙型。
 * @return 変換結果。確保に失敗した場合は検査で停止する。
 */
template<typename TEnum>
TArray<FString> DebugTopMakeEnumOptions()
{
	TArray<FString> Options;
	const bool bBuilt = DebugTopMakeEnumOptions<TEnum>( Options );
	ACS_CHECKF( bBuilt, "DebugTopMakeEnumOptions failed" );
	return Options;
}

/**
 * 列挙型から選択肢を作った Enum 行を親へ足す。
 *
 * @details
 * 選択肢を手で並べる必要が無く、列挙子を足せば行にも自動で増える。
 * @code
 * ACS_ENUM()
 * enum class EQuality : u8 { Low, Middle, High };
 * DebugTopAddEnumRow<EQuality>( *ChildMenu, "Quality", EQuality::Middle );
 * @endcode
 * @tparam TEnum 選択肢にする列挙型。
 * @tparam TParent 行を足す先の型 (CDebugTopElement か ADebugTopEntity)。
 * @param Parent 行を足す先。
 * @param Label 左カラムへ出す表示名。
 * @param Current 初期選択にする列挙子。
 * @return 追加した行 (確保に失敗したら nullptr)。
 */
template<typename TEnum, typename TParent>
CDebugTopElementEnum* DebugTopAddEnumRow( TParent& Parent, const FString& Label, TEnum Current )
{
	TArray<FString> Options;
	if ( !DebugTopMakeEnumOptions<TEnum>( Options ) ) return nullptr;
	return Parent.template Add<CDebugTopElementEnum>( Label, Move( Options ), acs::EnumToIndex( Current ) );
}

/**
 * Enum 行の選択位置を列挙子へ戻す。
 *
 * @tparam TEnum 対象の列挙型。
 * @param Element 対象の Enum 行。
 * @return 選択中の列挙子。
 */
template<typename TEnum>
TEnum DebugTopGetEnumValue( const CDebugTopElementEnum& Element ) noexcept
{
	return acs::EnumFromIndex<TEnum>( Element.GetSelectedIndex() );
}
