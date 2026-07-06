#include "raylib.h"
#include "raymath.h"
#include <limits.h>
#include <stdlib.h>
#include <float.h>
#include <stdio.h>

#define DEFAULT 0
#define PLAYER 1 << 1
#define UPGRADER 1 << 2
#define ENEMY 1 << 3
#define HEALTH_HITBOX 1 << 4
#define HEALTH_PICKUP 1 << 5
#define UI_UPGRADER_BUTTON 1 << 6
#define UI_UPGRADER_IMAGE 1 << 7

#define NEXT_WAVE_TIMER 5.0f

#define Vector2(x,y) (Vector2){x, y}

#define min(a, b) a > b ? b : a
#define max(a, b) a > b ? a : b

void ReloadGame();
void EndWave();
void SpawnEnemies();
void SpawnHealthPickup(Vector2 position);
void SetUpgrades();

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
    int maxNumberOfDashes;
    int numberOfDashes;
    float dashCooldown;
    float currentDashCooldown;
    int health;
    int maxHealth;
    int attackDamage;
    float attackCooldown;
    float currentAttackCooldown;
    float attackRange;
    float damagedCooldown;
    float currentDamagedCooldown;
    Color damagedColor;
    float fovRange;
    float maxStamina;
    float stamina;
    bool isRegenerating;
    float staminaRegenerationSpeed;
    struct entity* parent;
    struct entity* child;
    float flipDelay;
    float flipTimer;
    bool isFlipped;
    bool isEnabled;
    bool isUI;
    void (*buttonCallback)(struct entity* thisButton);
    int cashDropAmount;
    int upgradeUIButtonIndex;
} entity;

int firstFreeIndex = 0;
int capacity = 4;
entity** entities;

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

int enemiesCount = 0;

int currentWave = 0;

float nextWaveTimer = NEXT_WAVE_TIMER;

int playerCash = 0;

entity* player;

entity* upgraderUIBackground;
entity* handgunUpgradeButton;
entity* dashUpgradeButton;
entity* maxHealthUpgradeButton;

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

entity* createNewEntity(Vector2 pos, Vector2 size, Vector2 pivot, Color defaultColor, uint entityType, float speed, int dashDistance, int maxNumberOfDashes, float dashCooldown, int maxHealth, int attackDamage, float attackCooldown, float attackRange, float damagedCooldown, Color damagedColor, float fovRange, float maxStamina, float staminaRegenerationSpeed, entity* parent, float flipDelay)
{
    entity* newEntity = malloc(sizeof(entity));
    newEntity->position = pos;
    newEntity->size = size;
    newEntity->pivot = pivot;
    newEntity->previousPosition = pos;
    newEntity->defaultColor = defaultColor;
    newEntity->entityType = entityType;
    newEntity->speed = speed;
    newEntity->dashDistance = dashDistance;
    newEntity->maxNumberOfDashes = maxNumberOfDashes;
    newEntity->numberOfDashes = maxNumberOfDashes;
    newEntity->dashCooldown = dashCooldown;
    newEntity->currentDashCooldown = dashCooldown;
    newEntity->maxHealth = maxHealth;
    newEntity->health = maxHealth;
    newEntity->attackDamage = attackDamage;
    newEntity->attackCooldown = attackCooldown;
    newEntity->currentAttackCooldown = attackCooldown;
    newEntity->attackRange = attackRange;
    newEntity->damagedCooldown = damagedCooldown;
    newEntity->currentDamagedCooldown = 0;
    newEntity->damagedColor = damagedColor;
    newEntity->fovRange = fovRange;
    newEntity->maxStamina = maxStamina;
    newEntity->stamina = maxStamina;
    newEntity->isRegenerating = false;
    newEntity->staminaRegenerationSpeed = staminaRegenerationSpeed;
    newEntity->parent = parent;
    newEntity->child = NULL;
    newEntity->flipDelay = flipDelay;
    newEntity->flipTimer = flipDelay;
    newEntity->isFlipped = false;
    newEntity->isEnabled = true;
    newEntity->isUI = false;
    return newEntity;
}

