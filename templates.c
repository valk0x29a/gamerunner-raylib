#include "raylib.h"
#include <sys/types.h>

#define Vector2(x,y) (Vector2){x, y}

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
    uint entityType;
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
} entity;


void Upgrade(entity* thisButton);
void BuyOrEquip(entity* thisButton);

entity playerTemplate = { 0 };

entity GetBasicTemplate()
{
    entity basic = { 0 };
    basic.isEnabled = true;
    return basic;
};

entity GetPlayerTemplate()
{
    entity player = GetBasicTemplate();
    player.size = Vector2(20, 20);
    player.pivot = Vector2(0.5f, 0.5f);
    player.defaultColor = VIOLET;
    player.entityType = PLAYER;
    player.speed = 2.5f;
    player.dashDistance = 128;
    player.numberOfDashes = 2;
    player.dashCooldown = 2.0f;
    player.health = 100;
    player.damagedCooldown = 0.125f;
    player.damagedColor = RED;
    player.templateIndex = 1;
    return player;
}

entity GetBigEnemyTemplate()
{
    entity bigEnemy = GetBasicTemplate();
    bigEnemy.size = Vector2(64, 64);
    bigEnemy.pivot = Vector2(0.5f, 0.5f);
    bigEnemy.defaultColor = DARKPURPLE;
    bigEnemy.entityType = ENEMY;
    bigEnemy.speed = 0.5f;
    bigEnemy.attackDamage = 20;
    bigEnemy.attackCooldown = 0.5f;
    bigEnemy.attackRange = 50.0f;
    bigEnemy.health = 40;
    bigEnemy.damagedCooldown = 0.125f;
    bigEnemy.damagedColor = RED;
    bigEnemy.fovRange = 350.0f;
    bigEnemy.stamina = 100.0f;
    bigEnemy.staminaRegenerationSpeed = 20.0f;
    bigEnemy.flipTimer = 2.0f;
    bigEnemy.cashDropAmount = 50;
    bigEnemy.templateIndex = 2;
    return bigEnemy;
}

entity GetFastEnemyTemplate()
{
    entity fastEnemy = GetBasicTemplate();
    fastEnemy.size = Vector2(16, 16);
    fastEnemy.pivot = Vector2(0.5f, 0.5f);
    fastEnemy.defaultColor = DARKPURPLE;
    fastEnemy.entityType = ENEMY;
    fastEnemy.speed = 3.0f;
    fastEnemy.attackDamage = 10;
    fastEnemy.attackCooldown = 0.25f;
    fastEnemy.attackRange = 25.0f;
    fastEnemy.health = 20;
    fastEnemy.damagedCooldown = 0.125f;
    fastEnemy.damagedColor = RED;
    fastEnemy.fovRange = 650.0f;
    fastEnemy.stamina = 500.0f;
    fastEnemy.staminaRegenerationSpeed = 100.0f;
    fastEnemy.flipTimer = 0.5f;
    fastEnemy.cashDropAmount = 25;
    fastEnemy.templateIndex = 3;
    return fastEnemy;
}

entity GetHealthHitboxTemplate()
{
    entity healthHitbox = GetBasicTemplate();
    healthHitbox.pivot = Vector2(0.5f, 0.5f);
    healthHitbox.defaultColor = BLUE;
    healthHitbox.entityType = HEALTH_HITBOX;
    healthHitbox.damagedCooldown = 0.125f;
    healthHitbox.damagedColor = RED;
    healthHitbox.templateIndex = 4;
    return healthHitbox;
}

entity GetHandgunTemplate()
{
    entity handgun = GetBasicTemplate();
    handgun.attackDamage = 10;
    handgun.attackCooldown = 0.1f;
    handgun.attackRange = 10.0f;
    handgun.ammoCount = 7;
    handgun.reloadCooldown = 1.0f;
    handgun.isAutomatic = true;
    handgun.templateIndex = 5;
    return handgun;
}

entity GetSharpenerTemplate()
{
    entity sharpener = GetBasicTemplate();
    sharpener.attackDamage = 5;
    sharpener.attackCooldown = 0.25f;
    sharpener.attackRange = 10.0f;
    sharpener.ammoCount = 30;
    sharpener.reloadCooldown = 10.0f;
    sharpener.isAutomatic = true;
    sharpener.canBeEquipped = true;
    sharpener.buttonIndex = 0;
    sharpener.templateIndex = 6;
    return sharpener;
}

entity GetShotgunTemplate()
{
    entity shotgun = GetBasicTemplate();
    shotgun.attackDamage = 20;
    shotgun.attackCooldown = 1.0f;
    shotgun.attackRange = 10.0f;
    shotgun.ammoCount = 4;
    shotgun.reloadCooldown = 10.0f;
    shotgun.isAutomatic = false;
    shotgun.canBeEquipped = true;
    shotgun.buttonIndex = 1;
    shotgun.templateIndex = 7;
    return shotgun;
}

entity GetSniperGunTemplate()
{
    entity sniperGun = GetBasicTemplate();
    sniperGun.attackDamage = 20;
    sniperGun.attackCooldown = 1.0f;
    sniperGun.attackRange = 20.0f;
    sniperGun.ammoCount = 4;
    sniperGun.reloadCooldown = 1.0f;
    sniperGun.isAutomatic = false;
    sniperGun.canBeEquipped = true;
    sniperGun.buttonIndex = 2;
    sniperGun.templateIndex = 8;
    return sniperGun;
}


