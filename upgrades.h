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

extern entity* exploder;

extern entity playerTemplate;
extern entity sharpenerTemplate;
extern entity shotgunTemplate;

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
int dashUpgradesCount = 3;
int dashUpgradesPrices[2];
typedef struct dashUpgrade
{
    int numberOfDashes;
    float dashCooldown;
} dashUpgrade;
dashUpgrade dashUpgrades[3];

int currentMaxHealthUpgrade = 0;
int maxHealthUpgradesCount = 2;
int maxHealthUpgradesPrices[1];
int maxHealthUpgrades[2];

int currentSharpenerMagazineUpgrade = 0;
int sharpenerMagazineUpgradesCount = 5;
int sharpenerMagazineUpgradesPrices[4];
int sharpenerMagazineUpgrades[5];

int currentSharpenerDamageUpgrade = 0;
int sharpenerDamageUpgradesCount = 5;
int sharpenerDamageUpgradesPrices[4];
int sharpenerDamageUpgrades[5];

int currentShotgunMagazineUpgrade = 0;
int shotgunMagazineUpgradesCount = 5;
int shotgunMagazineUpgradesPrices[4];
int shotgunMagazineUpgrades[5];

int currentShotgunCooldownUpgrade = 0;
int shotgunCooldownUpgradesCount = 5;
int shotgunCooldownUpgradesPrices[4];
float shotgunCooldownUpgrades[5];

int currentShotgunBulletCountUpgrade = 0;
int shotgunBulletCountUpgradesCount = 5;
int shotgunBulletCountUpgradesPrices[4];
float shotgunBulletCountUpgrades[5];

// typedef struct shotgunUpgrades
// {
//     int currentMagazine;
//     int magazineCount = 5;
//     int magazinePrices[4];
//     int magazine[5];
//
//     int currentCooldown;
//     int cooldownCount = 5;
//     int cooldownPrices[4];
//     float cooldown[5];
//
//     int currentPelletsAmount;
//     int pelletsAmountCount = 5;
//     int pelletsAmountPrices[4];
//     int pelletsAmount[5];
// } shotgunUpgrades;

int currentExploderUpgrade = 0;
int exploderUpgradesCount = 2;
int exploderUpgradesPrices[1];
typedef struct exploderUpgrade
{
    int attackDamage;
    float attackCooldown;
    float attackRange;
} exploderUpgrade;
exploderUpgrade exploderUpgrades[2];

int exploderPrice = 100;
bool isExploderUnlocked;

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
        case 6: return "Exploder";
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
        case 6: return exploderPrice;
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
        case 6: return &isExploderUnlocked;
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
        case 6: return exploder;
    };
    return 0;
}

const char* GetUpgradeButtonText(int upgradeIndex)
{
    switch(upgradeIndex)
    {
        case 0: return "Upgrade Handgun";
        case 1: return "Upgrade Dash";
        case 2: return "Upgrade Max Health";
        case 3: return "Upgrade Magazine"; // Sharpener
        case 4: return "Upgrade Damage";
        case 5: return "Upgrade Magazine"; // Shotgun
        case 6: return "Upgrade Cooldown";
        case 7: return "Upgrade Pellets Count";
        case 8: return "Upgrade Exploder";
    };
    return "Invalid Upgrade Index";
}

int* GetCurrentUpgrade(int upgradeIndex)
{
    switch(upgradeIndex)
    {
        case 0: return &currentHandgunUpgrade;
        case 1: return &currentDashUpgrade;
        case 2: return &currentMaxHealthUpgrade;
        case 3: return &currentSharpenerMagazineUpgrade;
        case 4: return &currentSharpenerDamageUpgrade;
        case 5: return &currentShotgunMagazineUpgrade;
        case 6: return &currentShotgunCooldownUpgrade;
        case 7: return &currentShotgunBulletCountUpgrade;
        case 8: return &currentExploderUpgrade;
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
        case 3: return sharpenerMagazineUpgradesCount;
        case 4: return sharpenerDamageUpgradesCount;
        case 5: return shotgunMagazineUpgradesCount;
        case 6: return shotgunCooldownUpgradesCount;
        case 7: return shotgunBulletCountUpgradesCount;
        case 8: return exploderUpgradesCount;
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
        case 3: return sharpenerMagazineUpgradesPrices[currentUpgrade];
        case 4: return sharpenerDamageUpgradesPrices[currentUpgrade];
        case 5: return shotgunMagazineUpgradesPrices[currentUpgrade];
        case 6: return shotgunCooldownUpgradesPrices[currentUpgrade];
        case 7: return shotgunBulletCountUpgradesPrices[currentUpgrade];
        case 8: return exploderUpgradesPrices[currentUpgrade];
    };
    return INT_MAX;
}

