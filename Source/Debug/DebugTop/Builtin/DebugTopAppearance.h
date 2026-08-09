#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElementBool.h"
#include "Debug/DebugTop/Element/DebugTopElementNumber.h"
#include "Debug/DebugTop/Page/DebugTopEntity.h"

using namespace acs;

class ADebugTopHUD;

/**
 * メニュー自身の見た目を、メニューから変えるページ。
 *
 * @details
 * 文字が小さすぎる / 大きすぎるのは環境によって変わるのに、いちいちコードを直して
 * 再ビルドするのは重い。ここで変えれば次の描画から効く。
 *
 * 値を変えるとその場で HUD へ反映する (保存を待たない)。設定として保存もされるので、
 * 次の起動でも同じ大きさで立ち上がる。
 *
 * 同梱の既製ページ。継承点 (ADebugTopEntity) をモジュール自身が使っている形。
 */
class ADebugTopAppearanceEntity : public ADebugTopEntity
{
public:
	/**
	 * ページを構築する。
	 *
	 * @param Name パンくずへ出すページ名。
	 * @param HUD 見た目を変える相手。ページより長く生きること。
	 */
	ADebugTopAppearanceEntity( const FString& Name, ADebugTopHUD& HUD );

protected:
	/** 行を並べる。 */
	void OnBuild() noexcept override;

private:
	/** 文字サイズの行が変わったときに HUD へ流す。 */
	void ApplyFontSize();

	/** 漢字を焼くかの行が変わったときに HUD へ流す。 */
	void ApplyIncludeCjk();

	/** 見た目を変える相手。所有はしない。 */
	ADebugTopHUD* m_HUD = nullptr;

	/** 文字サイズの行。所有はしない (このページが所有している)。 */
	CDebugTopElementFloat* m_FontSize = nullptr;

	/** 漢字を焼くかの行。所有はしない (このページが所有している)。 */
	CDebugTopElementBool* m_IncludeCjk = nullptr;
};
