// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "StateTreeLinker.h"
#include "StateTreeExecutionContext.h"
#include "AI/MonsterAITypes.h"
#include "STC_StimulusType.generated.h"

class AAIController;

/** Instance data for FSTC_StimulusType. Empty -- condition is stateless. */
USTRUCT()
struct PROJECTWALKINGSIM_API FSTC_StimulusTypeInstanceData
{
	GENERATED_BODY()
};

/**
 * State Tree condition: check the Wendigo's last stimulus type.
 *
 * Reads the SuspicionComponent's LastStimulusType and compares against RequiredType.
 * Enables the State Tree to branch between sight and sound investigation states,
 * allowing different investigation speeds and behaviors per stimulus type.
 *
 * Usage examples in State Tree transitions:
 *   Sight investigation:  RequiredType=Sight, bInvertCondition=false
 *   Sound investigation:  RequiredType=Sound, bInvertCondition=false
 *   Not-sight check:      RequiredType=Sight, bInvertCondition=true
 */
USTRUCT(meta = (DisplayName = "Stimulus Type"))
struct PROJECTWALKINGSIM_API FSTC_StimulusType : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTC_StimulusTypeInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool Link(FStateTreeLinker& Linker) override;

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	/** External data handle for the AI controller. Linked automatically by the StateTreeAIComponentSchema. */
	TStateTreeExternalDataHandle<AAIController> ControllerHandle;

	/** The stimulus type to compare against. */
	UPROPERTY(EditAnywhere, Category = "Stimulus")
	EStimulusType RequiredType = EStimulusType::Sight;

	/**
	 * If true, the condition passes when the stimulus type does NOT match RequiredType.
	 * Useful for "not sight" or "not sound" branches.
	 */
	UPROPERTY(EditAnywhere, Category = "Stimulus")
	bool bInvertCondition = false;
};
