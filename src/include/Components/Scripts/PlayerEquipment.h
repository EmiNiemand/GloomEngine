#ifndef GLOOMENGINE_PLAYEREQUIPMENT_H
#define GLOOMENGINE_PLAYEREQUIPMENT_H


#include "Components/Component.h"
#include "Interfaces/IDataPersistance.h"
#include "MusicPattern.h"
#include <set>

class Instrument;

class PlayerEquipment : public Component {
private:
	inline static int maxCashReward = 5;
	inline static int maxRepReward = 10;

public:
    std::set<std::shared_ptr<Instrument>> instruments;

    int cash = 0;
    int rep = 0;

    PlayerEquipment(const std::shared_ptr<GameObject> &parent, int id);
    ~PlayerEquipment() override;

    void Setup(int startCash=0, int startRep=0);
    bool BuyInstrument(int price, const std::shared_ptr<Instrument>& instrument);
    std::shared_ptr<Instrument> GetInstrumentWithName(InstrumentName name);
    std::set<InstrumentName> GetInstrumentNames();

    void AddReward(float crowdSatisfaction);

    int GetCash() const;
    int GetRep() const;
};


#endif //GLOOMENGINE_PLAYEREQUIPMENT_H
