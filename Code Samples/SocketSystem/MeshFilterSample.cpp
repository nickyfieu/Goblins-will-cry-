#include "stdafx.h"
#include "MeshFilter.h"
#include "Material.h"
#include <algorithm>

/*
	Disclaimer: only code writen by Nicky Fieu are in these samples
*/

MeshFilter::~MeshFilter()
{
	for(SkeletonData* toDelete:m_SkeletonData)
	{
		SafeDelete(toDelete);
	}
	m_SkeletonData.clear();
}

short MeshFilter::GetBoneIndexByName(const std::wstring& boneName) const
{
	// hashes the given bone name and tries to find if a valid skeleton data exists with that name and returns the index of that skeleton data
	size_t boneNameHash = std::hash<std::wstring>{}(boneName);
	auto it = std::find_if(m_SkeletonData.cbegin(), m_SkeletonData.cend(), [&boneNameHash](const SkeletonData* cmp) {return cmp->nameHash == boneNameHash; });
	if (it != m_SkeletonData.cend())
		return (*it)->index;

	Logger::LogWarning(L"MeshFilter::GetBoneIndexByName returning 0 as index due to invalid bonename:" + boneName);
	return 0;
}

SkeletonData* MeshFilter::GetSkeletonDataByIndex(short boneIndex) const
{
	// tries to find the skeleton data that has the given bone index and returns it
	auto it = std::find_if(m_SkeletonData.cbegin(), m_SkeletonData.cend(), [&boneIndex](const SkeletonData* cmp) {return cmp->index == boneIndex; });
	if (it != m_SkeletonData.cend())
		return (*it);

	Logger::LogWarning(L"MeshFilter::GetSkeletonDataByIndex nullpt due to unexistant bone index");
	return nullptr;
}

BoneObject* MeshFilter::GetBoneObjectByIndex(short boneIndex) const
{
	return m_BoneObjects[boneIndex];
}
