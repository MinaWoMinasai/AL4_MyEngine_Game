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
	/// マップチップの生成
	/// </summary>
	void GenerateBlocks();

	/// <summary>
	/// プレイヤーとブロックの当たり判定
	/// </summary>
	void CheckCollisionPlayerAndBlocks(AxisXYZ axis);

	/// <summary>
	/// 敵の弾とブロックの当たり判定
	/// </summary>
	void CheckCollisionBulletsAndBlocks();

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
	std::unique_ptr<Enemy> enemy_;

	// 衝突マネージャ
	std::unique_ptr<CollisionManager> collisionManager_;


	struct Block {
		Transform worldTransform;
		std::unique_ptr<Object3d> object;
		AABB aabb;
		bool isActive = false;
	};

	// ブロック用のワールドトランスフォーム
	std::vector<std::vector<Block>> blocks_;
	// マップチップ
	std::unique_ptr<MapChip> mapChip_ = nullptr;

	size_t currentLineIndex_ = 0;

	std::vector<AABB> mergedBlocks_;
};
