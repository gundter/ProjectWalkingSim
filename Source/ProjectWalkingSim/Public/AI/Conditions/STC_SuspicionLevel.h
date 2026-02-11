// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "StateTreeLinker.h"
#include "StateTreeExecutionContext.h"
#include "AI/MonsterAITypes.h"
#include "STC_SuspicionLevel.generated.h"

class AAIController;

/** Instance data for FSTC_SuspicionLevel. Empty -- condition is stateless. */
USTRUCT()
struct PROJECTWALKINGSIM_API FSTC_SuspicionLevelInstanceData
{
	GENERATED_BODY()
};

/**
 * State Tree condition: check the Wendigo's current alert level against a threshold.
 *
 * Used in State Tree transitions to switch between patrol, suspicious, and alert states.
 * The condition is lightweight -- it performs a single enum comparison with no allocations.
 *
 * EAlertLevel is ordered: Patrol(0) < Suspicious(1) < Alert(2), so >= comparison works
 * naturally for threshold checks.
 *
 * Usage examples in State Tree transitions:
 *   Patrol -> Suspicious:  RequiredLevel=Suspicious, bInvertCondition=false (>= Suspicious)
 *   Suspicious -> Alert:   RequiredLevel=Alert, bInvertCondition=false (>= Alert)
 *   Suspicious -> Patrol:  RequiredLevel=Suspicious, bInvertCondition=true (< Suspicious)
 *   Alert -> Patrol:       RequiredLevel=Suspicious, bInvertCondition=true (< Suspicious)
 */
USTRUCT(meta = (DisplayName = "Check Suspicion Level"))
struct PROJECTWALKINGSIM_API FSTC_SuspicionLevel : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTC_SuspicionLevelInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool Link(FStateTreeLinker& Linker) override;

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	/** External data handle for the AI controller. Linked automatically by the StateTreeAIComponentSchema. */
	TStateTreeExternalDataHandle<AAIController> ControllerHandle;

	/** The alert level the Wendigo must meet or exceed for the condition to pass. */
	UPROPERTY(EditAnywhere, Category = "Suspicion")
	EAlertLevel RequiredLevel = EAlertLevel::Suspicious;

	/**
	 * If true, the condition passes when the alert level is BELOW RequiredLevel.
	 * Useful for "return to patrol" transitions.
	 */
	UPROPERTY(EditAnywhere, Category = "Suspicion")
	bool bInvertCondition = false;
};
