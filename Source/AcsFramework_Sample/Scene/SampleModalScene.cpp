#include "SampleModalScene.h"

#include "AcsFramework_Core/AcsFramework.h"

namespace
{
	/** 下の画面を覆う色 (完全には塗らない。重なっていることが分かるように)。 */
	constexpr FVec4 kBackdropColor{ 0.06f, 0.03f, 0.10f, 0.70f };

	/** 文字色。 */
	constexpr FVec4 kTextColor{ 0.98f, 0.92f, 0.70f, 1.0f };
}


void ASampleModalScene::OnEnter() noexcept
{
	// 渡されたものは、この時点でもう読める。
	if ( const CSampleModalOpen* const Open = TravelContext<CSampleModalOpen>() ) m_OpenCount = Open->OpenCount;
}


void ASampleModalScene::OnUpdate( f32 DeltaSeconds ) noexcept
{
	AScene::OnUpdate( DeltaSeconds );

	if ( CInput::IsKeyPressed( EKey::Enter ) ) Close( true );
	else if ( CInput::IsKeyPressed( EKey::Escape ) ) Close( false );
}


void ASampleModalScene::Close( bool bAccepted ) noexcept
{
	CSceneTravelSubsystem* const Travel = GetSubsystem<CSceneTravelSubsystem>();
	if ( Travel == nullptr ) return;

	// 答えを添えて下ろす。戻り先はこれを OnResume で受け取る。
	TUniquePtr<CSampleModalResult> Result = MakeUnique<CSampleModalResult>();
	Result->bAccepted = bAccepted;
	Result->OpenCount = m_OpenCount;
	Travel->PopScene( Move( Result ), ESceneTransition::Fade );
}


void ASampleModalScene::OnDrawHud( FRenderContext& RenderContext, CSpriteBatch& Batch ) noexcept
{
	const f32 Width = static_cast<f32>( RenderContext.Width() );
	const f32 Height = static_cast<f32>( RenderContext.Height() );

	Batch.DrawRect( 0.0f, 0.0f, Width, Height, kBackdropColor );

	CUiFontSubsystem* const UiFont = GetSubsystem<CUiFontSubsystem>();
	FFont* const Font = UiFont != nullptr ? UiFont->Peek() : nullptr;
	if ( Font == nullptr && !RenderContext.HasFont() ) return;

	const FFont& Used = Font != nullptr ? *Font : RenderContext.GetFont();

	FString Text( "重ねた画面 (ASampleModalScene)" );
	Text.Append( "\n\n下のシーンは畳まれず、止まって残っています。" );
	Text.Append( "\n下ろすと元の続きから始まります。" );
	if ( const CSceneTravelSubsystem* const Travel = GetSubsystem<CSceneTravelSubsystem>() )
	{
		Text.AppendFormat( "\n\nDepth = %u", Travel->GetDepth() );
	}

	Batch.DrawString( Used, Text.Data(), Width * 0.5f - 240.0f, Height * 0.5f - 60.0f, kTextColor );
}
