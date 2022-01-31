// Fill out your copyright notice in the Description page of Project Settings.


#include "RGBRokokoAnimNode.h"
#include "Animation/AnimInstanceProxy.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "ILiveLinkClient.h"
#include "Kismet/KismetMathLibrary.h"

FRGBRokokoAnimNode::FRGBRokokoAnimNode()
{
	firstTime = true;
}

///		INITIALIZES THE BONE REFERENCES FOR THE SKELETON (BONE REFERENCES ARE THEN USED TO ACCESS THE BONES AND APPLY TRANSFORMS TO THEM IN RUNTIME)
void FRGBRokokoAnimNode::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	for (size_t i = 0; i < RequiredBones.GetReferenceSkeleton().GetNum(); i++)
	{
		BoneReferencesArray.Add(FBoneReference(RequiredBones.GetReferenceSkeleton().GetBoneName(i)));
		BoneReferencesArray[i].Initialize(RequiredBones);
	}
}

///		FETCHES THE FRAME DATA FROM THE LIVE LINK CLIENT GIVEN THE SUBJECT NAME, AND APPLIES THEM TO APPROPRIATE BONES USING BONE REFERENCES 
void FRGBRokokoAnimNode::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	///		GETTING THE SUBJECT FRAME DATA FROM LIVE LINK SUBJECT NAME PROVIDED IN BLUEPRINT
	FLiveLinkSubjectName LiveLinkSubjectName = GetLiveLinkSubjectName();
	TSubclassOf<ULiveLinkRole> SubjectRole = LiveLinkClient_AnyThread->GetSubjectRole(LiveLinkSubjectName);
	FLiveLinkSubjectFrameData SubjectFrameData;
	if (SubjectRole) {
		if (SubjectRole->IsChildOf(ULiveLinkAnimationRole::StaticClass())) {
			//Process animation data if the subject is from that type
			if (LiveLinkClient_AnyThread->EvaluateFrame_AnyThread(LiveLinkSubjectName, ULiveLinkAnimationRole::StaticClass(), SubjectFrameData)) {}
			else {
				return;
			}
		}
		else {
			return;
		}
	}
	else
		return;

	///		GETTING THE SKELETON DATA (STRUCTURE) AND FRAME DATA (BONE ROTATIONS AND TRANSLATIONS) FROM THE LIVE LINK SUBJECT FRAME
	FLiveLinkSkeletonStaticData* SkeletonData = SubjectFrameData.StaticData.Cast<FLiveLinkSkeletonStaticData>();
	FLiveLinkAnimationFrameData* FrameData = SubjectFrameData.FrameData.Cast<FLiveLinkAnimationFrameData>();

	const TArray<FName>& SourceBoneNames = SkeletonData->PropertyNames;
	for (size_t i = 0; i < SkeletonData->BoneNames.Num(); i++)
	{

		int boneReferenceIndex = -1;
		FBoneReference boneReference;
		boneReference.BoneName = SkeletonData->BoneNames[i];
		if (BoneReferencesArray.Find(boneReference, boneReferenceIndex)) {
			UE_LOG(LogTemp, Warning, TEXT("Inside the source Location is %s %f %f %f Rotation is : x =  %f y =  %f z = %f w = %f"), *SkeletonData->BoneNames[i].ToString(), FrameData->Transforms[i].GetLocation().X, FrameData->Transforms[i].GetLocation().Y, FrameData->Transforms[i].GetLocation().Z
				, FrameData->Transforms[i].GetRotation().X, FrameData->Transforms[i].GetRotation().Y, FrameData->Transforms[i].GetRotation().Z, FrameData->Transforms[i].GetRotation().W);
			
			///		APPLYING THE TRANSFORMS TO APPROPRIATE BONE USING BONE REFERENCES

			// the way we apply transform is same as FMatrix or FTransform
			// we apply scale first, and rotation, and translation
			// if you'd like to translate first, you'll need two nodes that first node does translate and second nodes to rotate.
			FQuat Rotation = FrameData->Transforms[i].GetRotation();
			FVector Translation = FrameData->Transforms[i].GetTranslation();
			FVector Scale = FrameData->Transforms[i].GetScale3D();

			const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
			FCompactPoseBoneIndex CompactPoseBoneToModify = BoneReferencesArray[boneReferenceIndex].GetCompactPoseIndex(BoneContainer);
			FTransform NewBoneTM = Output.Pose.GetComponentSpaceTransform(CompactPoseBoneToModify);
			FTransform ComponentTransform = Output.AnimInstanceProxy->GetComponentTransform();

			//	// Convert to Bone Space.
			FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, EBoneControlSpace::BCS_BoneSpace);
			NewBoneTM.SetScale3D(NewBoneTM.GetScale3D() * Scale);
			// Convert back to Component Space.
			FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, EBoneControlSpace::BCS_BoneSpace);

			//Rotation
			FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, EBoneControlSpace::BCS_BoneSpace);
			const FQuat BoneQuat(Rotation);
			NewBoneTM.SetRotation(BoneQuat * NewBoneTM.GetRotation());
			// Convert back to Component Space.
			FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, EBoneControlSpace::BCS_BoneSpace);

			// Convert to Bone Space.
			FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, EBoneControlSpace::BCS_BoneSpace);
			NewBoneTM.AddToTranslation(FVector(0,0,0));

			// Convert back to Component Space.
			FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, EBoneControlSpace::BCS_BoneSpace);

			FCompactPoseBoneIndex compactBoneINdex= BoneReferencesArray[boneReferenceIndex].GetCompactPoseIndex(BoneContainer);
			//OutBoneTransforms.Add(FBoneTransform(compactBoneINdex, NewBoneTM));
			CopyOfOutBoneTransforms.Add(FBoneTransform(compactBoneINdex, NewBoneTM));
			Output.Pose.LocalBlendCSBoneTransforms(CopyOfOutBoneTransforms, 1.0f);
			CopyOfOutBoneTransforms.Reset();
		}
	}
}





