// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 通知の種類。
 *
 * @details 左上のアイコンと縁の色がこれで決まる。文字を読まなくても種類が分かるようにする。
 */
ACS_ENUM()
enum class EDebugTopToastKind : u8
{
	/** ただのお知らせ (青)。 */
	Info,

	/** うまくいったことの報告 (緑)。 */
	Success,

	/** 気に留めてほしいこと (黄)。 */
	Warning,

	/** 失敗したことの報告 (赤)。 */
	Error,
};


/**
 * 通知へ置くボタン 1 つ。
 *
 * @details
 * 押すと結び付けたデリゲートが走り、その通知は閉じる。「保存先のフォルダを開く」
 * 「エラーのログを開く」のように、通知から次の行動へ直接つなぐために使う。
 */
struct FDebugTopToastButton
{
	/** ボタンへ出す文字。 */
	FString Label;

	/** 押されたときに走らせるもの。 */
	FSimpleDelegate OnPressed;

	/** 画面上のボタンの左端 X (描画のたびに更新される。当たり判定に使う)。 */
	f32 X = 0.0f;

	/** 画面上のボタンの幅 (描画のたびに更新される)。 */
	f32 Width = 0.0f;
};


/**
 * 画面の右下へ出す通知 1 件。
 *
 * @details
 * 出てから 5 秒ほど留まり、あとは薄くなって消える。マウスを重ねている間は数えないので、
 * ボタンへ手を伸ばしている最中に消えることはない。閉じるボタンでいつでも消せる。
 * 位置と大きさは描画側が決めるので、この型は「何を出すか」と「いつ消えるか」だけを持つ。
 */
class CDebugTopToast
{
public:
	/**
	 * 通知を構築する。
	 *
	 * @param Kind 通知の種類。
	 * @param Title 1 行目へ出す見出し。
	 * @param Message 見出しの下へ出す本文 (空なら出さない。\\n で複数行)。
	 */
	CDebugTopToast( EDebugTopToastKind Kind, const FString& Title, const FString& Message );

	/**
	 * ボタンを足す。
	 *
	 * @details 出した後でも足せる。並びは足した順で、通知の下段へ横に並ぶ。
	 * @param Label ボタンへ出す文字。
	 * @param OnPressed 押されたときに走らせるもの。
	 * @return 続けて足せるように自分自身を返す。
	 */
	CDebugTopToast& AddButton( const FString& Label, FSimpleDelegate OnPressed );

	/**
	 * 留まる長さを変える。
	 *
	 * @details 既定は 5 秒。読むのに時間がかかる通知だけ延ばす。
	 * @param Seconds 留まる秒数。
	 * @return 続けて設定できるように自分自身を返す。
	 */
	CDebugTopToast& SetDuration( f32 Seconds ) noexcept;

	/** 通知の種類を返す。 */
	EDebugTopToastKind GetKind() const noexcept { return m_Kind; }

	/** 見出しを返す。 */
	const FString& GetTitle() const noexcept { return m_Title; }

	/** 本文を返す。 */
	const FString& GetMessage() const noexcept { return m_Message; }

	/** ボタンを返す。 */
	TArray<FDebugTopToastButton>& GetButtons() noexcept { return m_Buttons; }

	/** ボタンを返す (読み取り専用)。 */
	const TArray<FDebugTopToastButton>& GetButtons() const noexcept { return m_Buttons; }

	/**
	 * 1 フレーム進める。
	 *
	 * @param DeltaSeconds 前フレームからの経過秒。
	 * @param bHovered マウスが重なっているか (重なっている間は留まる時間を数えない)。
	 */
	void Update( f32 DeltaSeconds, bool bHovered );

	/** 閉じ始める (閉じるボタンとボタン押下から呼ぶ)。 */
	void Dismiss() noexcept;

	/** 消え終わったかを返す。 */
	bool IsFinished() const noexcept;

	/**
	 * 出てくる途中の進み具合を返す。
	 *
	 * @return 0 で画面の外、1 で所定の位置。
	 */
	f32 GetSlideRatio() const noexcept;

	/** いまの濃さ (0..1) を返す。消えぎわで薄くなる。 */
	f32 GetOpacity() const noexcept;

	/** 画面上の左端 X を返す (描画のたびに更新される)。 */
	f32 GetX() const noexcept { return m_X; }

