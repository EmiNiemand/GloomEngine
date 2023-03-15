//
// Created by szymo on 13/03/2023.
//

#ifndef OPENGLGP_ENGINEHID_H
#define OPENGLGP_ENGINEHID_H

#include "include/ProjectSettings.h"
#include "GLFW/glfw3.h"

#include <vector>
#include <memory>

class GloomEngine;

class EngineHID {
public:
    std::shared_ptr<GloomEngine> gloomEngine;
    std::vector<Key> keysDown;
    std::vector<Key> keysUp;
    std::vector<Key> keysPressed;
    static std::vector<Key> keysDownBuffer;
    static std::vector<Key> keysUpBuffer;
    static std::vector<Key> keysPressedBuffer;
public:
    explicit EngineHID(const std::shared_ptr<GloomEngine> &gloomEngine);

    /// Should be called at the end of engine update function
    void Update();

    bool IsKeyDown(Key key);
    bool IsKeyPressed(Key key);
    bool IsKeyUp(Key key);

    static void KeyActionCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
};


#endif //OPENGLGP_ENGINEHID_H
