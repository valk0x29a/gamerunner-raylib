#include "raylib.h"
#include <stdlib.h>
#include <float.h>

#define DEFAULT 0
#define PLAYER 1 << 1
#define UPGRADER 1 << 2
#define ENEMY 1 << 3

typedef struct entity
{
    int xPos;
    int yPos;
    int xSize;
    int ySize;
    Color defaultColor;
    uint entityType;
    int speed;
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

entity* createNewEntity(int xPos, int yPos, int xSize, int ySize, Color defaultColor, uint entityType, int speed, int dashDistance)
{
    entity* newEntity = malloc(sizeof(entity));
    newEntity->xPos = xPos;
    newEntity->yPos = yPos;
    newEntity->xSize = xSize;
    newEntity->ySize = ySize;
    newEntity->defaultColor = defaultColor;
    newEntity->entityType = entityType;
    newEntity->speed = speed;
    newEntity->dashDistance = dashDistance;
}

entity* allocNewEntity(entity copiedEntity)
{
    entity* newEntity = malloc(sizeof(entity));
    newEntity->xPos = copiedEntity.xPos;
    newEntity->yPos = copiedEntity.yPos;
    newEntity->xSize = copiedEntity.xSize;
    newEntity->ySize = copiedEntity.ySize;
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
            player->yPos -= speed;
            dashYDirection = -1;
        }
        if(IsKeyDown(KEY_S))
        {
            player->yPos += speed;
            dashYDirection = 1;
        }
        if(IsKeyDown(KEY_A))
        {
            player->xPos -= speed;
            dashXDirection = -1;
        }
        if(IsKeyDown(KEY_D))
        {
            player->xPos += speed;
            dashXDirection = 1;
        }
        if(IsKeyPressed(KEY_SPACE))
        {   
            if(dashXDirection == 0 && dashYDirection == 0)
            {
                dashYDirection = 1;
            }
            player->xPos += dashXDirection * player->dashDistance;
            player->yPos += dashYDirection * player->dashDistance;
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
        int xPos = entities[i]->xPos;
        int yPos = entities[i]->yPos;
        for(int j = 0; j < firstFreeIndex; j++)
        {
            if(entities[j]->entityType != PLAYER) { continue; }
            float dist = (xPos - entities[j]->xPos)*(xPos - entities[j]->xPos) + (yPos - entities[j]->yPos)*(yPos - entities[j]->yPos);
            if(dist < minDist)
            {
                minDist = dist;
                minIndex = j;
            }
        }
        if(minIndex >= 0)
        {
            int speed = entities[i]->speed;
            if(xPos < entities[minIndex]->xPos)
            {
                entities[i]->xPos += speed;
            }
            if(xPos > entities[minIndex]->xPos)
            {
                entities[i]->xPos -= speed;
            }
            if(yPos < entities[minIndex]->yPos)
            {
                entities[i]->yPos += speed;
            }
            if(yPos > entities[minIndex]->yPos)
            {
                entities[i]->yPos -= speed;
            }
        }
    }
}

int main()
{
    entities = malloc(sizeof(entity*) * capacity);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(800, 450, "raylib example - basic window");

    int count = 0;
    entity* testEntity = createNewEntity(400, 200, 20, 20, VIOLET, PLAYER, 2, 128);
    addEntity(testEntity);
    entity* anotherEntity = createNewEntity(0,0,100, 100, GOLD, DEFAULT, 0, 0);
    addEntity(anotherEntity);
    entity* meow = createNewEntity(100, 200, 64, 64, DARKPURPLE, ENEMY, 1, 0);
    addEntity(meow);
    while (!WindowShouldClose())
    {
        char text = (char)(count + '0') + '\0';
        // text += (char)(count + '0') + '\0';
        BeginDrawing();
            ClearBackground(SKYBLUE);
            UpdatePlayer();
            UpdateEnemies();
            for(int i = 0; i < firstFreeIndex; i++)
            {
                DrawRectangle(entities[i]->xPos, entities[i]->yPos, entities[i]->xSize, entities[i]->ySize, entities[i]->defaultColor);
            }

            DrawFPS(0, 0);
        EndDrawing();
        count++;
    }

    CloseWindow();

    return 0;
}