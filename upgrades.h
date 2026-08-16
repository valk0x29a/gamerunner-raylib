#include <limits.h>
#include "entity.h"

// NOLINTBEGIN(misc-definitions-in-headers)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wwritable-strings"

extern entity* equippedWeapon;
extern entity* handgun; // always at first slot
extern entity* inventorySecondSlot;
//available weapons
extern entity* sharpener;
extern entity* shotgun;
extern entity* sniperGun;

extern entity* equippedGrenade;
extern entity* decoy;
extern entity* freezingGrenade;
extern entity* explosiveGrenade;

extern entity playerTemplate;

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
    return 0;
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
    return 0;
}

int* GetCurrentUpgrade(int upgradeIndex)
{
    switch(upgradeIndex)
    {
        case 0: return &currentHandgunUpgrade;
        case 1: return &currentDashUpgrade;
        case 2: return &currentMaxHealthUpgrade;
    };
    return 0;
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
    playerTemplate.dashCooldown = dashUpgrades[currentDashUpgrade].dashCooldown;
    playerTemplate.numberOfDashes = dashUpgrades[currentDashUpgrade].numberOfDashes;
    playerTemplate.health = maxHealthUpgrades[currentMaxHealthUpgrade];
    playerTemplate.attackCooldown = handgunUpgrades[currentHandgunUpgrade].attackCooldown;
    playerTemplate.attackDamage = handgunUpgrades[currentHandgunUpgrade].attackDamage;
}


//NOLINTEND(misc-definitions-in-headers)

