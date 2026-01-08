#include "GameScene.h"

GameScene::GameScene() {}

GameScene::~GameScene() {

}

void GameScene::Initialize() {

	input_ = Input::GetInstance();

	debugCamera = std::make_unique<DebugCamera>();

	camera = std::make_unique<Camera>();

	camera->SetTranslate(Vector3(17.0f, 21.0f, -80.0f));

	Object3dCommon::GetInstance()->SetDefaultCamera(camera.get());
	Object3dCommon::GetInstance()->SetDebugDefaultCamera(debugCamera.get());

	object3d = std::make_unique<Object3d>();
	object3d->Initialize();

	enemyObject_ = std::make_unique<Object3d>();
	enemyObject_->Initialize();

	object3d3 = std::make_unique<Object3d>();
	object3d3->Initialize();

	playerObject_ = std::make_unique<Object3d>();
	playerObject_->Initialize();

	object3d->SetModel("teapot.obj");
	enemyObject_->SetModel("enemy.obj");
	object3d3->SetModel("plane.obj");
	playerObject_->SetModel("player.obj");

	mapChip_ = std::make_unique<MapChip>();
	mapChip_->LoadMapChipCsv("resources/map.csv");
	GenerateBlocks();

	player_ = std::make_unique<Player>();
	player_->Initialize(playerObject_.get(), Vector3(30.0f, 30.0f, 0.0f));

	// 敵キャラの生成
	enemy_ = std::make_unique<Enemy>();
	// 敵キャラの初期化
	enemy_->SetPlayer(player_.get());
	enemy_->Initialize(enemyObject_.get(), Vector3(20.0f, 20.0f, 0.0f), this);

	// 衝突マネージャの生成
	collisionManager_ = std::make_unique<CollisionManager>();

	fade_ = std::make_unique<Fade>();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	shotGide = std::make_unique<Sprite>();
	shotGide->Initialize(SpriteCommon::GetInstance(), "resources/shotGide.png");
	shotGide->SetPosition({ 20.0f, 450.0f });
	shotGide->SetSize({ 200.0f, 50.0f });

	wasdGide = std::make_unique<Sprite>();
	wasdGide->Initialize(SpriteCommon::GetInstance(), "resources/wasd.png");
	wasdGide->SetPosition({ 20.0f, 530.0f });
	wasdGide->SetSize({ 200.0f, 50.0f });

	toTitleGide = std::make_unique<Sprite>();
	toTitleGide->Initialize(SpriteCommon::GetInstance(), "resources/toTitle.png");
	toTitleGide->SetPosition({ 20.0f, 610.0f });
	toTitleGide->SetSize({ 200.0f, 50.0f });
}

