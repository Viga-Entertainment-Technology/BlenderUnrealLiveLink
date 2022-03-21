
// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "RgbPoseLiveLinkSource.h"

#include <string>

#include "ILiveLinkClient.h"
#include "LiveLinkTypes.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "Roles/LiveLinkTransformRole.h"
#include "Roles/LiveLinkTransformTypes.h"

#include "Async/Async.h"
#include "Common/UdpSocketBuilder.h"
#include "HAL/RunnableThread.h"
#include "Json.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "PoseFrame.h"
#include "Containers/UnrealString.h"
#include "Misc/Char.h"
#include "Containers/Array.h"

#define LOCTEXT_NAMESPACE "RgbPoseLiveLinkSource"

#define RECV_BUFFER_SIZE 1024 * 1024

FRgbPoseLiveLinkSource::FRgbPoseLiveLinkSource(FIPv4Endpoint InEndpoint)
: Socket(nullptr)
, Stopping(false)
, Thread(nullptr)
, WaitTime(FTimespan::FromMilliseconds(100))
{
	// defaults
	DeviceEndpoint = InEndpoint;

	SourceStatus = LOCTEXT("SourceStatus_DeviceNotFound", "Device Not Found");
	SourceType = LOCTEXT("RgbPoseLiveLinkSourceType", "RgbPose LiveLink");
	SourceMachineName = LOCTEXT("RgbPoseLiveLinkSourceMachineName", "localhost");

	//setup socket
	if (DeviceEndpoint.Address.IsMulticastAddress())
	{
		Socket = FUdpSocketBuilder(TEXT("RgbPoseSOCKET"))
			.AsNonBlocking()
			.AsReusable()
			.BoundToPort(DeviceEndpoint.Port)
			.WithReceiveBufferSize(RECV_BUFFER_SIZE)

			.BoundToAddress(FIPv4Address::Any)
			.JoinedToGroup(DeviceEndpoint.Address)
			.WithMulticastLoopback()
			.WithMulticastTtl(2);
					
	}
	else
	{
		Socket = FUdpSocketBuilder(TEXT("RgbPoseSOCKET"))
			.AsNonBlocking()
			.AsReusable()
			.BoundToAddress(DeviceEndpoint.Address)
			.BoundToPort(DeviceEndpoint.Port)
			.WithReceiveBufferSize(RECV_BUFFER_SIZE);
	}

	RecvBuffer.SetNumUninitialized(RECV_BUFFER_SIZE);

	if ((Socket != nullptr) && (Socket->GetSocketType() == SOCKTYPE_Datagram))
	{
		SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

		Start();

		SourceStatus = LOCTEXT("SourceStatus_Receiving", "Receiving");
	}
}

FRgbPoseLiveLinkSource::~FRgbPoseLiveLinkSource()
{
	Stop();
	if (Thread != nullptr)
	{
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}
	if (Socket != nullptr)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
	}
}

void FRgbPoseLiveLinkSource::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid)
{
	Client = InClient;
	SourceGuid = InSourceGuid;
}

bool FRgbPoseLiveLinkSource::IsSourceStillValid() const
{
	// Source is valid if we have a valid thread and socket
	bool bIsSourceValid = !Stopping && Thread != nullptr && Socket != nullptr;
	return bIsSourceValid;
}

bool FRgbPoseLiveLinkSource::RequestSourceShutdown()
{
	Stop();

	return true;
}
// FRunnable interface

