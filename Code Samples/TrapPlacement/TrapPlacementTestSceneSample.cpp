#include "stdafx.h"
#include "TrapPlacementTestScene.h"

#include "PhysxProxy.h"
#include "PhysxManager.h"
#include "RayCast.h"

#include "Components.h"
#include "../../Materials/Materials.h"

#include "SpriteFont.h"
#include "TextRenderer.h"

#include "ContentManager.h"
#include "InputManager.h"
#include "MaterialManager.h"

#include "../PersonalClasses/SpikeTrapPrefab.h"

#include "../../LevelDataReader/LevelDataReader.h"

TrapPlacementTestScene::TrapPlacementTestScene()
	: GameScene(L"TrapPlacementTestScene")
{
	m_pFont = ContentManager::Load<SpriteFont>(L"./Resources/SpriteFonts/Consolas_20.fnt");
}

void TrapPlacementTestScene::Initialize()
{
	Logger::LogInfo(L"TrapPlacementScene - Left click to place a trap when it is posible ( green )");
	Logger::LogInfo(L"TrapPlacementScene - right click to delete a trap when posible ( red )");
	GetPhysxProxy()->EnablePhysxDebugRendering(true);
	
	const auto gameContext = GetGameContext();
	gameContext.pInput->ForceMouseToCenter(true);

	auto physX = PhysxManager::GetInstance()->GetPhysics();

	auto pGround = new GameObject();
	pGround->AddComponent(new RigidBodyComponent(true));
	auto groundMat = physX->createMaterial(1.f, 1.f, 1.f);
	std::shared_ptr<physx::PxGeometry> geom(new physx::PxPlaneGeometry());
	pGround->AddComponent(new ColliderComponent(geom, *groundMat, physx::PxTransform(physx::PxQuat(DirectX::XM_PIDIV2, physx::PxVec3(0, 0, 1)))));
	AddChild(pGround);
	
	
	LevelDataReader levelLoader{};
	levelLoader.ReadLevel(L"./Resources/Levels/TrapTestLevel.OLFT", m_LevelGameObjects, this);

	const UINT trapTextureMaterialID = 59;
	DiffuseMaterial* trapTextureMaterial = new DiffuseMaterial();
	trapTextureMaterial->SetDiffuseTexture(L"./Resources/Textures/GameTextures/SpikeTrap.png");
	gameContext.pMaterialManager->AddMaterial(trapTextureMaterial, trapTextureMaterialID);

	const UINT trapIndicationMaterialID = 60;
	m_pPlacementIndicationMaterial = new TrapMaterial();
	m_pPlacementIndicationMaterial->EnableColor1(true);
	gameContext.pMaterialManager->AddMaterial(m_pPlacementIndicationMaterial, trapIndicationMaterialID);

	const float trapVisualIndicatorSize = 0.2525f;
	ModelComponent* trapVisualIndicator = new ModelComponent(L"./Resources/Meshes/Traps/SpikeTrapFloor.ovm");
	trapVisualIndicator->SetMaterial(trapIndicationMaterialID);
	m_pTrapVisualIndicatorObject = new GameObject();
	m_pTrapVisualIndicatorObject->AddComponent(trapVisualIndicator);
	m_pTrapVisualIndicatorObject->GetTransform()->Rotate(90.f, 0.f, 0.f);
	m_pTrapVisualIndicatorObject->GetTransform()->Scale(trapVisualIndicatorSize, trapVisualIndicatorSize, trapVisualIndicatorSize);
	AddChild(m_pTrapVisualIndicatorObject);
}

void TrapPlacementTestScene::Update()
{
	bool canPlaceTrap = true; // in the actual game this would be if build mode is true and it was a trap ability
	if (canPlaceTrap)
		CheckForTrapAbleTiles();
}