void GameScene::Update() {

	worldTransform_.translate.y += 0.01f;

	object3d->SetTranslate(worldTransform_.translate);
	object3d->Update();

	camera->Update();
	debugCamera->Update(input_->GetMouseState(), input_->GetKey(), input_->GetLeftStick());

	player_->Update(camera.get());

	// 弾とブロックの当たり判定
	CheckCollisionBulletsAndBlocks();

	// X軸移動
	Vector3 playerPos = player_->GetWorldPosition();
	playerPos.x += player_->GetMove().x;
	player_->SetWorldPosition(playerPos);
	CheckCollisionPlayerAndBlocks(X);

	// Y軸移動
	playerPos = player_->GetWorldPosition();
	playerPos.y += player_->GetMove().y;
	player_->SetWorldPosition(playerPos);
	CheckCollisionPlayerAndBlocks(Y);

	enemy_->Update();

	// 敵の弾とブロックの当たり判定
	CheckCollisionEnemyBulletsAndBlocks();

	Vector3 baseDir = enemy_->GetDir();
	
	// X移動
	Vector3 pos = enemy_->GetWorldPosition();
	pos.x += enemy_->GetMove().x;
	enemy_->SetWorldPosition(pos);
	bool hitX = CheckCollisionEnemyAndBlocks(X);

	// Y移動
	pos = enemy_->GetWorldPosition();
	pos.y += enemy_->GetMove().y;
	enemy_->SetWorldPosition(pos);
	bool hitY = CheckCollisionEnemyAndBlocks(Y);

	// 衝突マネージャの更新
	collisionManager_->CheckAllCollisions(player_.get(), enemy_.get());

	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();

		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}
		break;
	case Phase::kMain:
		if (input_->IsTrigger(input_->GetKey()[DIK_ESCAPE], input_->GetPreKey()[DIK_ESCAPE])) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kFadeOut;
		}

		if (player_->isFinished()) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kFadeOut;
		}

		if (enemy_->isFinished()) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kFadeOut;
		}

		break;
	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}
		break;
	}

	shotGide->Update();
	wasdGide->Update();
	toTitleGide->Update();

	if (cameraFollow_) {

		// プレイヤーが死んでいる場合カメラをプレイヤーに合わせる
		if (player_->IsDead()) {
			Vector3 playerPos = player_->GetWorldPosition();
			camera->SetTranslate({ playerPos.x, playerPos.y, -50.0f });
			cameraFollow_ = false;
		}

		// 敵が死んでいる場合カメラを敵に合わせる
		if (enemy_->IsDead()) {
			Vector3 enemyPos = enemy_->GetWorldPosition();
			camera->SetTranslate({ enemyPos.x, enemyPos.y, -50.0f });
			cameraFollow_ = false;
		}
	}
}

void GameScene::Draw() {

	Object3dCommon::GetInstance()->PreDraw();

	player_->Draw();

	enemy_->Draw();

	for (auto& line : blocks_) {
		for (auto& block : line) {
			if (!block.isActive) continue;

			block.object->Update();
			block.object->Draw();
		}
	}
}

void GameScene::DrawSprite() {

	enemy_->DrawSprite();
	player_->DrawSprite();
	shotGide->Draw();
	wasdGide->Draw();
	toTitleGide->Draw();
	fade_->Draw();
}

void GameScene::GenerateBlocks() {

	uint32_t numBlockHorizontal = mapChip_->GetNumBlockHorizontal();
	uint32_t numBlockVirtical = mapChip_->GetNumBlockVirtical();

	blocks_.resize(numBlockVirtical);
	for (uint32_t y = 0; y < numBlockVirtical; ++y) {
		blocks_[y].resize(numBlockHorizontal);
	}

	for (uint32_t y = 0; y < numBlockVirtical; ++y) {
		for (uint32_t x = 0; x < numBlockHorizontal; ++x) {

			if (mapChip_->GetMapChipTypeByIndex(x, y) == MapChipType::kBlock || mapChip_->GetMapChipTypeByIndex(x, y) == MapChipType::kDamageBlock) {

				Block& block = blocks_[y][x];

				block.worldTransform = InitWorldTransform();
				block.worldTransform.translate = mapChip_->GetMapChipPositionByIndex(x, y);
				block.object = std::make_unique<Object3d>();
				block.object->Initialize();
				if (mapChip_->GetMapChipTypeByIndex(x, y) == MapChipType::kBlock) {
					block.object->SetModel("cube.obj");
				} else {
					block.object->SetModel("cubeDamage.obj");
				}
				
				block.object->SetTransform(block.worldTransform);

				// 中心座標
				const auto& pos = block.worldTransform.translate;

				// AABB設定
				block.aabb.min = { pos.x - mapChip_->kBlockWidth / 2.0f, pos.y - mapChip_->kBlockHeight / 2.0f, pos.z - mapChip_->kBlockWidth / 2.0f };
				block.aabb.max = { pos.x + mapChip_->kBlockWidth / 2.0f, pos.y + mapChip_->kBlockHeight / 2.0f, pos.z + mapChip_->kBlockWidth / 2.0f };
				block.isActive = true;
				block.type = mapChip_->GetMapChipTypeByIndex(x, y);
			}
		}
	}

	// ブロック統合処理 
	auto FixAABB = [](AABB& aabb) {
		if (aabb.min.x > aabb.max.x)
			std::swap(aabb.min.x, aabb.max.x);
		if (aabb.min.y > aabb.max.y)
			std::swap(aabb.min.y, aabb.max.y);
		if (aabb.min.z > aabb.max.z)
			std::swap(aabb.min.z, aabb.max.z);
		};

	mergedBlocks_.clear();
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		bool merging = false;
		AABB mergedAABB{};
		MapChipType currentType{};

		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			const Block& block = blocks_[i][j];

			if (block.isActive) {
				if (!merging) {
					mergedAABB = block.aabb;
					currentType = block.type;
					merging = true;
				}
				// タイプが同じなら統合
				else if (block.type == currentType) {
					mergedAABB.max.x = block.aabb.max.x;
				}
				// タイプが違うなら確定
				else {
					mergedBlocks_.push_back({ mergedAABB, currentType });
					mergedAABB = block.aabb;
					currentType = block.type;
				}
			} else if (merging) {
				mergedBlocks_.push_back({ mergedAABB, currentType });
				merging = false;
			}
		}

		if (merging) {
			mergedBlocks_.push_back({ mergedAABB, currentType });
		}
	}
}