void FRGBRokokoAnimNode::GatherDebugData(FNodeDebugData& DebugData)
{
	FString DebugLine = DebugData.GetNodeName(this);

	DebugLine += "(";
	AddDebugNodeData(DebugLine);
	//DebugLine += FString::Printf(TEXT(" Target: %s)"), *BoneToModify.BoneName.ToString());
	DebugData.AddDebugItem(DebugLine);

	ComponentPose.GatherDebugData(DebugData);
}

bool FRGBRokokoAnimNode::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	return true;
	//return (BoneToModify.IsValid(RequiredBones));
}

void FRGBRokokoAnimNode::PreUpdate(const UAnimInstance* InAnimInstance)
{
	Super::PreUpdate(InAnimInstance);

	LiveLinkClient_AnyThread = LiveLinkClient_GameThread.GetClient();
}

FLiveLinkSubjectName FRGBRokokoAnimNode::GetLiveLinkSubjectName()
{
	return FName(RGBMocapActorName.ToString());
}

FVector FRGBRokokoAnimNode::GetVectorFromCurvesCpp(const FString BoneName)
{
	FString X = BoneName + "_X";
	FString Y = BoneName + "_Y";
	FString Z = BoneName + "_Z";

	float* x_val = LiveLinkCurvesCpp.Find(FName(X));
	float* y_val = LiveLinkCurvesCpp.Find(FName(Y));
	float* z_val = LiveLinkCurvesCpp.Find(FName(Z));

	return FVector(*x_val, *y_val, *z_val);
}

