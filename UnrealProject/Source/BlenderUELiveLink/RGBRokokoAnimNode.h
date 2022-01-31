// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AnimGraphRuntime/Public/BoneControllers/AnimNode_SkeletalControlBase.h"
//#include "AnimGraphNode_SkeletalControlBase.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "LiveLinkInterface/Public/LiveLinkTypes.h"
#include "LiveLinkClientReference.h"
#include "Engine/DataAsset.h"
#include "RGBRokokoAnimNode.generated.h"

class ILiveLinkClient;
/**
 * 
 */
//UCLASS()
//class MOCAPUNREAL_API URGBRokokoAnimNode : public UAnimGraphNode_SkeletalControlBase
//{
//	GENERATED_BODY()
//	
//};

UCLASS(BlueprintType)
class BLENDERUELIVELINK_API URGBRokokoBoneMap : public UDataAsset
{
	GENERATED_BODY()

		URGBRokokoBoneMap();
public:

	UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
		FName hip;

	UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
		FName leftShoulder;

	UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
		FName leftForearm;

	UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
		FName rightShoulder;

	UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
		FName rightForearm;


	UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
		FName spine;

	UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
		FName neck;


	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//FName headTop;
	UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
		FName leftUpleg;

	UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
		FName leftLeg;

	UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
		FName rightUpleg;

	UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
		FName rightLeg;



	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName chest;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//FName leftFingerTip;

	/*UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
		FName leftHand;
	UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
		FName leftArm;*/

	/*UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
		FName head;*/
	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightArm;
	//

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightHand;

	////UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	////FName rightFingerTip;

	//
	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftFoot;

	////UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	////FName leftToe;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightFoot;

	////UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	////FName rightToe;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftThumbProximal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftThumbMedial;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftThumbDistal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftThumbTip;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftIndexProximal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftIndexMedial;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftIndexDistal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftIndexTip;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftMiddleProximal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftMiddleMedial;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftMiddleDistal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftMiddleTip;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftRingProximal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftRingMedial;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftRingDistal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftRingTip;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftLittleProximal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftLittleMedial;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftLittleDistal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName leftLittleTip;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightThumbProximal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightThumbMedial;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightThumbDistal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightThumbTip;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightIndexProximal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightIndexMedial;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightIndexDistal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightIndexTip;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightMiddleProximal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightMiddleMedial;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightMiddleDistal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightMiddleTip;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightRingProximal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightRingMedial;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightRingDistal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightRingTip;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightLittleProximal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightLittleMedial;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightLittleDistal;

	//UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
	//	FName rightLittleTip;
};



USTRUCT()
struct BLENDERUELIVELINK_API FRGBRokokoAnimNode : public FAnimNode_SkeletalControlBase
{
	GENERATED_USTRUCT_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RGBSkeletalControl, meta = (PinShownByDefault))
		FName RGBMocapActorName;

	UPROPERTY(EditAnywhere, Category = RGBSkeletalControl)
		URGBRokokoBoneMap* BoneMapOverride;


	//For bone rotations 

	TMap<FName, float> LiveLinkCurvesCpp;

	float ScaleFactor;

	FVector NormalizedForward;
	FVector GetVectorFromCurvesCpp(const FString BoneName);
	FVector CalculateNormalCpp(const FVector A, const FVector B, const FVector C);

	float CalculateHeightCpp();

	FTransform GetTransformFromCurvesCpp(const FString BoneName);

	float CalculateScaleFactorCpp(float ManequinHeight, float HeightFromSolver);

	void TranslationMethodCpp();

	void RotationMethodCpp(TMap<FName, FTransform>& sourceBoneFinalTransforms);
	FTransform CalcTransformForRotation(const FString FirstBone, const FString SecondBone, const bool useXZ = false, const bool invertForward = false);

	TArray<FBoneReference> BoneReferencesArray;
	bool firstTime;
	TArray<FBoneTransform> CopyOfOutBoneTransforms;


	//Bone References; 
	FBoneReference rShldrBend;
	FBoneReference rForearmBend;
	FBoneReference rHand;
	FBoneReference lShldrBend;
	FBoneReference lForearmBend;
	FBoneReference lHand;
	FBoneReference rThighBend;
	FBoneReference rShin;
	FBoneReference rFoot;
	FBoneReference lThighBend;
	FBoneReference lShin;
	FBoneReference lFoot;
	FBoneReference abdomenUpper;
	FBoneReference hip;
	FBoneReference head;
	FBoneReference neck;
	FBoneReference spine;

	void ApplyBoneRotation(TMap<FName, FTransform> sourceBoneFinalTransforms , FName boneName, FBoneReference boneReference, FCSPose<FCompactPose>& MeshBases);
	void ApplyBonePosition(TMap<FName, FTransform> sourceBoneFinalTransforms , FName boneName, FBoneReference boneReference, FCSPose<FCompactPose>& MeshBases);
public:
	//	Constructor
	FRGBRokokoAnimNode();

	// 	// FAnimNode_Base interface
	 // Display Debug Information
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// 	// End of FAnimNode_Base interface

		// FAnimNode_SkeletalControlBase interface
	 // Change the position of the logic functions to achieve
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	// used to determine the effectiveness of bone	
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase interface
	virtual bool HasPreUpdate() const { return true; }

	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;

	FLiveLinkSubjectName GetLiveLinkSubjectName();
private:
	// FAnimNode_SkeletalControlBase interface
 // Initialize the bones references
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase interface


	FLiveLinkClientReference LiveLinkClient_GameThread;
	ILiveLinkClient* LiveLinkClient_AnyThread;
};