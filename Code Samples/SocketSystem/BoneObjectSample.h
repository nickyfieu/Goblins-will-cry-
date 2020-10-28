#pragma once
#include "GameObject.h"

/*
	Disclaimer: only code writen by Nicky Fieu are in these samples
*/

class BoneObject final : public GameObject
{
public:
	BoneObject(int boneId, int materialId, float length = 5.0f);
	~BoneObject() = default;

	BoneObject(const BoneObject& other) = delete;
	BoneObject(BoneObject&& other) = delete;
	BoneObject& operator=(const BoneObject& other) = delete;
	BoneObject& operator=(BoneObject&& other) = delete;

	void CalculateBindPose();
	void AddBone(BoneObject* pBone);
	void SetCollisionObject(GameObject* pCollisionObject);

	inline void SetGlobalBindPose(const DirectX::XMFLOAT4X4& pose) { m_GlobalBindPose = pose; }

	bool HasCollisionObject() const { return m_pCollisionObj != nullptr; }

	GameObject* GetCollisionObject() const { return m_pCollisionObj; };

	inline DirectX::XMFLOAT4X4 GetBindPose() const { return m_BindPose; }
	inline DirectX::XMFLOAT4X4 GetGlobalBindPose() const { return m_GlobalBindPose; }

protected:
	virtual void Initialize(const GameContext& gameContext) override;

private:
	int m_BoneId;
	int m_MaterialID;

	float m_Length;

	DirectX::XMFLOAT4X4 m_BindPose;
	DirectX::XMFLOAT4X4 m_GlobalBindPose;
	
	GameObject* m_pCollisionObj = nullptr;
};
