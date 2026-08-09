#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElement.h"

using namespace acs;

// 決定すると処理を走らせるだけの行。

/**
 * 決定キーで処理を実行する行 (シーン遷移やダンプ出力などに使う)。
 */
class CDebugTopElementAction : public CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @param Label 左カラムへ出す表示名。
	 * @param SubTitle 右カラムへ出す文字列。
	 * @param Action 決定キーで呼ぶデリゲート。
	 */
	CDebugTopElementAction( const FString& Label, const FString& SubTitle, FSimpleDelegate Action );

	/** 決定キーでデリゲートを呼ぶ。 */
	void OnDecide() override;

private:
	/** 決定キーで呼ぶデリゲート。 */
	FSimpleDelegate m_Action;
};
