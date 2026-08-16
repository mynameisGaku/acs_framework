#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElements.h"
#include "Debug/DebugTop/Page/DebugTopEntity.h"

using namespace acs;

// 行数の多いページ。スクロールと「まだ下にある」印の確認に使う。

/** スクロールの確認用に積む行数。 */
constexpr i32 kLongListRowCount = 40;


/**
 * 行数の多いページ (スクロールと「まだ下にある」印の確認用)。
 */
class ALongListEntity : public ADebugTopEntity
{
public:
	using ADebugTopEntity::ADebugTopEntity;

protected:
	void OnBuild() noexcept override
	{
		SetHeader( "Scroll check" );
		SetDescription( FString( "上下キーでカーソルを動かすと、端の手前でスクロールが始まります\n" "続きがある間は一覧の中央下に印が出ます" ) );

		for ( i32 Index = 0; Index < kLongListRowCount; ++Index )
		{
			FString Label;
			Label.AppendFormat( "Row %02d", Index );

			CDebugTopElementInt* const Row = Add<CDebugTopElementInt>( Label, Index, 0, 99 );

			// 行ごとの説明文。カーソルを合わせている間だけ右下に出る。
			FString Description;
			Description.AppendFormat( "%d 番目の行です\n左右キーか矢印クリックで値を変えられます", Index );
			Row->SetDescription( Description );
		}
	}
};
