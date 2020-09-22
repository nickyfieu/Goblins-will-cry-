#pragma once
#include "BaseComponent.h"
#include <unordered_map>
#include <string>

/*
	Disclaimer: only code writen by Nicky Fieu are in these samples
*/

struct SocketData
{
	SocketData() = default;
	GameObject* socket = nullptr;
	UINT socketIndex = {};
	std::wstring name = {};
	short boneIndex = {};
};

class ModelComponent : public BaseComponent
{
public:
	ModelComponent(const ModelComponent& other) = delete;
	ModelComponent(ModelComponent&& other) noexcept = delete;
	ModelComponent& operator=(const ModelComponent& other) = delete;
	ModelComponent& operator=(ModelComponent&& other) noexcept = delete;
	ModelComponent(std::wstring  assetFile, bool castShadows = true);
	virtual ~ModelComponent();
	
	GameObject* GetSocket(UINT socketID) const;
	int GetSocketIndexByName(const std::wstring& socketName) const;
	void AddSocket(short boneId, const std::wstring& socketName, const DirectX::XMFLOAT4X4& socketOffset);
	void TransformSocket(UINT socket, const DirectX::XMMATRIX& transform);
	
private:
	UINT m_NrOfSockets = 0;
	// socket index , socket data
	std::unordered_map<UINT, SocketData*> m_Sockets;
};
