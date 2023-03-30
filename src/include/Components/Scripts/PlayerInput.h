//
// Created by masterktos on 30.03.23.
//

#ifndef GLOOMENGINE_PLAYERINPUT_H
#define GLOOMENGINE_PLAYERINPUT_H

#include <unordered_map>
#include "ProjectSettings.h"

/**@attention
 * Default value of key is zero. For 1D or 2D inputs (e.g. Move)
 * inputs have increasing value, starting from up and going clockwise<br/>
 * ..N..........0..     <br/>
 * W...E......3...1     <br/>
 * ..S..........2..     <br/>
 */
class PlayerInput
{
public:
    inline static const std::unordered_map<Key, bool> StartSession = {
            {Key::KEY_SPACE, 0}
    };
    /// UP=0, RIGHT=1, DOWN=2, LEFT=3
    inline static std::unordered_map<Key, int> Move = {
            { Key::KEY_W, 0}, { Key::KEY_ARROW_UP, 0},
            { Key::KEY_D, 1}, { Key::KEY_ARROW_RIGHT, 1},
            { Key::KEY_S, 2}, { Key::KEY_ARROW_DOWN, 2},
            { Key::KEY_A, 3}, { Key::KEY_ARROW_LEFT, 3}
    };
    inline static std::unordered_map<Key, int> Interact = {
            {Key::KEY_E, 0}
    };
};

#endif //GLOOMENGINE_PLAYERINPUT_H
