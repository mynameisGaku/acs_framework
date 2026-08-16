#pragma once

#include <acs.h>

using namespace acs;

/**
 * 重ねた画面が下のシーンへ返す答え。
 *
 * @details
 * 「続ける / やめる」のような、重ねた側が決めたこと。下ろすときに添えると、戻り先が
 * OnResume の時点で TravelContext<T>() から読める。
 */
class CSampleModalResult final : public CSceneTravelContext
{
public:
	ACS_RTTI( CSampleModalResult, CSceneTravelContext )

	/** 決定されたか (false なら取り消し)。 */
	bool bAccepted = false;

	/** 何回目に開いたものか (受け渡しが効いていることを目で見るための値)。 */
	i32 OpenCount = 0;
};


/**
 * 重ねた画面を開くときに渡すもの。
 *
 * @details 何回目に開いたかを渡して、そのまま答えへ載せ返す。
 */
class CSampleModalOpen final : public CSceneTravelContext
{
public:
	ACS_RTTI( CSampleModalOpen, CSceneTravelContext )

	/** 何回目に開いたか。 */
	i32 OpenCount = 0;
};

/**
 * 下のシーンを残したまま重ねる、確認用の小さな画面。
 *
 * @details
 * 切り替え (TravelTo) と違い、下のシーンは畳まれずに止まっているだけ。下ろすと元の続きから
 * 始まる。ポーズ画面・ダイアログ・リザルトの雛形として置いてある。
 *
 * 決定 (Enter) と取り消し (Esc) で自分を下ろし、そのとき «答え» を下のシーンへ返す。
 * 誰へ返すのかは知らない (下ろした先が受け取る)。
 */
class ASampleModalScene : public AScene
{
public:
	/** 2D の標準サービス構成を要求する。 */
	ESvc WantedServices() const noexcept override { return kScene2DServices; }

	/** 積まれたときに 1 回呼ばれる (渡されたものをここで受け取る)。 */
	void OnEnter() noexcept override;

	/**
	 * 毎フレーム呼ばれる。
	 *
	 * @param DeltaSeconds 前フレームからの経過秒。
	 */
	void OnUpdate( f32 DeltaSeconds ) noexcept override;

	/** HUD view のカスタム描画。 */
	void OnDrawHud( FRenderContext& RenderContext, CSpriteBatch& Batch ) noexcept override;

	/**
	 * 答えを添えて自分を下ろす。
	 *
	 * @param bAccepted 決定なら true、取り消しなら false。
	 */
	void Close( bool bAccepted ) noexcept;

private:
	/** 何回目に開かれたか (渡されたもの)。 */
	i32 m_OpenCount = 0;
};
