#include "raylib.h"
#include <stdlib.h>
#include <float.h>

#define DEFAULT 0
#define PLAYER 1 << 1
#define UPGRADER 1 << 2
#define ENEMY 1 << 3

typedef struct entity
{
    Vector2 pos;
    Vector2 size;
    Color defaultColor;
    uint entityType;
    float speed;
    int dashDistance;
} entity;

int firstFreeIndex = 0;
int capacity = 2;
entity** entities;

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

entity* createNewEntity(int xPos, int yPos, int xSize, int ySize, Color defaultColor, uint entityType, float speed, int dashDistance)
{
    entity* newEntity = malloc(sizeof(entity));
    newEntity->pos.x = xPos;
    newEntity->pos.y = yPos;
    newEntity->size.x = xSize;
    newEntity->size.y = ySize;
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
    newEntity->defaultColor = copiedEntity.defaultColor;
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
            if(xPos < entities[minIndex]->pos.x)
            {
                entities[i]->pos.x += speed;
            }
            if(xPos > entities[minIndex]->pos.x)
            {
                entities[i]->pos.x -= speed;
            }
            if(yPos < entities[minIndex]->pos.y)
            {
                entities[i]->pos.y += speed;
            }
            if(yPos > entities[minIndex]->pos.y)
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
    entity* testEntity = createNewEntity(400, 200, 20, 20, VIOLET, PLAYER, 2.5f, 128); 
    addEntity(testEntity);
    entity* anotherEntity = createNewEntity(0,0,100, 100, GOLD, DEFAULT, 0, 0);
    addEntity(anotherEntity);
    entity* meow = createNewEntity(100, 200, 64, 64, DARKPURPLE, ENEMY, 0.5f, 0);
    addEntity(meow);
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