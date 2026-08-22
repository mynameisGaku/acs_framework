// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Assets/Image/ImageLibrary.h"
#include "Common/Test/TestHarness.h"

void RunImageLibraryTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CImageLibrary / ACS標準画像形式だけを受け付ける" );

	{
		Harness.Check( CImageLibrary::IsSupported( FStringView( "Textures/Marker.png" ) ), "pngを読める" );
		Harness.Check( CImageLibrary::IsSupported( FStringView( "Textures/Photo.JPG" ) ), "大文字jpgを読める" );
		Harness.Check( CImageLibrary::IsSupported( FStringView( "Sky/Studio.hdr" ) ), "hdrを読める" );
		Harness.Check( CImageLibrary::IsSupported( FStringView( "Mask.psd" ) ), "ACS対応psdを読める" );
		Harness.Check( !CImageLibrary::IsSupported( FStringView( "Textures/Vector.svg" ) ), "svgは対象外" );
		Harness.Check( !CImageLibrary::IsSupported( FStringView( "Textures/Image" ) ), "拡張子なしは対象外" );
		Harness.Check( !CImageLibrary::IsSupported( FStringView() ), "空名は対象外" );
		Harness.Check( !CImageLibrary::IsSupported( FStringView( "folder.png/image" ) ), "途中の拡張子は数えない" );
	}

	Harness.BeginSuite( "CImageLibrary / 未接続を明示的な失敗にする" );

	{
		CImageLibrary Library;
		Harness.Check( !Library.IsBound(), "登録簿を渡す前は未接続" );
		Harness.Check( !Library.Load( FStringView( "circle.png" ) ), "未接続では空を返す" );
	}
}
