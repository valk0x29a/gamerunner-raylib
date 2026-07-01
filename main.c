#include "raylib.h"
#include "raymath.h"
#include <complex.h>
#include <stdlib.h>
#include <float.h>
#include <stdio.h>

#define DEFAULT 0
#define PLAYER 1 << 1
#define UPGRADER 1 << 2
#define ENEMY 1 << 3
#define Vector2(x,y) (Vector2){x, y}

#define min(a, b) a > b ? b : a
#define max(a, b) a > b ? a : b

void ReloadGame();
void EndWave();
void SpawnEnemies();

typedef struct RayCastHitResult
{
    bool colliding;
    Vector2 hitPosition;
    int entityIndex;
} RayCastHitResult;

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
} entity;

int firstFreeIndex = 0;
int capacity = 2;
entity** entities;

int enemiesCount = 0;

int currentWave = 0;

float nextWaveTimer = 5.0f;

entity* player;

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
    if(firstFreeIndex == capacity)
    {
        capacity *= 2;
        entities = realloc(entities, sizeof(entity*) * capacity);
    }
    firstFreeIndex++;
    if(newEntity->entityType == ENEMY) { enemiesCount++; }
}

void removeEntity(int entityIndex)
{
    int lastFilledIndex = firstFreeIndex-1;
    if(entities[lastFilledIndex]->entityType == ENEMY)
    {
        enemiesCount--;
        if(enemiesCount <= 0)
        {
            EndWave();
        }
    }
    if(entityIndex != lastFilledIndex)
    {
        entity* endEntity = entities[lastFilledIndex];
        entity* deletedEntity = entities[entityIndex];
        entities[entityIndex] = endEntity;
        entities[lastFilledIndex] = deletedEntity;
    }
    free(entities[lastFilledIndex]);
    firstFreeIndex--;

}

entity* createNewEntity(Vector2 pos, Vector2 size, Vector2 pivot, Color defaultColor, uint entityType, float speed, int dashDistance, int maxNumberOfDashes, float dashCooldown, int maxHealth, int attackDamage, float attackCooldown)
{
    entity* newEntity = malloc(sizeof(entity));
    newEntity->position = pos;
    newEntity->size = size;
    newEntity->pivot = pivot;
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
    return newEntity;
}

