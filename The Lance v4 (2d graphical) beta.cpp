#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

class Character {
public:
    string name;
    int health;
    int attackPower;
    int defense;

    Character(string n, int h, int a, int d) : name(n), health(h), attackPower(a), defense(d) {}

    void attack(Character &target) {
        int damage = attackPower - target.defense;
        if (damage < 0) damage = 0;
        target.health -= damage;
        cout << name << " attacks " << target.name << " for " << damage << " damage!" << endl;
    }

    bool isAlive() {
        return health > 0;
    }
};

class Game {
public:
    vector<Character> characters;

    void addCharacter(Character c) {
        characters.push_back(c);
    }

    void startBattle() {
        srand(time(0));
        while (characters.size() > 1) {
            int attackerIndex = rand() % characters.size();
            int targetIndex = rand() % characters.size();
            if (attackerIndex != targetIndex) {
                characters[attackerIndex].attack(characters[targetIndex]);
                if (!characters[targetIndex].isAlive()) {
                    cout << characters[targetIndex].name << " has been defeated!" << endl;
                    characters.erase(characters.begin() + targetIndex);
                }
            }
        }
        cout << characters[0].name << " is the winner!" << endl;
    }
};

int main() {
    Game game;
    game.addCharacter(Character("Knight", 100, 20, 10));
    game.addCharacter(Character("Dragon", 150, 25, 5));
    game.addCharacter(Character("Wizard", 80, 30, 5));
    game.startBattle();
    return 0;
}
