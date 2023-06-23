#ifndef GLOOMENGINE_DRUMSSESSIONUI_H
#define GLOOMENGINE_DRUMSSESSIONUI_H

#include "SessionUI.h"

class DrumsSessionUI : public SessionUI {
public:
    DrumsSessionUI(const std::shared_ptr<GameObject> &parent, int id);

    void PlaySound(int index) override;

    void Setup(std::shared_ptr<Instrument> instrument, bool sessionMetronomeSound,
               bool sessionMetronomeVisuals, bool sessionBackingTrack) override;
};

#endif //GLOOMENGINE_DRUMSSESSIONUI_H