void FRgbPoseLiveLinkSource::Start()
{
	/*BoneMap.Add(0, TEXT("rShldrBend"));
	BoneMap.Add(1, TEXT("rForearmBend"));
	BoneMap.Add(2, TEXT("rHand"));
	BoneMap.Add(3, TEXT("rThumb2"));
	BoneMap.Add(4, TEXT("rMid1"));
	BoneMap.Add(5, TEXT("lShldrBend"));
	BoneMap.Add(6, TEXT("lForearmBend"));
	BoneMap.Add(7, TEXT("lHand"));
	BoneMap.Add(8, TEXT("lThumb2"));
	BoneMap.Add(9, TEXT("lMid1"));
	BoneMap.Add(10, TEXT("lEar"));
	BoneMap.Add(11, TEXT("lEye"));
	BoneMap.Add(12, TEXT("rEar"));
	BoneMap.Add(13, TEXT("rEye"));
	BoneMap.Add(14, TEXT("Nose"));
	BoneMap.Add(15, TEXT("rShin"));
	BoneMap.Add(16, TEXT("rFoot"));
	BoneMap.Add(17, TEXT("lToe"));
	BoneMap.Add(18, TEXT("abdomenUpper"));
	BoneMap.Add(19, TEXT("hip"));
	BoneMap.Add(20, TEXT("head"));
	BoneMap.Add(21, TEXT("neck"));
	BoneMap.Add(22, TEXT("spine"));*/
	firstTime = true; 
	ThreadName = "RgbPose UDP Receiver ";
	ThreadName.AppendInt(FAsyncThreadIndex::GetNext());
	
	Thread = FRunnableThread::Create(this, *ThreadName, 128 * 1024, TPri_AboveNormal, FPlatformAffinity::GetPoolThreadMask());
}

void FRgbPoseLiveLinkSource::Stop()
{
	Stopping = true;
}

uint32 FRgbPoseLiveLinkSource::Run()
{
	TSharedRef<FInternetAddr> Sender = SocketSubsystem->CreateInternetAddr();
	
	while (!Stopping)
	{
		if (Socket->Wait(ESocketWaitConditions::WaitForRead, WaitTime))
		{
			uint32 Size;

			while (Socket->HasPendingData(Size))
			{
				int32 Read = 0;

				if (Socket->RecvFrom(RecvBuffer.GetData(), RecvBuffer.Num(), Read, *Sender))
				{
					if (Read > 0)
					{
						TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData = MakeShareable(new TArray<uint8>());
						ReceivedData->SetNumUninitialized(Read);
						memcpy(ReceivedData->GetData(), RecvBuffer.GetData(), Read);
						AsyncTask(ENamedThreads::GameThread, [this, ReceivedData]() { HandleReceivedData2(ReceivedData); });
					}
				}
			}
		}
	}
	return 0;
}

FVector FRgbPoseLiveLinkSource::TriangleNormal(FVector a, FVector b, FVector c)
{
	FVector d1 = a - b;
	FVector d2 = a - c;

	FVector dd = FVector::CrossProduct(d1, d2);
	dd.Normalize();
	return dd;
}

void FRgbPoseLiveLinkSource::HandleReceivedData2(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData)
{
	/// CONVERTNG TO STRING
	FString recvedString;
	int32 Read = ReceivedData->Num();
	recvedString.Empty(ReceivedData->Num());
	for (uint8& Byte : *ReceivedData.Get())
	{
		recvedString += TCHAR(Byte);
	}
	///		CONVERTING TO POSE FRAME MAP ( BONENAME -> TRANSFORMS)
	TArray<FString> PoseMessageArray;
	recvedString.ParseIntoArray(PoseMessageArray, TEXT("||"), false);
	PoseFrame poseFrame = PoseFrame(PoseMessageArray);
	
	///		LIVE LINK SUBJECT NAME
	FString Subname;
	Subname = poseFrame.Subjectname;
	FName SubjectName = FName(*Subname);

	if (Subname_list.Num()==0)
	{
		FLiveLinkSubjectPreset Preset;
		Preset.Key = FLiveLinkSubjectKey(SourceGuid, SubjectName);
		Client->CreateSubject(Preset);
		Subname_list.Push(poseFrame.Subjectname);
	}
		if (!Subname_list.Contains(poseFrame.Subjectname))
		{
			FLiveLinkSubjectPreset Preset;
			Preset.Key = FLiveLinkSubjectKey(SourceGuid, SubjectName);
			Client->CreateSubject(Preset);
			Subname_list.Push(poseFrame.Subjectname);
		}
		///		CREATING FRAME DATA TO SEND 
		Client->SetSubjectEnabled(FLiveLinkSubjectKey(SourceGuid, SubjectName),true);
		FTimer timer;
		FLiveLinkFrameDataStruct FrameData1(FLiveLinkAnimationFrameData::StaticStruct());
		FLiveLinkAnimationFrameData& AnimFrameData = *FrameData1.Cast<FLiveLinkAnimationFrameData>();
		AnimFrameData.WorldTime = FLiveLinkWorldTime((double)(timer.GetCurrentTime()));

		///		DEFINING SKELETON STRUCTURE DATA 
			AddStaticSkeletonData(SubjectName, poseFrame.BoneName_TransformMap);
		///		SENDING ACTUAL TRANSFORMS TO ANIM FRAME DATA ACCORDING TO THE SKELETON STRUCTURE DEFINED 
		int count = 0;
		TArray<FTransform> transforms;
		transforms.Reset(poseFrame.BoneName_TransformMap.Num());
		int32 transformIndex = transforms.AddUninitialized(poseFrame.BoneName_TransformMap.Num());
		bool first = true;

		for (const TPair<FString, FTransform>& pair : poseFrame.BoneName_TransformMap)
		{
			{
				transforms[count] = pair.Value;
			}
			count++;
		}

		AnimFrameData.Transforms.Append(transforms);
		Client->PushSubjectFrameData_AnyThread(FLiveLinkSubjectKey(SourceGuid, SubjectName), MoveTemp(FrameData1));
}

