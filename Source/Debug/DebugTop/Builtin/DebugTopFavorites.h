#pragma once

#include <acs.h>

#include "Debug/DebugTop/Page/DebugTopEntity.h"

using namespace acs;

class ADebugTopHUD;

/**
 * ピン留めした行だけを集めるページ。
 *
 * @details
 * メニュー全体から IsFavorite の行を拾って並べる。項目が数百になっても、よく使うものだけを
 * 1 ページから触れる。結果を決定すると、その行があるページへ移動する。並びはメニューを辿る
 * 順のままなので、留め外しをしても位置が入れ替わらない。
 * 留め外しは各行の上で F を押す。版が変わったときだけ組み直すので、毎フレーム作り直さない。
 */
class ADebugTopFavoritesEntity : public ADebugTopEntity
{
public:
	/**
	 * ページを構築する。
	 *
	 * @param Name パンくずへ出すページ名。
	 * @param HUD 探す対象のメニュー。ページより長く生きること。
	 */
	ADebugTopFavoritesEntity( const FString& Name, ADebugTopHUD& HUD );

	/** 留め外しがあれば組み直してから、通常どおり 1 フレーム進める。 */
	void Update( f32 DeltaSeconds ) noexcept override;

protected:
	/** 初回の一覧を作る。 */
	void OnBuild() noexcept override;

private:
	/** ピン留めされた行を集めて並べ直す。 */
	void RebuildResults();

	/** 探す対象のメニュー。所有はしない。 */
	ADebugTopHUD* m_HUD = nullptr;

	/** 組み直した時点のピン留めの版。 */
	u32 m_Version = 0;

	/** まだ一度も組んでいないか。 */
	bool m_bDirty = true;
};
