#pragma once
#define NOMINMAX
#include "Audio.h"
#include "debugCamera.h"
#include "Dump.h"
#include "Easing.h"
#include "Resource.h"
#include "Sprite.h"
#include "WinApp.h"
#include "Object3d.h"
#include "Model.h"
#include "ModelManager.h"
#include "SrvManager.h"
#include "Player.h"
#include "Enemy.h"
#include "CollisionManager.h"
#include "MapChip.h"
#include "Fade.h"

struct MergedBlock {
	AABB aabb;
	MapChipType type;
};

struct Block {
	Transform worldTransform;
	std::unique_ptr<Object3d> object;
	AABB aabb;
	bool isActive = false;
	MapChipType type;
};

// ゲームシーン
class GameScene {

public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	GameScene();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameScene();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 描画
	/// </summary>
	void DrawSprite();

	/// <summary>
	/// マップチップの生成
	/// </summary>
	void GenerateBlocks();

	/// <summary>
	/// プレイヤーとブロックの当たり判定
	/// </summary>
	void CheckCollisionPlayerAndBlocks(AxisXYZ axis);

	/// <summary>
	/// 敵とブロックの当たり判定
	/// </summary>
	/// <param name="axis"></param>
	//bool CheckCollisionEnemyAndBlocks(AxisXYZ axis);

	/// <summary>
	/// 敵の弾とブロックの当たり判定
	/// </summary>
	void CheckCollisionBulletsAndBlocks();

	const std::vector<std::vector<Block>>& GetBlocks() const;

	//void CheckCollisionEnemyBulletsAndBlocks();

	bool IsFinished() const { return finished_; }

private:


	std::unique_ptr<DebugCamera> debugCamera;
	std::unique_ptr<Camera> camera;
	
	std::unique_ptr<Object3d> object3d;
	std::unique_ptr<Object3d> enemyObject_;
	std::unique_ptr<Object3d> object3d3;

	std::unique_ptr<Object3d> playerObject_;

	// 入力
	Input* input_;

	// ワールドトランスフォーム
	Transform worldTransform_;

	// プレイヤー
	std::unique_ptr<Player> player_;

	// 敵
	//std::unique_ptr<Enemy> enemy_;

	// 衝突マネージャ
	std::unique_ptr<CollisionManager> collisionManager_;

	// ブロック用のワールドトランスフォーム
	std::vector<std::vector<Block>> blocks_;
	// マップチップ
	std::unique_ptr<MapChip> mapChip_ = nullptr;

	size_t currentLineIndex_ = 0;

	std::vector<MergedBlock> mergedBlocks_;

	// 終了フラグ
	bool finished_ = false;

	std::unique_ptr<Fade> fade_ = nullptr;
	Phase phase_ = Phase::kFadeIn;

	std::unique_ptr<Sprite> shotGide;
	std::unique_ptr<Sprite> wasdGide;
	std::unique_ptr<Sprite> toTitleGide;

	// カメラ合わせフラグ
	bool cameraFollow_ = true;
};
