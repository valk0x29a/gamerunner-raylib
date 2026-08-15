#include "raylib.h"
#include "raymath.h"
#include <limits.h>
#include <stdlib.h>
#include <float.h>
#include <stdio.h>
#include <string.h>

#define NEXT_WAVE_TIMER 5.0f

#define Vector2(x,y) (Vector2){x, y}

#define min(a, b) a > b ? b : a
#define max(a, b) a > b ? a : b
#include "templates.c"

void ReloadGame();
void EndWave();
void SpawnEnemies();
void SpawnHealthPickup(Vector2 position);
void SetUpgrades();
void SpawnDamageText(Vector2 position, int damage);
void SpawnHitPoint(Vector2 position);
void BuyOrEquipSharpener();

typedef struct RayCastHitResult
{
    bool colliding;
    Vector2 hitPosition;
    int entityIndex;
} RayCastHitResult;

typedef struct RayCastAllHitsResult
{
    int indexesCount;
    Vector2* hitPositions;
    int* entityIndexes;
} RayCastAllHitsResult;

int firstFreeIndex = 0;
int capacity = 4;
entity** entities;

entity* equippedWeapon;
entity* handgun; // always at first slot
entity* inventorySecondSlot = NULL;
//available weapons
entity* sharpener;
entity* shotgun;
entity* sniperGun;

entity* equippedGrenade = NULL;
entity* decoy;
entity* freezingGrenade;
entity* explosiveGrenade;

// const int sharpenerUpgradeIndex = 3;
int sharpenerPrice = 100;
bool isSharpenerUnlocked = false;
// const int sharpenerUpgradeIndex = 3;
int shotgunPrice = 100;
bool isShotgunUnlocked = false;
int sniperGunPrice = 0;
bool isSniperGunUnlocked = false;

int decoyPrice = 100;
int freezingGrenadePrice = 100;
int explosiveGrenadePrice = 100;

int currentHandgunUpgrade = 0;
int handgunUpgradesCount = 2;
int handgunUpgradesPrices[1];
typedef struct handgunUpgrade
{
    int attackDamage;
    float attackCooldown;
} handgunUpgrade;
handgunUpgrade handgunUpgrades[2];

int currentDashUpgrade = 0;
int dashUpgradesCount = 2;
int dashUpgradesPrices[1];
typedef struct dashUpgrade
{
    int numberOfDashes;
    float dashCooldown;
} dashUpgrade;
dashUpgrade dashUpgrades[2];

int currentMaxHealthUpgrade = 0;
int maxHealthUpgradesCount = 2;
int maxHealthUpgradesPrices[1];
int maxHealthUpgrades[2];

const char* GetUpgradeButtonText(int upgradeIndex)
{
    switch(upgradeIndex)
    {
        case 0: return "Upgrade Handgun";
        case 1: return "Upgrade Dash";
        case 2: return "Upgrade Max Health";
    };
    return "Invalid Upgrade Index";
}

char* GetItemName(int buyIndex)
{
    switch(buyIndex)
    {
        case 0: return "Sharpener";
        case 1: return "Shotgun";
        case 2: return "Sniper Gun";
        case 3: return "Decoy";
        case 4: return "Freezing Grenade";
        case 5: return "Explosive Grenade";
    };
    return "Invalid Buy Index";
}

int enemiesCount = 0;

int currentWave = 0;

float nextWaveTimer = NEXT_WAVE_TIMER;

int playerCash = 0;

entity* player;

int isUpgraderUIActive = false;

Vector2 GetEntityCorner(entity* entity)
{
    Vector2 offset = Vector2Multiply(entity->pivot, entity->size);
    return Vector2Subtract(entity->position, offset);
}

float getSqrDistance(Vector2 a, Vector2 b)
{
    return (a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y);
}

void addEntity(entity* newEntity)
{
    entities[firstFreeIndex] = newEntity;
    firstFreeIndex++;
    if(firstFreeIndex == capacity)
    {
        capacity *= 2;
        entities = realloc(entities, sizeof(entity*) * capacity);
    }
    if(newEntity->entityType == ENEMY) { enemiesCount++; }
}

void removeEntity(int entityIndex)
{
    if(entities[entityIndex]->child != NULL)
    {
        entities[entityIndex]->child->parent = NULL;
    }
    if(entities[entityIndex]->parent != NULL)
    {
        entities[entityIndex]->parent->child = NULL;
    }

    if(entities[entityIndex]->entityType == ENEMY)
    {
        enemiesCount--;
        if(enemiesCount <= 0)
        {
            EndWave();
        }
    }
    int lastFilledIndex = firstFreeIndex-1;
    if(entityIndex != lastFilledIndex)
    {
        entity* endEntity = entities[lastFilledIndex];
        entity* deletedEntity = entities[entityIndex];
        entities[entityIndex] = endEntity;
        entities[lastFilledIndex] = deletedEntity;
    }

    free(entities[lastFilledIndex]);
    entities[lastFilledIndex] = NULL;
    firstFreeIndex--;
}

entity* allocAndAddEntity(entity copiedEntity)
{
    entity* newEntity = calloc(1, sizeof(entity));
    *newEntity = copiedEntity;
    addEntity(newEntity);
    return newEntity;
}

bool areEntitiesColliding(entity* entityA, entity* entityB)
{
    Vector2 entityACorner = GetEntityCorner(entityA);
    Vector2 entityBCorner = GetEntityCorner(entityB);
    bool isInsideOnXAxis = entityACorner.x + entityA->size.x >= entityBCorner.x && entityACorner.x <= entityBCorner.x + entityB->size.x;
    bool isInsideOnYAxis = entityACorner.y + entityA->size.y >= entityBCorner.y && entityACorner.y <= entityBCorner.y + entityB->size.y;
    return isInsideOnXAxis && isInsideOnYAxis;
}