void TrapPlacementTestScene::CheckForTrapAbleTiles()
{
	const auto gameContext = GetGameContext();

	CameraComponent* currentCamera = gameContext.pCamera;
	RayCastData placeAbleData{};
	DirectX::XMFLOAT3 rayPos{ 0.f,0.f,0.f };
	if (currentCamera->CheckForTrapPlaceAbleLocation(gameContext, placeAbleData, rayPos))
	{

		RayCastData trapData{};
		bool canDelete{};
		if (CheckIfTrapIsAlreadyThere(placeAbleData, trapData, rayPos, canDelete))
		{
			m_pPlacementIndicationMaterial->EnableColor1(true);
			m_pTrapVisualIndicatorObject->GetTransform()->Translate(rayPos);

			// removes the trap at the indicators location (RMB)
			if (GetGameContext().pInput->IsMouseButtonDown(0x02, false) && canDelete)
			{
				this->RemoveChild(trapData.gameObject->GetParent(), true);
			}
		}
		else if (CheckIfCanPlaceTrap(placeAbleData, rayPos))
		{
			m_pPlacementIndicationMaterial->EnableColor1(false);
			m_pTrapVisualIndicatorObject->GetTransform()->Translate(rayPos);

			// places a new trap at the indicators location  (LMB)
			if (GetGameContext().pInput->IsMouseButtonDown(0x01, false))
			{
				SpikeTrapPrefab* trap = new SpikeTrapPrefab(0.25f);
				trap->GetTransform()->Translate(rayPos);
				trap->GetTransform()->Rotate(90.f, 0.f, 0.f);
				trap->AddCollision(rayPos, DirectX::XMFLOAT3(49.f, 49.f, 6.f));
				AddChild(trap); // adds trap to this scene
			}
		}
	}
	else // cannot place trap (Note: if camera is looking at the void rayPos equals 0 0 0 !) 
	{
		m_pPlacementIndicationMaterial->EnableColor1(true);
		m_pTrapVisualIndicatorObject->GetTransform()->Translate(rayPos);
	}

}

bool TrapPlacementTestScene::CheckIfTrapIsAlreadyThere(const RayCastData& data, RayCastData& trapData, const DirectX::XMFLOAT3& rayPos, bool& canDelete)
{
	DirectX::XMFLOAT3 rayDir = data.hitSurfaceNormal;
	rayDir = DirectX::XMFLOAT3(-rayDir.x,-rayDir.y,-rayDir.z);

	// calculating the 4 points to check
	const float xzOffset = 25.f;
	const float yOffset = 50.f;
	DirectX::XMFLOAT3 rayPos1 = rayPos;
	rayPos1.x -= xzOffset;
	rayPos1.y += yOffset;
	rayPos1.z -= xzOffset;
	DirectX::XMFLOAT3 rayPos2 = rayPos1;
	rayPos2.z += 2.0f * xzOffset;
	DirectX::XMFLOAT3 rayPos3 = rayPos2;
	rayPos3.x += 2.0f * xzOffset;
	DirectX::XMFLOAT3 rayPos4 = rayPos3;
	rayPos4.z -= 2.0f * xzOffset;
	DirectX::XMFLOAT3 rayPositions[m_AmountOfRayChecks]{ rayPos1,rayPos2 ,rayPos3 ,rayPos4 };
	
	physx::PxQueryFilterData filter;
	filter.data.word0 = (UINT)CollisionGroupFlag::Group8;
	RayCastData cornerCheckData;
	bool isTrapAlreadyThere{ false };

	// raycasts the center of the current selected trap area
	isTrapAlreadyThere = RayCast(GetPhysxProxy(), rayPos, rayDir, PX_MAX_F32, filter, trapData);
	canDelete = isTrapAlreadyThere; // only allow deletion of traps when the selected trap fully overlaps with the visual idicator

	// raycasts each half corner point ( from center half the distance to the corner )
	for (UINT i = 0; i < m_AmountOfRayChecks; i++)
	{
		isTrapAlreadyThere = isTrapAlreadyThere || RayCast(GetPhysxProxy(), rayPositions[i], rayDir, PX_MAX_F32, filter, cornerCheckData);
	}

	return isTrapAlreadyThere;
}

