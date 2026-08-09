#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElement.h"

using namespace acs;

// 数値の行 (整数と実数)。範囲を持てばスライダーが付き、欄へ直接打ち込める。

/**
 * i32 を左右キーで変える行。
 *
 * @details
 * 動作は 2 通りある。AddData で候補を 1 つも登録しなければ Min / Max の範囲を Step 刻みで
 * 増減する。候補を登録すると左右キーは候補の切り替えになり、右カラムとサブタイトルが
 * 選択中の候補の Title になる。
 */
class CDebugTopElementInt : public CDebugTopElement
{
public:
	/** 選択候補 1 つ分。 */
	struct FData
	{
		/** 右カラムとサブタイトルへ出す表示名。 */
		FString Title;

		/** この候補が表す値。 */
		i32 Value = 0;
	};

	/**
	 * 行を構築する。
	 *
	 * @param Label 左カラムへ出す表示名。
	 * @param Value 初期値。
	 * @param Min 下限 (候補モードでは使わない)。
	 * @param Max 上限 (候補モードでは使わない)。
	 * @param Step 左右キー 1 回あたりの増減量 (候補モードでは使わない)。
	 */
	CDebugTopElementInt( const FString& Label, i32 Value, i32 Min, i32 Max, i32 Step = 1 );

	/**
	 * 選択候補を 1 つ追加する。
	 *
	 * @details
	 * 1 つでも追加すると左右キーは増減ではなく候補の切り替えになる。最初の 1 つを追加した時点で
	 * その候補が選択され、サブタイトルもその Title になる。候補の値は Min / Max でクランプしない。
	 * @param Title 候補の表示名。
	 * @param Value 候補が表す値。
	 */
	void AddData( const FString& Title, i32 Value );

	/** 登録された候補の数を返す (0 なら増減モード)。 */
	usize GetDataCount() const noexcept { return m_Data.Num(); }

	/** 選択中の候補の位置を返す (候補が無ければ -1)。 */
	i32 GetSelectedDataIndex() const noexcept;

	/**
	 * 選択中の候補を設定する (範囲外は無視し、変化したときだけ通知する)。
	 *
	 * @param Index 設定する候補の位置。
	 */
	void SetSelectedDataIndex( i32 Index );

	/** 現在値を返す。 */
	i32 GetValue() const noexcept { return m_Value; }

	/**
	 * 値を設定する。
	 *
	 * @details 候補モードでは値が一致する候補へ合わせる (一致する候補が無ければ何もしない)。
	 * 増減モードでは上下限へクランプする。どちらも変化したときだけ通知する。
	 * @param Value 設定する値。
	 */
	void SetValue( i32 Value );

	/** 右カラムへ、候補モードなら Title を、増減モードなら現在値を 10 進で出す。 */
	FString GetValueText() const override;

	/** 左右キーで候補を切り替えるか、Step 分だけ増減する。 */
	void OnLeftRight( i32 Delta ) override;

	/** 左右キーに反応する。 */
	bool IsLeftRightAdjustable() const noexcept override { return true; }

	/** 候補モードなら選択位置と候補数を返す。 */
	bool TryGetSelection( i32& OutIndex, i32& OutCount ) const noexcept override;

	/** 値の種類として Int を返す。 */
	EDebugTopValueKind GetValueKind() const noexcept override { return EDebugTopValueKind::Int; }

	/** 現在値を i32 として取り出す。 */
	bool TryGetInt( i32& OutValue ) const noexcept override;

	/** SetValue と同じ規則で値を書き込む。 */
	bool TrySetInt( i32 Value ) override;

	/** 候補列を持たない行は、決定すると値を直接打ち込める。 */
	bool CanTypeValue() const noexcept override { return m_Data.IsEmpty(); }

	/** 打ち込みの初期値として現在値を返す。 */
	FString GetEditText() const override;

	/** 打ち終えた文字列を整数として読み、上下限へ丸めて書き込む。 */
	bool CommitEditText( const FString& Text ) override;

