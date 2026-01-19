#pragma once
#include "Player.h"
#include "PlayerDrone.h"
#include "Enemy.h"
#include "Bullet.h"
#include "MapChip.h"

struct MergedBlock {
	AABB aabb;
	MapChipType type;
};

struct Block {
	Transform worldTransform;
	std::unique_ptr<Object3d> object;
	AABB aabb;
	bool isActive = false;
	MapChipType type;
};

class Stage {
public:
    void Initialize();
    void Update();
	void Draw();

	/// <summary>
	/// マップチップの生成
	/// </summary>
	void GenerateBlocks();

    void ResolvePlayerCollision(Player& player, AxisXYZ axis);
	void ResolvePlayerDroneCollision(PlayerDrone& playerDrone, AxisXYZ axis);
    void ResolveEnemyCollision(Enemy& enemy, AxisXYZ axis);
	void ResolveBulletsCollision(const std::vector<Bullet*>& bullets);

    const std::vector<MergedBlock>& GetMergedBlocks() const;

	const std::vector<std::vector<Block>>& GetBlocks() const;

private:

	// ブロック用のワールドトランスフォーム
	std::vector<std::vector<Block>> blocks_;
	// マップチップ
	std::unique_ptr<MapChip> mapChip_ = nullptr;

	std::vector<MergedBlock> mergedBlocks_;
};
