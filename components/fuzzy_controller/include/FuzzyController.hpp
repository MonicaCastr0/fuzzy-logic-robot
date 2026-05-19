#pragma once

struct FuzzyInput {
    float frontDistanceCm;
};

struct FuzzyOutput {
    int motorASpeed;
    int motorBSpeed;
};

class FuzzyController {
public:
    FuzzyController();

    void init();
    FuzzyOutput evaluate(const FuzzyInput& input);
};