void FRGBRokokoAnimNode::RotationMethodCpp(TMap<FName, FTransform>& sourceBoneFinalTransforms)
{
	FVector norm = CalculateNormalCpp(GetVectorFromCurvesCpp("hip"), GetVectorFromCurvesCpp("lThighBend"), GetVectorFromCurvesCpp("rThighBend"));

	FRotator rot = UKismetMathLibrary::FindLookAtRotation(norm, GetVectorFromCurvesCpp("hip"));
	norm.Normalize();
	NormalizedForward = norm;

	//rot += {89.998116, 89.790634, 180};
	rot = UKismetMathLibrary::ComposeRotators({ 90, 90, 0 }, rot);
	FTransform HipTrans(rot, GetVectorFromCurvesCpp("hip"), FVector(2, 2, 2));//GetVectorFromCurvesCpp("hip")
	//HipTrans.SetLocation(GetVectorFromCurvesCpp("hip"));
	//HipTrans.SetRotation(rot.Quaternion());

	//mocapPawn->hip = HipTrans;
	//
	//mocapPawn->spine = CalcTransformForRotation("neck", "spine");
	//mocapPawn->neck = CalcTransformForRotation("head", "neck");
	//
	//mocapPawn->lThighBend = CalcTransformForRotation("lThighBend", "lShin");
	//mocapPawn->lShin = CalcTransformForRotation("lShin", "lFoot");
	//
	//mocapPawn->rThighBend = CalcTransformForRotation("rShin", "rThighBend", true);
	//mocapPawn->rShin = CalcTransformForRotation("rFoot", "rShin", false, true);
	//
	//mocapPawn->lShldrBend = CalcTransformForRotation("lForearmBend", "lShldrBend", false, true);
	//mocapPawn->lForearmBend = CalcTransformForRotation("lHand", "lForearmBend");
	//
	//mocapPawn->rShldrBend = CalcTransformForRotation("rShldrBend", "rForearmBend", false, true);
	//mocapPawn->rForearmBend = CalcTransformForRotation("rForearmBend", "rHand", false);

	sourceBoneFinalTransforms.Add("hip", HipTrans);
	//hip = HipTrans;

	//spine = CalcTransformForRotation("neck", "spine");
	sourceBoneFinalTransforms.Add("spine", CalcTransformForRotation("neck", "spine"));
	sourceBoneFinalTransforms.Add("neck", CalcTransformForRotation("head", "neck"));

	//lThighBend = CalcTransformForRotation("lThighBend", "lShin");
	sourceBoneFinalTransforms.Add("lThighBend", CalcTransformForRotation("lThighBend", "lShin"));
	//lShin = CalcTransformForRotation("lShin", "lFoot");
	sourceBoneFinalTransforms.Add("lShin", CalcTransformForRotation("lShin", "lFoot"));

	sourceBoneFinalTransforms.Add("rThighBend", CalcTransformForRotation("rShin", "rThighBend", true));
	sourceBoneFinalTransforms.Add("rShin", CalcTransformForRotation("rFoot", "rShin", false, true));

	sourceBoneFinalTransforms.Add("lShldrBend", CalcTransformForRotation("lForearmBend", "lShldrBend", false, true));
	sourceBoneFinalTransforms.Add("lForearmBend", CalcTransformForRotation("lHand", "lForearmBend"));

	sourceBoneFinalTransforms.Add("rShldrBend", CalcTransformForRotation("rShldrBend", "rForearmBend", false, true));
	sourceBoneFinalTransforms.Add("rForearmBend", CalcTransformForRotation("rForearmBend", "rHand", false));

	//sourceBoneFinalTransforms.Add(FTransform());
	//if (it.Key.Compare("spine")) sourceBoneFinalTransforms[count] = spine;
	//else if (it.Key.Compare("neck")) sourceBoneFinalTransforms[count] = neck;
	//else if (it.Key.Compare("lThighBend"))  sourceBoneFinalTransforms[count] = lThighBend;
	//else if (it.Key.Compare("lShin"))  sourceBoneFinalTransforms[count] = lShin;
	//else if (it.Key.Compare("rThighBend"))  sourceBoneFinalTransforms[count] = rThighBend;
	//else if (it.Key.Compare("rShin"))  sourceBoneFinalTransforms[count] = rShin;
	//else if (it.Key.Compare("lShldrBend"))  sourceBoneFinalTransforms[count] = lShldrBend;
	//else if (it.Key.Compare("lForearmBend"))  sourceBoneFinalTransforms[count] = lForearmBend;
	//else if (it.Key.Compare("rShldrBend"))  sourceBoneFinalTransforms[count] = rShldrBend;
	//else if (it.Key.Compare("rForearmBend"))  sourceBoneFinalTransforms[count] = rForearmBend;
	//	count++;
	//}
}

FVector FRGBRokokoAnimNode::CalculateNormalCpp(const FVector A, const FVector B, const FVector C)
{
	return FVector::CrossProduct(B - A, C - A);
}

float FRGBRokokoAnimNode::CalculateHeightCpp()
{
	float headNneck = FVector::Distance(GetVectorFromCurvesCpp("head"), GetVectorFromCurvesCpp("neck"));
	float neckNspine = FVector::Distance(GetVectorFromCurvesCpp("neck"), GetVectorFromCurvesCpp("spine"));
	float spineNThighBend = FVector::Distance(GetVectorFromCurvesCpp("spine"), (GetVectorFromCurvesCpp("rThighBend") + GetVectorFromCurvesCpp("lThighBend")) / 2.0);

	float thighNshin = (FVector::Distance(GetVectorFromCurvesCpp("rThighBend"), GetVectorFromCurvesCpp("rShin")) + FVector::Distance(GetVectorFromCurvesCpp("lThighBend"), GetVectorFromCurvesCpp("lShin"))) / 2;
	float shinNfoot = (FVector::Distance(GetVectorFromCurvesCpp("rShin"), GetVectorFromCurvesCpp("rFoot")) + FVector::Distance(GetVectorFromCurvesCpp("lShin"), GetVectorFromCurvesCpp("lFoot"))) / 2;

	return headNneck + neckNspine + spineNThighBend + thighNshin + shinNfoot;
}