bool isEntityColliding(int entityIndex, uint entityTypeMask)
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(i == entityIndex) { continue; }
        if((entities[i]->entityType & entityTypeMask) == 0) { continue; }
        if(areEntitiesColliding(entities[entityIndex], entities[i])) { return true; }
    }
    return false;
}

Vector2 isRayCollidingWithSegment(Vector2 s1, Vector2 s2, Vector2 r1, Vector2 r2, bool *colliding)
{
    float denominator = (r2.x - r1.x) * (s2.y - s1.y) - (s2.x - s1.x) * (r2.y - r1.y);
    float r = ((s2.x - s1.x) * (r1.y - s1.y) - (r1.x - s1.x) * (s2.y - s1.y)) / denominator;
    if(r < 0) { *colliding = false; return Vector2Zero(); }
    float s = ((s1.x - r1.x) * (r2.y - r1.y) - (r2.x - r1.x) * (s1.y - r1.y)) / denominator;
    if(s < 0 || s > 1) { *colliding = false; return Vector2Zero(); }
    *colliding = true;
    Vector2 result = { s * (s2.x - s1.x) + s1.x, s * (s2.y - s1.y) + s1.y };
    return result;
}

bool isRayCollidingWithEntity(Vector2 r1, Vector2 r2, entity* e)
{
    // A----B
    // |    |
    // |    |
    // D----C
    Vector2 entityCornerA = GetEntityCorner(e);
    Vector2 entityCornerB = { entityCornerA.x + e->size.x, entityCornerA.y };
    Vector2 entityCornerC = { entityCornerA.x + e->size.x, entityCornerA.y + e->size.y };
    Vector2 entityCornerD = { entityCornerA.x, entityCornerA.y + e->size.y };
    bool a, b, c, d;
    isRayCollidingWithSegment(entityCornerA, entityCornerB, r1, r2, &a);
    isRayCollidingWithSegment(entityCornerB, entityCornerC, r1, r2, &b);
    isRayCollidingWithSegment(entityCornerC, entityCornerD, r1, r2, &c);
    isRayCollidingWithSegment(entityCornerD, entityCornerA, r1, r2, &d);
    return a || b || c || d;
}

RayCastHitResult RayCastHit(Vector2 r1, Vector2 r2, uint entityTypeMask)
{
    Vector2 closestHit = Vector2Zero();
    float closestDistance = FLT_MAX;
    int closestEntityIndex = -1;
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if((entities[i]->entityType & entityTypeMask) == 0) { continue; }

        // A----B
        // |    |
        // |    |
        // D----C
        Vector2 entityCornerA = GetEntityCorner(entities[i]);
        Vector2 entityCornerB = { entityCornerA.x + entities[i]->size.x, entityCornerA.y };
        Vector2 entityCornerC = { entityCornerA.x + entities[i]->size.x, entityCornerA.y + entities[i]->size.y };
        Vector2 entityCornerD = { entityCornerA.x, entityCornerA.y + entities[i]->size.y };

        Vector2 results[4];
        bool colliding[4];
        results[0] = isRayCollidingWithSegment(entityCornerA, entityCornerB, r1, r2, &colliding[0]);
        results[1] = isRayCollidingWithSegment(entityCornerB, entityCornerC, r1, r2, &colliding[1]);
        results[2] = isRayCollidingWithSegment(entityCornerC, entityCornerD, r1, r2, &colliding[2]);
        results[3] = isRayCollidingWithSegment(entityCornerD, entityCornerA, r1, r2, &colliding[3]);
        float distance;
        for(int j = 0; j < 4; j++)
        {
            if(!colliding[j]) { continue; }
            distance = getSqrDistance(r1, results[j]);
            printf("result index: %d, pos x: %f , y: %f\n", j, results[j].x, results[j].y);
            if(distance < closestDistance)
            {
                closestHit = results[j];
                closestDistance = distance;
                closestEntityIndex = i;
            }
        }
    }

    RayCastHitResult result;
    result.colliding = closestDistance != FLT_MAX;
    result.hitPosition = closestHit;
    SpawnHitPoint(result.hitPosition);
    result.entityIndex = closestEntityIndex;
    return result;
}

RayCastAllHitsResult RayCastAllHits(Vector2 r1, Vector2 r2, uint entityTypeMask, float sqrMaxDistance)
{
    int capacity = 2;
    Vector2* hitPositions = malloc(sizeof(Vector2) * capacity);
    int* entityIndexes = malloc(sizeof(int) * capacity);
    int indexesCount = 0;
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if((entities[i]->entityType & entityTypeMask) == 0) { continue; }

        // A----B
        // |    |
        // |    |
        // D----C
        Vector2 entityCornerA = GetEntityCorner(entities[i]);
        Vector2 entityCornerB = { entityCornerA.x + entities[i]->size.x, entityCornerA.y };
        Vector2 entityCornerC = { entityCornerA.x + entities[i]->size.x, entityCornerA.y + entities[i]->size.y };
        Vector2 entityCornerD = { entityCornerA.x, entityCornerA.y + entities[i]->size.y };

        Vector2 results[4];
        bool colliding[4];
        results[0] = isRayCollidingWithSegment(entityCornerA, entityCornerB, r1, r2, &colliding[0]);
        results[1] = isRayCollidingWithSegment(entityCornerB, entityCornerC, r1, r2, &colliding[1]);
        results[2] = isRayCollidingWithSegment(entityCornerC, entityCornerD, r1, r2, &colliding[2]);
        results[3] = isRayCollidingWithSegment(entityCornerD, entityCornerA, r1, r2, &colliding[3]);

        for(int j = 0; j < 4; j++)
        {
            if(!colliding[j]) { continue; }
            if(getSqrDistance(r1, results[j]) > sqrMaxDistance) { continue; }
            hitPositions[indexesCount] = results[j];
            entityIndexes[indexesCount] = i;
            indexesCount++;
            if(indexesCount == capacity)
            {
                capacity *= 2;
                hitPositions = realloc(hitPositions, sizeof(Vector2) * capacity);
                entityIndexes = realloc(entityIndexes, sizeof(int) * capacity);
            }
        }
    }

    RayCastAllHitsResult result;
    result.indexesCount = indexesCount;
    result.hitPositions = hitPositions;
    result.entityIndexes = entityIndexes;
    return result;
}

