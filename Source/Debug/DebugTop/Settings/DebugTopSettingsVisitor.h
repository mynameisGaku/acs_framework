// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElement.h"

using namespace acs;

class ADebugTopHUD;

/**
 * 値を持つ行を 1 つずつ受け取る訪問者。
 *
 * @details メニューの走査順とキーの決め方を、保存側と読み込み側で共有するために挟む。
 */
class IDebugTopSettingsVisitor
{
public:
	/** 派生を正しく破棄するための仮想デストラクタ。 */
	virtual ~IDebugTopSettingsVisitor() = default;

	/**
	 * 値を持つ行を 1 つ受け取る。
	 *
	 * @param Key その行の保存キー。
	 * @param Element 対象の行。
	 */
	virtual void OnValue( const FString& Key, CDebugTopElement& Element ) = 0;

	/**
	 * 値を持たない行も含めて、全ての行を受け取る。
	 *
	 * @details
	 * 値ではなく行そのものに付く状態 (ピン留め等) を保存するために使う。キーは OnValue と
	 * 同じ規則で決まるので、同じ行なら同じキーになる。
	 * @param Key その行の保存キー。
	 * @param Element 対象の行。
	 */
	virtual void OnRow( const FString& Key, CDebugTopElement& Element ) { (void)Key; (void)Element; }
};


/**
 * メニュー全体を辿り、値を持つ行を訪問する。
 *
 * @details
 * キーは行に SetSaveKey があればそれを絶対キーとして使い、無ければ
 * 「ページ名/親行の表示名/行の表示名」を組み立てる。ここでの表示名は SetLabel の静的な方で、
 * SetLabelProvider の結果は使わない (毎フレーム変わる文字列はキーにできないため。動的ラベルの
 * 行を保存対象にするなら SetSaveKey を付けること)。同じ階層でキーが重なった場合は末尾へ
 * #2、#3 を足して区別する。保存と読み込みで同じ順に辿るため、両者のキーは必ず一致する。
 * @param HUD 辿る対象のメニュー。
 * @param Visitor 値を持つ行を受け取る訪問者。
 */
void DebugTopVisitSettings( const ADebugTopHUD& HUD, IDebugTopSettingsVisitor& Visitor );