	/** 画面上の上端 Y を返す (描画のたびに更新される)。 */
	f32 GetY() const noexcept { return m_Y; }

	/** 幅を返す (描画のたびに更新される)。 */
	f32 GetWidth() const noexcept { return m_Width; }

	/** 高さを返す (描画のたびに更新される)。 */
	f32 GetHeight() const noexcept { return m_Height; }

	/**
	 * 画面上の位置と大きさを控える。
	 *
	 * @details 描画側が決めた矩形を、次のフレームの当たり判定で使うために持たせる。
	 * @param X 左端 X。
	 * @param Y 上端 Y。
	 * @param Width 幅。
	 * @param Height 高さ。
	 */
	void SetRect( f32 X, f32 Y, f32 Width, f32 Height ) noexcept;

	/** 閉じるボタンの左端 X を返す。 */
	f32 GetCloseX() const noexcept { return m_CloseX; }

	/** 閉じるボタンの上端 Y を返す。 */
	f32 GetCloseY() const noexcept { return m_CloseY; }

	/** 閉じるボタンの一辺を返す。 */
	f32 GetCloseSize() const noexcept { return m_CloseSize; }

	/**
	 * 閉じるボタンの位置を控える。
	 *
	 * @param X 左端 X。
	 * @param Y 上端 Y。
	 * @param Size 一辺。
	 */
	void SetCloseRect( f32 X, f32 Y, f32 Size ) noexcept;

	/** ボタンの段の上端 Y を返す。 */
	f32 GetButtonY() const noexcept { return m_ButtonY; }

	/** ボタンの段の高さを返す。 */
	f32 GetButtonHeight() const noexcept { return m_ButtonHeight; }

	/**
	 * ボタンの段の位置を控える。
	 *
	 * @param Y 上端 Y。
	 * @param Height 高さ。
	 */
	void SetButtonRow( f32 Y, f32 Height ) noexcept;

	/** 縦位置を合わせる途中の値を返す (積み上がりが滑らかに動くように保つ)。 */
	f32 GetAnimatedBottom() const noexcept { return m_AnimatedBottom; }

	/**
	 * 縦位置を合わせる途中の値を設定する。
	 *
	 * @param Bottom 設定する下端 Y。
	 */
	void SetAnimatedBottom( f32 Bottom ) noexcept { m_AnimatedBottom = Bottom; }

	/** 縦位置がまだ一度も決まっていないかを返す (出た直後は所定の位置から始める)。 */
	bool HasPlacement() const noexcept { return m_bPlaced; }

	/** 縦位置が決まったことを記録する。 */
	void MarkPlaced() noexcept { m_bPlaced = true; }

private:
	/** 通知の種類。 */
	EDebugTopToastKind m_Kind;

	/** 見出し。 */
	FString m_Title;

	/** 本文。 */
	FString m_Message;

	/** 押せるボタン。 */
	TArray<FDebugTopToastButton> m_Buttons;

	/** 出てからの経過秒 (マウスを重ねている間は進めない)。 */
	f32 m_Elapsed = 0.0f;

	/** 留まる秒数。 */
	f32 m_Duration = 0.0f;

	/** 閉じ始めてからの経過秒 (閉じていなければ負)。 */
	f32 m_DismissElapsed = -1.0f;

	/** 画面上の左端 X。 */
	f32 m_X = 0.0f;

	/** 画面上の上端 Y。 */
	f32 m_Y = 0.0f;

	/** 幅。 */
	f32 m_Width = 0.0f;

	/** 高さ。 */
	f32 m_Height = 0.0f;

	/** 閉じるボタンの左端 X。 */
	f32 m_CloseX = 0.0f;

	/** 閉じるボタンの上端 Y。 */
	f32 m_CloseY = 0.0f;

	/** 閉じるボタンの一辺。 */
	f32 m_CloseSize = 0.0f;

	/** ボタンの段の上端 Y。 */
	f32 m_ButtonY = 0.0f;

	/** ボタンの段の高さ。 */
	f32 m_ButtonHeight = 0.0f;

	/** 積み上がりを滑らかに動かすための、いまの下端 Y。 */
	f32 m_AnimatedBottom = 0.0f;

	/** 縦位置が一度でも決まったか。 */
	bool m_bPlaced = false;
};
