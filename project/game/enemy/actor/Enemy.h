#pragma once
#define NOMINMAX
#include "Collider.h"
#include "EnemyBullet.h"
#include <Windows.h>
#include <algorithm>
#include "Object3d.h"
#include "Sprite.h"

struct EnemyParticle {
	std::unique_ptr<Object3d> object;
	Vector3 velocity;
	Vector3 rotateSpeed;
	float timer;
	float lifeTime;

	float startAlpha;
};

class Player;
class GameScene;

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
	void Initialize(Object3d* object, const Vector3& position, GameScene* gameScene);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// スプライト描画
	/// </summary>
	void DrawSprite();

	/// <summary>
	/// 弾発射
	/// </summary>
	void Fire();

	/// <summary>
	/// 散弾発射
	/// </summary>
	/// <param name="spreadAngle"></param>
	/// <param name="canReflect"></param>
	void ShotgunFire(int bulletCount, float spreadAngleDeg, float bulletSpeed = 0.3f, bool randam = true);

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
	
	// セッター
	void SetWorldPosition(const Vector3& pos) {
		worldTransform_.translate = pos;
		object_->SetTransform(worldTransform_);
		object_->Update();
	}

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

	AABB GetAABB();

	Vector3 GetDir() { return dir_; }

	Segment MakeForwardRay(float length) const;

	bool IsBlockNearByRay();

	Vector3 WallAvoidByRay();
	
	float ScoreDir(const Vector3& dir);

	Segment MakeRayToPlayer() const;

	bool HitPlayerByRay(const Segment& ray);

	bool HasLineOfSightToPlayer();

	Vector3 GetMove() { return velocity_; }

	void Die(); // ← プレイヤー消滅

	// 演出終了か
	bool isFinished();

	bool IsDead() const { return isDead_; }

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

	//bool isDead_ = false;

	float radius_ = 2.8f;

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

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 3.2f;
	static inline const float kHeight = 3.2f;
	
	Vector3 dir_;

	GameScene* gameScene_ = nullptr;
	
	bool isWallFollowing_ = false;
	float wallFollowTimer_ = 0.0f;
	Vector3 wallFollowDir_;

	// 射撃感覚タイマー
	float kFireTimerMax_ = 0.15f;
	float fireTimer_ = 0.0f;
	
	Vector3 velocity_{ 0, 0, 0 };

	float maxSpeed_ = 0.30f;
	float accel_ = 0.08f;
	float friction_ = 0.90f;
	
	std::unique_ptr<Sprite> bossHpFont;
	std::unique_ptr<Sprite> sprite;
	std::unique_ptr<Sprite> bossHpRed;

	int hp_ = 200;

	bool isDead_ = false;

	bool isExploding_ = false;

	std::vector<EnemyParticle> particles_;

private:
	void SpawnParticles();
	void UpdateParticles(float deltaTime = 1.0f / 60.0f);
	const float deltaTime = 1.0f / 60.0f;
};