#include "raylib.h"
#include <sys/types.h>

#define Vector2(x,y) (Vector2){x, y}

#include "entity.h"
// NOLINTBEGIN(misc-definitions-in-headers)

void Upgrade(entity* thisButton);
void BuyOrEquip(entity* thisButton);

entity playerTemplate = { 0 };
entity sharpenerTemplate = { 0 };
entity shotgunTemplate = { 0 };

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
    player.type = PLAYER;
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

entity GetNormalEnemyTemplate()
{
    entity newEntity = GetBasicTemplate();
    newEntity.size = Vector2(20, 20);
    newEntity.pivot = Vector2(0.5f, 0.5f);
    newEntity.defaultColor = GREEN;
    newEntity.type = ENEMY;
    newEntity.speed = 2.0f;
    newEntity.attackDamage = 10;
    newEntity.attackCooldown = 0.5f;
    newEntity.attackRange = 25.0f;
    newEntity.health = 30;
    newEntity.damagedCooldown = 0.125f;
    newEntity.damagedColor = RED;
    newEntity.stamina = 250.0f;
    newEntity.staminaRegenerationSpeed = 50.0f;
    newEntity.flipTimer = 1.0f;
    newEntity.cashDropAmount = 50;
    newEntity.templateIndex = 11;
    newEntity.drag = 12.0f;
    return newEntity;
}

entity GetBigEnemyTemplate()
{
    entity bigEnemy = GetBasicTemplate();
    bigEnemy.size = Vector2(64, 64);
    bigEnemy.pivot = Vector2(0.5f, 0.5f);
    bigEnemy.defaultColor = DARKPURPLE;
    bigEnemy.type = ENEMY;
    bigEnemy.speed = 0.5f;
    bigEnemy.attackDamage = 20;
    bigEnemy.attackCooldown = 0.5f;
    bigEnemy.attackRange = 50.0f;
    bigEnemy.health = 40;
    bigEnemy.damagedCooldown = 0.125f;
    bigEnemy.damagedColor = RED;
    bigEnemy.stamina = 100.0f;
    bigEnemy.staminaRegenerationSpeed = 20.0f;
    bigEnemy.flipTimer = 2.0f;
    bigEnemy.cashDropAmount = 100;
    bigEnemy.templateIndex = 2;
    bigEnemy.drag = 16.0f;
    return bigEnemy;
}

entity GetFastEnemyTemplate()
{
    entity fastEnemy = GetBasicTemplate();
    fastEnemy.size = Vector2(16, 16);
    fastEnemy.pivot = Vector2(0.5f, 0.5f);
    fastEnemy.defaultColor = DARKPURPLE;
    fastEnemy.type = ENEMY;
    fastEnemy.speed = 3.0f;
    fastEnemy.attackDamage = 10;
    fastEnemy.attackCooldown = 0.25f;
    fastEnemy.attackRange = 25.0f;
    fastEnemy.health = 20;
    fastEnemy.damagedCooldown = 0.125f;
    fastEnemy.damagedColor = RED;
    fastEnemy.stamina = 500.0f;
    fastEnemy.staminaRegenerationSpeed = 100.0f;
    fastEnemy.flipTimer = 0.5f;
    fastEnemy.cashDropAmount = 50;
    fastEnemy.templateIndex = 3;
    fastEnemy.drag = 8.0f;
    return fastEnemy;
}

entity GetHealthHitboxTemplate()
{
    entity healthHitbox = GetBasicTemplate();
    healthHitbox.pivot = Vector2(0.5f, 0.5f);
    healthHitbox.defaultColor = BLUE;
    healthHitbox.type = HEALTH_HITBOX;
    healthHitbox.damagedCooldown = 0.125f;
    healthHitbox.damagedColor = RED;
    healthHitbox.templateIndex = 4;
    return healthHitbox;
}

