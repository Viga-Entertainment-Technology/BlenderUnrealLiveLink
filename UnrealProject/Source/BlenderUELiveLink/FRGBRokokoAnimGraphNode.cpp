// Fill out your copyright notice in the Description page of Project Settings.


#include "FRGBRokokoAnimGraphNode.h"


#define LOCTEXT_NAMESPACE "A3Nodes"
UFRGBRokokoAnimGraphNode::UFRGBRokokoAnimGraphNode(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

FText UFRGBRokokoAnimGraphNode::GetTooltipText() const
{
	return LOCTEXT("FRGBRokokoAnimGraphNode_Tooltip", "Well no description RGB");
}

FText UFRGBRokokoAnimGraphNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("FRGBRokokoAnimGraphNode", "RGB Rokooko Pose");
}

FText UFRGBRokokoAnimGraphNode::GetControllerDescription() const
{
	return LOCTEXT("CopyParentBone", "RGB MOcal new Graph node");
}
#undef LOCTEXT_NAMESPACE
