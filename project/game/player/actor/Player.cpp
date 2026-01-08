#include "Player.h"

Player::~Player() {
	
}

void Player::Attack() {

	if (input_->IsPress(input_->GetKey()[DIK_SPACE]) || input_->IsPress(input_->GetMouseState().rgbButtons[0])) {

		// 弾のクールタイムを計算する
		bulletCoolTime--;

		if (bulletCoolTime <= 0) {

			// 弾の速度
			const float kBulletSpeed = 0.4f;
			Vector3 velocity = dir * kBulletSpeed;

			auto bullet = std::make_unique<PlayerBullet>();
			bullet->Initialize(dir * 0.3f + worldTransform_.translate, velocity, 2);

			bullets_.push_back(std::move(bullet));

			bulletCoolTime = kBulletTime;
		}
	} else {
		bulletCoolTime = 0;
	}
}

void Player::RotateToMouse(Camera* viewProjection) {
	// --- 1. マウス座標取得 ---
	POINT mousePosition;
	GetCursorPos(&mousePosition);
	HWND hwnd = WinApp::GetInstance()->GetHwnd();
	ScreenToClient(hwnd, &mousePosition);

	// --- 2. 逆変換用の行列を準備 ---
	Matrix4x4 matViewport = MakeViewportMatrix(0, 0, WinApp::kClientWidth, WinApp::kClientHeight, 0, 1);
	Matrix4x4 matVPV = viewProjection->GetViewMatrix() * viewProjection->GetProjectionMatrix() * matViewport;
	Matrix4x4 matInverseVPV = Inverse(matVPV);

	// --- 3. マウス座標をワールドに変換 ---
	Vector3 posNear = Vector3((float)mousePosition.x, (float)mousePosition.y, 0);
	Vector3 posFar = Vector3((float)mousePosition.x, (float)mousePosition.y, 1);

	posNear = TransformMatrix(posNear, matInverseVPV);
	posFar = TransformMatrix(posFar, matInverseVPV);

	// --- 4. レイと Z=0 平面の交差 ---
	Vector3 mouseDirection = posFar - posNear;
	Vector3 rayDir = Normalize(mouseDirection);
	float t = -posNear.z / rayDir.z;
	Vector3 target = posNear + rayDir * t;

	// --- 5. プレイヤーの位置と方向ベクトル ---
	Vector3 playerPos = worldTransform_.translate;
	Vector3 targetPos = target - playerPos;
	dir = Normalize(targetPos);

	// --- 6. 回転角度を算出 ---
	float angle = atan2(dir.y, dir.x);
	worldTransform_.rotate.z = angle;
	object_->SetRotate(worldTransform_.rotate);
}

void Player::Initialize(Object3d* object, const Vector3& position) {

	object_ = object;

	worldTransform_ = InitWorldTransform();
	worldTransform_.translate = position;
	object_->SetTransform(worldTransform_);
	object_->Update();

	for (int i = 0; i < hp_; ++i) {
		auto sprite = std::make_unique<Sprite>();

		sprite->Initialize(SpriteCommon::GetInstance(),
			"resources/playerSprite.png");
		sprite->SetPosition({ 40.0f + 50.0f * i, 80.0f  });

		hpSprites_.push_back(std::move(sprite));
	}

	hpFont = std::make_unique<Sprite>();
	hpFont->Initialize(SpriteCommon::GetInstance(), "resources/PlayerHp.png");
	hpFont->SetPosition({ 20.0f, 20.0f });
	hpFont->SetSize({ 120.0f, 40.0f });

	// シングルトンインスタンス
	input_ = Input::GetInstance();

	// 衝突属性を設定
	SetCollisionAttribute(kCollisionAttributePlayer);
	// 衝突対象を自分の属性以外に設定
	SetCollisionMask(kCollisionAttributeEnemy | kCollisionAttributeEnemyBullet);
}

void Player::Update(Camera* viewProjection) {

	invincibleTimer_ -= 1.0f / 60.0f;

	RotateToMouse(viewProjection);
	inputDir_ = { 0, 0, 0 };

	if (input_->IsPress(input_->GetKey()[DIK_A])) inputDir_.x -= 1.0f;
	if (input_->IsPress(input_->GetKey()[DIK_D])) inputDir_.x += 1.0f;
	if (input_->IsPress(input_->GetKey()[DIK_W])) inputDir_.y += 1.0f;
	if (input_->IsPress(input_->GetKey()[DIK_S])) inputDir_.y -= 1.0f;

	if (Length(inputDir_) > 1.0f) {
		inputDir_ = Normalize(inputDir_);
	}

	// --- 目標速度 ---
	Vector3 targetVelocity = inputDir_ * maxSpeed_;

	// --- 慣性処理 ---
	float dt = 1.0f / 60.0f;
	float accel = (Length(inputDir_) > 0.0f) ? accel_ : decel_;

	velocity_ += (targetVelocity - velocity_) * accel * dt;

	if (!isDead_){
		// 攻撃処理
		Attack();

		for (auto& bullet : bullets_) {
			bullet->Update();
		}

		bullets_.erase(
			std::remove_if(
				bullets_.begin(),
				bullets_.end(),
				[](const std::unique_ptr<PlayerBullet>& bullet) {
					return bullet->IsDead();
				}),
			bullets_.end());

	}
	
	object_->SetTransform(worldTransform_);
	object_->Update();
	for (auto& sprite : hpSprites_) {
		sprite->Update();
	}
	hpFont->Update();

	if (hp_ <= 0) {
		Die();
	}
	if (isExploding_) {
		UpdateParticles(deltaTime);
	}
	
}

