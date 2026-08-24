// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Bridge3D/Bridge3DSpawnParams.h"

#include <cmath>


FBridge3DSpawnParams FBridge3DSpawnParams::FromDimensions( f32 InWidth,
	f32 InLength, f32 InRailingHeight, FVec3 InEntranceCenter,
	EBridge3DDirection InDirection ) noexcept
{
	FBridge3DSpawnParams Params;
	Params.Width = InWidth;
	Params.Length = InLength;
	Params.RailingHeight = InRailingHeight;
	Params.EntranceCenter = InEntranceCenter;
	Params.Direction = InDirection;
	return Params;
}


bool FBridge3DSpawnParams::TryBuildParts( FGround3DSpawnParams& OutDeck,
	FFence3DSpawnParams& OutNegativeRailing,
	FFence3DSpawnParams& OutPositiveRailing ) const noexcept
{
	if ( !std::isfinite( Width ) || !std::isfinite( Length )
		|| !std::isfinite( DeckThickness ) || !std::isfinite( RailingHeight )
		|| !std::isfinite( PostThickness ) ) return false;
	if ( Width <= PostThickness * 2.0f || Length <= PostThickness ) return false;

	// 入口から出口へ進む軸と、橋幅の正側へ進む軸。
	FVec3 Forward;
	FVec3 WidthAxis;
	FVec2 DeckSize;
	EFence3DDirection FenceDirection;
	switch ( Direction )
	{
	case EBridge3DDirection::PositiveX:
		Forward = FVec3{ 1.0f, 0.0f, 0.0f };
		WidthAxis = FVec3{ 0.0f, 0.0f, 1.0f };
		DeckSize = FVec2{ Length, Width };
		FenceDirection = EFence3DDirection::PositiveX;
		break;
	case EBridge3DDirection::NegativeX:
		Forward = FVec3{ -1.0f, 0.0f, 0.0f };
		WidthAxis = FVec3{ 0.0f, 0.0f, 1.0f };
		DeckSize = FVec2{ Length, Width };
		FenceDirection = EFence3DDirection::NegativeX;
		break;
	case EBridge3DDirection::PositiveZ:
		Forward = FVec3{ 0.0f, 0.0f, 1.0f };
		WidthAxis = FVec3{ 1.0f, 0.0f, 0.0f };
		DeckSize = FVec2{ Width, Length };
		FenceDirection = EFence3DDirection::PositiveZ;
		break;
	case EBridge3DDirection::NegativeZ:
		Forward = FVec3{ 0.0f, 0.0f, -1.0f };
		WidthAxis = FVec3{ 1.0f, 0.0f, 0.0f };
		DeckSize = FVec2{ Width, Length };
		FenceDirection = EFence3DDirection::NegativeZ;
		break;
	default:
		return false;
	}

	// 支柱中心を床板端から半幅内側へ置くための距離。
	const f32 SideOffset = Width * 0.5f - PostThickness * 0.5f;
	// 始終端の支柱を床板内へ半幅ずつ収めた中心間距離。
	const f32 RailingLength = Length - PostThickness;
	// 床板中心と、入口側支柱中心までの前方向距離。
	const f32 DeckCenterOffset = Length * 0.5f;
	const f32 RailingStartOffset = PostThickness * 0.5f;
	if ( !std::isfinite( SideOffset ) || !std::isfinite( RailingLength )
		|| !std::isfinite( DeckCenterOffset ) || !std::isfinite( RailingStartOffset ) ) return false;

	FGround3DSpawnParams Deck = FGround3DSpawnParams::FromSize(
		DeckSize, EntranceCenter + Forward * DeckCenterOffset );
	Deck.Thickness = DeckThickness;
	Deck.Color = DeckColor;
	Deck.Metallic = DeckMetallic;
	Deck.Roughness = DeckRoughness;
	Deck.bCastsShadow = bDeckCastsShadow;
	Deck.CollisionLayer = CollisionLayer;
	Deck.Name = DeckName;

	FFence3DSpawnParams NegativeRailing = FFence3DSpawnParams::FromDimensions(
		RailingLength, RailingHeight,
		EntranceCenter + Forward * RailingStartOffset - WidthAxis * SideOffset,
		FenceDirection );
	NegativeRailing.MaximumPostSpacing = MaximumPostSpacing;
	NegativeRailing.PostThickness = PostThickness;
	NegativeRailing.RailCount = RailCount;
	NegativeRailing.RailHeight = RailHeight;
	NegativeRailing.RailThickness = RailThickness;
	NegativeRailing.Color = RailingColor;
	NegativeRailing.Metallic = RailingMetallic;
	NegativeRailing.Roughness = RailingRoughness;
	NegativeRailing.bCastsShadow = bRailingsCastShadow;
	NegativeRailing.CollisionLayer = CollisionLayer;
	NegativeRailing.PostName = RailingPostName;
	NegativeRailing.RailName = RailingRailName;

	FFence3DSpawnParams PositiveRailing = NegativeRailing;
	PositiveRailing.StartPostBottomCenter = EntranceCenter
		+ Forward * RailingStartOffset + WidthAxis * SideOffset;

	if ( !Deck.IsValid() || !NegativeRailing.IsValid()
		|| !PositiveRailing.IsValid() ) return false;
	OutDeck = Deck;
	OutNegativeRailing = NegativeRailing;
	OutPositiveRailing = PositiveRailing;
	return true;
}


bool FBridge3DSpawnParams::IsValid() const noexcept
{
	FGround3DSpawnParams Deck;
	FFence3DSpawnParams NegativeRailing;
	FFence3DSpawnParams PositiveRailing;
	return TryBuildParts( Deck, NegativeRailing, PositiveRailing );
}
