#include "PoseFrame.h"



PoseFrame::PoseFrame(TArray<FString> PoseFrameArray)
{
	for (int i = 0; i < PoseFrameArray.Num(); i++)
	{
		//		O_Cube:(1.0,2.0,3.0,4.0,5.0,6.0,7.0)||
		//		A_Skeleton1:Bone1:(8.0,9.0,10.0,11.0,12.0,13.0,14.0)|
		//					Bone2:(15.0,16.0,17.0,18.0,19.0,20.0,21.0)|
		//					Bone3:(22.0,23.0,24.0,25.0,26.0,27.0,28.0)||

		if (PoseFrameArray[i].IsEmpty()) break;
		FString PoseFrameElementString = PoseFrameArray[i]; //rForearmBend:(13.93308, 32.54413, -24.76695)
		TArray<FString> PoseFrameKeyValuePair;
		PoseFrameElementString.ParseIntoArray(PoseFrameKeyValuePair, TEXT("="), false);
		char* name = TCHAR_TO_UTF8(*PoseFrameKeyValuePair[0]);
		
		//Check name is a skeleton or object
		if (name[0] == 'O' && name[1] == '_') {
			//OBject 
			FString vec = TCHAR_TO_UTF8(*PoseFrameKeyValuePair[1]); 
			FTransform objectTransform = ConvertToTransform(vec);
			//Save the object name and its transform
			ObjectName_TransformMap.Add( name, objectTransform);
		}
		else if (name[0] == 'A' && name[1] == '_') {
			//Armature 
			//PoseFrameKeyValuePair[0].RemoveAt(0);
			TArray<FString> BoneNameTransformPair;
			PoseFrameKeyValuePair[1].ParseIntoArray(BoneNameTransformPair, TEXT("|"), false);
			for (size_t j = 0; j < BoneNameTransformPair.Num(); j++)
			{
				//FString PoseFrameElementString = BoneNameTransformPair[j]; //rForearmBend:(13.93308, 32.54413, -24.76695)
				TArray<FString> BoneNameTransformPairSplitArr;
				BoneNameTransformPair[j].ParseIntoArray(BoneNameTransformPairSplitArr, TEXT(":"), false);
				char* boneName= TCHAR_TO_UTF8(*BoneNameTransformPairSplitArr[0]);
				FString vec = TCHAR_TO_UTF8(*BoneNameTransformPairSplitArr[1]);
				FTransform boneTransform = ConvertToTransform(vec);
				//Save the bone name and its transform in a different array 
				BoneName_TransformMap.Add(boneName,boneTransform);
			}
		}
	}
}


//PoseFrameMap.Add(name, FVector(x, -y, z));
//PoseFrameRot.Add(name, FQuat(rx, -ry, rz, rw));
FTransform PoseFrame::ConvertToTransform(FString transformText) {
	//Remove the round brackets at end terminals
	transformText = transformText.RightChop(1);
	transformText = transformText.LeftChop(1);
	//Split the array by comma delimiter
	TArray<FString> vectorElementArray;
	transformText.ParseIntoArray(vectorElementArray, TEXT(","), false);
	float x = 100.0f * FCString::Atof(*vectorElementArray[0]);
	float y = 100.0f * FCString::Atof(*vectorElementArray[1]);
	float z = 100.0f * FCString::Atof(*vectorElementArray[2]);
	float rx = FCString::Atof(*vectorElementArray[3]);
	float ry = FCString::Atof(*vectorElementArray[4]);
	float rz = FCString::Atof(*vectorElementArray[5]);
	float rw = FCString::Atof(*vectorElementArray[6]);
	/*float sx = 100 * FCString::Atof(*vectorElementArray[7]);
	float sy = 100 * FCString::Atof(*vectorElementArray[8]);
	float sz = 100 * FCString::Atof(*vectorElementArray[9]);*/
	return FTransform(FQuat(rx, -ry,rz,rw), FVector(x,-y, z));
}