entity GetHandgunTemplate()
{
    entity handgun = GetBasicTemplate();
    handgun.attackRange = 10.0f;
    handgun.ammoCount = 7;
    handgun.reloadCooldown = 4.0f;
    handgun.isAutomatic = true;
    handgun.templateIndex = 5;
    return handgun;
}

entity GetSharpenerTemplate()
{
    entity sharpener = GetBasicTemplate();
    sharpener.attackCooldown = 0.25f;
    sharpener.attackRange = 10.0f;
    sharpener.reloadCooldown = 5.0f;
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
    shotgun.attackRange = 10.0f;
    shotgun.reloadCooldown = 10.0f;
    shotgun.isAutomatic = false;
    shotgun.canBeEquipped = true;
    shotgun.buttonIndex = 1;
    shotgun.size = Vector2(0, 64);
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

entity GetExploderTemplate()
{
    entity newEntity = GetBasicTemplate();
    newEntity.isAutomatic = false;
    newEntity.canBeEquipped = true;
    newEntity.buttonIndex = 6;
    return newEntity;
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
    upgrader.position = Vector2(640, 320);
    upgrader.previousPosition = upgrader.position;
    upgrader.size = Vector2(32, 32);
    upgrader.pivot = Vector2(0.5f, 0.5f);
    upgrader.type = UPGRADER;
    upgrader.defaultColor = GOLD;
    return upgrader;
}

entity GetUpgradeButtonTemplate()
{
    entity upgradeButton = GetBasicTemplate();
    upgradeButton.type = UI_UPGRADER_BUTTON;
    upgradeButton.isUI = true;
    upgradeButton.defaultColor = WHITE;
    upgradeButton.damagedCooldown = 0.125f;
    upgradeButton.damagedColor = GRAY;
    upgradeButton.buttonCallback = Upgrade;
    upgradeButton.templateIndex = 9;
    return upgradeButton;
}

entity GetBuyButtonTemplate()
{
    entity buyButton = GetBasicTemplate();
    buyButton.type = UI_UPGRADER_BUY_BUTTON;
    buyButton.isUI = true;
    buyButton.defaultColor = WHITE;
    buyButton.damagedCooldown = 0.125f;
    buyButton.damagedColor = GRAY;
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
    newEntity.type = HEALTH_PICKUP;
    return newEntity;
}

entity GetDamageTextTemplate()
{
    entity newEntity = GetBasicTemplate();
    newEntity.defaultColor = RED;
    newEntity.type = UI_DAMAGE_TEXT;
    newEntity.isUI = true;
    newEntity.speed = 16.0f;
    newEntity.destroyTimer = -1.0f;
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
    newEntity.type = DECOY;
    newEntity.destroyTimer = -10.0f;
    return newEntity;
}

entity GetFreezingGrenadeVFXTemplate()
{
    entity newEntity = GetBasicTemplate();
    newEntity.size = Vector2(100, 0);
    newEntity.pivot = Vector2(0.5f, 0.5f);
    newEntity.defaultColor = LIGHTGRAY;
    newEntity.type = RADIUS_VFX;
    newEntity.destroyTimer = -0.1f;
    return newEntity;
}

entity GetExplosiveGrenadeVFXTemplate()
{
    entity newEntity = GetBasicTemplate();
    newEntity.size = Vector2(100, 0);
    newEntity.pivot = Vector2(0.5f, 0.5f);
    newEntity.defaultColor = RED;
    newEntity.type = RADIUS_VFX;
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
        case 6: return sharpenerTemplate;
        case 7: return shotgunTemplate;
        case 8: return GetSniperGunTemplate();
        case 9: return GetUpgradeButtonTemplate();
        case 10: return GetBuyButtonTemplate();
        case 11: return GetNormalEnemyTemplate();
        default: return GetBasicTemplate();
    }
};

void InitializeTemplates()
{
    playerTemplate = GetPlayerTemplate();
    sharpenerTemplate = GetSharpenerTemplate();
    shotgunTemplate = GetShotgunTemplate();
}

//NOLINTEND(misc-definitions-in-headers)