void PrepareUpgrades()
{
    handgunUpgrades[0].attackCooldown = 0.5f;
    handgunUpgrades[0].attackDamage = 10;

    handgunUpgrades[1].attackCooldown = 0.25f;
    handgunUpgrades[1].attackDamage = 12;
    handgunUpgradesPrices[0] = 100;

    dashUpgrades[0].dashCooldown = 2.0f;
    dashUpgrades[0].numberOfDashes = 1;

    dashUpgrades[1].dashCooldown = 1.5f;
    dashUpgrades[1].numberOfDashes = 2;
    dashUpgradesPrices[0] = 100;

    dashUpgrades[2].dashCooldown = 0.75f;
    dashUpgrades[2].numberOfDashes = 3;
    dashUpgradesPrices[1] = 200;

    maxHealthUpgrades[0] = 100;
    maxHealthUpgrades[1] = 150;
    maxHealthUpgradesPrices[0] = 100;

    sharpenerMagazineUpgrades[0] = 20;
    sharpenerMagazineUpgrades[1] = 22;
    sharpenerMagazineUpgrades[2] = 24;
    sharpenerMagazineUpgrades[3] = 26;
    sharpenerMagazineUpgrades[4] = 28;
    sharpenerMagazineUpgradesPrices[0] = 100;
    sharpenerMagazineUpgradesPrices[1] = 200;
    sharpenerMagazineUpgradesPrices[2] = 300;
    sharpenerMagazineUpgradesPrices[3] = 400;

    sharpenerDamageUpgrades[0] = 5;
    sharpenerDamageUpgrades[1] = 6;
    sharpenerDamageUpgrades[2] = 8;
    sharpenerDamageUpgrades[3] = 10;
    sharpenerDamageUpgrades[4] = 12;
    sharpenerDamageUpgradesPrices[0] = 100;
    sharpenerDamageUpgradesPrices[1] = 200;
    sharpenerDamageUpgradesPrices[2] = 300;
    sharpenerDamageUpgradesPrices[3] = 400;

    shotgunMagazineUpgrades[0] = 3;
    shotgunMagazineUpgrades[1] = 4;
    shotgunMagazineUpgrades[2] = 5;
    shotgunMagazineUpgrades[3] = 6;
    shotgunMagazineUpgrades[4] = 7;
    shotgunMagazineUpgradesPrices[0] = 100;
    shotgunMagazineUpgradesPrices[1] = 200;
    shotgunMagazineUpgradesPrices[2] = 300;
    shotgunMagazineUpgradesPrices[3] = 400;

    shotgunCooldownUpgrades[0] = 2.0f;
    shotgunCooldownUpgrades[1] = 1.0f;
    shotgunCooldownUpgrades[2] = 0.75f;
    shotgunCooldownUpgrades[3] = 0.6f;
    shotgunCooldownUpgrades[4] = 0.5f;
    shotgunCooldownUpgradesPrices[0] = 100;
    shotgunCooldownUpgradesPrices[1] = 200;
    shotgunCooldownUpgradesPrices[2] = 300;
    shotgunCooldownUpgradesPrices[3] = 400;

    shotgunBulletCountUpgrades[0] = 2;
    shotgunBulletCountUpgrades[1] = 3;
    shotgunBulletCountUpgrades[2] = 4;
    shotgunBulletCountUpgrades[3] = 5;
    shotgunBulletCountUpgrades[4] = 6;
    shotgunBulletCountUpgradesPrices[0] = 100;
    shotgunBulletCountUpgradesPrices[1] = 200;
    shotgunBulletCountUpgradesPrices[2] = 300;
    shotgunBulletCountUpgradesPrices[3] = 400;

    exploderUpgrades[0].attackCooldown = 4.0f;
    exploderUpgrades[0].attackDamage = 16;
    exploderUpgrades[0].attackRange = 100.0f;

    exploderUpgrades[1].attackCooldown = 2.0f;
    exploderUpgrades[1].attackDamage = 16;
    exploderUpgrades[1].attackRange = 200.0f;
    exploderUpgradesPrices[0] = 100;
}

void SetUpgrades()
{
    playerTemplate.dashCooldown = dashUpgrades[currentDashUpgrade].dashCooldown;
    playerTemplate.numberOfDashes = dashUpgrades[currentDashUpgrade].numberOfDashes;
    playerTemplate.health = maxHealthUpgrades[currentMaxHealthUpgrade];
    handgun->attackCooldown = handgunUpgrades[currentHandgunUpgrade].attackCooldown;
    handgun->attackDamage = handgunUpgrades[currentHandgunUpgrade].attackDamage;
    sharpenerTemplate.ammoCount = sharpenerMagazineUpgrades[currentSharpenerMagazineUpgrade];
    sharpener->attackDamage = sharpenerDamageUpgrades[currentSharpenerDamageUpgrade];
    shotgunTemplate.ammoCount = shotgunMagazineUpgrades[currentShotgunMagazineUpgrade];
    shotgun->attackCooldown = shotgunCooldownUpgrades[currentShotgunCooldownUpgrade];
    shotgun->count = shotgunBulletCountUpgrades[currentShotgunBulletCountUpgrade];
    exploder->attackCooldown = exploderUpgrades[currentExploderUpgrade].attackCooldown;
    exploder->attackDamage = exploderUpgrades[currentExploderUpgrade].attackDamage;
    exploder->attackRange = exploderUpgrades[currentExploderUpgrade].attackRange;
}


//NOLINTEND(misc-definitions-in-headers)

