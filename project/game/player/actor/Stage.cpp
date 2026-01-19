#include "Stage.h"

void Stage::Initialize() {

	mapChip_ = std::make_unique<MapChip>();
	mapChip_->LoadMapChipCsv("resources/map.csv");
	GenerateBlocks();
}

void Stage::Update() {}

void Stage::Draw() {
	for (auto& line : blocks_) {
		for (auto& block : line) {
			if (!block.isActive) continue;

			block.object->Update();
			block.object->Draw();
		}
	}
}

void Stage::GenerateBlocks() {

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

void Stage::ResolvePlayerCollision(Player& player, AxisXYZ axis)
{

	AABB playerAABB = player.GetAABB();
	Vector3 playerPos = player.GetWorldPosition();
	Vector3 velocity = player.GetMove();

	const float kEpsilon = 0.01f;

	for (const MergedBlock& block : mergedBlocks_) {

		if (!IsCollision(playerAABB, block.aabb)) {
			continue;
		}

		// ダメージ床
		if (block.type == MapChipType::kDamageBlock) {
			//if (!enemy.IsDead()) {
			//	player.Damage();
			//}
		}

		Vector3 overlap = {
			std::min(playerAABB.max.x, block.aabb.max.x) - std::max(playerAABB.min.x, block.aabb.min.x),
			std::min(playerAABB.max.y, block.aabb.max.y) - std::max(playerAABB.min.y, block.aabb.min.y),
			0.0f
		};

		// --------------------
		// X方向衝突
		// --------------------
		if (axis == X) {
			if (playerAABB.min.x < block.aabb.min.x) {
				playerPos.x -= overlap.x + kEpsilon;
			} else {
				playerPos.x += overlap.x + kEpsilon;
			}
			velocity.x = 0.0f;
		}

		// --------------------
		// Y方向衝突
		// --------------------
		if (axis == Y) {
			if (playerAABB.min.y < block.aabb.min.y) {
				playerPos.y -= overlap.y + kEpsilon;
			} else {
				playerPos.y += overlap.y + kEpsilon;
			}
			velocity.y = 0.0f;
		}

		player.SetWorldPosition(playerPos);
		player.SetVelocity(velocity);
		playerAABB = player.GetAABB();
	}
}

void Stage::ResolvePlayerDroneCollision(PlayerDrone& playerDrone, AxisXYZ axis)
{

	AABB playerAABB = playerDrone.GetAABB();
	Vector3 playerPos = playerDrone.GetWorldPosition();
	Vector3 velocity = playerDrone.GetMove();

	const float kEpsilon = 0.01f;

	for (const MergedBlock& block : mergedBlocks_) {

		if (!IsCollision(playerAABB, block.aabb)) {
			continue;
		}

		// ダメージ床
		if (block.type == MapChipType::kDamageBlock) {
			//if (!enemy.IsDead()) {
			//	player.Damage();
			//}
		}

		Vector3 overlap = {
			std::min(playerAABB.max.x, block.aabb.max.x) - std::max(playerAABB.min.x, block.aabb.min.x),
			std::min(playerAABB.max.y, block.aabb.max.y) - std::max(playerAABB.min.y, block.aabb.min.y),
			0.0f
		};

		// --------------------
		// X方向衝突
		// --------------------
		if (axis == X) {
			if (playerAABB.min.x < block.aabb.min.x) {
				playerPos.x -= overlap.x + kEpsilon;
			} else {
				playerPos.x += overlap.x + kEpsilon;
			}
			velocity.x = 0.0f;
		}

		// --------------------
		// Y方向衝突
		// --------------------
		if (axis == Y) {
			if (playerAABB.min.y < block.aabb.min.y) {
				playerPos.y -= overlap.y + kEpsilon;
			} else {
				playerPos.y += overlap.y + kEpsilon;
			}
			velocity.y = 0.0f;
		}

		playerDrone.SetWorldPosition(playerPos);
		playerDrone.SetVelocity(velocity);
		playerAABB = playerDrone.GetAABB();
	}
}

void Stage::ResolveEnemyCollision(Enemy& enemy, AxisXYZ axis)
{
	AABB enemyAABB = enemy.GetAABB();
	Vector3 enemyPos = enemy.GetWorldPosition();
	const float kEpsilon = 0.01f;
	//bool hit = false;

	for (const MergedBlock& block : mergedBlocks_) {

		if (!IsCollision(enemyAABB, block.aabb)) {
			continue;
		}

		//hit = true;

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

		enemy.SetWorldPosition(enemyPos);
		enemyAABB = enemy.GetAABB();
	}
}

void Stage::ResolveBulletsCollision(const std::vector<Bullet*>& bullets)
{
	for (Bullet* bullet : bullets) {
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

			if (bullet->IsReflectable()) {

				bullet->SetVelocity(vel);
				bullet->SetWorldPosition(bulletPos);
			} else {
				// 反射しない弾は消滅
				bullet->SetWorldPosition(bulletPos);
				bullet->Die();
			}

		}
	}
}

const std::vector<MergedBlock>& Stage::GetMergedBlocks() const
{
	return mergedBlocks_;
}

const std::vector<std::vector<Block>>& Stage::GetBlocks() const {
	return blocks_;
}