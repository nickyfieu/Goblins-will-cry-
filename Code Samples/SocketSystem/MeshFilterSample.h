#pragma once
#include <vector>
#include "EffectHelper.h"
#include "BoneObject.h"

/*
	Disclaimer: only code writen by Nicky Fieu are in these samples
*/

struct SkeletonData
{
	SkeletonData(const std::wstring& dataName);

	USHORT index = {};
	std::wstring name = {};
	short parrentIndex = {};
	DirectX::XMFLOAT4X4 offset = {};
	DirectX::XMFLOAT4X4 localPose = {};
	DirectX::XMFLOAT4X4 globalPose = {};
};

class MeshFilter final
{
public:
	short GetBoneIndexByName(const std::wstring& boneName) const;
	BoneObject* GetBoneRoot() const { return m_pBoneRoot; }
	BoneObject* GetBoneObjectByIndex(short boneIndex) const;

private:
	SkeletonData* GetSkeletonDataByIndex(short boneIndex) const;

	//Skeleton DATA
	std::vector<SkeletonData*> m_SkeletonData;
	BoneObject* m_pBoneRoot = nullptr;
	std::vector<BoneObject*> m_BoneObjects;

};