void Player::Draw() {

	// 弾の描画
	for (auto& bullet : bullets_) {
		bullet->Draw();
	}
	if (!isDead_) {
		// 無敵時間中は点滅

		if (invincibleTimer_ > 0.0f) {
			if (static_cast<int>(invincibleTimer_ * 10) % 2 == 0) {
				return;
			}
		}
		object_->Draw();
	}

	for (auto& p : particles_) {
		p.object->Draw();
	}
}

void Player::DrawSprite()
{
	for (auto& sprite : hpSprites_) {
		sprite->Draw();
	}
	hpFont->Draw();
}

Vector3 Player::GetWorldPosition() const {

	// ワールド座標を入れる
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.translate.x;
	worldPos.y = worldTransform_.translate.y;
	worldPos.z = worldTransform_.translate.z;

	return worldPos;
}

Vector2 Player::GetMoveInput() {
	Vector2 move = {0, 0};
	if (input_->IsPress(input_->GetKey()[DIK_A]))
		move.x -= kCharacterSpeed;
	if (input_->IsPress(input_->GetKey()[DIK_D]))
		move.x += kCharacterSpeed;
	if (input_->IsPress(input_->GetKey()[DIK_W]))
		move.y += kCharacterSpeed;
	if (input_->IsPress(input_->GetKey()[DIK_S]))
		move.y -= kCharacterSpeed;
	return move;
}

std::vector<PlayerBullet*> Player::GetBulletPtrs() const {
	std::vector<PlayerBullet*> result;
	result.reserve(bullets_.size());
	for (const auto& b : bullets_) {
		result.push_back(b.get());
	}
	return result;
}

void Player::OnCollision(Collider* other) {
	
	Vector3 hitDir =
		worldTransform_.translate - other->GetWorldPosition();

	if (Length(hitDir) < 0.0001f) {
		return;
	}

	hitDir = Normalize(hitDir);

	const float kKnockBackPower = 0.3f;

	velocity_ += hitDir * kKnockBackPower * other->GetHitPower();
}

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = { worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f };
	aabb.max = { worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f };

	return aabb;
}

void Player::Damage()
{
	if (invincibleTimer_ <= 0.0f) {
		if (hp_ > 0) {
			hpSprites_.pop_back();
		}
		hp_--;
		invincibleTimer_ = 2.0f;
	}
}

void Player::Die()
{
	if (isDead_) return;

	isDead_ = true;
	isExploding_ = true;

	SpawnParticles();

	// すべての弾を消す
	bullets_.clear();
}

bool Player::isFinished()
{
	if (isDead_ && !isExploding_) {
		return true;
	}
	return false;
}

void Player::SpawnParticles()
{
	const int particleCount = 30;
	Vector3 center = object_->GetTranslate();

	for (int i = 0; i < particleCount; ++i) {

		PlayerParticle p{};

		p.object = std::make_unique<Object3d>();
		p.object->Initialize();
		p.object->SetModel("playerParticle.obj");

		p.object->SetTranslate(center);

		// ----------------------
		// ランダム方向 → 正規化
		// ----------------------
		Vector3 dir = Rand(
			Vector3{ -1.0f, -1.0f, -1.0f },
			Vector3{ 1.0f,  1.0f,  1.0f }
		);
		dir = Normalize(dir);

		p.velocity = dir * Rand(3.0f, 6.0f);

		// 回転速度
		p.rotateSpeed = Rand(
			Vector3{ -6.0f, -6.0f, -6.0f },
			Vector3{ 6.0f,  6.0f,  6.0f }
		);

		// ----------------------
		// 寿命 & 透明度
		// ----------------------
		p.timer = 0.0f;
		p.lifeTime = Rand(0.9f, 1.4f);     // ★ 個体差
		p.startAlpha = Rand(0.6f, 1.0f);   // ★ 初期アルファ

		// Addブレンド映え用（少し明るめ）
		Vector4 color = {
			Rand(0.8f, 1.2f),
			Rand(0.8f, 1.2f),
			Rand(0.8f, 1.2f),
			p.startAlpha
		};
		p.object->SetColor(color);

		particles_.push_back(std::move(p));
	}
}

void Player::UpdateParticles(float deltaTime)
{
	for (auto& p : particles_) {

		p.timer += deltaTime;
		if (p.timer >= p.lifeTime) continue;

		float t = p.timer / p.lifeTime; // 0.0 → 1.0

		// ----------------------
		// 移動
		// ----------------------
		Vector3 pos = p.object->GetTranslate();
		pos += p.velocity * deltaTime;

		// 減速
		p.velocity *= 0.97f;

		// 重力
		p.velocity.y -= 6.0f * deltaTime;

		// ----------------------
		// 回転
		// ----------------------
		Vector3 rot = p.object->GetRotate();
		rot += p.rotateSpeed * deltaTime;

		// ----------------------
		// フェードアウト
		// ----------------------
		float alpha = p.startAlpha * (1.0f - t);
		alpha = std::max(alpha, 0.0f);

		Vector4 color = p.object->GetColor();
		color.w = alpha;
		p.object->SetColor(color);

		// ----------------------
		p.object->SetTranslate(pos);
		p.object->SetRotate(rot);
		p.object->Update();
	}

	// 寿命切れ削除
	particles_.erase(
		std::remove_if(
			particles_.begin(),
			particles_.end(),
			[](const PlayerParticle& p) {
				return p.timer >= p.lifeTime;
			}),
		particles_.end()
	);

	if (particles_.empty()) {
		isExploding_ = false;
	}
}
