#include "Player.h"

Player::~Player() {
	
}

void Player::Attack() {

	if (input_->IsPress(input_->GetKey()[DIK_SPACE]) || input_->IsPress(input_->GetMouseState().rgbButtons[0])) {

		// 弾のクールタイムを計算する
		bulletCoolTime--;

		if (bulletCoolTime <= 0) {

			// 弾の速度
			const float kBulletSpeed = 0.5f;
			Vector3 velocity = dir * kBulletSpeed;

			auto bullet = std::make_unique<PlayerBullet>();
			bullet->Initialize(worldTransform_.translate, velocity);

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

	// シングルトンインスタンス
	input_ = Input::GetInstance();

	// 衝突属性を設定
	SetCollisionAttribute(kCollisionAttributePlayer);
	// 衝突対象を自分の属性以外に設定
	SetCollisionMask(kCollisionAttributeEnemy | kCollisionAttributeEnemyBullet);
}

void Player::Update(Camera* viewProjection) {

	RotateToMouse(viewProjection);

	move_ = {0, 0, 0};

	// 押した方向で移動ベクトルを変更
	if (input_->IsPress(input_->GetKey()[DIK_A]))
		move_.x -= kCharacterSpeed;
	if (input_->IsPress(input_->GetKey()[DIK_D]))
		move_.x += kCharacterSpeed;
	if (input_->IsPress(input_->GetKey()[DIK_W]))
		move_.y += kCharacterSpeed;
	if (input_->IsPress(input_->GetKey()[DIK_S]))
		move_.y -= kCharacterSpeed;
	
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
	
	object_->SetTransform(worldTransform_);
	object_->Update();
}

void Player::Draw() {

	// 弾の描画
	for (auto& bullet : bullets_) {
		bullet->Draw();
	}

	object_->Draw();
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

void Player::OnCollision(Collider* other) { (void)other; }

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = { worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f };
	aabb.max = { worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f };

	return aabb;
}
