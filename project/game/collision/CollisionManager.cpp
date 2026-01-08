#include "CollisionManager.h"
#include "Enemy.h"
#include "Player.h"

void CollisionManager::CheckAllCollisions(Player* player, Enemy* enemy) {

	// 衝突マネージャのリストをクリア
	colliders_.clear();

	// コライダーをリストに登録
	SetColliders(player, enemy);
	
	// リスト内のペアの総当たり
	std::list<Collider*>::iterator itrA = colliders_.begin();
	for (; itrA != colliders_.end(); ++itrA) {

		// イテレータAからコライダーAを取得
		Collider* colliderA = *itrA;

		std::list<Collider*>::iterator itrB = itrA;
		itrB++;

		for (; itrB != colliders_.end(); ++itrB) {

			// イテレータBからコライダーBを取得
			Collider* colliderB = *itrB;

			// ペアのあたり判定
			CheckCollisionPair(colliderA, colliderB);
		}
	}
}

void CollisionManager::CheckCollisionPair(Collider* colliderA, Collider* colliderB) {

	bool hit = false;

	// --- Sphere × Sphere ---
	if (colliderA->GetShape() == ColliderShape::Sphere && colliderB->GetShape() == ColliderShape::Sphere) {
		float dist = Length(colliderA->GetWorldPosition() - colliderB->GetWorldPosition());
		hit = dist < (colliderA->GetRadius() + colliderB->GetRadius());
	}

	// --- Capsule（Laser） × Sphere ---
	else if (colliderA->GetShape() == ColliderShape::Capsule && colliderB->GetShape() == ColliderShape::Sphere) {
		Sphere s{colliderB->GetWorldPosition(), colliderB->GetRadius()};
		hit = IsCollision(colliderA->GetSegment(), s, colliderA->GetCapsuleRadius());
	} else if (colliderB->GetShape() == ColliderShape::Capsule && colliderA->GetShape() == ColliderShape::Sphere) {
		Sphere s{colliderA->GetWorldPosition(), colliderA->GetRadius()};
		hit = IsCollision(colliderB->GetSegment(), s, colliderB->GetCapsuleRadius());
	}

	if (!hit) {
		return;
	}

	// フィルタリング
	bool canA = (colliderA->GetCollisionMask() & colliderB->GetCollisionAttribute());
	bool canB = (colliderB->GetCollisionMask() & colliderA->GetCollisionAttribute());
	if (!(canA || canB))
		return;

	colliderA->OnCollision(colliderB);
	colliderB->OnCollision(colliderA);
}

void CollisionManager::SetColliders(Player* player, Enemy* enemy) {

	// プレイヤーを登録
	colliders_.push_back(player);

	// 敵を登録
	colliders_.push_back(enemy);

	// プレイヤーの弾を登録
	for (PlayerBullet* bullet : player->GetBulletPtrs()) {
		colliders_.push_back(bullet);
	}

	// 敵の弾を登録
	for (EnemyBullet* bullet : enemy->GetBulletPtrs()) {
		colliders_.push_back(bullet);
	}
}