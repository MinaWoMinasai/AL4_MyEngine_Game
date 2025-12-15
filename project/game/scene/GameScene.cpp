#include "GameScene.h"

GameScene::GameScene() {}

GameScene::~GameScene() {

}

void GameScene::Initialize() {

	input_ = Input::GetInstance();

	debugCamera = std::make_unique<DebugCamera>();

	camera = std::make_unique<Camera>();
	
	camera->SetTranslate(Vector3(23.0f, 20.7f, -80.0f));

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
	enemyObject_->SetModel("bunny.obj");
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
	enemy_->Initialize(enemyObject_.get(), Vector3(20.0f, 20.0f, 0.0f));

	// 衝突マネージャの生成
	collisionManager_ = std::make_unique<CollisionManager>();

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

	// 衝突マネージャの更新
	collisionManager_->CheckAllCollisions(player_.get(), enemy_.get());

}

void GameScene::Draw() {

	Object3dCommon::GetInstance()->PreDraw();

	player_->Draw();

	enemy_->Draw();

	object3d->Draw();
	
	for (auto& line : blocks_) {
		for (auto& block : line) {
			if (!block.isActive) continue;

			block.object->Update();
			block.object->Draw();
		}
	}
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

			if (mapChip_->GetMapChipTypeByIndex(x, y) == MapChipType::kBlock) {

				Block& block = blocks_[y][x];

				block.worldTransform = InitWorldTransform();
				block.worldTransform.translate = mapChip_->GetMapChipPositionByIndex(x, y);
				block.object = std::make_unique<Object3d>();
				block.object->Initialize();
				block.object->SetModel("cube.obj");
				block.object->SetTransform(block.worldTransform);

				// 中心座標
				const auto& pos = block.worldTransform.translate;

				// AABB設定
				block.aabb.min = { pos.x - mapChip_->kBlockWidth / 2.0f, pos.y - mapChip_->kBlockHeight / 2.0f, pos.z - mapChip_->kBlockWidth / 2.0f };
				block.aabb.max = { pos.x + mapChip_->kBlockWidth / 2.0f, pos.y + mapChip_->kBlockHeight / 2.0f, pos.z + mapChip_->kBlockWidth / 2.0f };
				block.isActive = true; 
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

	// 横方向の連続ブロックを統合
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		bool merging = false;
		AABB mergedAABB{};

		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			const Block& block = blocks_[i][j];
			if (block.isActive) {
				if (!merging) {
					mergedAABB = block.aabb;
					merging = true;
				} else {
					mergedAABB.max.x = block.aabb.max.x;
				}
			} else if (merging) {
				mergedBlocks_.push_back(mergedAABB);
				merging = false;
			}
		}
		if (merging) {
			mergedBlocks_.push_back(mergedAABB);
		}
	}
}

void GameScene::CheckCollisionPlayerAndBlocks(AxisXYZ axis) {

	AABB playerAABB = player_->GetAABB();
	Vector3 playerPos = player_->GetWorldPosition();
	const float kEpsilon = 0.01f;

	for (const AABB& blockAABB : mergedBlocks_) {

		if (!IsCollision(playerAABB, blockAABB)) {
			continue;
		}

		Vector3 overlap = {
			std::min(playerAABB.max.x, blockAABB.max.x) - std::max(playerAABB.min.x, blockAABB.min.x),
			std::min(playerAABB.max.y, blockAABB.max.y) - std::max(playerAABB.min.y, blockAABB.min.y),
			0.0f
		};

		if (axis == X) {
			playerPos.x += (playerAABB.min.x < blockAABB.min.x)
				? -(overlap.x + kEpsilon)
				: +(overlap.x + kEpsilon);
		} else if (axis == Y) {
			playerPos.y += (playerAABB.min.y < blockAABB.min.y)
				? -(overlap.y + kEpsilon)
				: +(overlap.y + kEpsilon);
		}

		player_->SetWorldPosition(playerPos);
		playerAABB = player_->GetAABB();
	}
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

		for (const auto& blockAABB : mergedBlocks_) {

			if (!IsCollision(blockAABB, bulletSphere)) continue;

			Vector3 closestPoint{
				std::clamp(bulletSphere.center.x, blockAABB.min.x, blockAABB.max.x),
				std::clamp(bulletSphere.center.y, blockAABB.min.y, blockAABB.max.y),
				std::clamp(bulletSphere.center.z, blockAABB.min.z, blockAABB.max.z)
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
