#include "raylib.h"
#include "raymath.h"
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
    int health;
    int maxHealth;
    int attackDamage;
} entity;

int firstFreeIndex = 0;
int capacity = 2;
entity** entities;

Vector2 GetEntityCorner(entity* entity)
{
    Vector2 offset = Vector2Multiply(entity->pivot, entity->size);
    return Vector2Subtract(entity->position, offset);
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
}

entity* createNewEntity(Vector2 pos, Vector2 size, Vector2 pivot, Color defaultColor, uint entityType, float speed, int dashDistance, int maxHealth, int attackDamage)
{
    entity* newEntity = malloc(sizeof(entity));
    newEntity->position = pos;
    newEntity->size = size;
    newEntity->pivot = pivot;
    newEntity->defaultColor = defaultColor;
    newEntity->entityType = entityType;
    newEntity->speed = speed;
    newEntity->dashDistance = dashDistance;
    newEntity->maxHealth = maxHealth;
    newEntity->health = maxHealth;
    newEntity->attackDamage = attackDamage;
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
    newEntity->maxHealth = copiedEntity.maxHealth;
    newEntity->health = copiedEntity.maxHealth;
    newEntity->attackDamage = copiedEntity.attackDamage;
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
        if(IsKeyPressed(KEY_SPACE))
        {   
            if(dashXDirection == 0 && dashYDirection == 0)
            {
                dashYDirection = 1;
            }
            player->position.x += dashXDirection * player->dashDistance;
            player->position.y += dashYDirection * player->dashDistance;
        }

        for(int j = 0; j < firstFreeIndex; j++)
        {
            if(j == i) { continue; }
            if(entities[j]->entityType != ENEMY) { continue; }
            if(areEntitiesColliding(player, entities[j]))
            { 
                printf("Player is inside Enemy!!! ");
                entities[j]->defaultColor = RED;
                player->health -= entities[j]->attackDamage;
                printf("Health: %d ", player->health);
            }
            else
            {
                entities[j]->defaultColor = DARKPURPLE;
            }
        }

        printf("x: %f y: %f \n", player->position.x, player->position.y);
    }
}
void UpdateEnemies()
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(entities[i]->entityType != ENEMY) { continue; }
        float minDist = FLT_MAX;
        int minIndex = -1;
        int xPos = entities[i]->position.x;
        int yPos = entities[i]->position.y;
        for(int j = 0; j < firstFreeIndex; j++)
        {
            if(entities[j]->entityType != PLAYER) { continue; }
            float dist = (xPos - entities[j]->position.x)*(xPos - entities[j]->position.x) + (yPos - entities[j]->position.y)*(yPos - entities[j]->position.y);
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
    }
}

int main()
{
    entities = malloc(sizeof(entity*) * capacity);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(800, 450, "GameRunner - Raylib - C");

    int count = 0;
    entity* testEntity = createNewEntity(Vector2(400, 200), Vector2(20, 20), Vector2(0.5f, 0.5f), VIOLET, PLAYER, 2.5f, 128, 100, 10);
    addEntity(testEntity);
    entity* anotherEntity = createNewEntity(Vector2(0,0), Vector2(100, 100), Vector2(0.5f, 0.5f), GOLD, DEFAULT, 0, 0, 0 ,0);
    addEntity(anotherEntity);
    entity* enemy = createNewEntity(Vector2(100, 200), Vector2(64, 64), Vector2(0.5f, 0.5f), DARKPURPLE, ENEMY, 0.5f, 0, 20, 5);
    addEntity(enemy);
    entity* fastEnemy = createNewEntity(Vector2(200, 300), Vector2(16, 16), Vector2(0.5f, 0.5f), DARKPURPLE, ENEMY, 1.5f, 0, 10, 10);
    addEntity(fastEnemy);
    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(SKYBLUE);
            UpdatePlayer();
            UpdateEnemies();
            for(int i = 0; i < firstFreeIndex; i++)
            {
                Vector2 offset = Vector2Multiply(entities[i]->pivot, entities[i]->size);
                Vector2 startPos = Vector2Subtract(entities[i]->position, offset);
                DrawRectangleV(startPos, entities[i]->size, entities[i]->defaultColor);
            }

            DrawFPS(0, 0);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
