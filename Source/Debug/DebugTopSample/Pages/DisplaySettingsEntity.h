#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElements.h"
#include "Debug/DebugTop/Page/DebugTopEntity.h"

using namespace acs;

// 画面を分けるほどでもない小さなまとまり。親ページへその場で展開される。

/**
 * 画面を分けるほどでもない小さなまとまり。
 *
 * @details 親ページへ EDebugTopAttachMode::Inline で組み込むと、その場で展開できる 1 行になる。
 */
class ADisplaySettingsEntity : public ADebugTopEntity
{
public:
	using ADebugTopEntity::ADebugTopEntity;

protected:
	void OnBuild() noexcept override
	{
		Add<CDebugTopElementBool>( "BoolValue", true );

		TArray<FString> QualityOptions;
		QualityOptions.Add( FString( "Low" ) );
		QualityOptions.Add( FString( "Middle" ) );
		QualityOptions.Add( FString( "High" ) );
		Add<CDebugTopElementEnum>( "EnumValue", Move( QualityOptions ), 1 )
			->SetValueColor( FVec4{ 0.55f, 0.95f, 0.60f, 1.0f } );
	}
};
