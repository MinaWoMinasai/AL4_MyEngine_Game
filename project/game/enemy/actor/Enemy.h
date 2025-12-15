#pragma once
#include "Collider.h"
#include "EnemyBullet.h"
#include <Windows.h>
#include <algorithm>
#include "Object3d.h"

class Player;

class Enemy : public Collider {

public:

	enum class AIState {
		Attack, // 攻撃、射撃
		KeepDistance, // 距離をとる
		Evade, // 回避
		Wander, // 徘徊
	};

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Enemy();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3d* object, const Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 弾発射
	/// </summary>
	void Fire(bool canReflect = false);

	/// <summary>
	/// 散弾発射
	/// </summary>
	/// <param name="spreadAngle"></param>
	/// <param name="canReflect"></param>
	void ShotgunFire(int bulletCount, float spreadAngleDeg, bool canReflect, float bulletSpeed = 0.3f, bool randam = true);

	/// <summary>
	///
	/// </summary>
	/// <param name="speed"></param>
	void ApproachToPlayer(Vector3& startPos, Vector3& targetPos);

	// 状態クラス用 Getter/Setter
	Transform& GetWorldTransform() { return worldTransform_; }

	Vector3 GetWorldPosition() const override;

	// 弾のゲッター
	std::vector<EnemyBullet*> GetBulletPtrs();

	// 自キャラのセッター
	void SetPlayer(Player* player) { player_ = player; }

	//----------------------
	// 定数

	// 発射間隔
	static inline const int32_t kFireInterval = 60;

	/// <summary>
	/// 衝突判定
	/// </summary>
	void OnCollision(Collider* other) override;

	float GetRadius() const override { return radius_; }

	bool GetIsDead() const { return isDead_; }

	bool IsDead() { return isDead_; }

	/// <summary>
	/// 移動の傾向
	/// </summary>
	void AIStateMovePower();

	/// <summary>
	/// ステートによる移動
	/// </summary>
	void Move();
	Vector3 RandomDirection();
	
	Vector3 EvadeBullets();
	void UpdateAIState();

private:
	// ワールド変換データ
	Transform worldTransform_;
	// モデル
	Object3d* object_ = nullptr;

	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;

	// 弾
	std::vector<std::unique_ptr<EnemyBullet>> bullets_;

	// 発射タイマー
	int32_t fireIntervalTimer = 0;

	// 自キャラ
	Player* player_ = nullptr;

	// 最初の弾までの時間
	uint32_t time_ = 60;

	// 体力
	uint32_t hp = 20;
	
	bool isDead_ = false;

	float radius_ = 3.0f;

	std::string behaviorName_;
	// 弾のクールダウン
	uint32_t bulletCooldown_ = 30;

	AIState aiState_ = AIState::Wander;

	float attackPower = 0.0f;
	float evadePower = 0.0f;
	float wanderPower = 0.0f;

	const float kPower = 0.1f;

	float wanderChangeTimer = 1.0f;
	Vector3 evadeVec = {0.0f, 0.0f, 0.0f};
	Vector3 wanderVec = {0.0f, 0.0f, 0.0f};
};