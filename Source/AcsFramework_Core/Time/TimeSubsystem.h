#pragma once

#include <acs.h>

using namespace acs;

/**
 * ゲームの時間の進み方を、どこからでも変えられるようにするサブシステム。
 *
 * @details
 * 時間の倍率そのものはエンジン (CGame::SetTimeScale) が持っている。ただし CGame への参照は
 * 普通のゲームコードからは辿れないので、この層で受け取って GetSubsystem<CTimeSubsystem>()
 * から頼めるようにする。
 *
 * **止める理由は名前で数える。** 「メニューを開いたから」「ポーズ画面を出したから」
 * 「ムービー中だから」が同時に重なることがあり、単なる真偽値だと片方が再開しただけで
 * 全部動き出してしまう。理由ごとに止め、全ての理由が外れて初めて動き出す。
 *
 * 止めることと遅くすることは別に持つ。止めている間に速さを変えても、再開したときに
 * その速さで動き出す。
 *
 * 画面には関わらない (「止まっている」ことをどう見せるかは別の担当)。逆に、画面が無くても
 * 時間は止められる。
 *
 * @code
 * if ( CTimeSubsystem* Time = GetSubsystem<CTimeSubsystem>() )
 * {
 *     Time->Pause( "PauseMenu" );      // 止める
 *     Time->SetSpeed( 0.25f );         // ゆっくり動かす
 *     Time->StepOnce();                // 止めたまま 1 フレームだけ進める
 *     Time->Resume( "PauseMenu" );     // この理由を外す (他の理由が残っていればまだ動かない)
 * }
 * @endcode
 */
class CTimeSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CTimeSubsystem )

	/**
	 * 時間の倍率を持っているものを受け取る。
	 *
	 * @details アプリの起動時に 1 度だけ呼ぶ。渡さない間は何を頼まれても何も起きない。
	 * @param Game 時間の倍率を持っているもの。
	 */
	void Bind( CGame& Game ) noexcept { m_Game = &Game; }

	/**
	 * 理由を 1 つ足して止める。
	 *
	 * @details
	 * 同じ理由を重ねて渡しても 1 つとして数える (毎フレーム呼んでも増え続けない)。
	 * @param Reason 止める理由 (再開するときに同じ文字列を渡す)。
	 */
	void Pause( const FString& Reason );

	/**
	 * 理由を 1 つ外す。
	 *
	 * @details 他の理由が残っていれば、まだ動き出さない。
	 * @param Reason 外す理由 (止めたときと同じ文字列)。
	 */
	void Resume( const FString& Reason );

	/** 理由を全て外して動き出す。 */
	void ResumeAll() noexcept;

	/** 止まっているかを返す。 */
	bool IsPaused() const noexcept { return m_Reasons.Num() > 0; }

	/**
	 * その理由で止めているかを返す。
	 *
	 * @details
	 * 「この理由で止まっているときだけ出す」といった見せ方 (ポーズ画面) のための口。
	 * 他の理由で止まっていても false になる。
	 * @param Reason 調べる理由。
	 * @return その理由で止めていれば true。
	 */
	bool IsPausedBy( const FString& Reason ) const noexcept { return FindReason( Reason ) < m_Reasons.Num(); }

	/** 止めている理由の数を返す。 */
	usize GetPauseReasonCount() const noexcept { return m_Reasons.Num(); }

	/**
	 * 止めている理由を返す。
	 *
	 * @details 何が止めているのかを画面へ出す (デバッグ表示) ために使う。
	 * @param Index 何番目の理由か。
	 * @return 理由の文字列 (範囲外なら空文字列)。
	 */
	const FString& GetPauseReason( usize Index ) const noexcept;

	/**
	 * 動いているときの速さを設定する。
	 *
	 * @details
	 * 止めている間に変えてもよい (再開したときにこの速さで動き出す)。0 以下は 0 へ丸める。
	 * @param Speed 1 で等速、0.25 で 1/4 の速さ、2 で倍速。
	 */
	void SetSpeed( f32 Speed ) noexcept { m_Speed = Speed > 0.0f ? Speed : 0.0f; }

	/** 動いているときの速さを返す。 */
	f32 GetSpeed() const noexcept { return m_Speed; }

	/**
	 * 止めたまま 1 フレームだけ進める。
	 *
	 * @details
	 * 1 フレームずつ確かめたいとき (当たり判定の瞬間を見る等) に使う。止めていないときに
	 * 呼んでも何も変わらない。
	 */
	void StepOnce() noexcept { m_bStepRequested = true; }

	/**
	 * この回に実際に掛かっている倍率を返す。
	 *
	 * @details
	 * 止まっていれば 0、コマ送りの回は 1、それ以外は決めた速さ。ゲーム時間で進めたいもの
	 * (タイマー等) が、実経過秒へ掛けるために使う。答えは Update が決める。
	 * @return この回の倍率。
	 */
	f32 GetEffectiveScale() const noexcept { return m_EffectiveScale; }

	/**
	 * この回にシーンを進めてよいかを返す。
	 *
	 * @details
	 * 止まっている間は倍率を 0 にするだけでなく、シーンの更新そのものを飛ばす。倍率だけ 0 に
	 * してもシーンは呼ばれ続けるので、止めているつもりでもキー入力が奥まで届いてしまう。
	 * 答えは Update が決める。Update より前に聞くと、前の回の答えが返る。
	 * @return 進めてよいなら true。
	 */
	bool ShouldTickScenes() const noexcept { return m_bTickThisFrame; }

	/**
	 * 物理などを回す固定の刻み幅を設定する。
	 *
	 * @details
	 * OnFixedUpdate がこの幅で呼ばれる。実際の frame rate が揺れても、当たり判定や積分が
	 * 同じ幅で進むようにするためのもの。時間の倍率はこちらにも掛かるので、止めれば
	 * OnFixedUpdate も止まる。
	 * @param FixedSeconds 1 回ぶんの長さ (秒。既定は 1/60)。0 以下で固定刻みを止める。
	 * @param MaxStepsPerFrame 1 フレームで進める上限 (これを超えた遅れは切り捨てる。
	 *                         上限が無いと、重い 1 フレームの後に回し切れず更に遅れる)。
	 */
	void SetFixedTimestep( f32 FixedSeconds, u32 MaxStepsPerFrame = 8 ) noexcept;

	/**
	 * 固定の刻み幅を返す。
	 *
	 * @return 1 回ぶんの長さ (秒。配線前は 0)。
	 */
	f32 GetFixedTimestep() const noexcept;

	/**
	 * 1 フレーム進める。
	 *
	 * @details
	 * 決めた速さを実際の倍率へ反映する。シーンを進める前に呼ぶこと。コマ送りの要求は
	 * ここで 1 回ぶん使い切る。
	 */
	void Update() noexcept;

private:
	/**
	 * その理由で既に止めているかを返す。
	 *
	 * @param Reason 探す理由。
	 * @return 見つかれば添字、無ければ理由の数。
	 */
	usize FindReason( const FString& Reason ) const noexcept;

	/** 時間の倍率を持っているもの。所有はしない (アプリが持っている)。 */
	CGame* m_Game = nullptr;

	/** いま止めている理由。空なら動いている。 */
	TArray<FString> m_Reasons;

	/** 動いているときの速さ。 */
	f32 m_Speed = 1.0f;

	/** 止めたまま 1 フレームだけ進める要求が出ているか。 */
	bool m_bStepRequested = false;

	/**
	 * この回にシーンを進めるか (Update が決める)。
	 *
	 * @details まだ Update を通っていない間は進める側にしておく (配線前に止まらないように)。
	 */
	bool m_bTickThisFrame = true;

	/** この回に実際に掛かっている倍率 (Update が決める)。 */
	f32 m_EffectiveScale = 1.0f;
};