void UpdatePlayerMovement()
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(entities[i]->entityType != PLAYER) { continue; }
        entity* player = entities[i];
        int speed = player->speed;
        int dashXDirection = 0;
        int dashYDirection = 0;
        if(IsKeyDown(KEY_W))
        {
            player->position.y -= speed;
            dashYDirection = -1;
        }
        if(IsKeyDown(KEY_S))
        {
            player->position.y += speed;
            dashYDirection = 1;
        }
        if(IsKeyDown(KEY_A))
        {
            player->position.x -= speed;
            dashXDirection = -1;
        }
        if(IsKeyDown(KEY_D))
        {
            player->position.x += speed;
            dashXDirection = 1;
        }
        // printf("%d\n", player->numberOfDashes);
        // printf("%f\n", player->currentDashCooldown);
        if(IsKeyPressed(KEY_SPACE) && player->numberOfDashes > 0)
        {
            if(dashXDirection == 0 && dashYDirection == 0)
            {
                dashYDirection = 1;
            }
            player->position.x += dashXDirection * player->dashDistance;
            player->position.y += dashYDirection * player->dashDistance;
            player->numberOfDashes--;

            RayCastAllHitsResult result = RayCastAllHits(player->position, player->previousPosition, ENEMY, getSqrDistance(player->position, player->previousPosition));
            // printf("%d\n", result.indexesCount);
            for(int j = 0; j < result.indexesCount; j++)
            {
                int entityIndex = result.entityIndexes[j];
                entities[entityIndex]->stamina -= 25.0f; //hits two times the same entity
                if(entities[entityIndex]->stamina < -1.0f)
                {
                    entities[entityIndex]->stamina = -1.0f;
                }
                // printf("%f\n", entities[result.entityIndexes[j]]->stamina);
            }
            free(result.entityIndexes);
            free(result.hitPositions);
        }

        if(player->numberOfDashes < player->template->numberOfDashes)
        {
            player->dashCooldown -= GetFrameTime();
            if(player->dashCooldown <= 0)
            {
                player->numberOfDashes++;
                player->dashCooldown = player->template->dashCooldown;
            }
        }
        player->previousPosition = player->position;
        //printf("x: %f y: %f \n", player->position.x, player->position.y);
    }
}

