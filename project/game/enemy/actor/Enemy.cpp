#include "Enemy.h"
#include "Calculation.h"
#include "Player.h"
#include <DirectXMath.h>
using namespace DirectX;

static constexpr float kDeltaTime = 1.0f / 60.0f;

Enemy::~Enemy() {}

void Enemy::Fire(bool canReflect) {

	assert(player_);

	// 弾の速度
	const float kBulletSpeed = 0.3f;

	// 自キャラの位置を取得
	Vector3 playerPos = player_->GetWorldPosition();
	// 敵キャラのワールド座標を取得
	Vector3 enemyPos = GetWorldPosition();
	// 敵キャラから自キャラへのベクトルを求める
	Vector3 direction = playerPos - enemyPos;
	// ベクトルの正規化
	direction = Normalize(direction);
	// ベクトルの長さを速さに合わせる
	direction = kBulletSpeed * direction;

	// 自機と同じ位置なら発射しない
	if (direction.x == 0.0f && direction.y == 0.0f && direction.z == 0.0f) {
		return;
	}

	auto bullet = std::make_unique<EnemyBullet>();
	bullet->Initialize(worldTransform_.translate, direction);

	bullets_.push_back(std::move(bullet));

}

void Enemy::ShotgunFire(int bulletCount, float spreadAngleDeg, bool canReflect, float bulletSpeed, bool randam) {
	assert(player_);

	// 自キャラと敵の位置
	Vector3 playerPos = player_->GetWorldPosition();
	Vector3 enemyPos = GetWorldPosition();

	// プレイヤー方向の基準ベクトル
	Vector3 baseDir;
	if (randam) {
		baseDir = playerPos - enemyPos;
		baseDir = Normalize(baseDir);
	} else {
		baseDir = (enemyPos + Vector3(-10.0f, 0.0f, 0.0f)) - enemyPos;
		baseDir = Normalize(baseDir);
	}
	// 中心角度（散弾のうち真ん中の弾の角度）
	float halfSpread = spreadAngleDeg * 0.5f;

	// 弾をランダムか規則的に発射する
	if (randam) {

		for (int i = 0; i < bulletCount; ++i) {
			// 弾ごとの角度オフセット（左から右へ等間隔）
			float angleOffset = Rand(-halfSpread, halfSpread);

			// 回転行列で方向を回す（Z軸周りに回転する2D前提）
			float rad = XMConvertToRadians(angleOffset);
			Vector3 dirRotated;
			dirRotated.x = baseDir.x * cosf(rad) - baseDir.y * sinf(rad);
			dirRotated.y = baseDir.x * sinf(rad) + baseDir.y * cosf(rad);
			dirRotated.z = baseDir.z; // 2Dならzは固定

			// 速度を掛ける
			Vector3 velocity = bulletSpeed * dirRotated;

			auto bullet = std::make_unique<EnemyBullet>();
			bullet->Initialize(worldTransform_.translate, velocity);

			bullets_.push_back(std::move(bullet));
		}

	} else {

		for (int i = 0; i < bulletCount; ++i) {
			// 弾ごとの角度オフセット（左から右へ等間隔）
			float angleOffset = -halfSpread + (spreadAngleDeg / (bulletCount - 1)) * i;

			// 回転行列で方向を回す（Z軸周りに回転する2D前提）
			float rad = XMConvertToRadians(angleOffset);
			Vector3 dirRotated;
			dirRotated.x = baseDir.x * cosf(rad) - baseDir.y * sinf(rad);
			dirRotated.y = baseDir.x * sinf(rad) + baseDir.y * cosf(rad);
			dirRotated.z = baseDir.z; // 2Dならzは固定

			// 速度を掛ける
			Vector3 velocity = bulletSpeed * dirRotated;

			// 弾を生成
			auto bullet = std::make_unique<EnemyBullet>();
			bullet->Initialize(worldTransform_.translate, velocity);

			bullets_.push_back(std::move(bullet));

		}
	}
}

void Enemy::Initialize(Object3d* object, const Vector3& position) {

	object_ = object;
	worldTransform_ = InitWorldTransform();
	worldTransform_.translate = position;
	worldTransform_.scale = Vector3(2.0f, 2.0f, 2.0f);

	// 衝突属性を設定
	SetCollisionAttribute(kCollisionAttributeEnemy);
	// 衝突対象をプレイヤーとプレイヤーの弾に設定
	SetCollisionMask(kCollisionAttributePlayer | kCollisionAttributePlayerBullet);
}

