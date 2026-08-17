// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Assets/Model3D/AssetRoot.h"
#include "AcsFramework_Core/Assets/Model3D/ModelLibrary.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"
#include "Common/Test/TestHarness.h"

void RunModelLibraryTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CModelLibrary / 読める形式を見分ける" );

	{
		Harness.Check( CModelLibrary::IsSupported( FStringView( "Robot.fbx" ) ), "fbx は読める" );
		Harness.Check( CModelLibrary::IsSupported( FStringView( "Robot.FBX" ) ), "大文字でも読める" );
		Harness.Check( CModelLibrary::IsSupported( FStringView( "a/b/Robot.gltf" ) ), "gltf は読める" );
		Harness.Check( CModelLibrary::IsSupported( FStringView( "Robot.glb" ) ), "glb は読める" );
		Harness.Check( CModelLibrary::IsSupported( FStringView( "Robot.obj" ) ), "obj は読める" );
	}

	{
		// ここで弾いておかないと、«読めないのか、置き場に無いのか» が混ざる。
		Harness.Check( !CModelLibrary::IsSupported( FStringView( "Robot.blend" ) ), "blend は読めない" );
		Harness.Check( !CModelLibrary::IsSupported( FStringView( "Robot" ) ), "拡張子が無いものは読めない" );
		Harness.Check( !CModelLibrary::IsSupported( FStringView( "" ) ), "空は読めない" );
		Harness.Check( !CModelLibrary::IsSupported( FStringView( "fbx" ) ), "拡張子だけの名前は読めない" );
		Harness.Check( !CModelLibrary::IsSupported( FStringView( "a.fbx/b" ) ), "途中の .fbx は数えない" );
	}

	Harness.BeginSuite( "CAssetRoot / 置き場の外を指させない" );

	CAssetRoot::Override( FStringView( "C:\\test\\Assets" ) );

	{
		FString Full;
		Harness.Check( CAssetRoot::Resolve( FStringView( "Models/Robot.fbx" ), Full ), "相対名は直せる" );
		Harness.Check( Full.Size() > 0u && Full[0] == 'C', "置き場から始まる" );
	}

	{
		// 置き場の外を指せると、配る段になって «自分の機械にしか無いファイル» を掴んで
		// いたことに気付く。
		FString Full;
		Harness.Check( !CAssetRoot::Resolve( FStringView( "../secret.fbx" ), Full ), ".. は拒む" );
		Harness.Check( !CAssetRoot::Resolve( FStringView( "a/../../b.fbx" ), Full ), "途中の .. も拒む" );
		Harness.Check( !CAssetRoot::Resolve( FStringView( "C:\\other\\Robot.fbx" ), Full ), "絶対パスは拒む" );
		Harness.Check( !CAssetRoot::Resolve( FStringView( "\\Robot.fbx" ), Full ), "根から始まる名前は拒む" );
		Harness.Check( !CAssetRoot::Resolve( FStringView( "" ), Full ), "空は拒む" );
	}

	Harness.BeginSuite( "CModelLibrary / 渡す前に呼んでも落ちない" );

	{
		CModelLibrary Library;
		Harness.Check( !Library.IsBound(), "渡す前は使えない" );
		Harness.Check( !Library.Load( FStringView( "Models/Robot.fbx" ) ), "渡す前の読み込みは空を返す" );
	}

	Harness.BeginSuite( "CModel3DSpawner / 読めないものは置かない" );

	{
		// 置いてから «出ない» と悩むより、置かない方が早く気付ける。
		CModelLibrary Library;
		TObjectPtr<ANode> Root = NewObject<ANode>();

		FModel3DSpawnParams Params =
			FModel3DSpawnParams::FromMesh( FStringView( "Models/NotHere.fbx" ), FVec3{ 0.0f, 0.0f, 0.0f } );

		Harness.Check( CModel3DSpawner::SpawnInto( *Root, Params, Library ) == nullptr,
			"読めなければ nullptr" );
		Harness.CheckEqualU64( Root->ChildCount(), 0u, "読めなければ木にも足さない" );
	}

	{
		// プリミティブは読むものが無いので、置き場が無くても置ける。
		CModelLibrary Library;
		TObjectPtr<ANode> Root = NewObject<ANode>();

		const FModel3DSpawnParams Params =
			FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Cube, FVec3{ 0.0f, 0.0f, 0.0f } );

		Harness.Check( CModel3DSpawner::SpawnInto( *Root, Params, Library ) != nullptr,
			"プリミティブは置き場に関係なく置ける" );
	}

	// 他のテストへ影響させない。
	CAssetRoot::Override( FStringView( "" ) );
}
