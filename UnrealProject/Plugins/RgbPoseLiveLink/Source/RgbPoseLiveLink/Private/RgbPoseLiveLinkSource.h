// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ILiveLinkSource.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "IMessageContext.h"
#include "Chaos/AABB.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Roles/LiveLinkAnimationTypes.h"

class FRunnableThread;
class FSocket;
class ILiveLinkClient;
class ISocketSubsystem;
class PoseFrame;

//TMap<int32, FString> BoneMap;

class RGBPOSELIVELINK_API FRgbPoseLiveLinkSource : public ILiveLinkSource, public FRunnable
{
public:
	bool firstTime; 
	FRgbPoseLiveLinkSource(FIPv4Endpoint Endpoint);

	virtual ~FRgbPoseLiveLinkSource();

	// Begin ILiveLinkSource Interface
	
	virtual void ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) override;

	virtual bool IsSourceStillValid() const override;

	virtual bool RequestSourceShutdown() override;

	virtual FText GetSourceType() const override { return SourceType; };
	virtual FText GetSourceMachineName() const override { return SourceMachineName; }
	virtual FText GetSourceStatus() const override { return SourceStatus; }

	// End ILiveLinkSource Interface

	// Begin FRunnable Interface

	virtual bool Init() override { return true; }
	virtual uint32 Run() override;
	void Start();
	virtual void Stop() override;
	virtual void Exit() override { }

	// End FRunnable Interface

	void HandleReceivedData(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData);
	void HandleReceivedData2(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData);
	FTransform CalculateLookRotaion(FVector Source, FVector Target);
	FVector TriangleNormal(FVector a, FVector b, FVector c);

private:

	ILiveLinkClient* Client;

	// Our identifier in LiveLink
	FGuid SourceGuid;

	FMessageAddress ConnectionAddress;

	FText SourceType;
	FText SourceMachineName;
	FText SourceStatus;

	FIPv4Endpoint DeviceEndpoint;
	// Socket to receive data on
	FSocket* Socket;

	// Subsystem associated to Socket
	ISocketSubsystem* SocketSubsystem;

	// Threadsafe Bool for terminating the main thread loop
	FThreadSafeBool Stopping;

	// Thread to run socket operations on
	FRunnableThread* Thread;

	// Name of the sockets thread
	FString ThreadName;

	// Time to wait between attempted receives
	FTimespan WaitTime;

	// List of subjects we've already encountered
	TArray<FString>Subname_list;

	// Buffer to receive socket data into
	TArray<uint8> RecvBuffer;

	// Check if static data is setup

	// timeStamp for measuring FPS
	double LastFrameTime = 0;

	void AddAnimFrameData(FVector* inVector, FLiveLinkAnimationFrameData& animFrameData);
	void AddAnimFrameData(FQuat* inQuat, FLiveLinkAnimationFrameData& animFrameData);

	void AddStaticSkeletonData(FName subjectName, TMap<FString, FTransform> BoneName_TransformMap);

	void CreateJoint(TArray<FTransform>& transforms, bool hasParent, FTransform ParentTransform, FVector ParentPosition, FVector PointPosition);
};
