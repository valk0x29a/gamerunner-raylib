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

typedef struct entity
{
    Vector2 pos;
    Vector2 size;
    Vector2 pivot;
    Color defaultColor;
    uint entityType;
    float speed;
    int dashDistance;
} entity;

int firstFreeIndex = 0;
int capacity = 2;
entity** entities;

Vector2 GetEntityPivot(entity* entity)
{
    // int x = entity->pos.x + entity->pivot.x * entity->size.x;
    // int y = entity->pos.y + entity->pivot.y * entity->size.y;
    Vector2 offset = Vector2Multiply(entity->pivot, entity->size);
    return Vector2Add(entity->pos, offset);
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

entity* createNewEntity(Vector2 pos, Vector2 size, Vector2 pivot, Color defaultColor, uint entityType, float speed, int dashDistance)
{
    entity* newEntity = malloc(sizeof(entity));
    newEntity->pos = pos;
    newEntity->size = size;
    newEntity->pivot = pivot;
    newEntity->defaultColor = defaultColor;
    newEntity->entityType = entityType;
    newEntity->speed = speed;
    newEntity->dashDistance = dashDistance;
}

entity* allocNewEntity(entity copiedEntity)
{
    entity* newEntity = malloc(sizeof(entity));
    newEntity->pos = copiedEntity.pos;
    newEntity->size = copiedEntity.size;
    newEntity->pivot = copiedEntity.pivot;
    newEntity->defaultColor = copiedEntity.defaultColor;
    newEntity->entityType = copiedEntity.entityType;
    newEntity->speed = copiedEntity.speed;
    newEntity->dashDistance = copiedEntity.dashDistance;
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
            player->pos.y -= speed;
            dashYDirection = -1;
        }
        if(IsKeyDown(KEY_S))
        {
            player->pos.y += speed;
            dashYDirection = 1;
        }
        if(IsKeyDown(KEY_A))
        {
            player->pos.x -= speed;
            dashXDirection = -1;
        }
        if(IsKeyDown(KEY_D))
        {
            player->pos.x += speed;
            dashXDirection = 1;
        }
        if(IsKeyPressed(KEY_SPACE))
        {   
            if(dashXDirection == 0 && dashYDirection == 0)
            {
                dashYDirection = 1;
            }
            player->pos.x += dashXDirection * player->dashDistance;
            player->pos.y += dashYDirection * player->dashDistance;
        }

        for(int j = 0; j < firstFreeIndex; j++)
        {
            if(j == i) { continue; }
            if(entities[j]->entityType != ENEMY) { continue; }
            bool isInsideOnXAxis = player->pos.x >= entities[j]->pos.x && player->pos.x <= entities[j]->pos.x + entities[j]->size.x;
            bool isInsideOnYAxis = player->pos.y >= entities[j]->pos.y && player->pos.y <= entities[j]->pos.y + entities[j]->size.y;
            if(isInsideOnXAxis && isInsideOnYAxis) 
            { 
                printf("Player is inside Enemy!!! ");
                entities[j]->defaultColor = RED;
            }
            else
            {
                entities[j]->defaultColor = DARKPURPLE;
            }
        }
        printf("x: %f y: %f \n", player->pos.x, player->pos.y);
    }
}
void UpdateEnemies()
{
    for(int i = 0; i < firstFreeIndex; i++)
    {
        if(entities[i]->entityType != ENEMY) { continue; }
        float minDist = FLT_MAX;
        int minIndex = -1;
        int xPos = entities[i]->pos.x;
        int yPos = entities[i]->pos.y;
        for(int j = 0; j < firstFreeIndex; j++)
        {
            if(entities[j]->entityType != PLAYER) { continue; }
            float dist = (xPos - entities[j]->pos.x)*(xPos - entities[j]->pos.x) + (yPos - entities[j]->pos.y)*(yPos - entities[j]->pos.y);
            if(dist < minDist)
            {
                minDist = dist;
                minIndex = j;
            }
        }
        if(minIndex >= 0)
        {
            float speed = entities[i]->speed;
            Vector2 playerPivot = GetEntityPivot(entities[minIndex]);
            Vector2 enemyPivot = GetEntityPivot(entities[i]);
            if(enemyPivot.x < playerPivot.x)
            {
                entities[i]->pos.x += speed;
            }
            if(enemyPivot.x > playerPivot.x)
            {
                entities[i]->pos.x -= speed;
            }
            if(enemyPivot.y < playerPivot.y)
            {
                entities[i]->pos.y += speed;
            }
            if(enemyPivot.y > playerPivot.y)
            {
                entities[i]->pos.y -= speed;
            }
        }
    }
}

int main()
{
    entities = malloc(sizeof(entity*) * capacity);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(800, 450, "GameRunner - Raylib - C");

    int count = 0;
    entity* testEntity = createNewEntity(Vector2(400, 200), Vector2(20, 20), Vector2(0.5f, 0.5f), VIOLET, PLAYER, 2.5f, 128); 
    addEntity(testEntity);
    entity* anotherEntity = createNewEntity(Vector2(0,0), Vector2(100, 100), Vector2(0.5f, 0.5f), GOLD, DEFAULT, 0, 0);
    addEntity(anotherEntity);
    entity* enemy = createNewEntity(Vector2(100, 200), Vector2(64, 64), Vector2(0.5f, 0.5f), DARKPURPLE, ENEMY, 0.5f, 0);
    addEntity(enemy);
    entity* fastEnemy = createNewEntity(Vector2(200, 300), Vector2(16, 16), Vector2(0.5f, 0.5f), DARKPURPLE, ENEMY, 1.5f, 0);
    addEntity(fastEnemy);
    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(SKYBLUE);
            UpdatePlayer();
            UpdateEnemies();
            for(int i = 0; i < firstFreeIndex; i++)
            {
                DrawRectangleV(entities[i]->pos, entities[i]->size, entities[i]->defaultColor);
            }

            DrawFPS(0, 0);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