void UpdatePlayerAttack()
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(entities[i]->entityType != PLAYER) { continue; }
        entity* player = entities[i];
        if(player->attackCooldown > 0)
        {
            player->attackCooldown -= GetFrameTime();
        }
        // printf("%f\n", player->currentAttackCooldown);
        if(IsKeyPressed(KEY_R))
        {
            player->reloadCooldown = equippedWeapon->reloadCooldown;
        }
        if(player->reloadCooldown > 0)
        {
            player->reloadCooldown -= GetFrameTime();
            if(player->reloadCooldown <= 0)
            {
                equippedWeapon->ammoCount = equippedWeapon->template->ammoCount;
            }
        }

        if(IsKeyPressed(KEY_ONE))
        {
            equippedWeapon = handgun;
            player->reloadCooldown = 0;
        }
        if(IsKeyPressed(KEY_TWO) && inventorySecondSlot != NULL)
        {
            equippedWeapon = inventorySecondSlot;
            player->reloadCooldown = 0;
        }
        if(IsKeyPressed(KEY_THREE) && decoy->count > 0)
        {
            equippedGrenade = decoy;
        }
        if(IsKeyPressed(KEY_FOUR) && freezingGrenade->count > 0)
        {
            equippedGrenade = freezingGrenade;
        }
        if(IsKeyPressed(KEY_FIVE) && explosiveGrenade->count > 0)
        {
            equippedGrenade = explosiveGrenade;
        }

        bool shouldShot = equippedWeapon->isAutomatic ? IsMouseButtonDown(MOUSE_LEFT_BUTTON) : IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        if(shouldShot && player->attackCooldown <= 0 && player->reloadCooldown <= 0 && !isUpgraderUIActive && equippedWeapon->ammoCount > 0)
        {
            Vector2 mousePosition = GetMousePosition();
            Vector2 offset = Vector2Subtract(mousePosition, player->position);
            offset = Vector2Multiply(offset, Vector2(3,3));
            DrawLineV(player->position, Vector2Add(player->position, offset), RED);
            RayCastHitResult result = RayCastHit(player->position, mousePosition, ENEMY | HEALTH_HITBOX);
            if(result.colliding)
            {
                int entityIndex = result.entityIndex;
                entity* e = entities[entityIndex];
                if(entities[entityIndex]->entityType == HEALTH_HITBOX)
                {
                    entities[entityIndex]->damagedCooldown = entities[entityIndex]->template->damagedCooldown;
                    e = entities[entityIndex]->parent;
                }
                float x = result.hitPosition.x;
                float marginOfError = 1.0f;
                bool isHittingFront = e->isFlipped ? x < e->position.x + e->size.x - marginOfError : x > e->position.x + marginOfError;
                float attackMultiplier = isHittingFront ? 0.5f : 1.0f;
                e->health -= equippedWeapon->attackDamage * attackMultiplier;
                e->damagedCooldown = e->template->damagedCooldown;
                if(e->health <= 0)
                {
                    // printf("%d\n", entities[entityIndex]->entityType);
                    if(entities[entityIndex]->entityType == HEALTH_HITBOX)
                    {
                        SpawnHealthPickup(e->position);
                    }
                }
                Vector2 damageTextPosition = Vector2Add(result.hitPosition, Vector2(0, -20));
                SpawnDamageText(damageTextPosition, equippedWeapon->attackDamage * attackMultiplier);
            }
            player->attackCooldown = equippedWeapon->attackCooldown;
            equippedWeapon->ammoCount--;
            if(equippedWeapon->ammoCount <= 0)
            {
                player->reloadCooldown = equippedWeapon->reloadCooldown;
            }
        }

        if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && equippedGrenade != NULL && equippedGrenade->count > 0)
        {
            if(equippedGrenade == decoy)
            {
                entity placedDecoy = GetPlacedDecoyTemplate();
                placedDecoy.position = GetMousePosition();
                allocAndAddEntity(placedDecoy);
            }
            else
            {
                for(int j = 0; j < firstFreeIndex; j++)
                {
                    if(entities[j]->entityType != ENEMY) { continue; }
                    float dist = getSqrDistance(GetMousePosition(), entities[j]->position);
                    if(dist > equippedGrenade->attackRange*equippedGrenade->attackRange ) { continue; }
                    if(equippedGrenade == freezingGrenade)
                    {
                        entities[j]->stamina += equippedGrenade->stamina;
                    }
                    if(equippedGrenade == explosiveGrenade)
                    {
                        entities[j]->health -= equippedGrenade->attackDamage;
                        Vector2 damageTextPosition = Vector2Add(entities[j]->position, Vector2(0, -20));
                        SpawnDamageText(damageTextPosition, equippedWeapon->attackDamage);
                    }
                }
            }
            equippedGrenade->count--;
        }

        if(player->damagedCooldown > 0)
        {
            player->damagedCooldown -= GetFrameTime();
        }
    }
}

void DeleteDeadEnemies()
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(entities[i]->entityType != ENEMY) { continue; }
        if(entities[i]->health <= 0)
        {
            playerCash += entities[i]->cashDropAmount;
            removeEntity(i);
            continue;
        }
    }
}

void UpdateEnemiesTarget()
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(entities[i]->entityType != ENEMY) { continue; }
        float minDist = FLT_MAX;
        int minIndex = -1;
        for(int j = 0; j < firstFreeIndex; j++)
        {
            if(entities[j]->entityType != PLAYER && entities[j]->entityType != DECOY) { continue; }
            float dist = getSqrDistance(entities[i]->position, entities[j]->position);
            if(dist > entities[i]->fovRange*entities[i]->fovRange) { continue; }
            if(dist < minDist)
            {
                minDist = dist;
                minIndex = j;
            }
        }
        entities[i]->target = minIndex >= 0 ? entities[minIndex] : NULL;
    }
}

void UpdateEnemiesMovement()
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(entities[i]->entityType != ENEMY) { continue; }
        if(!entities[i]->isRegenerating)
        {
            if(entities[i]->target != NULL)
            {
                float speed = entities[i]->speed;
                Vector2 enemyPos = entities[i]->position;
                Vector2 playerPos = entities[i]->target->position;
                if(enemyPos.x < playerPos.x)
                {
                    if(entities[i]->isFlipped)
                    {
                        entities[i]->flipTimer -= GetFrameTime();
                        if(entities[i]->flipTimer < 0)
                        {
                            entities[i]->flipTimer = entities[i]->flipDelay;
                            entities[i]->isFlipped = !entities[i]->isFlipped;
                            entities[i]->stamina -= 10.0f;
                        }
                    }
                    else
                    {
                        entities[i]->position.x += min(speed, fabsf(enemyPos.x - playerPos.x));
                        entities[i]->flipTimer = entities[i]->flipDelay;
                    }
                }
                if(enemyPos.x > playerPos.x)
                {
                    if(!entities[i]->isFlipped)
                    {
                        entities[i]->flipTimer -= GetFrameTime();
                        if(entities[i]->flipTimer < 0)
                        {
                            entities[i]->flipTimer = entities[i]->flipDelay;
                            entities[i]->isFlipped = !entities[i]->isFlipped;
                            entities[i]->stamina -= 10.0f;
                        }
                    }
                    else
                    {
                        entities[i]->position.x -= min(speed, fabsf(enemyPos.x - playerPos.x));
                        entities[i]->flipTimer = entities[i]->flipDelay;
                    }
                }
                if(enemyPos.y < playerPos.y)
                {
                    entities[i]->position.y += min(speed, fabsf(enemyPos.y - playerPos.y));
                }
                if(enemyPos.y > playerPos.y)
                {
                    entities[i]->position.y -= min(speed, fabsf(enemyPos.y - playerPos.y));
                }
            }
        }
        else
        {
            entities[i]->stamina += GetFrameTime() * entities[i]->staminaRegenerationSpeed;
            if(entities[i]->stamina > entities[i]->template->stamina)
            {
                entities[i]->isRegenerating = false;
            }
        }

        if(isEntityColliding(i, ENEMY | PLAYER))
        {
            entities[i]->position = entities[i]->previousPosition;
        }
        // printf("%f\n", entities[i]->stamina);
        // printf("%f\n", entities[i]->template->stamina);
        entities[i]->stamina -= getSqrDistance(entities[i]->position, entities[i]->previousPosition) * 0.1f;
        if(entities[i]->stamina < 0)
        {
            entities[i]->isRegenerating = true;
        }
        entities[i]->previousPosition = entities[i]->position;
        // printf("%d:  %f\n", i, entities[i]->currentAttackCooldown);
    }
}