FTransform FRGBRokokoAnimNode::GetTransformFromCurvesCpp(const FString BoneName)
{
	FString X = BoneName + "_X";
	FString Y = BoneName + "_Y";
	FString Z = BoneName + "_Z";

	float* x_val = LiveLinkCurvesCpp.Find(FName(X));
	float* y_val = LiveLinkCurvesCpp.Find(FName(Y));
	float* z_val = LiveLinkCurvesCpp.Find(FName(Z));

	FTransform transfrom;
	transfrom.SetLocation(FVector(*x_val, *y_val, *z_val) * ScaleFactor);

	return transfrom;
}

float FRGBRokokoAnimNode::CalculateScaleFactorCpp(float ManequinHeight, float HeightFromSolver)
{
	return ManequinHeight / HeightFromSolver;
}

void FRGBRokokoAnimNode::TranslationMethodCpp()
{
	//mocapPawn->rShldrBend = GetTransformFromCurvesCpp("rShldrBend");
	//mocapPawn->rForearmBend = GetTransformFromCurvesCpp("rForearmBend");
	//mocapPawn->rHand = GetTransformFromCurvesCpp("rHand");
	//mocapPawn->lShldrBend = GetTransformFromCurvesCpp("lShldrBend");
	//mocapPawn->lForearmBend = GetTransformFromCurvesCpp("lForearmBend");
	//mocapPawn->lHand = GetTransformFromCurvesCpp("lHand");
	//mocapPawn->rThighBend = GetTransformFromCurvesCpp("rThighBend");
	//mocapPawn->rShin = GetTransformFromCurvesCpp("rShin");
	//mocapPawn->rFoot = GetTransformFromCurvesCpp("rFoot");
	//mocapPawn->lThighBend = GetTransformFromCurvesCpp("lThighBend");
	//mocapPawn->lShin = GetTransformFromCurvesCpp("lShin");
	//mocapPawn->lFoot = GetTransformFromCurvesCpp("lFoot");
	//mocapPawn->abdomenUpper = GetTransformFromCurvesCpp("abdomenUpper");
	//mocapPawn->head = GetTransformFromCurvesCpp("head");
	//mocapPawn->neck = GetTransformFromCurvesCpp("neck");
	//mocapPawn->spine = GetTransformFromCurvesCpp("spine");

	FTransform hipTrans = GetTransformFromCurvesCpp("hip");
	FVector normal = CalculateNormalCpp(GetVectorFromCurvesCpp("hip"),
		GetVectorFromCurvesCpp("lThighBend"),
		GetVectorFromCurvesCpp("rThighBend"));

	FRotator rot = UKismetMathLibrary::FindLookAtRotation(GetVectorFromCurvesCpp("hip"), normal);
	hipTrans.SetRotation(rot.Quaternion());
}

FTransform FRGBRokokoAnimNode::CalcTransformForRotation(const FString FirstBone, const FString SecondBone, const bool useXZ, const bool invertForward)
{
	FVector diff = GetVectorFromCurvesCpp(FirstBone) - GetVectorFromCurvesCpp(SecondBone);

	diff.Normalize();

	if (invertForward)
		NormalizedForward *= -1.0f;

	FTransform Trans;

	if (!useXZ)
		Trans.SetRotation(UKismetMathLibrary::MakeRotFromXY(diff, NormalizedForward).Quaternion());
	else
		Trans.SetRotation(UKismetMathLibrary::MakeRotFromXZ(diff, NormalizedForward).Quaternion());

	return Trans;
}

void FRGBRokokoAnimNode::ApplyBoneRotation(TMap<FName, FTransform> sourceBoneFinalTransforms, FName boneName, FBoneReference boneReference, FCSPose<FCompactPose>& MeshBases) {
	//const FBoneContainer BoneContainer = MeshBases.GetPose().GetBoneContainer();
	//FCompactPoseBoneIndex CompactPoseBoneToModify = boneReference.GetCompactPoseIndex(BoneContainer);
	//BoneContainer.compant
	////OutBoneTransforms.Add(FBoneTransform(CompactPoseBoneToModify, FTransform(sourceBoneFinalTransforms["rShldrBend"].GetRotation())));
	//EBoneControlSpace space = BCS_ComponentSpace;
	//FTransform NewBoneTM = MeshBases.GetComponentSpaceTransform(CompactPoseBoneToModify);
	//const FQuat BoneQuat(sourceBoneFinalTransforms[boneName].GetRotation());
	//NewBoneTM.SetRotation(BoneQuat);
	//MeshBases.
	//MeshBases.SetComponentSpaceTransform(CompactPoseBoneToModify, NewBoneTM);
}

