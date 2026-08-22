// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 3D位置へ文字を1件重ねるときの見た目と表示範囲。 */
struct FWorldLabel3DParams
{
	/** 1件で保持できるUTF-8文字列の最大byte数。 */
	static constexpr usize kMaximumTextBytes = 4096u;

	/** レイヤー追加時に内部へ複製するUTF-8文字列。空文字列は無効。 */
	FStringView Text;

	/** ノード位置または指定位置へ加えるworld空間のずれ。 */
	FVec3 WorldOffset{ 0.0f, 1.8f, 0.0f };

	/** 射影後の画面位置へ加えるpixel単位のずれ。 */
	FVec2 ScreenOffset{ 0.0f, -8.0f };

	/** 文字の線形RGBA色。各成分は0から1。 */
	FVec4 TextColor{ 0.96f, 0.98f, 1.0f, 1.0f };

	/** 文字の背面へ敷く線形RGBA色。各成分は0から1。 */
	FVec4 BackgroundColor{ 0.025f, 0.045f, 0.075f, 0.78f };

	/** カメラからこの距離を超えたラベルを隠すworld単位の上限。 */
	f32 MaximumDistance = 120.0f;

	/** 背景の左右へ足すpixel幅。 */
	f32 HorizontalPadding = 8.0f;

	/** 背景の上下へ足すpixel幅。 */
	f32 VerticalPadding = 4.0f;

	/** trueなら半透明の背景を文字より先に描く。 */
	bool bDrawBackground = true;

	/** trueなら射影点を文字列の横中央に合わせる。 */
	bool bCentered = true;

	/**
	 * 射影とHUD描画へ安全に渡せる値だけか確かめる。
	 *
	 * @return 文字が1から4096byteで、位置、色、距離、余白が有限かつ許容範囲ならtrue。
	 */
	bool IsValid() const noexcept;
};