void UpdateEnemiesAttack()
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(entities[i]->entityType != ENEMY) { continue; }
        if(entities[i]->damagedCooldown > 0)
        {
            entities[i]->damagedCooldown -= GetFrameTime();
        }

        if(entities[i]->target == NULL || entities[i]->isRegenerating) { continue; }
        if(entities[i]->target->entityType == DECOY) { continue; }
        entity* player = entities[i]->target;
        if(getSqrDistance(player->position, entities[i]->position) <= entities[i]->attackRange*entities[i]->attackRange)
        {
            if(entities[i]->attackCooldown > 0)
            {
                entities[i]->attackCooldown -= GetFrameTime();
            }
            else
            {
                player->health -= entities[i]->attackDamage;
                player->damagedCooldown = player->template->damagedCooldown;
                //printf("Health: %d ", player->health);
                if(player->health <= 0)
                {
                    ReloadGame();
                }
                entities[i]->attackCooldown = entities[i]->template->attackCooldown;
            }
        }
        else
        {
            entities[i]->attackCooldown = entities[i]->template->attackCooldown;
        }
    }
}

void UpdateHealthHitBoxes()
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(entities[i]->entityType != HEALTH_HITBOX) { continue; }
        if(entities[i]->parent == NULL) { removeEntity(i); continue; }
        Vector2 offset = entities[i]->previousPosition;
        if(entities[i]->parent->isFlipped)
        {
            offset.x = -offset.x;
        }
        entities[i]->position = Vector2Add(entities[i]->parent->position, offset);
        if(entities[i]->damagedCooldown > 0)
        {
            entities[i]->damagedCooldown -= GetFrameTime();
        }
    }
}

void UpdateHealthPickups()
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(entities[i]->entityType != HEALTH_PICKUP) { continue; }
        float dist = getSqrDistance(entities[i]->position, player->position);
        if(dist < 72.0f)
        {
            if(player->health < player->template->health)
            {
                player->health += 10.0f;
                if(player->health > player->template->health)
                {
                    player->health = player->template->health;
                }
                removeEntity(i);
            }
        }
    }
}

void UpdateWaves()
{
    // printf("number of enemies: %d\n", enemiesCount);
    if(nextWaveTimer > 0)
    {
        nextWaveTimer -= GetFrameTime();
        if(nextWaveTimer <= 0)
        {
            SpawnEnemies();
            currentWave++;
            isUpgraderUIActive = false;
        }
    }
}

void UpdateUpgrader()
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(entities[i]->entityType != UPGRADER) { continue; }
        if(nextWaveTimer <= 0) { entities[i]->isEnabled = false; continue; }
        else
        {
            entities[i]->isEnabled = true;
        }
        float dist = getSqrDistance(player->position, entities[i]->position);
        if(dist < 32.0f*32.0f)
        {
            if(IsKeyPressed(KEY_E))
            {
                isUpgraderUIActive = !isUpgraderUIActive;
            }
        }
        else
        {
            isUpgraderUIActive = false;
        }
    }
}

void UpdateDestroyTimer()
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        entity* e = entities[i];
        if(e->destroyTimer < 0.0f)
        {
            e->destroyTimer += GetFrameTime();
            if(e->destroyTimer >= 0.0f)
            {
                removeEntity(i);
            }
        }
    }
}

void UpdateDamageTexts()
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(!entities[i]->isUI) { continue; }
        if(entities[i]->entityType == UI_DAMAGE_TEXT)
        {
            entities[i]->position.y -= entities[i]->speed * GetFrameTime();
        }
    }
}

void EndWave()
{
    nextWaveTimer = NEXT_WAVE_TIMER;
}

char* GetGrenadeText()
{
    if(equippedGrenade == decoy) { return "Decoy"; }
    if(equippedGrenade == freezingGrenade) { return "Freezing Grenade"; }
    if(equippedGrenade == explosiveGrenade) { return "Explosvie Grenade"; }
    else { return "Not a Grenade!!!"; }
}

void DrawPlayerHUD()
{
    const char* waveText;
    if(nextWaveTimer > 0)
    {
        waveText = TextFormat("Next Wave starts in: %.1fs", nextWaveTimer);
    }
    else
    {
        waveText = TextFormat("Current Wave: %d", currentWave);
    }
    DrawText(waveText, 300, 20, 20, RED);

    DrawText(TextFormat("Number of Dashes: %d", player->numberOfDashes), 300, 400, 20, RED);
    DrawText(TextFormat("Player Health: %d", player->health), 25, 415, 20, RED);
    DrawText(TextFormat("Cash: %d", playerCash), 25, 25, 20, RED);
    const char* text = player->reloadCooldown > 0 ? "Reloading..." : TextFormat("Ammo: %d/%d", equippedWeapon->ammoCount, equippedWeapon->template->ammoCount);
    DrawText(text, 675, 415, 20, RED);
    DrawText(equippedWeapon == handgun ? "Handgun" : GetItemName(equippedWeapon->buttonIndex), 675, 390, 20, RED);
    if(equippedGrenade == NULL) { return; }
    DrawText(TextFormat("Equipped Grenade:\n %s", GetGrenadeText()), 600, 325, 20, RED);
    DrawText(TextFormat("Count: %d", equippedGrenade->count), 645, 365, 20, RED);
}

