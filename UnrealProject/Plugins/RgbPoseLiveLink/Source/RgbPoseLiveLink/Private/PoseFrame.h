#pragma once

class PoseFrame
{
public:
    TMap<FString, FTransform> ObjectName_TransformMap;
    TMap<FString, FTransform> BoneName_TransformMap;
    PoseFrame(TArray<FString> PoseFrameArray);

    /// <summary>
    /// Changes string form of transform to FTransform object (x,y,z,qw,qx,qy,qz,sx,sy,sz) -> FTransform
    /// </summary>
    FTransform ConvertToTransform(FString transformText);
};