bool TrapPlacementTestScene::CheckIfCanPlaceTrap(const RayCastData& placeAbleData, const DirectX::XMFLOAT3& rayPos)
{
	DirectX::XMFLOAT3 rayDir = placeAbleData.hitSurfaceNormal;
	rayDir = DirectX::XMFLOAT3(-rayDir.x,-rayDir.y,-rayDir.z);

	// calculating the 4 points to check
	const float xzOffset = 25.f;
	const float yOffset = 50.f;
	DirectX::XMFLOAT3 rayPos1 = rayPos;
	rayPos1.x -= xzOffset;
	rayPos1.y += yOffset;
	rayPos1.z -= xzOffset;
	DirectX::XMFLOAT3 rayPos2 = rayPos1;
	rayPos2.z += 2.0f * xzOffset;
	DirectX::XMFLOAT3 rayPos3 = rayPos2;
	rayPos3.x += 2.0f * xzOffset;
	DirectX::XMFLOAT3 rayPos4 = rayPos3;
	rayPos4.z -= 2.0f * xzOffset;
	DirectX::XMFLOAT3 rayPositions[m_AmountOfRayChecks]{ rayPos1,rayPos2 ,rayPos3 ,rayPos4 };

	physx::PxQueryFilterData filter;
	filter.data.word0 = (UINT)CollisionGroupFlag::Group9;
	RayCastData rayData[m_AmountOfRayChecks]{};

	bool CanPlaceTrap{true};

	for (UINT i = 0; i < m_AmountOfRayChecks; i++)
	{
		CanPlaceTrap = CanPlaceTrap && RayCast(GetPhysxProxy(), rayPositions[i], rayDir, PX_MAX_F32, filter, rayData[i]);
	}

	if (!CanPlaceTrap)
	{
		m_pPlacementIndicationMaterial->EnableColor1(true);
		return CanPlaceTrap;
	}
	
	for (UINT i = 1; i < m_AmountOfRayChecks; i++)
	{
		float y1 = rayData[i - 1].hitPos.y;
		float y2 = rayData[i].hitPos.y;
		const float maxYHalfOffset = 0.1f;
		CanPlaceTrap = CanPlaceTrap && (y1 >= (y2 - maxYHalfOffset)) && ( y1 <= (y2 + maxYHalfOffset));
	}

	if (!CanPlaceTrap)
	{
		m_pPlacementIndicationMaterial->EnableColor1(true);
		return CanPlaceTrap;
	}

	return CanPlaceTrap;
}

void TrapPlacementTestScene::Draw() const
{
	const auto gameContext = GetGameContext();
	if (m_pFont->GetFontName() != L"")
	{
		std::wstringstream sFps;
		sFps << L"FPS: " << gameContext.pGameTime->GetFPS();

		TextRenderer::GetInstance()->DrawText(m_pFont, sFps.str(), DirectX::XMFLOAT2(10, 10), DirectX::XMFLOAT4(DirectX::Colors::Yellow));
	}
}

// Camera code used in this sample 

bool CameraComponent::CheckForTrapPlaceAbleLocation(const GameContext&, RayCastData& placeData, DirectX::XMFLOAT3& rayPos) const
{
	PhysxProxy* physxProxy = this->GetGameObject()->GetScene()->GetPhysxProxy();

	TransformComponent* transform = this->GetTransform();
	DirectX::XMFLOAT3 forward = transform->GetForward();
	DirectX::XMVECTOR fCalc;
	fCalc = DirectX::XMLoadFloat3(&forward);
	fCalc = DirectX::XMVector3Normalize(fCalc);// just to be safe that this is an actual normalized vector

	DirectX::XMFLOAT3 direction;
	DirectX::XMStoreFloat3(&direction, fCalc);

	physx::PxQueryFilterData filterData{};
	filterData.data.word0 = ~(physx::PxU32)m_TrapPlaceIgnoreGroups;
	DirectX::XMFLOAT3 origin = transform->GetWorldPosition();

	bool ret = RayCast(physxProxy, origin, direction, m_TrapPlaceMaxDist, filterData, placeData);
	if (!ret)
		return false;

	rayPos = { placeData.hitPos };
	ToGrid(rayPos.x);
	ToGrid(rayPos.z);
	ToGrid(rayPos.y);
	rayPos.y += m_TrapSize;

	RayCastData trapableLocationData{};
	DirectX::XMFLOAT3 rayDir = { -placeData.hitSurfaceNormal.x ,-placeData.hitSurfaceNormal.y ,-placeData.hitSurfaceNormal.z };
	physx::PxQueryFilterData filter;
	filter.data.word0 = (UINT)CollisionGroupFlag::Group9;
	RayCast(physxProxy, rayPos, rayDir, (m_TrapSize * 2), filter, trapableLocationData);
	placeData.hitPos.y = trapableLocationData.hitPos.y;
	rayPos.y = trapableLocationData.hitPos.y;
	return true;
}

void CameraComponent::ToGrid(float& val) const
{
	float halfTrapSize = m_TrapSize * 0.5f;
	(val > 0.f) ? val += halfTrapSize : val -= halfTrapSize;
	val = (int(val / halfTrapSize) * 0.5f) * m_TrapSize;
};