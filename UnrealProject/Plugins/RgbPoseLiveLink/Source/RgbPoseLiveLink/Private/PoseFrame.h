#pragma once
#include<vector>
class PoseFrame
{
public:
    TMap<FString, FTransform> ObjectName_TransformMap;
    std::vector<TMap<FString, FTransform>> bonevect;
    TArray<FString> Subjectname;
    PoseFrame(TArray<FString> PoseFrameArray);

    /// <summary>
    /// Changes string form of transform to FTransform object (x,y,z,qw,qx,qy,qz,sx,sy,sz) -> FTransform
    /// </summary>
    FTransform ConvertToTransform(FString transformText);
};
