// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_SkeletalControlBase.h"
#include "RGBRokokoAnimNode.h"
#include "FRGBRokokoAnimGraphNode.generated.h"

/**
 * 
 */
UCLASS()
class BLENDERUELIVELINK_API UFRGBRokokoAnimGraphNode : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category = Settings)
	FRGBRokokoAnimNode Node;

	// UEdGraphNode interface
 // mouse over Node prompt text
	virtual FText GetTooltipText() const override;
	// Node name text	
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	// End of UEdGraphNode interface

protected:
	// UAnimGraphNode_SkeletalControlBase interface
 // returns the controller description
	virtual FText GetControllerDescription() const override;
	// returns AnimNode cited
	virtual const FAnimNode_SkeletalControlBase* GetNode() const override { return &Node; }

};
