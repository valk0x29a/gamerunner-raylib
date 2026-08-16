#pragma once
#include "raylib.h"
#include <sys/types.h>

#define DEFAULT 0
#define PLAYER 1 << 1
#define UPGRADER 1 << 2
#define ENEMY 1 << 3
#define HEALTH_HITBOX 1 << 4
#define HEALTH_PICKUP 1 << 5
#define UI_UPGRADER_BUTTON 1 << 6
#define UI_UPGRADER_IMAGE 1 << 7
#define UI_DAMAGE_TEXT 1 << 8
#define UI_UPGRADER_BUY_BUTTON 1 << 9
#define DECOY 1 << 10
#define RADIUS_VFX 1 << 11

typedef struct entity
{
    Vector2 position;
    Vector2 size;
    Vector2 pivot;
    Vector2 previousPosition;
    Color defaultColor;
    uint type;
    float speed;
    int dashDistance;
    int numberOfDashes;
    float dashCooldown;
    int health;
    int attackDamage;
    float attackCooldown;
    float attackRange;
    float damagedCooldown;
    Color damagedColor;
    float fovRange;
    float stamina;
    bool isRegenerating;
    float staminaRegenerationSpeed;
    struct entity* parent;
    struct entity* child;
    float flipTimer;
    bool isFlipped;
    bool isEnabled;
    bool isUI;
    void (*buttonCallback)(struct entity* thisButton);
    int cashDropAmount;
    int buttonIndex;
    float destroyTimer;
    int ammoCount;
    float reloadCooldown;
    bool isAutomatic;
    bool canBeEquipped;
    int count;
    struct entity* target;
    int templateIndex;
    Vector2 velocity;
    float drag;
} entity;