FTransform FRgbPoseLiveLinkSource::CalculateLookRotaion(FVector Source, FVector Target)
{
	FVector newForward = Target - Source;
	newForward.Normalize();

	const FVector WorldUp(0.0f, 0.0f, 1.0f);

	FVector newRight = FVector::CrossProduct(newForward, WorldUp);
	FVector newUp = FVector::CrossProduct(newRight, newForward);

	return FTransform(newForward, newRight, newUp, Source);
}

void FRgbPoseLiveLinkSource::CreateJoint(TArray<FTransform>& transforms, bool hasParent, FTransform ParentTransform, FVector ParentPosition, FVector PointPosition)
{
	int32 transformIndex = transforms.AddUninitialized(1);
	if (!hasParent) 
	{
		transforms[transformIndex].SetLocation(PointPosition);
		transforms[transformIndex].SetRotation(FQuat::Identity);
		transforms[transformIndex].SetScale3D(FVector(1, 1, 1));
	}
	else
	{
		ParentTransform.GetRotation().Inverse() * (PointPosition - ParentPosition);
	}
	transforms[transformIndex].SetScale3D(FVector(1, 1, 1));
}

//void FRgbPoseLiveLinkSource::HandleReceivedData(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData)
//{
//	FString recvedString;
//	int32 Read = ReceivedData->Num();
//	recvedString.Empty(ReceivedData->Num());
//	for (uint8& Byte : *ReceivedData.Get())
//	{
//		recvedString += TCHAR(Byte);
//	}
//	// UE_LOG(LogTemp, Warning, TEXT("num bytes = %i,    %s"), Read, *recvedString);
//	TArray<FString> PoseMessageArray;
//	recvedString.ParseIntoArray(PoseMessageArray, TEXT("|"), false);
//	PoseMessageArray.RemoveAt(PoseMessageArray.Num() - 1);
//
//	// static FLiveLinkFrameData LiveLinkFrame;
//	FName SubjectName = FName(TEXT("RgbPose"));
//
//	FLiveLinkSubjectKey Key = FLiveLinkSubjectKey(SourceGuid, SubjectName);
//	double currentTime = FPlatformTime::Seconds();
//	//UE_LOG(LogTemp, Warning, TEXT("%f"), 1.0 / (currentTime - LastFrameTime));
//	LastFrameTime = currentTime;
//	
//	FTimer timer;
//	FLiveLinkFrameDataStruct FrameData(FLiveLinkAnimationFrameData::StaticStruct());
//	FLiveLinkAnimationFrameData& AnimFrameData = *FrameData.Cast<FLiveLinkAnimationFrameData>();
//	AnimFrameData.WorldTime = FLiveLinkWorldTime(currentTime);
//
//	
//	if (true)
//	{
//		FLiveLinkStaticDataStruct StaticData(FLiveLinkSkeletonStaticData::StaticStruct());
//		FLiveLinkSkeletonStaticData* SkeletonData = StaticData.Cast<FLiveLinkSkeletonStaticData>();	
//		SkeletonData->PropertyNames.Add("Cube_X");
//		SkeletonData->PropertyNames.Add("Cube_Y");
//		SkeletonData->PropertyNames.Add("Cube_Z");
//		SkeletonData->PropertyNames.Add("Cube_RotX");
//		SkeletonData->PropertyNames.Add("Cube_RotY");
//		SkeletonData->PropertyNames.Add("Cube_RotZ");
//		SkeletonData->PropertyNames.Add("Cube_RotW");
//
//		Client->PushSubjectStaticData_AnyThread(Key, ULiveLinkAnimationRole::StaticClass(), MoveTemp(StaticData));
//		PoseLabelsLoaded = true;
//	}
//	PoseFrame poseFrame = PoseFrame(PoseMessageArray);
//
//	AddAnimFrameData(poseFrame.PoseFrameMap.Find(FString("Cube")), AnimFrameData);
//	AddAnimFrameData(poseFrame.PoseFrameRot.Find(FString("Cube")), AnimFrameData);
//
//	Client->PushSubjectFrameData_AnyThread(FLiveLinkSubjectKey(SourceGuid, SubjectName), MoveTemp(FrameData));
//}