void GameScene::CheckCollisionPlayerAndBlocks(AxisXYZ axis) {

	AABB playerAABB = player_->GetAABB();
	Vector3 playerPos = player_->GetWorldPosition();
	const float kEpsilon = 0.01f;

	for (const MergedBlock& block : mergedBlocks_) {

		if (!IsCollision(playerAABB, block.aabb)) {
			continue;
		}

		if (block.type == MapChipType::kDamageBlock) {
			
			// 敵が死んでいなければダメージ
			if (!enemy_->IsDead()) {
				player_->Damage();
			}
		}

		Vector3 overlap = {
			std::min(playerAABB.max.x, block.aabb.max.x) - std::max(playerAABB.min.x, block.aabb.min.x),
			std::min(playerAABB.max.y, block.aabb.max.y) - std::max(playerAABB.min.y, block.aabb.min.y),
			0.0f
		};

		if (axis == X) {
			playerPos.x += (playerAABB.min.x < block.aabb.min.x)
				? -(overlap.x + kEpsilon)
				: +(overlap.x + kEpsilon);
		} else if (axis == Y) {
			playerPos.y += (playerAABB.min.y < block.aabb.min.y)
				? -(overlap.y + kEpsilon)
				: +(overlap.y + kEpsilon);
		}

		player_->SetWorldPosition(playerPos);
		playerAABB = player_->GetAABB();
	}
}

bool GameScene::CheckCollisionEnemyAndBlocks(AxisXYZ axis) {

	AABB enemyAABB = enemy_->GetAABB();
	Vector3 enemyPos = enemy_->GetWorldPosition();
	const float kEpsilon = 0.01f;
	bool hit = false;

	for (const MergedBlock& block : mergedBlocks_) {

		if (!IsCollision(enemyAABB, block.aabb)) {
			continue;
		}

		hit = true;

		Vector3 overlap = {
			std::min(enemyAABB.max.x, block.aabb.max.x) - std::max(enemyAABB.min.x, block.aabb.min.x),
			std::min(enemyAABB.max.y, block.aabb.max.y) - std::max(enemyAABB.min.y, block.aabb.min.y),
			0.0f
		};

		if (axis == X) {
			enemyPos.x += (enemyAABB.min.x < block.aabb.min.x)
				? -(overlap.x + kEpsilon)
				: +(overlap.x + kEpsilon);
		} else if (axis == Y) {
			enemyPos.y += (enemyAABB.min.y < block.aabb.min.y)
				? -(overlap.y + kEpsilon)
				: +(overlap.y + kEpsilon);
		}

		enemy_->SetWorldPosition(enemyPos);
		enemyAABB = enemy_->GetAABB();
	}

	return hit;
}

