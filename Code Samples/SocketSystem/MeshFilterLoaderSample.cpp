#include "stdafx.h"
#include "MeshFilterLoader.h"
#include "BinaryReader.h"
#include "EffectHelper.h"
#include "TransformComponent.h"

/*
	Disclaimer: only code writen by Nicky Fieu are in these samples
*/

// reads custom mesh ovm file type 
MeshFilter* MeshFilterLoader::LoadContent(const std::wstring& assetFile)
{
	/*  Code inbetween (rough explenation) that wasn't written by Nicky Fieu
		opening file to read ( binary )
		some checks
		creating a new meshfilter
	*/

	for (;;)
	{
		/*  Code inbetween (rough explenation) that wasn't written by Nicky Fieu
			getting the mesh data type
			check if not end
			getting the data offset
		*/

		switch(meshDataType)
		{
		/* Implementing reading this data was done in one of the introduction weeks to overlord engine
		case MeshDataType::HEADER:
			break;
		case MeshDataType::POSITIONS:
			break;
		case MeshDataType::INDICES:
			break;
		case MeshDataType::NORMALS:
			break;
		case MeshDataType::TANGENTS:
			break;
		case MeshDataType::BINORMALS:
			break;
		case MeshDataType::TEXCOORDS:
			break;
		case MeshDataType::BLENDWEIGHTS:
			break;
		case MeshDataType::ANIMATIONCLIPS:
			break;
		*/
		case MeshDataType::SKELETON:
			// had to personaly figure out the (binary)layout for this data and how to use this data
		{	// skeleton data used for parrenting objects to specific bones
			

			// getting the amount of bones in the mesh and checking with the hard set limit
			pMesh->m_BoneCount = binReader->Read<unsigned short>();
			const unsigned short boneLimit = 70;
			if (pMesh->m_BoneCount > boneLimit)
			{
				std::wstring msg{ L"MeshFilterLoader::LoadContent - MeshDataType::SKELETON: More than the allowed bone limit! bone limit is" + std::to_wstring(boneLimit) + L" whilst this mesh has " + std::to_wstring(pMesh->m_BoneCount) + L" bones!" };
				assert(msg.c_str());
			}

			pMesh->m_SkeletonData.reserve(pMesh->m_BoneCount);
			pMesh->m_BoneObjects.resize(pMesh->m_BoneCount);

			// for the amount of bones read the binary data from the file
			// layout 
			//		2 bytes for boneID
			//		? bytes for boneName
			//		2 bytes for the parrentBoneID
			//		64 bytes for offset matrix
			//		64 bytes for localpose matrix
			//		64 bytes for globalpose matrix
			for (int i = 0; i < pMesh->m_BoneCount; i++)
			{
				USHORT id = binReader->Read<unsigned short>();
				std::wstring name = binReader->ReadString();

				// due to the converter sometimes not adding this it broke some of the mixamo animations
				if (name.find(L"mixamorig:") == std::string::npos)
					name = L"mixamorig:" + name;

				short parrent = binReader->Read<short>();
				// fills the matrices row by row
				// matrix layout
				// [ _11, _12, _13, _14 ]
				// [ _21, _22, _23, _24 ]
				// [ _31, _32, _33, _34 ]
				// [ _41, _42, _43, _44 ]
				DirectX::XMFLOAT4X4 transformMatrices[3]{};
				for (int j = 0; j < 3;j++)
				{
					transformMatrices[j]._11 = binReader->Read<float>();
					transformMatrices[j]._12 = binReader->Read<float>();
					transformMatrices[j]._13 = binReader->Read<float>();
					transformMatrices[j]._14 = binReader->Read<float>();
					transformMatrices[j]._21 = binReader->Read<float>();
					transformMatrices[j]._22 = binReader->Read<float>();
					transformMatrices[j]._23 = binReader->Read<float>();
					transformMatrices[j]._24 = binReader->Read<float>();
					transformMatrices[j]._31 = binReader->Read<float>();
					transformMatrices[j]._32 = binReader->Read<float>();
					transformMatrices[j]._33 = binReader->Read<float>();
					transformMatrices[j]._34 = binReader->Read<float>();
					transformMatrices[j]._41 = binReader->Read<float>();
					transformMatrices[j]._42 = binReader->Read<float>();
					transformMatrices[j]._43 = binReader->Read<float>();
					transformMatrices[j]._44 = binReader->Read<float>();
				}

				// creating a skeletondata object, setting the values and pushing it to the mesh filter
				SkeletonData* data = new SkeletonData(name);
				data->index = id;
				data->parrentIndex = parrent;
				data->offset = transformMatrices[0];
				data->localPose = transformMatrices[1];
				data->globalPose = transformMatrices[2];
				pMesh->m_SkeletonData.push_back(data);
			}

			// after adding all the skeletondata to the mesh filter
			// calculate all the bone lengths and adding them
			for (SkeletonData* data : pMesh->m_SkeletonData)
			{
				DirectX::XMMATRIX mSelf = DirectX::XMMatrixIdentity();
				DirectX::XMMATRIX mParent = DirectX::XMMatrixIdentity();
				DirectX::XMMATRIX mBaseBoneRot = DirectX::XMMatrixIdentity();
				
				// the current bone object self matrix is the inverse of the offset
				mSelf = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&data->offset));
				
				// if this bone has a parrent get the parrents inverse of the offset
				SkeletonData* parrent = pMesh->GetSkeletonDataByIndex(data->parrentIndex);
				if (parrent)
					mParent = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&parrent->offset));

				// calculating the initial global bind pose
				DirectX::XMVECTOR posSelf;
				DirectX::XMVECTOR posParrent;
				DirectX::XMVECTOR rotationSelf;
				DirectX::XMVECTOR notUsed;
				mBaseBoneRot = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(0), DirectX::XMConvertToRadians(0), DirectX::XMConvertToRadians(0));
				mSelf = mBaseBoneRot * mSelf;
				DirectX::XMMatrixDecompose(&notUsed, &notUsed, &posParrent, mParent);
				DirectX::XMMatrixDecompose(&notUsed, &rotationSelf, &posSelf, mSelf );
				
				// create a new bone object and add it to the meshfilter
				const int boneMaterialIndex = 100; // if there is no material with id 100 ( should be define in the levels OLFT file )
				BoneObject* bone = new BoneObject(data->index, boneMaterialIndex, 5.0f);
				pMesh->m_BoneObjects[data->index] = bone;

				// set the global bind pose of the bone
				DirectX::XMFLOAT4X4 transform;
				DirectX::XMStoreFloat4x4(&transform, mSelf);
				bone->SetGlobalBindPose(transform);
				
				// transform the bone to the correct base position
				bone->GetTransform()->Translate(posSelf);
				bone->GetTransform()->Rotate(rotationSelf);
			}

			// the skeltondata that has a parrentindex that is negative is the root bone
			for (SkeletonData* data : pMesh->m_SkeletonData)
			{
				if (data->parrentIndex < 0)
				{
					pMesh->m_pBoneRoot = pMesh->m_BoneObjects[data->index];
				}
			}
		}
			break;
		default:
			break;
		}
	}

	/*  Code inbetween (rough explenation) that wasn't written by Nicky Fieu
		delete the binary reader 
	*/

	return pMesh;
}

void MeshFilterLoader::Destroy(MeshFilter* objToDestroy)
{
	SafeDelete(objToDestroy);
}
