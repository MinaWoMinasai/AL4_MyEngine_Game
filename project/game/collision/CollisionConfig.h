#pragma once
#include <cstdint>

// プレイヤー陣営
const uint32_t kCollisionAttributePlayer = 1 << 0;
// 敵陣営
const uint32_t kCollisionAttributeEnemy = 1 << 1;
// 敵の弾陣営
const uint32_t kCollisionAttributeEnemyBullet = 1 << 2;
// 跳ね返る弾陣営
const uint32_t kCollisionAttributeReflectedBullet = 1 << 3;
// 特殊属性
const uint32_t kCollisionAttributeUnique = 1 << 4;
// プレイヤー弾陣営
const uint32_t kCollisionAttributePlayerBullet = 1 << 5;