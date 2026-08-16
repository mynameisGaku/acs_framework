// SPDX-License-Identifier: Apache-2.0
#include "Debug/Perf/PerfCategoryPlan.h"

namespace
{
	/** 枠組みが元から測っている場所。ゲーム側の区分はここへ足さない。 */
	struct FDefaultCategory
	{
		const char* Name = nullptr;
		f32 BudgetMilliseconds = 0.0f;
	};

	constexpr FDefaultCategory kFrameworkDefaults[] =
	{
		{ "Scene/Update",   6.0f },
		{ "Scene/Render",   6.0f },
		{ "Sim/Update",     3.0f },
		{ "Assets/Load",    2.0f },
		{ "Audio/Update",   1.0f },
		{ "Debug/Overlay",  1.0f },
	};
}


bool CPerfCategoryPlan::Add( const FString& Category, f32 BudgetMilliseconds, u32 BudgetBytes ) noexcept
{
	const char* const StableName = m_Names.Intern( Category );
	if ( StableName == nullptr ) return false;

	const usize Found = FindIndex( StableName );
	if ( Found < m_Definitions.Num() )
	{
		m_Definitions[Found].BudgetMilliseconds = BudgetMilliseconds;
		m_Definitions[Found].BudgetBytes = BudgetBytes;
		return true;
	}

	FPerfCategoryDefinition Definition;
	Definition.Name = StableName;
	Definition.BudgetMilliseconds = BudgetMilliseconds;
	Definition.BudgetBytes = BudgetBytes;

	if ( !m_Definitions.TryAdd( Definition ) )
	{
		ACS_LOG_WARN( "CPerfCategoryPlan: カテゴリの確保に失敗しました '%s'", StableName );
		return false;
	}

	return true;
}


void CPerfCategoryPlan::AddFrameworkDefaults() noexcept
{
	for ( const FDefaultCategory& Default : kFrameworkDefaults )
	{
		Add( FString( Default.Name ), Default.BudgetMilliseconds, 0u );
	}
}


void CPerfCategoryPlan::ApplyTo( CPerfBudget& Budget ) const noexcept
{
	for ( usize Index = 0u; Index < m_Definitions.Num(); ++Index )
	{
		const FPerfCategoryDefinition& Definition = m_Definitions[Index];
		if ( !Definition.IsValid() ) continue;

		Budget.DefineCategory( Definition.Name, Definition.BudgetMilliseconds, Definition.BudgetBytes );
	}
}


usize CPerfCategoryPlan::FindIndex( const char* StableName ) const noexcept
{
	for ( usize Index = 0u; Index < m_Definitions.Num(); ++Index )
	{
		if ( m_Definitions[Index].Name == StableName ) return Index;
	}

	return m_Definitions.Num();
}