void Enemy::Update() {

	Move();

	for (auto& bullet : bullets_) {
		bullet->Update();
	}

	bullets_.erase(
		std::remove_if(
			bullets_.begin(),
			bullets_.end(),
			[](const std::unique_ptr<EnemyBullet>& bullet) {
				return bullet->IsDead();
			}),
		bullets_.end());

	object_->SetTransform(worldTransform_);
	object_->Update();

}

void Enemy::Draw() {
	object_->Draw();
	
	for (auto& bullet : bullets_) {
		bullet->Draw();
	}
}

void Enemy::ApproachToPlayer(Vector3& startPos, Vector3& targetPos) {

	// 自キャラの位置を取得
	Vector3 playerPos = player_->GetWorldPosition();
	// 敵キャラのワールド座標を取得
	Vector3 enemyPos = GetWorldPosition();

	startPos = enemyPos;
	targetPos = playerPos;
}

Vector3 Enemy::GetWorldPosition() const {
	// ワールド座標を入れる
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.translate.x;
	worldPos.y = worldTransform_.translate.y;
	worldPos.z = worldTransform_.translate.z;

	return worldPos;
}

void Enemy::OnCollision(Collider* other) {}

void Enemy::AIStateMovePower() {
	switch (aiState_) {
	case AIState::Attack:
		attackPower = 1.0f;
		evadePower = 0.9f;
		wanderPower = 0.1f;
		break;

	case AIState::Wander:
		attackPower = 0.3f;
		evadePower = 1.2f;
		wanderPower = 0.5f;
		break;

	case AIState::Evade:
		attackPower = 0.1f;
		evadePower = 3.0f;
		wanderPower = 0.3f;
		break;
	}
}

void Enemy::Move() { 
	
	UpdateAIState();

	AIStateMovePower();

	Vector3 toPlayer = player_->GetWorldPosition() - GetWorldPosition();
	Vector3 attackVec = Normalize(toPlayer) * attackPower;
	wanderChangeTimer -= 1.0f / 60.0f;
	if (wanderChangeTimer <= 0.0f) {
		wanderVec = RandomDirection() * wanderPower ;
		wanderChangeTimer = 3.0f;
	}

	evadeVec = EvadeBullets() * evadePower;
	Vector3 dir = attackVec + evadeVec + wanderVec;

	if (Length(dir) > 0.0001f) {
		dir = Normalize(dir);
	} else {
		dir = { 0, 0, 0 };
	}

	dir.z = 0.0f;
	worldTransform_.translate += dir * kPower;
}

Vector3 Enemy::RandomDirection() { return Rand(Vector3(-0.2f, -0.2f, 0.0f), Vector3(0.2f, 0.2f, 0.5f)); }

Vector3 Enemy::EvadeBullets() {
	Vector3 evade(0, 0, 0);

	for (PlayerBullet* bullet : player_->GetBulletPtrs()) {
		Vector3 toBullet = bullet->GetWorldPosition() - GetWorldPosition();
		float dist = Length(toBullet);

		const float kEvadeRadius = 10.0f;
		if (dist < kEvadeRadius) {

			toBullet *= -1.0f;
			Vector3 dir = Normalize(toBullet);
			float power = (kEvadeRadius - dist) / kEvadeRadius;

			evade += dir * power * 0.2f;
		}
	}

	return evade;
}

void Enemy::UpdateAIState() {

	Vector3 toPlayer = player_->GetWorldPosition() - GetWorldPosition();
	float dist = Length(toPlayer);

	// プレイヤー弾の危険判定
	bool bulletDanger = false;
	for (PlayerBullet* bullet : player_->GetBulletPtrs()) {
		float distB = Length(bullet->GetWorldPosition() - GetWorldPosition());
		if (distB < 8.0f) {
			bulletDanger = true;
			break;
		}
	}

	// ① 弾が危険 → Evade 強制
	if (bulletDanger) {
		aiState_ = AIState::Evade;
		return;
	}

	// ② 距離による切り替え
	if (dist > 25.0f) {
		aiState_ = AIState::Wander; // 遠い → 探しながら徘徊
	} else if (dist > 10.0f) {
		aiState_ = AIState::Attack; // 中距離 → 攻撃に近づく
	} else {
		aiState_ = AIState::Evade; // 近すぎる → 逃げる
	}
}