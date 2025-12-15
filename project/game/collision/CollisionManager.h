#pragma once
#include "Collider.h"
#include <list>

class Enemy;
class Player;
class EnemyBullet;

class CollisionManager {

public:
	/// <summary>
	/// 衝突判定と応答
	/// </summary>
	void CheckAllCollisions(Player* player, Enemy* enemy);

	/// <summary>
	/// コライダー二つの衝突判定と応答
	/// </summary>
	/// <param name="colliderA">コライダーA</param>
	/// <param name="colliderB">コライダーB</param>
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);

	/// <summary>
	/// コライダーを設定する
	/// </summary>
	/// <param name="player"></param>
	/// <param name="enemy"></param>
	void SetColliders(Player* player, Enemy* enemy);

private:
	// コライダーリスト
	std::list<Collider*> colliders_;
};