bool isMouseInside(entity* e)
{
    Vector2 position = e->position;
    Vector2 size = e->size;
    Vector2 mousePosition = GetMousePosition();
    bool a = mousePosition.x > position.x;
    bool b = mousePosition.x < position.x + size.x;
    bool c = mousePosition.y > position.y;
    bool d = mousePosition.y < position.y + size.y;
    return a && b && c && d;
}

int* GetCurrentUpgrade(int upgradeIndex)
{
    switch(upgradeIndex)
    {
        case 0: return &currentHandgunUpgrade;
        case 1: return &currentDashUpgrade;
        case 2: return &currentMaxHealthUpgrade;
    };
    return NULL;
}

int GetUpgradesCount(int upgradeIndex)
{
    switch(upgradeIndex)
    {
        case 0: return handgunUpgradesCount;
        case 1: return dashUpgradesCount;
        case 2: return maxHealthUpgradesCount;
    };
    return INT_MAX;
}

int GetUpgradeCost(int upgradeIndex)
{
    int currentUpgrade = *GetCurrentUpgrade(upgradeIndex);
    if(currentUpgrade == GetUpgradesCount(upgradeIndex)-1) { return INT_MAX; }
    switch(upgradeIndex)
    {
        case 0: return handgunUpgradesPrices[currentUpgrade];
        case 1: return dashUpgradesPrices[currentUpgrade];
        case 2: return maxHealthUpgradesPrices[currentUpgrade];
    };
    return INT_MAX;
}

void Upgrade(entity* thisButton)
{
    int upgradeIndex = thisButton->buttonIndex;
    thisButton->damagedCooldown = thisButton->template->damagedCooldown;
    int* currentUpgrade = GetCurrentUpgrade(upgradeIndex);
    int upgradesCount = GetUpgradesCount(upgradeIndex);
    if(*currentUpgrade < upgradesCount-1)
    {
        playerCash -= GetUpgradeCost(upgradeIndex);
        (*currentUpgrade)++;
        SetUpgrades();
    }
}

int GetPrice(int buyIndex)
{
    switch(buyIndex)
    {
        case 0: return sharpenerPrice;
        case 1: return shotgunPrice;
        case 2: return sniperGunPrice;
        case 3: return decoyPrice;
        case 4: return freezingGrenadePrice;
        case 5: return explosiveGrenadePrice;
    };
    return INT_MAX;
}

bool* IsBought(int buyIndex)
{
    switch(buyIndex)
    {
        case 0: return &isSharpenerUnlocked;
        case 1: return &isShotgunUnlocked;
        case 2: return &isSniperGunUnlocked;
    };
    return NULL;
}

entity* GetBuyItem(int buyIndex)
{
    switch(buyIndex)
    {
        case 0: return sharpener;
        case 1: return shotgun;
        case 2: return sniperGun;
        case 3: return decoy;
        case 4: return freezingGrenade;
        case 5: return explosiveGrenade;
    };
    return NULL;
}

void BuyOrEquip(entity* thisButton)
{
    int buyIndex = thisButton->buttonIndex;

    // thisButton->damagedCooldown = GetTemplate(thisButton->templateIndex).damagedCooldown;

    thisButton->damagedCooldown = thisButton->template->damagedCooldown;

    bool* isBought = IsBought(buyIndex);
    if(isBought != NULL)
    {
        if(!(*isBought))
        {
            if(playerCash < GetPrice(buyIndex)) { return; }
            playerCash -= GetPrice(buyIndex);
            *isBought = true;
        }
    }
    else
    {
        if(playerCash < GetPrice(buyIndex)) { return; }
        playerCash -= GetPrice(buyIndex);
    }
    entity* item = GetBuyItem(buyIndex);
    if(item->canBeEquipped)
    {
        inventorySecondSlot = item;
    }
    else
    {
        item->count++;
        if(equippedGrenade == NULL)
        {
            equippedGrenade = item;
        }
    }
}

void DrawDamageTexts()
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(!entities[i]->isUI) { continue; }
        if(entities[i]->entityType == UI_DAMAGE_TEXT)
        {
            DrawText(TextFormat("-%d", entities[i]->attackDamage), entities[i]->position.x, entities[i]->position.y, 20, entities[i]->defaultColor);
            continue;
        }
    }
}