	/** 構築時の値と違っていれば true。 */
	bool IsModified() const noexcept override { return m_Value != m_DefaultValue; }

	/** 構築時の値へ戻す。 */
	void ResetToDefault() override { SetValue( m_DefaultValue ); }

	/** 増減モードなら下限 0 上限 1 での現在位置を返す (候補モードは範囲を持たない)。 */
	bool TryGetRatio( f32& OutRatio ) const noexcept override;

	/** 下限から上限までの位置で値を書き込む。 */
	bool TrySetRatio( f32 Ratio ) override;

private:
	/** 選択中の候補を現在値とサブタイトルへ反映する (通知はしない)。 */
	void ApplySelectedData();

	/** 選択候補。空なら増減モード。 */
	TArray<FData> m_Data;

	/** 現在値。 */
	i32 m_Value;

	/** 下限。 */
	i32 m_Min;

	/** 上限。 */
	i32 m_Max;

	/** 左右キー 1 回あたりの増減量。 */
	i32 m_Step;

	/** 選択中の候補の位置 (候補が空の間は意味を持たない)。 */
	i32 m_Select = 0;

	/** 構築時の値 (既定値へ戻すときと、変更されたかの判定に使う)。 */
	i32 m_DefaultValue = 0;
};



/**
 * f32 を左右キーで増減する行。
 */
class CDebugTopElementFloat : public CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @param Label 左カラムへ出す表示名。
	 * @param Value 初期値。
	 * @param Min 下限。
	 * @param Max 上限。
	 * @param Step 左右キー 1 回あたりの増減量。
	 */
	CDebugTopElementFloat( const FString& Label, f32 Value, f32 Min, f32 Max, f32 Step = 0.1f );

	/** 現在値を返す。 */
	f32 GetValue() const noexcept { return m_Value; }

	/**
	 * 値を設定する (上下限へクランプし、変化したときだけ通知する)。
	 *
	 * @param Value 設定する値。
	 */
	void SetValue( f32 Value );

	/** 右カラムへ現在値を小数 3 桁で出す。 */
	FString GetValueText() const override;

	/** 左右キーで Step 分だけ増減する。 */
	void OnLeftRight( i32 Delta ) override;

	/** 左右キーに反応する。 */
	bool IsLeftRightAdjustable() const noexcept override { return true; }

	/** 値の種類として Float を返す。 */
	EDebugTopValueKind GetValueKind() const noexcept override { return EDebugTopValueKind::Float; }

	/** 現在値を f32 として取り出す。 */
	bool TryGetFloat( f32& OutValue ) const noexcept override;

	/** SetValue と同じ規則で値を書き込む。 */
	bool TrySetFloat( f32 Value ) override;

	/** 決定すると値を直接打ち込める。 */
	bool CanTypeValue() const noexcept override { return true; }

	/** 打ち込みの初期値として現在値を返す。 */
	FString GetEditText() const override;

	/** 打ち終えた文字列を実数として読み、上下限へ丸めて書き込む。 */
	bool CommitEditText( const FString& Text ) override;

	/** 構築時の値と違っていれば true。 */
	bool IsModified() const noexcept override { return m_Value != m_DefaultValue; }

	/** 構築時の値へ戻す。 */
	void ResetToDefault() override { SetValue( m_DefaultValue ); }

	/** 下限 0 上限 1 での現在位置を返す。 */
	bool TryGetRatio( f32& OutRatio ) const noexcept override;

	/** 下限から上限までの位置で値を書き込む。 */
	bool TrySetRatio( f32 Ratio ) override;

private:
	/** 現在値。 */
	f32 m_Value;

	/** 下限。 */
	f32 m_Min;

	/** 上限。 */
	f32 m_Max;

	/** 左右キー 1 回あたりの増減量。 */
	f32 m_Step;

	/** 構築時の値 (既定値へ戻すときと、変更されたかの判定に使う)。 */
	f32 m_DefaultValue = 0.0f;
};
