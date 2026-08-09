#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElement.h"
#include "Debug/DebugTop/Service/DebugTopFileDialog.h"

using namespace acs;

// 文字列の行と、その派生であるパスの行 (決定するとダイアログが開く)。

/**
 * 文字列を打ち込む行。
 *
 * @details
 * 決定すると打ち込みが始まり、Enter で確定・Esc で取り消す。名前やパス、検索語のように
 * 左右キーで送れない値を入れるために使う。値は保存にも文字列として乗る。
 */
class CDebugTopElementString : public CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @param Label 左カラムへ出す表示名。
	 * @param Value 初期の文字列。
	 */
	CDebugTopElementString( const FString& Label, const FString& Value );

	/** 現在の文字列を返す。 */
	const FString& GetValue() const noexcept { return m_Value; }

	/**
	 * 文字列を設定する。
	 *
	 * @details 変化したときだけ通知する。改行は 1 行に収まらないので空白へ潰す。
	 * @param Value 設定する文字列。
	 */
	void SetValue( const FString& Value );

	/** 右カラムへ現在の文字列を出す。 */
	FString GetValueText() const override { return m_Value; }

	/** 文字列の行であることを伝える。 */
	EDebugTopValueKind GetValueKind() const noexcept override { return EDebugTopValueKind::String; }

	/** 決定すると打ち込みが始まる。 */
	bool CanTypeValue() const noexcept override { return true; }

	/** 打ち込みの初期値として現在の文字列を返す。 */
	FString GetEditText() const override { return m_Value; }

	/** 打ち終えた文字列をそのまま値にする。 */
	bool CommitEditText( const FString& Text ) override;

	/** 構築時の文字列へ戻す。 */
	void ResetToDefault() override;

	/** 構築時から変わっているかを返す。 */
	bool IsModified() const noexcept override;

private:
	/** 現在の文字列。 */
	FString m_Value;

	/** 構築時の文字列 (既定値へ戻すときと、変更されたかの判定に使う)。 */
	FString m_DefaultValue;
};



/**
 * ファイルやフォルダのパスを持つ行。
 *
 * @details
 * 文字列の行に「選ぶ」操作を足したもの。決定すると OS の選択ダイアログが開き、選んだパスが
 * 入る。手で直したいときは欄をクリックして打ち込める (文字列の行と同じ)。保存にも文字列
 * としてそのまま乗る。
 */
class CDebugTopElementPath : public CDebugTopElementString
{
public:
	/**
	 * 行を構築する。
	 *
	 * @param Label 左カラムへ出す表示名。
	 * @param Value 初期のパス。
	 * @param Kind 選ばせるものの種類 (フォルダ / ファイル)。
	 */
	CDebugTopElementPath( const FString& Label, const FString& Value, EDebugTopPickKind Kind );

	/** 決定するとパスを選ぶ一覧を開く。 */
	void OnDecide() override;

	/** 決定は打ち込みではなく一覧を開く方に使う (欄をクリックすれば手でも打てる)。 */
	bool PrefersDecide() const noexcept override { return true; }

	/** 選ばせるものの種類を返す。 */
	EDebugTopPickKind GetPickKind() const noexcept { return m_Kind; }

private:
	/** 選ばせるものの種類。 */
	EDebugTopPickKind m_Kind;
};


/** グラフへ出す値をその場で取るためのデリゲート。 */
