#pragma once
#include <Windows.h>
#include <algorithm>
#include "PlayerBullet.h"
#include <list>
#include "Calculation.h"
#include "Collider.h"
#include "CollisionConfig.h"
#include "Object3d.h"

/// <summary>
/// 自キャラ
/// </summary>
class Player : public Collider {

public:

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Player();

	/// <summary>
	/// 攻撃
	/// </summary>
	void Attack();

	/// <summary>
	/// マウスの方を向く
	/// </summary>
	/// <param name="viewProjection"></param>
	void RotateToMouse(Camera* viewProjection);

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="camera">カメラ</param>
	/// <param name="position">初期座標</param>
	void Initialize(Object3d* objectBullet, const Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update(Camera* viewProjection);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 衝突判定
	/// </summary>
	void OnCollision(Collider* other) override;

	// ワールド座標を取得

	// 半径
	static inline const float kRadius = 1.0f;

	Vector3 GetWorldPosition() const override;

	Vector3 GetMove() { return move_; }

	// セッター
	void SetWorldPosition(const Vector3& pos) {
		worldTransform_.translate = pos;
		object_->SetTransform(worldTransform_);
	}

	Vector2 GetMoveInput();

	// 弾のゲッター
	std::vector<PlayerBullet*> GetBulletPtrs() const;

	AABB GetAABB();

private:
	// ワールド変換データ
	Transform worldTransform_;
	
	// モデル
	Object3d* object_ = nullptr;
	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;

	// キーボード入力
	Input* input_ = nullptr;

	// 弾
	std::vector<std::unique_ptr<PlayerBullet>> bullets_;
	Vector3 dir;

	// キャラクターの移動速さ
	float kCharacterSpeed = 0.2f;
	Vector3 move_;
	
	const int kBulletTime = 5;
	int bulletCoolTime = 0;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 1.6f;
	static inline const float kHeight = 1.6f;

};

