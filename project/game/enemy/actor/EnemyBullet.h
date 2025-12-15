#pragma once
#include <Windows.h>
#include <algorithm>
#include "Calculation.h"
#include "Collider.h"
#include "CollisionConfig.h"
#include <Object3d.h>

class EnemyBullet : public Collider {

public:

	void Initialize(const Vector3& position, const Vector3& velocity);

	void Update();

	void Draw();

	bool IsDead() const { return isDead_; }

	/// <summary>
	/// 衝突判定
	/// </summary>
	void OnCollision(Collider* other) override;

	// ワールド座標を取得
	Vector3 GetWorldPosition() const override;

	Vector3 GetMove() { return velocity_; }

	// セッター
	//void SetWorldPosition(const Vector3& pos) {
	//	worldTransform_.translation_ = pos;
	//	WorldTransformUpdate(worldTransform_);
	//}

	AABB GetAABB();

	void SetVelocity(const Vector3& v) { velocity_ = v; }
	AABB ComputeAABBAt(const Vector3& pos);

	float GetRadius() const override { return radius_; }

private:

	std::unique_ptr<Object3d> object_;

	// ワールドトランスフォーム
	Transform worldTransform_;

	// 速度
	Vector3 velocity_;

	// 寿命
	static const int32_t kLifeTime = 200;
	// デスタイマー
	int32_t deathTimer_ = kLifeTime;
	// デスフラグ
	bool isDead_ = false;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 1.6f;
	static inline const float kHeight = 1.6f;

	float radius_ = 1.0f;
};