void DrawUpgraderUI()
{
    if(!isUpgraderUIActive) { return; }
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(!entities[i]->isUI) { continue; }
        if(entities[i]->entityType == UI_UPGRADER_IMAGE)
        {
            DrawRectangleV(entities[i]->position, entities[i]->size, entities[i]->defaultColor);
            continue;
        }

        if(entities[i]->entityType == UI_UPGRADER_BUTTON)
        {
            int upgradeIndex = entities[i]->buttonIndex;
            bool isUpgradeAvailable = GetUpgradeCost(upgradeIndex) <= playerCash;
            Color outputColor = entities[i]->damagedColor;
            if(isUpgradeAvailable)
            {
                outputColor = entities[i]->damagedCooldown > 0 ? entities[i]->damagedColor : entities[i]->defaultColor;
            }

            Vector2 buttonPos = entities[i]->position;
            DrawRectangleV(buttonPos, entities[i]->size, outputColor);
            DrawText(GetUpgradeButtonText(upgradeIndex), buttonPos.x + 12, buttonPos.y + 22, 20, RED);

            int upgradeCost = GetUpgradeCost(upgradeIndex);
            const char* detailsText = "Upgrade Maximazed";
            if(upgradeCost < INT_MAX)
            {
                detailsText = TextFormat("Cash Required: %d", upgradeCost);
            }

            DrawText(detailsText, buttonPos.x + 24, buttonPos.y + 44, 12, RED);

            if(isMouseInside(entities[i]) && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && isUpgradeAvailable)
            {
                (*entities[i]->buttonCallback)(entities[i]);
            }

            if(entities[i]->damagedCooldown > 0)
            {
                entities[i]->damagedCooldown -= GetFrameTime();
            }
            continue;
        }
        int buyIndex = entities[i]->buttonIndex;
        bool isBuyAvailable = GetPrice(buyIndex) <= playerCash || (IsBought(buyIndex) != NULL && *IsBought(buyIndex) && GetBuyItem(buyIndex)->canBeEquipped);
        Color outputColor = entities[i]->damagedColor;
        if(isBuyAvailable)
        {
            outputColor = entities[i]->damagedCooldown > 0 ? entities[i]->damagedColor : entities[i]->defaultColor;
        }
        Vector2 buttonPos = entities[i]->position;
        DrawRectangleV(buttonPos, entities[i]->size, outputColor);

        char* text = IsBought(buyIndex) != NULL && *IsBought(buyIndex) && GetBuyItem(buyIndex)->canBeEquipped ? "Equip " : "Buy";
        DrawText(TextFormat("%s %s", text, GetItemName(buyIndex)), buttonPos.x + 12, buttonPos.y + 22, 20, RED);

        int buyCost = GetPrice(buyIndex);
        const char* detailsText = TextFormat("%s Accquired", GetItemName(buyIndex));
        if(IsBought(buyIndex) == NULL || !(*IsBought(buyIndex) && GetBuyItem(buyIndex)->canBeEquipped)) // NAND = OR of Negatives
        {
            detailsText = TextFormat("Cash Required: %d", buyCost);
        }
        if(GetBuyItem(buyIndex)->canBeEquipped && inventorySecondSlot == GetBuyItem(buyIndex))
        {
            detailsText = TextFormat("%s Equipped", GetItemName(buyIndex));
        }

        DrawText(detailsText, buttonPos.x + 24, buttonPos.y + 44, 12, RED);

        if(isMouseInside(entities[i]) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && isBuyAvailable)
        {
            (*entities[i]->buttonCallback)(entities[i]);
        }

        if(entities[i]->damagedCooldown > 0)
        {
            entities[i]->damagedCooldown -= GetFrameTime();
        }
    }
}

void PrepareUpgrades()
{
    handgunUpgrades[0].attackCooldown = 0.5f;
    handgunUpgrades[0].attackDamage = 10;

    handgunUpgrades[1].attackCooldown = 0.25f;
    handgunUpgrades[1].attackDamage = 20;
    handgunUpgradesPrices[0] = 100;

    dashUpgrades[0].dashCooldown = 2.0f;
    dashUpgrades[0].numberOfDashes = 2;

    dashUpgrades[1].dashCooldown = 1.0f;
    dashUpgrades[1].numberOfDashes = 4;
    dashUpgradesPrices[0] = 100;

    maxHealthUpgrades[0] = 100;
    maxHealthUpgrades[1] = 150;
    maxHealthUpgradesPrices[0] = 100;
}

void SetUpgrades()
{
    player->template->dashCooldown = dashUpgrades[currentDashUpgrade].dashCooldown;
    player->template->numberOfDashes = dashUpgrades[currentDashUpgrade].numberOfDashes;
    player->template->health = maxHealthUpgrades[currentMaxHealthUpgrade];
    player->template->attackCooldown = handgunUpgrades[currentHandgunUpgrade].attackCooldown;
    player->template->attackDamage = handgunUpgrades[currentHandgunUpgrade].attackDamage;
}

void SpawnHitPoint(Vector2 position)
{
    entity hitPoint = GetHitPointTemplate();
    SetStartPosition(&hitPoint, position);
    allocAndAddEntity(hitPoint);
}

void SpawnDamageText(Vector2 position, int damage)
{
    entity damageText = GetDamageTextTemplate();
    SetStartPosition(&damageText, position);
    damageText.attackDamage = damage;
    allocAndAddEntity(damageText);
}

void SpawnHealthPickup(Vector2 position)
{
    entity healthPickup = GetHealthPickupTemplate();
    SetStartPosition(&healthPickup, position);
    allocAndAddEntity(healthPickup);
}

