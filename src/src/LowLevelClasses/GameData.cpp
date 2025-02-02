//
// Created by szymo on 24/03/2023.
//

#include "LowLevelClasses/GameData.h"

GameData::GameData() {
    money = 0;
    playerPosition = glm::vec3(1.5, 0.5, -1.5);
    shopkeeperEvent = false;
    tutorial = false;
    saveDate = "";
    for (int type = (int)PlayerBadges::DRUMS; type <= (int)PlayerBadges::GUITAR; ++type) {
        badges.insert({static_cast<PlayerBadges>(type), false});
    }
}
