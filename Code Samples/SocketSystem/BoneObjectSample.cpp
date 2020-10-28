#include "stdafx.h"
#include "BoneObject.h"
#include "Components.h"
#include "CubePrefab.h"
#include "PhysxManager.h"

/*
	Disclaimer: only code writen by Nicky Fieu are in these samples
*/

BoneObject::BoneObject(int boneId, int materialId, float length)
	: GameObject()
	, m_BoneId(boneId)
	, m_MaterialID(materialId)
	, m_Length(length)
	, m_pCollisionObj{nullptr}
{
	// default initializes bindpose to a identity matrix
	DirectX::XMStoreFloat4x4(&m_BindPose, DirectX::XMMatrixIdentity());
}

// parrents the given bone object to the this bone object and translates it to the end of this bone.
void BoneObject::AddBone(BoneObject* pBone)
{
	pBone->GetTransform()->Translate(m_Length, 0.f, 0.f);
	AddChild(pBone);
}

void BoneObject::CalculateBindPose()
{
	// to get this bones bind pose we get the parrents ( if there is one ) world transform and take the inverse of it
	BoneObject* Parent = static_cast<BoneObject*>(this->GetParent());
	
	if (Parent == nullpr)
	{
		return;
	}
	
	DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&Parent->GetTransform()->GetWorld());
	DirectX::XMStoreFloat4x4(&m_BindPose, DirectX::XMMatrixInverse(nullptr, world));
}

void BoneObject::Initialize(const GameContext& gameContext)
{
	UNREFERENCED_PARAMETER(gameContext);
}

// parrenting the collision object to this bone object
void BoneObject::SetCollisionObject(GameObject* pCollisionObject)
{
	if (pCollisionObject == nullptr)
	{
		return Logger::LogInfo(L"BoneObject::SetCollisionObject invalid collisionObject!");
	}
		
	if (pCollisionObject->HasComponent<RigidBodyComponent>())
	{
		return Logger::LogError(L"BoneObject::SetCollisionObject collisionObject cannot have a rigidbody component!");
	}
		
	AddChild(pCollisionObject);
	m_pCollisionObj = pCollisionObject;
}