void FRGBRokokoAnimNode::ApplyBonePosition(TMap<FName, FTransform> sourceBoneFinalTransforms, FName boneName, FBoneReference boneReference, FCSPose<FCompactPose>& MeshBases) {
	const FBoneContainer BoneContainer = MeshBases.GetPose().GetBoneContainer();
	//BoneContainer.GetCompactPoseIndexFromSkeletonIndex();
	FCompactPoseBoneIndex CompactPoseBoneToModify = boneReference.GetCompactPoseIndex(BoneContainer);
	
	//OutBoneTransforms.Add(FBoneTransform(CompactPoseBoneToModify, FTransform(sourceBoneFinalTransforms["rShldrBend"].GetRotation())));
	EBoneControlSpace space = BCS_ComponentSpace;
	FTransform NewBoneTM = MeshBases.GetComponentSpaceTransform(CompactPoseBoneToModify);
	const FVector BonePos(sourceBoneFinalTransforms[boneName].GetLocation());
	NewBoneTM.SetLocation(BonePos);
	MeshBases.SetComponentSpaceTransform(CompactPoseBoneToModify, NewBoneTM);
}

URGBRokokoBoneMap::URGBRokokoBoneMap()
{
	hip = "hip";
	spine = "spine";
	neck = "neck";
	//headTop = "headTop";
	leftShoulder = "leftShoulder";
	leftForearm = "leftForearm";
	//leftFingerTip = "leftFingerTip";
	rightForearm = "rightForearm";
	rightShoulder = "rightShoulder";
	//rightFingerTip = "rightFingerTip";
	leftUpleg = "leftUpleg";
	leftLeg = "leftLeg";
	//leftToe = "leftToe";
	rightUpleg = "rightUpleg";
	rightLeg = "rightLeg";
	//rightArm = "rightArm";
	//rightHand = "rightHand";
	//leftFoot = "leftFoot";
	//rightFoot = "rightFoot";
	////rightToe = "rightToe";
	//leftThumbProximal = "leftThumbProximal";
	//leftThumbMedial = "leftThumbMedial";
	//leftThumbDistal = "leftThumbDistal";
	//leftThumbTip = "leftThumbTip";
	//leftIndexProximal = "leftIndexProximal";
	//leftIndexMedial = "leftIndexMedial";
	//leftIndexDistal = "leftIndexDistal";
	//leftIndexTip = "leftIndexTip";
	//leftMiddleProximal = "leftMiddleProximal";
	//leftMiddleMedial = "leftMiddleMedial";
	//leftMiddleDistal = "leftMiddleDistal";
	//leftMiddleTip = "leftMiddleTip";
	//leftRingProximal = "leftRingProximal";
	//leftRingMedial = "leftRingMedial";
	//leftRingDistal = "leftRingDistal";
	//leftRingTip = "leftRingTip";
	//leftLittleProximal = "leftLittleProximal";
	//leftLittleMedial = "leftLittleMedial";
	//leftLittleDistal = "leftLittleDistal";
	//leftLittleTip = "leftLittleTip";
	//rightThumbProximal = "rightThumbProximal";
	//rightThumbMedial = "rightThumbMedial";
	//rightThumbDistal = "rightThumbDistal";
	//rightThumbTip = "rightThumbTip";
	//rightIndexProximal = "rightIndexProximal";
	//rightIndexMedial = "rightIndexMedial";
	//rightIndexDistal = "rightIndexDistal";
	//rightIndexTip = "rightIndexTip";
	//rightMiddleProximal = "rightMiddleProximal";
	//rightMiddleMedial = "rightMiddleMedial";
	//rightMiddleDistal = "rightMiddleDistal";
	//rightMiddleTip = "rightMiddleTip";
	//rightRingProximal = "rightRingProximal";
	//rightRingMedial = "rightRingMedial";
	//rightRingDistal = "rightRingDistal";
	//rightRingTip = "rightRingTip";
	//rightLittleProximal = "rightLittleProximal";
	//rightLittleMedial = "rightLittleMedial";
	//rightLittleDistal = "rightLittleDistal";
	//rightLittleTip = "rightLittleTip";
	/*leftArm = "leftArm";
	chest = "chest";
	leftHand = "leftHand";
	head = "head";*/
}