void GameScene::CheckCollisionBulletsAndBlocks() {

	auto bullets = player_->GetBulletPtrs();

	for (PlayerBullet* bullet : bullets) {
		if (!bullet) continue;

		Vector3 bulletPos = bullet->GetWorldPosition();
		float radius = bullet->GetRadius();
		Sphere bulletSphere{ bulletPos, radius };

		bool isCollided = false;
		float nearestDist = std::numeric_limits<float>::max();
		Vector3 nearestClosestPoint{};
		Vector3 nearestNormal{};

		for (const auto& block : mergedBlocks_) {

			if (!IsCollision(block.aabb, bulletSphere)) continue;

			Vector3 closestPoint{
				std::clamp(bulletSphere.center.x, block.aabb.min.x, block.aabb.max.x),
				std::clamp(bulletSphere.center.y, block.aabb.min.y, block.aabb.max.y),
				std::clamp(bulletSphere.center.z, block.aabb.min.z, block.aabb.max.z)
			};

			Vector3 normal = bulletSphere.center - closestPoint;
			normal.z = 0.0f;
			if (Length(normal) < 0.0001f) continue;
			normal = Normalize(normal);

			float dist = Length(bulletSphere.center - closestPoint);
			if (dist < nearestDist) {
				nearestDist = dist;
				nearestClosestPoint = closestPoint;
				nearestNormal = normal;
				isCollided = true;
			}
		}

		if (isCollided) {
			bulletPos = nearestClosestPoint + nearestNormal * (radius + 0.01f);

			Vector3 vel = bullet->GetMove();
			vel = vel - 2.0f * Dot(vel, nearestNormal) * nearestNormal;

			bullet->SetVelocity(vel);
			bullet->SetWorldPosition(bulletPos);
		}
	}
}

void GameScene::CheckCollisionEnemyBulletsAndBlocks() {

	auto bullets = enemy_->GetBulletPtrs();

	for (EnemyBullet* bullet : bullets) {
		if (!bullet) continue;

		Vector3 bulletPos = bullet->GetWorldPosition();
		float radius = bullet->GetRadius();
		Sphere bulletSphere{ bulletPos, radius };

		bool isCollided = false;
		float nearestDist = std::numeric_limits<float>::max();
		Vector3 nearestClosestPoint{};
		Vector3 nearestNormal{};

		for (const auto& block : mergedBlocks_) {

			if (!IsCollision(block.aabb, bulletSphere)) continue;

			Vector3 closestPoint{
				std::clamp(bulletSphere.center.x, block.aabb.min.x, block.aabb.max.x),
				std::clamp(bulletSphere.center.y, block.aabb.min.y, block.aabb.max.y),
				std::clamp(bulletSphere.center.z, block.aabb.min.z, block.aabb.max.z)
			};

			Vector3 normal = bulletSphere.center - closestPoint;
			normal.z = 0.0f;
			if (Length(normal) < 0.0001f) continue;
			normal = Normalize(normal);

			float dist = Length(bulletSphere.center - closestPoint);
			if (dist < nearestDist) {
				nearestDist = dist;
				nearestClosestPoint = closestPoint;
				nearestNormal = normal;
				isCollided = true;
			}
		}

		if (isCollided) {
			bulletPos = nearestClosestPoint + nearestNormal * (radius + 0.01f);

			Vector3 vel = bullet->GetMove();
			vel = vel - 2.0f * Dot(vel, nearestNormal) * nearestNormal;

			bullet->SetVelocity(vel);
			bullet->SetWorldPosition(bulletPos);
		}
	}
}

const std::vector<std::vector<Block>>& GameScene::GetBlocks() const {
    return blocks_;
}