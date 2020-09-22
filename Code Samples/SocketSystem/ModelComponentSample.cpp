#include "stdafx.h"
#include "ModelComponent.h"
#include "ContentManager.h"
#include "MeshFilter.h"
#include "Material.h"
#include "ModelAnimator.h"
#include "Components.h"

/*
	Disclaimer: only code writen by Nicky Fieu are in these samples
*/

ModelComponent::~ModelComponent()
{
	for (std::pair<UINT, SocketData*> socket : m_Sockets)
	{
		SafeDelete(socket.second);
	}
	m_Sockets.clear();
}

void ModelComponent::Initialize(const GameContext& gameContext)
{
	if (m_pMeshFilter->m_HasAnimations)
	{
		for (BoneObject* toAdd : m_pMeshFilter->m_BoneObjects)
			if (!toAdd->GetParent())
				this->GetGameObject()->AddChild(toAdd);
	}
};

GameObject* ModelComponent::GetSocket(UINT socketID) const
{
	if (socketID < m_NrOfSockets)
	{
		return m_Sockets.at(socketID)->socket;
	}
	Logger::LogError(L"ModelAnimator::GetSocket invalid socket");
	return nullptr;
}

void ModelComponent::AddSocket(short boneId, const std::wstring& socketName, const DirectX::XMFLOAT4X4& socketOffset)
{
	if (boneId < 0)
		return Logger::LogWarning(L"ModelAnimator::AddSocket trying to add a socket with an index smaller than 0!");
	
	if (m_Sockets[m_NrOfSockets] != nullptr)
		return Logger::LogWarning(L"ModelAnimator::AddSocket socket was not created");

	SocketData* socketData = new SocketData();
	socketData->socket = new GameObject();
	socketData->boneIndex = boneId;
	socketData->name = socketName;
	socketData->socketIndex = m_NrOfSockets;

	BoneObject* boneobj = nullptr;
	if ((size_t)boneId < m_pMeshFilter->m_BoneObjects.size())
	{
		boneobj = m_pMeshFilter->GetBoneObjectByIndex(boneId);
	}

	if (boneobj == nullptr)
	{
		SafeDelete(socket);
		return Logger::LogWarning(L"ModelAnimator::AddSocket invalid bone id");
	}
	
	DirectX::XMVECTOR pos;
	DirectX::XMVECTOR rot;
	DirectX::XMVECTOR size;
	DirectX::XMFLOAT3 s;
	DirectX::XMMatrixDecompose(&size, &rot, &pos, DirectX::XMLoadFloat4x4(&socketOffset));
	DirectX::XMStoreFloat3(&s, size);
	auto transform = socketData->socket->GetTransform();
	transform->Translate(pos);
	transform->Rotate(rot);
	transform->Scale(s);
	
	boneobj->AddChild(socketData->socket);
	m_Sockets[socketData->socketIndex] = socketData;
	m_NrOfSockets++;
}

void ModelComponent::TransformSocket(UINT socket, const DirectX::XMMATRIX& transform)
{
	if (socket >= m_NrOfSockets)
		return;
	SocketData* socketData = m_Sockets.at(socket);
	DirectX::XMVECTOR pos;
	DirectX::XMVECTOR rot;
	DirectX::XMVECTOR size;
	DirectX::XMFLOAT3 s;
	DirectX::XMMatrixDecompose(&size, &rot, &pos, transform);
	DirectX::XMStoreFloat3(&s, size);
	auto transformc = socketData->socket->GetTransform();
	transformc->Translate(pos);
	transformc->Rotate(rot);
	transformc->Scale(s);
}

int ModelComponent::GetSocketIndexByName(const std::wstring& socketName) const
{
	for (std::pair<UINT, SocketData*> socket : m_Sockets)
		if (socket.second != nullptr)
			if (socket.second->name.compare(socketName) == 0)
				return socket.second->socketIndex;

	Logger::LogWarning(L"ModelAnimator::GetSocketIndexByName returning 0 due to not finding a socket with given name : " + socketName);
	return -1;
}