//With Tranforms
//void FRgbPoseLiveLinkSource::HandleReceivedData(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData)
//{
//	FString recvedString;
//	int32 Read = ReceivedData->Num();
//	recvedString.Empty(ReceivedData->Num());
//	for (uint8& Byte : *ReceivedData.Get())
//	{
//		recvedString += TCHAR(Byte);
//	}
//	// UE_LOG(LogTemp, Warning, TEXT("num bytes = %i,    %s"), Read, *recvedString);
//	TArray<FString> PoseMessageArray;
//	recvedString.ParseIntoArray(PoseMessageArray, TEXT("|"), false);
//	PoseMessageArray.RemoveAt(PoseMessageArray.Num() - 1);
//	// static FLiveLinkFrameData LiveLinkFrame;
//	FName SubjectName = FName(TEXT("RgbPose"));
//	FLiveLinkSubjectKey Key = FLiveLinkSubjectKey(SourceGuid, SubjectName);
//	double currentTime = FPlatformTime::Seconds();
//	//UE_LOG(LogTemp, Warning, TEXT("%f"), 1.0 / (currentTime - LastFrameTime));
//	LastFrameTime = currentTime;
//	FTimer timer;
//	FLiveLinkFrameDataStruct FrameData(FLiveLinkTransformFrameData::StaticStruct());
//	//FLiveLinkAnimationFrameData& AnimFrameData = *FrameData.Cast<FLiveLinkAnimationFrameData>();
//	FLiveLinkTransformFrameData& TransformData = *FrameData.Cast<FLiveLinkTransformFrameData>();
//	//TransformData.Transform
//	//AnimFrameData.WorldTime = FLiveLinkWorldTime(currentTime);
//	TransformData.WorldTime = FLiveLinkWorldTime(currentTime);
//	PoseFrame poseFrame = PoseFrame(PoseMessageArray);
//	FTransform currentFrameTransform(*poseFrame.PoseFrameRot.Find(FString("Cube")), *poseFrame.PoseFrameMap.Find(FString("Cube")));
//	TransformData.Transform = currentFrameTransform;
//	TransformData.PropertyValues.Add(1.0f);
//	
//	if (false)
//	{
//		FLiveLinkStaticDataStruct StaticData(FLiveLinkSkeletonStaticData::StaticStruct());
//		FLiveLinkSkeletonStaticData* SkeletonData = StaticData.Cast<FLiveLinkSkeletonStaticData>();
//		SkeletonData->PropertyNames.Add("Cube_X");
//		SkeletonData->PropertyNames.Add("Cube_Y");
//		SkeletonData->PropertyNames.Add("Cube_Z");
//		SkeletonData->PropertyNames.Add("Cube_RotX");
//		SkeletonData->PropertyNames.Add("Cube_RotY");
//		SkeletonData->PropertyNames.Add("Cube_RotZ");
//		SkeletonData->PropertyNames.Add("Cube_RotW");
//		Client->PushSubjectStaticData_AnyThread(Key, ULiveLinkAnimationRole::StaticClass(), MoveTemp(StaticData));
//		PoseLabelsLoaded = true;
//	}
//	FLiveLinkStaticDataStruct StaticData(FLiveLinkTransformStaticData::StaticStruct());
//	FLiveLinkTransformStaticData* TransformDataStatic = StaticData.Cast<FLiveLinkTransformStaticData>();
//	Client->PushSubjectStaticData_AnyThread(Key, ULiveLinkTransformRole::StaticClass(), MoveTemp(StaticData));
//	
//	/*AddAnimFrameData(poseFrame.PoseFrameMap.Find(FString("Cube")), AnimFrameData);
//	AddAnimFrameData(poseFrame.PoseFrameRot.Find(FString("Cube")), AnimFrameData);*/
//	Client->PushSubjectFrameData_AnyThread(FLiveLinkSubjectKey(SourceGuid, SubjectName), MoveTemp(FrameData));
//}

