#include "raylib.h"
#include <stdlib.h>

typedef struct entity
{
    int xPos;
    int yPos;
    int xSize;
    int ySize;
    Color defaultColor;
    bool isPlayer;
    bool isUpgrader;
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

entity* createNewEntity(int xPos, int yPos, int xSize, int ySize, Color defaultColor, bool isPlayer)
{
    entity* newEntity = malloc(sizeof(entity));
    newEntity->xPos = xPos;
    newEntity->yPos = yPos;
    newEntity->xSize = xSize;
    newEntity->ySize = ySize;
    newEntity->defaultColor = defaultColor;
    newEntity->isPlayer = isPlayer;
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
        if(!entities[i]->isPlayer) { continue; }
        entities[i]->yPos = (entities[i]->yPos + 1) % 450;
    }
}

int main()
{
    entities = malloc(sizeof(entity*) * capacity);
    InitWindow(800, 450, "raylib example - basic window");

    int count = 0;
    entity* testEntity = createNewEntity(400, 200, 20, 20, VIOLET, true);
    addEntity(testEntity);
    entity* anotherEntity = createNewEntity(0,0,100, 100, GOLD, false);
    addEntity(anotherEntity);
    entity* meow = createNewEntity(100, 200, 64, 64, DARKPURPLE, false);
    addEntity(meow);
    while (!WindowShouldClose())
    {
        char text = (char)(count + '0') + '\0';
        // text += (char)(count + '0') + '\0';
        BeginDrawing();
            ClearBackground(SKYBLUE);
            UpdatePlayer();
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