entity* allocNewEntity(entity copiedEntity)
{
    entity* newEntity = malloc(sizeof(entity));
    newEntity->position = copiedEntity.position;
    newEntity->size = copiedEntity.size;
    newEntity->pivot = copiedEntity.pivot;
    newEntity->previousPosition = copiedEntity.position;
    newEntity->defaultColor = copiedEntity.defaultColor;
    newEntity->entityType = copiedEntity.entityType;
    newEntity->speed = copiedEntity.speed;
    newEntity->dashDistance = copiedEntity.dashDistance;
    newEntity->maxNumberOfDashes = copiedEntity.maxNumberOfDashes;
    newEntity->numberOfDashes = copiedEntity.maxNumberOfDashes;
    newEntity->dashCooldown = copiedEntity.dashCooldown;
    newEntity->currentDashCooldown = copiedEntity.dashCooldown;
    newEntity->maxHealth = copiedEntity.maxHealth;
    newEntity->health = copiedEntity.maxHealth;
    newEntity->attackDamage = copiedEntity.attackDamage;
    newEntity->attackCooldown = copiedEntity.attackCooldown;
    newEntity->currentAttackCooldown = copiedEntity.attackCooldown;
    newEntity->attackRange = copiedEntity.attackRange;
    newEntity->damagedCooldown = copiedEntity.damagedCooldown;
    newEntity->currentDamagedCooldown = 0;
    newEntity->damagedColor = copiedEntity.damagedColor;
    newEntity->fovRange = copiedEntity.fovRange;
    newEntity->maxStamina = copiedEntity.maxStamina;
    newEntity->stamina = copiedEntity.maxStamina;
    newEntity->isRegenerating = false;
    newEntity->staminaRegenerationSpeed = copiedEntity.staminaRegenerationSpeed;
    newEntity->parent = copiedEntity.parent;
    newEntity->child = NULL;
    newEntity->flipDelay = copiedEntity.flipDelay;
    newEntity->flipTimer = copiedEntity.flipDelay;
    newEntity->isFlipped = false;
    newEntity->isEnabled = true;
    newEntity->isUI = false;
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

void UpdatePlayer()
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

        if(player->numberOfDashes < player->maxNumberOfDashes)
        {
            player->currentDashCooldown -= GetFrameTime();
            if(player->currentDashCooldown <= 0)
            {
                player->numberOfDashes++;
                player->currentDashCooldown = player->dashCooldown;
            }
        }

        if(player->currentAttackCooldown > 0)
        {
            player->currentAttackCooldown -= GetFrameTime();
        }
        // printf("%f\n", player->currentAttackCooldown);
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && player->currentAttackCooldown <= 0 && !isUpgraderUIActive)
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
                    entities[entityIndex]->currentDamagedCooldown = entities[entityIndex]->damagedCooldown;
                    e = entities[entityIndex]->parent;
                }
                e->health -= player->attackDamage;
                e->currentDamagedCooldown = e->damagedCooldown;
                if(e->health <= 0)
                {
                    printf("%d\n", entities[entityIndex]->entityType);
                    if(entities[entityIndex]->entityType == HEALTH_HITBOX)
                    {
                        SpawnHealthPickup(e->position);
                    }
                }
            }
            player->currentAttackCooldown = player->attackCooldown;
        }

        if(player->currentDamagedCooldown > 0)
        {
            player->currentDamagedCooldown -= GetFrameTime();
        }
        player->previousPosition = player->position;
        //printf("x: %f y: %f \n", player->position.x, player->position.y);
    }
}
void UpdateEnemies()
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(entities[i]->entityType != ENEMY) { continue; }
        if(entities[i]->health <= 0)
        {
            removeEntity(i);
            continue;
        }
        float minDist = FLT_MAX;
        int minIndex = -1;
        for(int j = 0; j < firstFreeIndex; j++)
        {
            if(entities[j]->entityType != PLAYER) { continue; }
            float dist = getSqrDistance(entities[i]->position, entities[j]->position);
            if(dist > entities[i]->fovRange*entities[i]->fovRange) { continue; }
            if(dist < minDist)
            {
                minDist = dist;
                minIndex = j;
            }
        }
        if(!entities[i]->isRegenerating)
        {
            if(minIndex >= 0)
            {
                float speed = entities[i]->speed;
                Vector2 enemyPos = entities[i]->position;
                Vector2 playerPos = entities[minIndex]->position;
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
            if(entities[i]->stamina > entities[i]->maxStamina)
            {
                entities[i]->isRegenerating = false;
            }
        }

        if(isEntityColliding(i, ENEMY | PLAYER))
        {
            entities[i]->position = entities[i]->previousPosition;
        }
        // printf("%f\n", entities[i]->stamina);
        entities[i]->stamina -= getSqrDistance(entities[i]->position, entities[i]->previousPosition) * 0.1f;
        if(entities[i]->stamina < 0)
        {
            entities[i]->isRegenerating = true;
        }
        entities[i]->previousPosition = entities[i]->position;


        if(entities[i]->currentDamagedCooldown > 0)
        {
            entities[i]->currentDamagedCooldown -= GetFrameTime();
        }

        if(minIndex == -1 || entities[i]->isRegenerating) { continue; }
        entity* player = entities[minIndex];
        if(getSqrDistance(player->position, entities[i]->position) <= entities[i]->attackRange*entities[i]->attackRange)
        {
            if(entities[i]->currentAttackCooldown > 0)
            {
                entities[i]->currentAttackCooldown -= GetFrameTime();
            }
            else
            {
                player->health -= entities[i]->attackDamage;
                player->currentDamagedCooldown = player->damagedCooldown;
                //printf("Health: %d ", player->health);
                if(player->health <= 0)
                {
                    ReloadGame();
                }
                entities[i]->currentAttackCooldown = entities[i]->attackCooldown;
            }
        }
        else
        {
            entities[i]->currentAttackCooldown = entities[i]->attackCooldown;
        }
        // printf("%d:  %f\n", i, entities[i]->currentAttackCooldown);
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
        if(entities[i]->currentDamagedCooldown > 0)
        {
            entities[i]->currentDamagedCooldown -= GetFrameTime();
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
            if(player->health < player->maxHealth)
            {
                player->health += 10.0f;
                if(player->health > player->maxHealth)
                {
                    player->health = player->maxHealth;
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

void EndWave()
{
    nextWaveTimer = NEXT_WAVE_TIMER;
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


void Upgrade(entity* thisButton)
{
    int upgradeIndex = thisButton->upgradeUIButtonIndex;
    thisButton->currentDamagedCooldown = thisButton->damagedCooldown;
    int* currentUpgrade = GetCurrentUpgrade(upgradeIndex);
    int upgradesCount = GetUpgradesCount(upgradeIndex);
    if(*currentUpgrade < upgradesCount-1)
    {
        (*currentUpgrade)++;
        SetUpgrades();
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
        int upgradeIndex = entities[i]->upgradeUIButtonIndex;
        Vector2 buttonPos = entities[i]->position;
        Color outputColor = entities[i]->currentDamagedCooldown > 0 ? entities[i]->damagedColor : entities[i]->defaultColor;
        DrawRectangleV(buttonPos, entities[i]->size, outputColor);
        DrawText(GetUpgradeButtonText(upgradeIndex), buttonPos.x + 12, buttonPos.y + 22, 20, RED);
        if(isMouseInside(entities[i]) && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            (*entities[i]->buttonCallback)(entities[i]);
        }
        if(entities[i]->currentDamagedCooldown > 0)
        {
            entities[i]->currentDamagedCooldown -= GetFrameTime();
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
    player->dashCooldown = dashUpgrades[currentDashUpgrade].dashCooldown;
    player->maxNumberOfDashes = dashUpgrades[currentDashUpgrade].numberOfDashes;
    player->maxHealth = maxHealthUpgrades[currentMaxHealthUpgrade];
    player->attackCooldown = handgunUpgrades[currentHandgunUpgrade].attackCooldown;
    player->attackDamage = handgunUpgrades[currentHandgunUpgrade].attackDamage;
}

void SpawnHealthPickup(Vector2 position)
{
    entity* health_pickup = createNewEntity(position, Vector2(8,8), Vector2(0.5f, 0.5f), BLUE, HEALTH_PICKUP, 0, 0, 0, 0, 0, 0, 0, 0, 0, RED, 0, 0, 0, NULL, 0);
    addEntity(health_pickup);
}

void SpawnUI()
{
    upgraderUIBackground = createNewEntity(Vector2(0,0), Vector2(800, 450), Vector2(0,0), (Color){128, 128, 128, 128}, UI_UPGRADER_IMAGE, 0, 0, 0, 0, 0, 0, 0, 0, 0, RED, 0, 0, 0, NULL, 0);
    addEntity(upgraderUIBackground);
    handgunUpgradeButton = createNewEntity(Vector2(32, 64), Vector2(208,64), Vector2(0,0), WHITE, UI_UPGRADER_BUTTON, 0, 0, 0, 0, 0, 0, 0, 0, 0.125f, GRAY, 0, 0, 0, NULL, 0);
    addEntity(handgunUpgradeButton);
    dashUpgradeButton = createNewEntity(Vector2(256, 64), Vector2(192,64), Vector2(0,0), WHITE, UI_UPGRADER_BUTTON, 0, 0, 0, 0, 0, 0, 0, 0, 0.125f, GRAY, 0, 0, 0, NULL, 0);
    addEntity(dashUpgradeButton);
    maxHealthUpgradeButton = createNewEntity(Vector2(480, 64), Vector2(224,64), Vector2(0,0), WHITE, UI_UPGRADER_BUTTON, 0, 0, 0, 0, 0, 0, 0, 0, 0.125f, GRAY, 0, 0, 0, NULL, 0);
    addEntity(maxHealthUpgradeButton);

    upgraderUIBackground->isUI = true;
    handgunUpgradeButton->isUI = true;
    handgunUpgradeButton->buttonCallback = Upgrade;
    handgunUpgradeButton->upgradeUIButtonIndex = 0;
    dashUpgradeButton->isUI = true;
    dashUpgradeButton->buttonCallback = Upgrade;
    dashUpgradeButton->upgradeUIButtonIndex = 1;
    maxHealthUpgradeButton->isUI = true;
    maxHealthUpgradeButton->buttonCallback = Upgrade;
    maxHealthUpgradeButton->upgradeUIButtonIndex = 2;
}

void SpawnEntites()
{
    player = createNewEntity(Vector2(400, 200), Vector2(20, 20), Vector2(0.5f, 0.5f), VIOLET, PLAYER, 2.5f, 128, 2, 2.0f, 100, 10, 0.5f, 10.0f, 0.125f, RED, 0, 0, 0, NULL, 0);
    addEntity(player);
    entity* upgrader = createNewEntity(Vector2(400,250), Vector2(32, 32), Vector2(0.5f, 0.5f), GOLD, UPGRADER, 0, 0, 0, 0, 0 ,0, 0, 0, 0, RED, 0, 0, 0, NULL, 0);
    addEntity(upgrader);
    SpawnUI();
}

void SpawnEnemies()
{
    entity* bigEnemy = createNewEntity(Vector2(100, 200), Vector2(64, 64), Vector2(0.5f, 0.5f), DARKPURPLE, ENEMY, 0.5f, 0, 0, 0, 40, 20, 0.5f, 50.0f, 0.125f, RED, 350.0f, 100.0f, 20.0f, NULL, 2.0f);
    addEntity(bigEnemy);
    entity* health_hitbox = createNewEntity(Vector2(-36.0f, 0.0f), Vector2(16,16), Vector2(0.5f, 0.5f), BLUE, HEALTH_HITBOX, 0, 0, 0, 0, 0, 0, 0, 0, 0.125f, RED, 0, 0, 0, bigEnemy, 0);
    addEntity(health_hitbox);
    bigEnemy->child = health_hitbox;
    bigEnemy->cashDropAmount = 50;
    for(int i = 0; i < 3; i++)
    {
        entity* fastEnemy = createNewEntity(Vector2(200, 300 + i * 30), Vector2(16, 16), Vector2(0.5f, 0.5f), DARKPURPLE, ENEMY, 3.0f, 0, 0, 0, 20, 10, 0.25f, 25.0f, 0.125f, RED, 650.0f, 500.0f, 100.0f, NULL, 0.5f);
        addEntity(fastEnemy);
        entity* health_hitbox = createNewEntity(Vector2(-10.0f, 0.0f), Vector2(4,4), Vector2(0.5f, 0.5f), BLUE, HEALTH_HITBOX, 0, 0, 0, 0, 0, 0, 0, 0, 0.125f, RED, 0, 0, 0, fastEnemy, 0);
        addEntity(health_hitbox);
        fastEnemy->child = health_hitbox;
        fastEnemy->cashDropAmount = 25;
    }
}

void ReloadGame()
{
    firstFreeIndex = 0;
    nextWaveTimer = NEXT_WAVE_TIMER;
    currentWave = 0;
    enemiesCount = 0;
    SpawnEntites();
}

int main()
{
    entities = malloc(sizeof(entity*) * capacity);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(800, 450, "GameRunner - Raylib - C");

    SpawnEntites();
    PrepareUpgrades();
    SetUpgrades();
    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(SKYBLUE);
            UpdatePlayer();
            UpdateEnemies();
            UpdateHealthHitBoxes();
            UpdateHealthPickups();
            UpdateWaves();
            UpdateUpgrader();
            for(int i = 0; i < firstFreeIndex; i++)
            {
                if(!entities[i]->isEnabled || entities[i]->isUI) { continue; }
                //printf("i: %d\n", i);
                // printf("entityType: %d\n", entities[i]->entityType);
                Vector2 startPos = GetEntityCorner(entities[i]);
                if(entities[i]->currentDamagedCooldown > 0)
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
            DrawUpgraderUI();

            DrawFPS(0, 0);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
