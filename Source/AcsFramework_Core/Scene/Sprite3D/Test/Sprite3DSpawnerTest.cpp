// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Sprite3D/Sprite3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 1画素の有効なRGBA画像を作る。 */
	TSharedPtr<AAsset> MakeImage() noexcept
	{
		TArray<byte> Pixels;
		Pixels.SetNum( 4u );
		if ( Pixels.Num() != 4u ) return TSharedPtr<AAsset>();
		Pixels[0] = 80u;
		Pixels[1] = 180u;
		Pixels[2] = 255u;
		Pixels[3] = 255u;
		return MakeShared<AImageAsset>( 1u, 1u, EPixelFormat::R8G8B8A8, Move( Pixels ) );
	}

	/** ノードに付いた3Dスプライトを返す。 */
	const ASprite3DComponent* SpriteOf( ANode* Node ) noexcept
	{
		return Node != nullptr ? Node->GetComponent<ASprite3DComponent>() : nullptr;
	}
}


void RunSprite3DSpawnerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FSprite3DSpawnParams / 画像名と見える大きさを検証する" );

	{
		const FSprite3DSpawnParams Empty;
		Harness.Check( !Empty.IsValid(), "画像源が無ければ置けない" );

		const FSprite3DSpawnParams FromPath = FSprite3DSpawnParams::FromImage(
			FStringView( "circle.png" ), FVec3{ 1.0f, 2.0f, 3.0f }, FVec2{ 0.8f, 0.6f } );
		Harness.Check( FromPath.IsValid(), "画像名があれば読込前指定として有効" );
		Harness.Check( !FromPath.IsReady(), "読込前は直接配置できない" );
		Harness.CheckEqualF32( FromPath.Size.x, 0.8f, "幅を保持する" );

		FSprite3DSpawnParams Ready = FromPath;
		Ready.ImageAsset = MakeImage();
		Harness.Check( Ready.IsReady(), "画像を渡すと直接配置できる" );

		FSprite3DSpawnParams ZeroSize = Ready;
		ZeroSize.Size.x = 0.0f;
		Harness.Check( !ZeroSize.IsValid(), "幅0は見えないため拒む" );

		FSprite3DSpawnParams Mirrored = Ready;
		Mirrored.Size.x = -0.8f;
		Harness.Check( Mirrored.IsValid(), "負の幅は左右反転として通す" );

		FSprite3DSpawnParams NotFinite = Ready;
		NotFinite.Position.z = std::numeric_limits<f32>::infinity();
		Harness.Check( !NotFinite.IsValid(), "無限位置は拒む" );

		FSprite3DSpawnParams BrokenRotation = Ready;
		BrokenRotation.RotationDeg.y = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !BrokenRotation.IsValid(), "非数回転は拒む" );

		FSprite3DSpawnParams WrongAsset = FromPath;
		WrongAsset.ImageAsset = MakeShared<ABinaryAsset>();
		Harness.Check( !WrongAsset.IsValid(), "画像以外のアセットは拒む" );
	}

	Harness.BeginSuite( "CSprite3DSpawner / 読込済み画像を親へ置く" );

	{
		TObjectPtr<ANode> Parent = NewObject<ANode>();
		FSprite3DSpawnParams Params = FSprite3DSpawnParams::FromImage(
			FStringView( "circle.png" ), FVec3{ 1.0f, 2.0f, 3.0f }, FVec2{ 0.8f, 0.6f } );
		Params.ImageAsset = MakeImage();
		Params.RotationDeg = FVec3{ 0.0f, 90.0f, 0.0f };
		Params.Name = FStringView( "Marker" );

		ANode* const Placed = CSprite3DSpawner::SpawnInto( *Parent, Params );
		Harness.Check( Placed != nullptr, "画像板を置ける" );
		Harness.CheckEqualU64( Parent->ChildCount(), 1u, "親へ1ノードだけ足す" );

		if ( Placed != nullptr )
		{
			Harness.Check( Placed->Name() == FStringView( "Marker" ), "名前を入れる" );
			Harness.CheckEqualF32( Placed->Local().position.y, 2.0f, "位置を入れる" );
			Harness.CheckEqualF32( Placed->Local().scale.x, 0.8f, "幅をX倍率へ入れる" );
			Harness.CheckEqualF32( Placed->Local().scale.y, 0.6f, "高さをY倍率へ入れる" );
			Harness.CheckEqualF32( Placed->Local().scale.z, 1.0f, "板の奥行倍率は1に保つ" );
			Harness.Check( Placed->Local().rotation.y > 0.69f && Placed->Local().rotation.y < 0.72f,
				"回転を度として扱う" );
		}

		const ASprite3DComponent* const Sprite = SpriteOf( Placed );
		Harness.Check( Sprite != nullptr, "3Dスプライト部品を付ける" );
		if ( Sprite != nullptr )
		{
			Harness.Check( Sprite->TexturePath() == FStringView( "circle.png" ), "画像名を部品へコピーする" );
			Harness.Check( Sprite->HasImageAsset(), "画像の共有所有権を部品へ渡す" );
			Harness.Check( Sprite->ImageAsset().Get() == Params.ImageAsset.Get(), "同じ画像を共有する" );
		}
	}

	Harness.BeginSuite( "CSprite3DSpawner / シーンへ識別子付きで置く" );

	{
		CSceneNodeGraph Graph;
		FSprite3DSpawnParams Params = FSprite3DSpawnParams::FromImage(
			FStringView( "circle.png" ), FVec3{}, FVec2{ 0.5f, 0.5f } );
		Params.ImageAsset = MakeImage();

		ANode* const Placed = CSprite3DSpawner::SpawnInto( Graph, Params );
		Harness.Check( Placed != nullptr, "シーンへ置ける" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 2u, "ルートと画像板を登録する" );
		Harness.Check( Placed != nullptr && Placed->Id().IsValid(), "有効な識別子を付ける" );
		Harness.Check( Placed != nullptr && Graph.Get( Placed->Id() ) == Placed, "識別子から同じノードを取れる" );
	}

	Harness.BeginSuite( "CSprite3DSpawner / 半端なノードを残さない" );

	{
		TObjectPtr<ANode> Parent = NewObject<ANode>();
		const FSprite3DSpawnParams PathOnly = FSprite3DSpawnParams::FromImage(
			FStringView( "circle.png" ), FVec3{} );
		Harness.Check( CSprite3DSpawner::SpawnInto( *Parent, PathOnly ) == nullptr,
			"読込済み画像なしの直接配置は失敗する" );
		Harness.CheckEqualU64( Parent->ChildCount(), 0u, "失敗時は親を変えない" );

		CImageLibrary UnboundLibrary;
		Harness.Check( CSprite3DSpawner::SpawnInto( *Parent, PathOnly, UnboundLibrary ) == nullptr,
			"画像を読めなければ失敗する" );
		Harness.CheckEqualU64( Parent->ChildCount(), 0u, "読込失敗でも親を変えない" );

		FSprite3DSpawnParams Ready = PathOnly;
		Ready.ImageAsset = MakeImage();
		Harness.Check( CSprite3DSpawner::SpawnInto( *Parent, Ready, UnboundLibrary ) != nullptr,
			"読込済みなら未接続ライブラリへ触れず置ける" );
		Harness.CheckEqualU64( Parent->ChildCount(), 1u, "成功時だけ1ノード足す" );
	}
}