void NewSpawnUI()
{
    entity upgraderUIBackground = GetBasicTemplate();
    upgraderUIBackground.size = Vector2(800, 450);
    upgraderUIBackground.defaultColor = (Color){128, 128, 128, 128};
    upgraderUIBackground.entityType = UI_UPGRADER_IMAGE;
    upgraderUIBackground.isUI = true;
    allocAndAddEntity(upgraderUIBackground);

    entity upgradeButtonTemplate = GetUpgradeButtonTemplate();
    SetStartPositionAndSize(&upgradeButtonTemplate, Vector2(32, 64), Vector2(208, 64));
    upgradeButtonTemplate.buttonIndex = 0;
    allocAndAddEntity(upgradeButtonTemplate);
    SetStartPositionAndSize(&upgradeButtonTemplate, Vector2(256, 64), Vector2(192, 64));
    upgradeButtonTemplate.buttonIndex = 1;
    allocAndAddEntity(upgradeButtonTemplate);
    SetStartPositionAndSize(&upgradeButtonTemplate, Vector2(480, 64), Vector2(224, 64));
    upgradeButtonTemplate.buttonIndex = 2;
    allocAndAddEntity(upgradeButtonTemplate);

    entity buyButtonTemplate = GetBuyButtonTemplate();
    SetStartPositionAndSize(&buyButtonTemplate, Vector2(32, 144), Vector2(208, 64));
    buyButtonTemplate.buttonIndex = 0;
    allocAndAddEntity(buyButtonTemplate);
    SetStartPositionAndSize(&buyButtonTemplate, Vector2(32, 224), Vector2(208, 64));
    buyButtonTemplate.buttonIndex = 1;
    allocAndAddEntity(buyButtonTemplate);
    SetStartPositionAndSize(&buyButtonTemplate, Vector2(256, 144), Vector2(208, 64));
    buyButtonTemplate.buttonIndex = 3;
    allocAndAddEntity(buyButtonTemplate);
    SetStartPositionAndSize(&buyButtonTemplate, Vector2(256, 224), Vector2(208, 64));
    buyButtonTemplate.buttonIndex = 4;
    allocAndAddEntity(buyButtonTemplate);
    SetStartPositionAndSize(&buyButtonTemplate, Vector2(480, 224), Vector2(208, 64));
    buyButtonTemplate.buttonIndex = 5;
    allocAndAddEntity(buyButtonTemplate);
}

void SpawnEntities()
{
    entity playerTemp = GetPlayerTemplate();
    SetStartPosition(&playerTemp, Vector2(400, 200));
    playerTemp.damagedCooldown = 0;
    player = allocAndAddEntity(playerTemp);
    handgun = allocAndAddEntity(GetHandgunTemplate());
    equippedWeapon = handgun;
    sharpener = allocAndAddEntity(GetSharpenerTemplate());
    shotgun = allocAndAddEntity(GetShotgunTemplate());
    sniperGun = allocAndAddEntity(GetSniperGunTemplate());
    inventorySecondSlot = NULL;
    decoy = allocAndAddEntity(GetDecoyTemplate());
    freezingGrenade = allocAndAddEntity(GetFreezingGrenadeTemplate());
    explosiveGrenade = allocAndAddEntity(GetExplosiveGrenadeTemplate());
    allocAndAddEntity(GetUpgraderTemplate());
    NewSpawnUI();
}

void SpawnEnemies()
{
    entity bigEnemy = GetBigEnemyTemplate();
    SetStartPosition(&bigEnemy, Vector2(100, 200));
    entity* be = allocAndAddEntity(bigEnemy);

    entity healthHitbox = GetHealthHitboxTemplate();
    SetStartPositionAndSize(&healthHitbox, Vector2(-36.0f, 0.0f), Vector2(16, 16));
    healthHitbox.parent = be;
    entity* hh = allocAndAddEntity(healthHitbox);

    be->child = hh;
    for(int i = 0; i < 3; i++)
    {
        entity fastEnemy = GetFastEnemyTemplate();
        SetStartPosition(&fastEnemy, Vector2(200, 300 + i * 30));
        entity* fe = allocAndAddEntity(fastEnemy);

        entity healthHitbox = GetHealthHitboxTemplate();
        SetStartPositionAndSize(&healthHitbox, Vector2(-10.0f, 0.0f), Vector2(4, 4));
        healthHitbox.parent = fe;
        entity* hh = allocAndAddEntity(healthHitbox);

        fe->child = hh;
    }
}

void ReloadGame()
{
    firstFreeIndex = 0;
    nextWaveTimer = NEXT_WAVE_TIMER;
    currentWave = 0;
    enemiesCount = 0;
    playerCash = 0;
    currentHandgunUpgrade = 0;
    currentDashUpgrade = 0;
    currentMaxHealthUpgrade = 0;
    int index = 0;
    bool* isUnlocked = IsBought(index);
    while(isUnlocked != NULL)
    {
        *isUnlocked = false;
        index++;
        isUnlocked = IsBought(index);
    }
    inventorySecondSlot = NULL;
    equippedWeapon = NULL;
    equippedGrenade = NULL;
    SpawnEntities();
}

int main()
{
    InitializeTemplates();
    entities = malloc(sizeof(entity*) * capacity);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(800, 450, "GameRunner - Raylib - C");

    SpawnEntities();
    PrepareUpgrades();
    SetUpgrades();
    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(SKYBLUE);
            UpdatePlayerMovement();
            UpdatePlayerAttack();
            DeleteDeadEnemies();
            UpdateEnemiesTarget();
            UpdateEnemiesMovement();
            UpdateEnemiesAttack();
            UpdateHealthHitBoxes();
            UpdateHealthPickups();
            UpdateWaves();
            UpdateUpgrader();
            UpdateDamageTexts();
            UpdateDestroyTimer();
            for(int i = 0; i < firstFreeIndex; i++)
            {
                if(!entities[i]->isEnabled || entities[i]->isUI) { continue; }
                //printf("i: %d\n", i);
                // printf("entityType: %d\n", entities[i]->entityType);
                Vector2 startPos = GetEntityCorner(entities[i]);
                if(entities[i]->damagedCooldown > 0)
                {
                    DrawRectangleV(startPos, entities[i]->size, entities[i]->damagedColor);
                }
                else
                {
                    Color outputColor = entities[i]->isRegenerating ? GRAY : entities[i]->defaultColor;
                    DrawRectangleV(startPos, entities[i]->size, outputColor);
                }
            }
            DrawPlayerHUD();
            DrawDamageTexts();
            DrawUpgraderUI();

            DrawFPS(0, 0);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