//With Animations 
void FRgbPoseLiveLinkSource::HandleReceivedData(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData)
{
	FString recvedString;
	int32 Read = ReceivedData->Num();
	recvedString.Empty(ReceivedData->Num());
	for (uint8& Byte : *ReceivedData.Get())
	{
		recvedString += TCHAR(Byte);
	}
	// UE_LOG(LogTemp, Warning, TEXT("num bytes = %i,    %s"), Read, *recvedString);
	TArray<FString> PoseMessageArray;
	recvedString.ParseIntoArray(PoseMessageArray, TEXT("|"), false);
	PoseMessageArray.RemoveAt(PoseMessageArray.Num() - 1);
	double currentTime = FPlatformTime::Seconds();
	//UE_LOG(LogTemp, Warning, TEXT("%f"), 1.0 / (currentTime - LastFrameTime));
	LastFrameTime = currentTime;
	FTimer timer;

	PoseFrame poseFrame = PoseFrame(PoseMessageArray);
	//FTransform currentFrameTransform(*poseFrame.PoseFrameRot.Find(FString("Cube")), *poseFrame.PoseFrameMap.Find(FString("Cube")));
	// static FLiveLinkFrameData LiveLinkFrame;
	///		LIVE LINK SUBJECT NAME
	FString Subname;
	Subname = poseFrame.Subjectname;
	FName SubjectName = FName(*Subname);

		FLiveLinkSubjectKey Key = FLiveLinkSubjectKey(SourceGuid, SubjectName);
		Client->SetSubjectEnabled(Key,true);
		AddStaticSkeletonData(SubjectName, poseFrame.BoneName_TransformMap);
		FLiveLinkFrameDataStruct FrameData(FLiveLinkAnimationFrameData::StaticStruct());
		FLiveLinkAnimationFrameData& AnimationData = *FrameData.Cast<FLiveLinkAnimationFrameData>();
		//AnimationData.Transforms.Reserve(1);
		AnimationData.WorldTime = FLiveLinkWorldTime(currentTime);
		//AnimationData.Transforms.Reserve(1);

		//Addding transforms to data frame
		TArray<FTransform> transforms;
		transforms.Reset(poseFrame.BoneName_TransformMap.Num());
		//int count = 0;
		for (const TPair<FString, FTransform>& pair : poseFrame.BoneName_TransformMap)
		{
			int32 transformIndex = transforms.AddUninitialized(1);
			transforms[transformIndex] = pair.Value;
			//boneNames.Add(FName(*pair.Key));
			//boneParents.Add(count); //0 - root
			//count++;
		}

		AnimationData.Transforms.Append(transforms);
		//AnimationData.Transforms.Append(currentFrameTransform);
		UE_LOG(LogClass, Warning, TEXT("My Int Value: %d"), AnimationData.Transforms.Num());
		Client->PushSubjectFrameData_AnyThread(Key, MoveTemp(FrameData));

	/*if (false)
	{
		FLiveLinkStaticDataStruct StaticData(FLiveLinkSkeletonStaticData::StaticStruct());
		FLiveLinkSkeletonStaticData* SkeletonData = StaticData.Cast<FLiveLinkSkeletonStaticData>();
		SkeletonData->PropertyNames.Add("Cube_X");
		SkeletonData->PropertyNames.Add("Cube_Y");
		SkeletonData->PropertyNames.Add("Cube_Z");
		SkeletonData->PropertyNames.Add("Cube_RotX");
		SkeletonData->PropertyNames.Add("Cube_RotY");
		SkeletonData->PropertyNames.Add("Cube_RotZ");
		SkeletonData->PropertyNames.Add("Cube_RotW");
		Client->PushSubjectStaticData_AnyThread(Key, ULiveLinkAnimationRole::StaticClass(), MoveTemp(StaticData));
		PoseLabelsLoaded = true;
	}*/
	/*FLiveLinkStaticDataStruct StaticData(FLiveLinkTransformStaticData::StaticStruct());
	FLiveLinkTransformStaticData* TransformDataStatic = StaticData.Cast<FLiveLinkTransformStaticData>();
	Client->PushSubjectStaticData_AnyThread(Key, ULiveLinkTransformRole::StaticClass(), MoveTemp(StaticData));
	*/
	/*AddAnimFrameData(poseFrame.PoseFrameMap.Find(FString("Cube")), AnimFrameData);
	AddAnimFrameData(poseFrame.PoseFrameRot.Find(FString("Cube")), AnimFrameData);*/
}