entity GetDecoyTemplate()
{
    entity decoy = GetBasicTemplate();
    decoy.buttonIndex = 3;
    return decoy;
}

entity GetFreezingGrenadeTemplate()
{
    entity freezingGrenade = GetBasicTemplate();
    freezingGrenade.stamina = -100.0f;
    freezingGrenade.attackRange = 100.0f;
    freezingGrenade.buttonIndex = 4;
    return freezingGrenade;
}

entity GetExplosiveGrenadeTemplate()
{
    entity explosiveGrenade = GetBasicTemplate();
    explosiveGrenade.attackDamage = 10;
    explosiveGrenade.attackRange = 100.0f;
    explosiveGrenade.buttonIndex = 5;
    return explosiveGrenade;
}

entity GetUpgraderTemplate()
{
    entity upgrader = GetBasicTemplate();
    upgrader.position = Vector2(400, 250);
    upgrader.previousPosition = upgrader.position;
    upgrader.size = Vector2(32, 32);
    upgrader.pivot = Vector2(0.5f, 0.5f);
    upgrader.entityType = UPGRADER;
    upgrader.defaultColor = GOLD;
    return upgrader;
}

entity GetUpgradeButtonTemplate()
{
    entity upgradeButton = GetBasicTemplate();
    upgradeButton.entityType = UI_UPGRADER_BUTTON;
    upgradeButton.defaultColor = WHITE;
    upgradeButton.damagedCooldown = 0.125f;
    upgradeButton.damagedColor = GRAY;
    upgradeButton.isUI = true;
    upgradeButton.buttonCallback = Upgrade;
    upgradeButton.templateIndex = 9;
    return upgradeButton;
}

entity GetBuyButtonTemplate()
{
    entity buyButton = GetBasicTemplate();
    buyButton.entityType = UI_UPGRADER_BUY_BUTTON;
    buyButton.defaultColor = WHITE;
    buyButton.damagedCooldown = 0.125f;
    buyButton.damagedColor = GRAY;
    buyButton.isUI = true;
    buyButton.buttonCallback = BuyOrEquip;
    buyButton.templateIndex = 10;
    return buyButton;
}

entity GetHealthPickupTemplate()
{
    entity newEntity = GetBasicTemplate();
    newEntity.size = Vector2(8, 8);
    newEntity.pivot = Vector2(0.5f, 0.5f);
    newEntity.defaultColor = BLUE;
    newEntity.entityType = HEALTH_PICKUP;
    return newEntity;
}

entity GetDamageTextTemplate()
{
    entity newEntity = GetBasicTemplate();
    newEntity.defaultColor = RED;
    newEntity.entityType = UI_DAMAGE_TEXT;
    newEntity.speed = 16.0f;
    newEntity.destroyTimer = -1.0f;
    newEntity.isUI = true;
    return newEntity;
}

entity GetHitPointTemplate()
{
    entity newEntity = GetBasicTemplate();
    newEntity.size = Vector2(4, 4);
    newEntity.pivot = Vector2(0.5f, 0.5f);
    newEntity.defaultColor = RED;
    newEntity.destroyTimer = -0.5f;
    return newEntity;
}

entity GetPlacedDecoyTemplate()
{
    entity newEntity = GetBasicTemplate();
    newEntity.size = Vector2(16, 16);
    newEntity.pivot = Vector2(0.5f, 0.5f);
    newEntity.defaultColor = GRAY;
    newEntity.entityType = DECOY;
    newEntity.destroyTimer = -10.0f;
    return newEntity;
}

entity GetFreezingGrenadeVFXTemplate()
{
    entity newEntity = GetBasicTemplate();
    newEntity.size = Vector2(100, 0);
    newEntity.pivot = Vector2(0.5f, 0.5f);
    newEntity.defaultColor = LIGHTGRAY;
    newEntity.entityType = RADIUS_VFX;
    newEntity.destroyTimer = -0.1f;
    return newEntity;
}

entity GetExplosiveGrenadeVFXTemplate()
{
    entity newEntity = GetBasicTemplate();
    newEntity.size = Vector2(100, 0);
    newEntity.pivot = Vector2(0.5f, 0.5f);
    newEntity.defaultColor = RED;
    newEntity.entityType = RADIUS_VFX;
    newEntity.destroyTimer = -0.1f;
    return newEntity;
}

entity GetTemplate(int templateIndex)
{
    switch(templateIndex)
    {
        case 0: return GetBasicTemplate();
        case 1: return playerTemplate;
        case 2: return GetBigEnemyTemplate();
        case 3: return GetFastEnemyTemplate();
        case 4: return GetHealthHitboxTemplate();
        case 5: return GetHandgunTemplate();
        case 6: return GetSharpenerTemplate();
        case 7: return GetShotgunTemplate();
        case 8: return GetSniperGunTemplate();
        case 9: return GetUpgradeButtonTemplate();
        case 10: return GetBuyButtonTemplate();
        default: return GetBasicTemplate();
    }
};

void InitializeTemplates()
{
    playerTemplate = GetPlayerTemplate();
}


