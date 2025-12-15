#include "PlayerBullet.h"

void PlayerBullet::Initialize(const Vector3& position, const Vector3& velocity) {

	object_ = std::make_unique<Object3d>();
	object_->Initialize();
	object_->SetModel("bullet.obj");

	worldTransform_ = InitWorldTransform();
	worldTransform_.translate = position;
	velocity_ = velocity;
	object_->SetTranslate(worldTransform_.translate);

	// 衝突属性を設定
	SetCollisionAttribute(kCollisionAttributePlayerBullet);
	// 衝突対象を敵に設定
	SetCollisionMask(kCollisionAttributeEnemy | kCollisionAttributeEnemyBullet);
}

void PlayerBullet::Update() {

	// 座標を移動させる
	worldTransform_.translate += velocity_;

	// 時間経過でデス
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}

	object_->SetTransform(worldTransform_);
	object_->Update();
	
}

void PlayerBullet::Draw() {

	object_->Draw();

}

void PlayerBullet::OnCollision(Collider* other) {
	(void)other;
	// デスフラグを立てる
	isDead_ = true;
}

Vector3 PlayerBullet::GetWorldPosition() const {
	
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.translate.x;
	worldPos.y = worldTransform_.translate.y;
	worldPos.z = worldTransform_.translate.z;

	return worldPos;
}

AABB PlayerBullet::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

AABB PlayerBullet::ComputeAABBAt(const Vector3& pos){
	AABB aabb;
	Vector3 halfSize = {kWidth / 2.0f, kHeight / 2.0f, kWidth / 2.0f};
	aabb.min = pos - halfSize;
	aabb.max = pos + halfSize;
	return aabb;
}