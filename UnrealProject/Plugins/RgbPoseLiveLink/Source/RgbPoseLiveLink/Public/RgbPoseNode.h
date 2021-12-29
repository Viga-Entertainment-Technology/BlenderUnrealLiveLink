// #pragma once
//
// #include "Runtime/AnimGraphRuntime/Public/BoneControllers/AnimNode_SkeletalControlBase.h"
// #include "LiveLinkTypes.h"
//
// class ILiveLinkClient;
//
// USTRUCT()
// struct RGBPOSELIVELINK_API FRgbPoseNode : public FAnimNode_SkeletalControlBase
// {
// 	GENERATED_USTRUCT_BODY()
//     virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
// 	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
// 	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)  override;
//
// 	FLiveLinkSubjectName GetLiveLinkSubjectName();
// };
