/*
	Disclaimer: only code writen by Nicky Fieu are in these samples
*/

// this is an example used the overlord project to add the club ( weapon ) to the goblin ( enemie )

void GoblinGCController::AddWeapon(GameScene& currentScene)
{
	UNREFERENCED_PARAMETER(currentScene);
	const GameContext& gameContext = currentScene.GetGameContext();

	// check if the club material already exists if not create it
	const UINT clubMaterialID = 39;
	DaDfEmNmSp3D_ShadowMaterial* pWeaponMat = nullptr;
	if (!gameContext.pMaterialManager->HasMaterial(clubMaterialID))
	{
		pWeaponMat = new DaDfEmNmSp3D_ShadowMaterial();
		pWeaponMat->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());
		pWeaponMat->SetUseDiffuseMap(true);
		pWeaponMat->SetDiffuseTexture(L"./Resources/Textures/GameTextures/GoblinWeapon_Diffuse.png");
		pWeaponMat->SetUseNormalMap(true);
		pWeaponMat->SetNormalTexture(L"./Resources/Textures/GameTextures/GoblinWeapon_Normal.png");
		gameContext.pMaterialManager->AddMaterial(pWeaponMat, clubMaterialID);
	}

	// create the model for the club
	ModelComponent* pModel = new ModelComponent(L"./Resources/Meshes/Enemies/GoblinWeapon.ovm");
	pModel->SetMaterial(clubMaterialID);

	// get all the socket data needed to attach the club to a socket ( + extra transforms )
	GameObject* pSocket = static_cast<BoneObject*>(m_pGameCharacterRef->GetWeaponSocket());
	m_pWeaponSocketRef = new GameObject();
	m_pWeaponSocketRef->AddComponent(pModel);
	m_pWeaponSocketRef->GetTransform()->Scale(0.01f, 0.01f, 0.01f);
	m_pWeaponSocketRef->GetTransform()->Rotate(25.f, 30.f, -15.f);
	pSocket->AddChild(m_pWeaponSocketRef);

	// create the collision for the club
	physx::PxPhysics* pPhysX = PhysxManager::GetInstance()->GetPhysics();
	physx::PxMaterial* pMaterial = pPhysX->createMaterial(0.f, 0.f, 0.f);
	std::shared_ptr<physx::PxGeometry> boxCollision{ new physx::PxBoxGeometry(0.8f, 1.5f, 0.8f) };

	// add evrything we just created for the club to the club's gameobject / setting collision layers
	m_pWeapon = new GameObject();

	UINT32 collisionGroup = (UINT32)CollisionGroupFlag::Group1;
	RigidBodyComponent* pWeaponRigidBody = new RigidBodyComponent(false, collisionGroup, ~collisionGroup);
	pWeaponRigidBody->SetKinematic(true);

	ColliderComponent* pWeaponCollisionBox = new ColliderComponent(boxCollision, *pMaterial, physx::PxTransform{ -0.50f,1.6f,0.f, physx::PxQuat::createIdentity() });
	weaponCollisionBox->EnableTrigger(true);

	m_pWeapon->AddComponent(pWeaponRigidBody);
	m_pWeapon->AddComponent(pWeaponCollisionBox);

	// add the weapon to the scene 
	currentScene.AddChild(m_pWeapon);

	m_pWeapon->AddComponent(new ControllerRefComponent(this));
	m_pWeapon->SetOnTriggerCallBack(std::bind(&GoblinGCController::HandleClubCollision, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}
