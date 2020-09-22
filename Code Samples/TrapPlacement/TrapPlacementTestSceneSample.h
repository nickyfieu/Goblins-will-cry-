#pragma once
#include "GameScene.h"
#include <unordered_map>

class SpriteFont;
class CubePrefab;
class TrapMaterial;
class TrapPlacementTestScene final : public GameScene
{
public:
	TrapPlacementTestScene();
	virtual ~TrapPlacementTestScene() = default;

	TrapPlacementTestScene(const TrapPlacementTestScene& other) = delete;
	TrapPlacementTestScene(TrapPlacementTestScene&& other) noexcept = delete;
	TrapPlacementTestScene& operator=(const TrapPlacementTestScene& other) = delete;
	TrapPlacementTestScene& operator=(TrapPlacementTestScene&& other) noexcept = delete;

	// functions to be moved out later on
	void CheckForTrapAbleTiles();
	bool CheckIfTrapIsAlreadyThere(const RayCastData& data, RayCastData& trapData,const DirectX::XMFLOAT3& rayPos, bool& canDelete);
	bool CheckIfCanPlaceTrap(const RayCastData& placeAbleData, const DirectX::XMFLOAT3& rayPos);
	
protected:
	void Initialize() override;
	void Update() override;
	void Draw() const override;

private:
	// static for size of some arrays!
	const static UINT m_AmountOfRayChecks = 4;
	TrapMaterial* m_pPlacementIndicationMaterial;
	GameObject* m_pTrapVisualIndicatorObject;

	SpriteFont* m_pFont = nullptr;

	std::unordered_map<int, GameObject*> m_LevelGameObjects;
};