entity* allocNewEntity(entity copiedEntity)
{
    entity* newEntity = malloc(sizeof(entity));
    newEntity->position = copiedEntity.position;
    newEntity->size = copiedEntity.size;
    newEntity->pivot = copiedEntity.pivot;
    newEntity->defaultColor = copiedEntity.defaultColor;
    newEntity->entityType = copiedEntity.entityType;
    newEntity->speed = copiedEntity.speed;
    newEntity->dashDistance = copiedEntity.dashDistance;
    newEntity->maxNumberOfDashes = copiedEntity.maxNumberOfDashes;
    newEntity->numberOfDashes = copiedEntity.numberOfDashes;
    newEntity->dashCooldown = copiedEntity.dashCooldown;
    newEntity->currentDashCooldown = copiedEntity.currentDashCooldown;
    newEntity->maxHealth = copiedEntity.maxHealth;
    newEntity->health = copiedEntity.maxHealth;
    newEntity->attackDamage = copiedEntity.attackDamage;
    newEntity->attackCooldown = copiedEntity.attackCooldown;
    newEntity->currentAttackCooldown = copiedEntity.attackCooldown;
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

bool isEntityColliding(int entityIndex)
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(i == entityIndex) { continue; }
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
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && player->currentAttackCooldown <= 0)
        {
            Vector2 mousePosition = GetMousePosition();
            RayCastHitResult result = RayCastHit(player->position, mousePosition, ENEMY);
            if(result.colliding)
            {
                int entityIndex = result.entityIndex;
                entities[entityIndex]->health -= player->attackDamage;
                if(entities[entityIndex]->health <= 0)
                {
                    removeEntity(entityIndex);
                }
            }
            player->currentAttackCooldown = player->attackCooldown;
        }
        //printf("x: %f y: %f \n", player->position.x, player->position.y);
    }
}
void UpdateEnemies()
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(entities[i]->entityType != ENEMY) { continue; }
        float minDist = FLT_MAX;
        int minIndex = -1;
        for(int j = 0; j < firstFreeIndex; j++)
        {
            if(entities[j]->entityType != PLAYER) { continue; }
            float dist = getSqrDistance(entities[i]->position, entities[j]->position);
            if(dist < minDist)
            {
                minDist = dist;
                minIndex = j;
            }
        }
        if(minIndex >= 0)
        {
            float speed = entities[i]->speed;
            Vector2 enemyPos = entities[i]->position;
            Vector2 playerPos = entities[minIndex]->position;
            if(enemyPos.x < playerPos.x)
            {
                entities[i]->position.x += min(speed, fabsf(enemyPos.x - playerPos.x));
            }
            if(enemyPos.x > playerPos.x)
            {
                entities[i]->position.x -= min(speed, fabsf(enemyPos.x - playerPos.x));
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

        if(isEntityColliding(i))
        {
            entities[i]->position = entities[i]->previousPosition;
        }
        entities[i]->previousPosition = entities[i]->position;

        entity* player = entities[minIndex];
        if(areEntitiesColliding(player, entities[i]))
        {
            if(entities[i]->currentAttackCooldown > 0)
            {
                entities[i]->currentAttackCooldown -= GetFrameTime();
                entities[i]->defaultColor = DARKPURPLE;
            }
            else
            {

                //printf("Player is inside Enemy!!! ");
                entities[i]->defaultColor = RED;
                player->health -= entities[i]->attackDamage;
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
            entities[i]->defaultColor = DARKPURPLE;
            entities[i]->currentAttackCooldown = entities[i]->attackCooldown;
        }
        // printf("%d:  %f\n", i, entities[i]->currentAttackCooldown);
    }
}

void UpdateWaves()
{
    if(nextWaveTimer > 0)
    {
        nextWaveTimer -= GetFrameTime();
        if(nextWaveTimer <= 0)
        {
            SpawnEnemies();
            currentWave++;
        }
    }
}

void EndWave()
{
    nextWaveTimer = 5.0f;
}

void SpawnEntites()
{
    player = createNewEntity(Vector2(400, 200), Vector2(20, 20), Vector2(0.5f, 0.5f), VIOLET, PLAYER, 2.5f, 128, 2, 2.0f, 100, 10, 0.5f);
    addEntity(player);
    entity* anotherEntity = createNewEntity(Vector2(0,0), Vector2(100, 100), Vector2(0.5f, 0.5f), GOLD, DEFAULT, 0, 0, 0, 0, 0 ,0, 0);
    addEntity(anotherEntity);
}

void SpawnEnemies()
{
    entity* enemy = createNewEntity(Vector2(100, 200), Vector2(64, 64), Vector2(0.5f, 0.5f), DARKPURPLE, ENEMY, 0.5f, 0, 0, 0, 40, 5, 1.0f);
    addEntity(enemy);
    entity* fastEnemy = createNewEntity(Vector2(200, 300), Vector2(16, 16), Vector2(0.5f, 0.5f), DARKPURPLE, ENEMY, 1.5f, 0, 0, 0, 20, 10, 1.0f);
    addEntity(fastEnemy);
}

void ReloadGame()
{
    firstFreeIndex = 0;
    nextWaveTimer = 5.0f;
    currentWave = 0;
    SpawnEntites();
}

int main()
{
    entities = malloc(sizeof(entity*) * capacity);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(800, 450, "GameRunner - Raylib - C");

    SpawnEntites();
    const char* waveText;
    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(SKYBLUE);
            UpdatePlayer();
            UpdateEnemies();
            UpdateWaves();
            for(int i = 0; i < firstFreeIndex; i++)
            {
                Vector2 startPos = GetEntityCorner(entities[i]);
                DrawRectangleV(startPos, entities[i]->size, entities[i]->defaultColor);
            }
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

            DrawFPS(0, 0);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