void FRgbPoseLiveLinkSource::AddAnimFrameData(FVector* inVector, FLiveLinkAnimationFrameData& animFrameData)
{
	animFrameData.PropertyValues.Add(inVector->X);
	animFrameData.PropertyValues.Add(inVector->Y);
	animFrameData.PropertyValues.Add(inVector->Z);
}

void FRgbPoseLiveLinkSource::AddAnimFrameData(FQuat* inQuat, FLiveLinkAnimationFrameData& animFrameData)
{
	animFrameData.PropertyValues.Add(inQuat->X);
	animFrameData.PropertyValues.Add(inQuat->Y);
	animFrameData.PropertyValues.Add(inQuat->Z);
	animFrameData.PropertyValues.Add(inQuat->W);
}

void FRgbPoseLiveLinkSource::AddStaticSkeletonData(FName subjectName, TMap<FString, FTransform> BoneName_TransformMap)
{
	//TArray<FName> boneNames;
	    TArray<FName> boneNames;
		TArray<int32> boneParents;
		int count = 0;
		for (const TPair<FString, FTransform>& pair : BoneName_TransformMap)
		{
			boneNames.Add(FName(*pair.Key));
			int boneParent = (count == 0) ? 0 : (count - 1);
			boneParents.Add(boneParent); //0 - root
			count++;
			//if (count > 2) break;
		}

		FLiveLinkSubjectKey Key = FLiveLinkSubjectKey(SourceGuid, subjectName);
		FLiveLinkStaticDataStruct StaticData(FLiveLinkSkeletonStaticData::StaticStruct());
		FLiveLinkSkeletonStaticData* SkeletonData = StaticData.Cast<FLiveLinkSkeletonStaticData>();
		SkeletonData->SetBoneNames(boneNames);
		SkeletonData->SetBoneParents(boneParents);
		Client->PushSubjectStaticData_AnyThread(Key, ULiveLinkAnimationRole::StaticClass(), MoveTemp(StaticData));
}

#undef LOCTEXT_NAMESPACE
