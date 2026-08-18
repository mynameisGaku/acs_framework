// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElement.h"

using namespace acs;

// ON / OFF の行。チェックボックスとして描かれる。

/**
 * ON / OFF を切り替える行。
 */
class CDebugTopElementBool : public CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @param Label 左カラムへ出す表示名。
	 * @param bValue 初期値。
	 */
	CDebugTopElementBool( const FString& Label, bool bValue );

	/** 現在値を返す。 */
	bool GetValue() const noexcept { return m_bValue; }

	/**
	 * 値を設定する (変化したときだけ通知する)。
	 *
	 * @param bValue 設定する値。
	 */
	void SetValue( bool bValue );

	/** 右カラムへ ON / OFF を出す。 */
	FString GetValueText() const override;

	/** 左右どちらのキーでも反転する。 */
	void OnLeftRight( i32 Delta ) override;

	/** 子行を持たないときは決定キーでも反転する。 */
	void OnDecide() override;

	/** 左右キーに反応する。 */
	bool IsLeftRightAdjustable() const noexcept override { return true; }

	/** OFF を 0/2、ON を 1/2 として返す。 */
	bool TryGetSelection( i32& OutIndex, i32& OutCount ) const noexcept override;

	/** 値の種類として Bool を返す。 */
	EDebugTopValueKind GetValueKind() const noexcept override { return EDebugTopValueKind::Bool; }

	/** 現在値を bool として取り出す。 */
	bool TryGetBool( bool& bOutValue ) const noexcept override;

	/** 値を書き込む。 */
	bool TrySetBool( bool bValue ) override;

	/** 構築時の値と違っていれば true。 */
	bool IsModified() const noexcept override { return m_bValue != m_bDefaultValue; }

	/** 構築時の値へ戻す。 */
	void ResetToDefault() override { SetValue( m_bDefaultValue ); }

private:
	/** 現在値。 */
	bool m_bValue;

	/** 構築時の値 (既定値へ戻すときと、変更されたかの判定に使う)。 */
	bool m_bDefaultValue;
};
