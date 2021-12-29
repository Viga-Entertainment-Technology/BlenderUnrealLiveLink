// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
#include "RgbPoseLiveLinkSourceFactory.h"
#include "RgbPoseLiveLinkSource.h"
#include "SRgbPoseLiveLinkSourceFactory.h"

#define LOCTEXT_NAMESPACE "RgbPoseLiveLinkSourceFactory"

FText URgbPoseLiveLinkSourceFactory::GetSourceDisplayName() const
{
	return LOCTEXT("SourceDisplayName", "RgbPose LiveLink");
}

FText URgbPoseLiveLinkSourceFactory::GetSourceTooltip() const
{
	return LOCTEXT("SourceTooltip", "Creates a connection to a RgbPose UDP Stream");
}

TSharedPtr<SWidget> URgbPoseLiveLinkSourceFactory::BuildCreationPanel(FOnLiveLinkSourceCreated InOnLiveLinkSourceCreated) const
{
	return SNew(SRgbPoseLiveLinkSourceFactory)
        .OnOkClicked(SRgbPoseLiveLinkSourceFactory::FOnOkClicked::CreateUObject(this, &URgbPoseLiveLinkSourceFactory::OnOkClicked, InOnLiveLinkSourceCreated));
}

TSharedPtr<ILiveLinkSource> URgbPoseLiveLinkSourceFactory::CreateSource(const FString& InConnectionString) const
{
	FIPv4Endpoint DeviceEndPoint;
	if (!FIPv4Endpoint::Parse(InConnectionString, DeviceEndPoint))
	{
		return TSharedPtr<ILiveLinkSource>();
	}

	return MakeShared<FRgbPoseLiveLinkSource>(DeviceEndPoint);
}

void URgbPoseLiveLinkSourceFactory::OnOkClicked(FIPv4Endpoint InEndpoint, FOnLiveLinkSourceCreated InOnLiveLinkSourceCreated) const
{
	InOnLiveLinkSourceCreated.ExecuteIfBound(MakeShared<FRgbPoseLiveLinkSource>(InEndpoint), InEndpoint.ToString());
}

#undef LOCTEXT_NAMESPACE