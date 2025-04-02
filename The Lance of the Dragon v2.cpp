Rpg. The Lance of the Dragon, the coin and death. (visual,/Graphical) 2D     

# current 2D Diablo-Style RPG game with D&D Rules & Dragonlance Setting

 transform your text-based framework into a fully functional 2D game with Diablo 1-style gameplay, D&D first edition rules, and Dragonlance lore, implement a more robust In-Game Architecture, paper screen inventory & mapOverview

```cpp
// main.cpp - Core game implementation
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <random>
#include <cmath>
#include <fstream>
#include <sstream>

// Constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int TILE_SIZE = 32;
const float PLAYER_SPEED = 150.0f;
const std::string GAME_TITLE = "Dragonlance: Chronicles of the Lance";

// Forward declarations
class Entity;
class Character;
class Player;
class Enemy;
class Item;
class Weapon;
class Armor;
class Spell;
class Projectile;
class World;
class Dungeon;
class GameState;
class ResourceManager;
class QuestManager;
class UIManager;
class SoundManager;

// Utility functions
namespace GameUtils {
    // Random number generator
    std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
   
    int rollDice(int sides) {
        std::uniform_int_distribution<int> dist(1, sides);
        return dist(rng);
    }
   
    int rollDice(int num, int sides) {
        int result = 0;
        for (int i = 0; i < num; i++) {
            result += rollDice(sides);
        }
        return result;
    }
   
    int getRandomInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng);
    }
   
    float getRandomFloat(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng);
    }
   
    // Calculate distance between two points
    float distance(float x1, float y1, float x2, float y2) {
        return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
    }
}

// Resource Manager
class ResourceManager {
private:
    std::map<std::string, sf::Texture> textures;
    std::map<std::string, sf::Font> fonts;
    std::map<std::string, sf::SoundBuffer> soundBuffers;
   
public:
    ResourceManager() {
        // Load default resources
        loadTexture("player", "assets/sprites/player.png");
        loadTexture("skeleton", "assets/sprites/skeleton.png");
        loadTexture("goblin", "assets/sprites/goblin.png");
        loadTexture("dragon", "assets/sprites/dragon.png");
        loadTexture("floor", "assets/tiles/floor.png");
        loadTexture("wall", "assets/tiles/wall.png");
        loadTexture("door", "assets/tiles/door.png");
        loadTexture("chest", "assets/tiles/chest.png");
        loadTexture("fireball", "assets/spells/fireball.png");
        loadTexture("healing", "assets/spells/healing.png");
        loadTexture("items", "assets/sprites/items.png");
        loadTexture("ui", "assets/ui/ui_elements.png");
       
        loadFont("main", "assets/fonts/main.ttf");
       
        loadSoundBuffer("attack", "assets/sounds/attack.wav");
        loadSoundBuffer("spell", "assets/sounds/spell.wav");
        loadSoundBuffer("hurt", "assets/sounds/hurt.wav");
        loadSoundBuffer("death", "assets/sounds/death.wav");
        loadSoundBuffer("item", "assets/sounds/item.wav");
        loadSoundBuffer("level_up", "assets/sounds/level_up.wav");
    }
   
    bool loadTexture(const std::string& id, const std::string& filepath) {
        sf::Texture texture;
        if (!texture.loadFromFile(filepath)) {
            std::cerr << "Failed to load texture: " << filepath << std::endl;
            // Use fallback texture instead of returning false
            sf::Image fallback;
            fallback.create(32, 32, sf::Color::Magenta);
            texture.loadFromImage(fallback);
        }
        textures[id] = texture;
        return true;
    }
   
    bool loadFont(const std::string& id, const std::string& filepath) {
        sf::Font font;
        if (!font.loadFromFile(filepath)) {
            std::cerr << "Failed to load font: " << filepath << std::endl;
            return false;
        }
        fonts[id] = font;
        return true;
    }
   
    bool loadSoundBuffer(const std::string& id, const std::string& filepath) {
        sf::SoundBuffer buffer;
        if (!buffer.loadFromFile(filepath)) {
            std::cerr << "Failed to load sound: " << filepath << std::endl;
            return false;
        }
        soundBuffers[id] = buffer;
        return true;
    }
   
    sf::Texture& getTexture(const std::string& id) {
        return textures[id];
    }
   
    sf::Font& getFont(const std::string& id) {
        return fonts[id];
    }
   
    sf::SoundBuffer& getSoundBuffer(const std::string& id) {
        return soundBuffers[id];
    }
};

// Sound Manager
class SoundManager {
private:
    ResourceManager& resources;
    std::vector<sf::Sound> sounds;
    sf::Music backgroundMusic;
    float volume;
   
public:
    SoundManager(ResourceManager& resources) : resources(resources), volume(100.0f) {
        // Preload background music
        if (!backgroundMusic.openFromFile("assets/music/main_theme.ogg")) {
            std::cerr << "Failed to load background music" << std::endl;
        }
        backgroundMusic.setLoop(true);
        backgroundMusic.setVolume(volume * 0.5f);
    }
   
    void playSound(const std::string& id) {
        // Find an available sound slot or create a new one
        sf::Sound* sound = nullptr;
        for (auto& s : sounds) {
            if (s.getStatus() == sf::Sound::Stopped) {
                sound = &s;
                break;
            }
        }
       
        if (!sound) {
            sounds.emplace_back();
            sound = &sounds.back();
        }
       
        sound->setBuffer(resources.getSoundBuffer(id));
        sound->setVolume(volume);
        sound->play();
    }
   
    void playMusic() {
        backgroundMusic.play();
    }
   
    void stopMusic() {
        backgroundMusic.stop();
    }
   
    void setVolume(float newVolume) {
        volume = std::max(0.0f, std::min(100.0f, newVolume));
        backgroundMusic.setVolume(volume * 0.5f);
        for (auto& sound : sounds) {
            sound.setVolume(volume);
        }
    }
};

// Entity class - base for all game objects
class Entity {
protected:
    sf::Sprite sprite;
    sf::Vector2f position;
    bool active;
    std::string name;
    std::string type;
   
public:
    Entity(const std::string& name, const std::string& type, ResourceManager& resources, const std::string& textureId)
        : name(name), type(type), active(true) {
        sprite.setTexture(resources.getTexture(textureId));
        sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
    }
   
    virtual ~Entity() = default;
   
    virtual void update(float deltaTime) {
        sprite.setPosition(position);
    }
   
    virtual void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }
   
    void setPosition(float x, float y) {
        position.x = x;
        position.y = y;
        sprite.setPosition(position);
    }
   
    sf::Vector2f getPosition() const {
        return position;
    }
   
    std::string getName() const {
        return name;
    }
   
    std::string getType() const {
        return type;
    }
   
    bool isActive() const {
        return active;
    }
   
    void setActive(bool value) {
        active = value;
    }
   
    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }
   
    bool intersects(const Entity& other) const {
        return getBounds().intersects(other.getBounds());
    }
   
    float distanceTo(const Entity& other) const {
        return GameUtils::distance(position.x, position.y,
                                 other.position.x, other.position.y);
    }
};

// Character class - base for player and enemies
class Character : public Entity {
protected:
    // D&D stats
    int strength;
    int dexterity;
    int constitution;
    int intelligence;
    int wisdom;
    int charisma;
   
    // Derived stats
    int health;
    int maxHealth;
    int mana;
    int maxMana;
    int armorClass;
    int attackBonus;
    int level;
   
    // Equipment
    std::shared_ptr<Weapon> equippedWeapon;
    std::shared_ptr<Armor> equippedArmor;
   
    // Animation
    int animationFrame;
    float animationTimer;
    std::string currentAnimation;
    bool facingRight;
   
    // Visual effects
    float damageFlashTimer;
   
    // Resources
    ResourceManager& resources;
    SoundManager& sounds;
   
public:
    Character(const std::string& name, const std::string& type,
              ResourceManager& resources, SoundManager& sounds,
              const std::string& textureId, int strength, int dexterity,
              int constitution, int intelligence, int wisdom, int charisma)
        : Entity(name, type, resources, textureId),
          resources(resources), sounds(sounds),
          strength(strength), dexterity(dexterity), constitution(constitution),
          intelligence(intelligence), wisdom(wisdom), charisma(charisma),
          animationFrame(0), animationTimer(0), damageFlashTimer(0),
          facingRight(true), level(1) {
       
        // Calculate derived stats
        maxHealth = 10 + constitution + GameUtils::rollDice(1, 8);
        health = maxHealth;
        maxMana = 10 + intelligence;
        mana = maxMana;
        armorClass = 10 + (dexterity - 10) / 2; // AC = 10 + DEX modifier
        attackBonus = (strength - 10) / 2; // Attack bonus = STR modifier
    }
   
    virtual void update(float deltaTime) override {
        Entity::update(deltaTime);
       
        // Update animation
        animationTimer += deltaTime;
        if (animationTimer >= 0.15f) {  // Animation speed
            animationTimer = 0;
            animationFrame = (animationFrame + 1) % 4;  // 4 frames per animation
            updateAnimation();
        }
       
        // Update damage flash effect
        if (damageFlashTimer > 0) {
            damageFlashTimer -= deltaTime;
            if (damageFlashTimer <= 0) {
                sprite.setColor(sf::Color::White);
            } else {
                // Pulsing red effect
                int alpha = static_cast<int>(255 * (0.5f + 0.5f * std::sin(damageFlashTimer * 30)));
                sprite.setColor(sf::Color(255, 100, 100, 255 - alpha));
            }
        }
    }
   
    virtual void updateAnimation() {
        // Base animation frame setup
        int frameWidth = sprite.getTexture()->getSize().x / 4;  // 4 frames horizontally
        int frameHeight = sprite.getTexture()->getSize().y / 4; // 4 animations vertically
       
        int row = 0; // Default animation row (idle)
        if (currentAnimation == "walk") row = 1;
        else if (currentAnimation == "attack") row = 2;
        else if (currentAnimation == "hurt") row = 3;
       
        // Set the texture rect based on animation frame and row
        sprite.setTextureRect(sf::IntRect(
            animationFrame * frameWidth,
            row * frameHeight,
            frameWidth,
            frameHeight
        ));
       
        // Flip sprite based on facing direction
        if (facingRight) {
            sprite.setScale(1.0f, 1.0f);
        } else {
            sprite.setScale(-1.0f, 1.0f);
        }
    }
   
    void setAnimation(const std::string& animation) {
        if (currentAnimation != animation) {
            currentAnimation = animation;
            animationFrame = 0;
            animationTimer = 0;
            updateAnimation();
        }
    }
   
    void setFacingDirection(bool right) {
        facingRight = right;
    }
   
    int rollAttack() const {
        int roll = GameUtils::rollDice(20);
        return roll + attackBonus + (equippedWeapon ? equippedWeapon->getAttackBonus() : 0);
    }
   
    int rollDamage() const {
        if (equippedWeapon) {
            return equippedWeapon->rollDamage() + std::max(0, (strength - 10) / 2);
        }
        return GameUtils::rollDice(1, 4) + std::max(0, (strength - 10) / 2); // Unarmed damage
    }
   
    void takeDamage(int amount) {
        if (amount <= 0) return;
       
        health -= amount;
        if (health < 0) health = 0;
       
        // Visual and audio feedback
        damageFlashTimer = 0.5f;
        sounds.playSound("hurt");
        setAnimation("hurt");
       
        if (health <= 0) {
            sounds.playSound("death");
            setActive(false);
        }
    }
   
    void heal(int amount) {
        if (amount <= 0) return;
       
        health += amount;
        if (health > maxHealth) health = maxHealth;
    }
   
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    int getMana() const { return mana; }
    int getMaxMana() const { return maxMana; }
    int getStrength() const { return strength; }
    int getDexterity() const { return dexterity; }
    int getConstitution() const { return constitution; }
    int getIntelligence() const { return intelligence; }
    int getWisdom() const { return wisdom; }
    int getCharisma() const { return charisma; }
    int getArmorClass() const {
        int ac = 10 + (dexterity - 10) / 2;
        if (equippedArmor) ac += equippedArmor->getDefense();
        return ac;
    }
    int getLevel() const { return level; }
   
    bool isAlive() const { return health > 0; }
   
    // Attack another character
    bool attack(Character& target) {
        setAnimation("attack");
        sounds.playSound("attack");
       
        int attackRoll = rollAttack();
        if (attackRoll >= target.getArmorClass()) {
            int damage = rollDamage();
            target.takeDamage(damage);
            return true;
        }
        return false;
    }
   
    // Equip weapon
    void equipWeapon(std::shared_ptr<Weapon> weapon) {
        equippedWeapon = weapon;
    }
   
    // Equip armor
    void equipArmor(std::shared_ptr<Armor> armor) {
        equippedArmor = armor;
    }
};

// Player class
class Player : public Character {
private:
    int experience;
    int gold;
    std::vector<std::shared_ptr<Item>> inventory;
    std::vector<std::shared_ptr<Spell>> spells;
    std::map<std::string, bool> questFlags;
   
    // Movement
    float moveSpeed;
    sf::Vector2f velocity;
   
    // Camera
    sf::View& gameView;
   
    // UI
    sf::RectangleShape healthBar;
    sf::RectangleShape manaBar;
    sf::Text nameText;
   
public:
    Player(const std::string& name, ResourceManager& resources, SoundManager& sounds, sf::View& gameView,
           int strength = 12, int dexterity = 12, int constitution = 12,
           int intelligence = 12, int wisdom = 12, int charisma = 12)
        : Character(name, "player", resources, sounds, "player",
                   strength, dexterity, constitution, intelligence, wisdom, charisma),
          experience(0), gold(50), moveSpeed(PLAYER_SPEED), gameView(gameView) {
       
        // Initialize UI elements
        healthBar.setSize(sf::Vector2f(50, 6));
        healthBar.setFillColor(sf::Color::Red);
        healthBar.setOutlineColor(sf::Color::Black);
        healthBar.setOutlineThickness(1);
       
        manaBar.setSize(sf::Vector2f(50, 4));
        manaBar.setFillColor(sf::Color::Blue);
        manaBar.setOutlineColor(sf::Color::Black);
        manaBar.setOutlineThickness(1);
       
        nameText.setFont(resources.getFont("main"));
        nameText.setString(name);
        nameText.setCharacterSize(12);
        nameText.setFillColor(sf::Color::White);
        nameText.setOutlineColor(sf::Color::Black);
        nameText.setOutlineThickness(1);
    }
   
    void update(float deltaTime) override {
        // Update base character
        Character::update(deltaTime);
       
        // Apply movement
        position += velocity * deltaTime;
        velocity = sf::Vector2f(0, 0);
       
        // Update camera to follow player
        gameView.setCenter(position);
       
        // Update UI elements
        healthBar.setPosition(position.x - 25, position.y - 40);
        healthBar.setSize(sf::Vector2f(50.0f * health / maxHealth, 6));
       
        manaBar.setPosition(position.x - 25, position.y - 32);
        manaBar.setSize(sf::Vector2f(50.0f * mana / maxMana, 4));
       
        nameText.setPosition(position.x - nameText.getLocalBounds().width / 2, position.y - 55);
    }
   
    void draw(sf::RenderWindow& window) override {
        // Draw character
        Character::draw(window);
       
        // Draw UI elements
        window.draw(healthBar);
        window.draw(manaBar);
        window.draw(nameText);
    }
   
    void move(float dx, float dy) {
        if (dx != 0 || dy != 0) {
            velocity = sf::Vector2f(dx, dy);
           
            // Normalize velocity for diagonal movement
            float length = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
            if (length > 0) {
                velocity /= length;
            }
           
            velocity *= moveSpeed;
           
            // Set facing direction
            if (dx != 0) {
                setFacingDirection(dx > 0);
            }
           
            setAnimation("walk");
        } else {
            setAnimation("idle");
        }
    }
   
    void gainExperience(int exp) {
        experience += exp;
       
        // Check for level up
        int requiredExp = level * 1000;
        if (experience >= requiredExp) {
            levelUp();
        }
    }
   
    void levelUp() {
        level++;
        experience -= level * 1000;
       
        // Increase stats based on D&D rules
        maxHealth += GameUtils::rollDice(1, 8) + (constitution - 10) / 2;
        health = maxHealth;
       
        maxMana += GameUtils::rollDice(1, 4) + (intelligence - 10) / 2;
        mana = maxMana;
       
        // Increase a random attribute
        int statChoice = GameUtils::getRandomInt(1, 6);
        switch (statChoice) {
            case 1: strength++; break;
            case 2: dexterity++; break;
            case 3: constitution++; break;
            case 4: intelligence++; break;
            case 5: wisdom++; break;
            case 6: charisma++; break;
        }
       
        // Recalculate derived stats
        armorClass = 10 + (dexterity - 10) / 2;
        attackBonus = (strength - 10) / 2;
       
        sounds.playSound("level_up");
    }
   
    // Inventory management
    void addItem(std::shared_ptr<Item> item) {
        inventory.push_back(item);
        sounds.playSound("item");
    }
   
    void removeItem(int index) {
        if (index >= 0 && index < inventory.size()) {
            inventory.erase(inventory.begin() + index);
        }
    }
   
    void addGold(int amount) {
        gold += amount;
        sounds.playSound("item");
    }
   
    // Quest management
    void setQuestFlag(const std::string& quest, bool completed) {
        questFlags[quest] = completed;
    }
   
    bool hasQuestFlag(const std::string& quest) const {
        auto it = questFlags.find(quest);
        return it != questFlags.end() && it->second;
    }
   
    // Spell management
    void learnSpell(std::shared_ptr<Spell> spell) {
        spells.push_back(spell);
    }
   
    bool castSpell(int spellIndex, Character& target) {
        if (spellIndex < 0 || spellIndex >= spells.size()) return false;
       
        auto spell = spells[spellIndex];
        if (mana < spell->getManaCost()) return false;
       
        mana -= spell->getManaCost();
        return spell->cast(*this, target);
    }
   
    // Getters
    int getExperience() const { return experience; }
    int getGold() const { return gold; }
    const std::vector<std::shared_ptr<Item>>& getInventory() const { return inventory; }
    const std::vector<std::shared_ptr<Spell>>& getSpells() const { return spells; }
};

// Enemy class
class Enemy : public Character {
private:
    int experienceValue;
    int goldValue;
    float detectionRange;
    float attackRange;
    Player* target;
    float actionTimer;
    float wanderTimer;
    sf::Vector2f wanderTarget;
    bool aggravated;
   
    // AI states
    enum class State { Idle, Wander, Chase, Attack };
    State currentState;
   
public:
    Enemy(const std::string& name, const std::string& type, ResourceManager& resources, SoundManager& sounds,
          int strength, int dexterity, int constitution, int intelligence, int wisdom, int charisma,
          int experienceValue, int goldValue)
        : Character(name, type, resources, sounds, type,
                   strength, dexterity, constitution, intelligence, wisdom, charisma),
          experienceValue(experienceValue), goldValue(goldValue),
          detectionRange(200.0f), attackRange(50.0f), target(nullptr),
          actionTimer(0.0f), wanderTimer(0.0f), aggravated(false),
          currentState(State::Idle) {
       
        // Set random wander target
        updateWanderTarget();
    }
   
    void update(float deltaTime) override {
        Character::update(deltaTime);
       
        if (!isAlive() || !target) return;
       
        actionTimer += deltaTime;
       
        // Update AI state based on distance to target
        float distanceToTarget = GameUtils::distance(position.x, position.y,
                                                   target->getPosition().x, target->getPosition().y);
       
        if (aggravated || distanceToTarget < detectionRange) {
            if (distanceToTarget <= attackRange) {
                currentState = State::Attack;
            } else {
                currentState = State::Chase;
                aggravated = true;
            }
        } else {
            wanderTimer += deltaTime;
            if (wanderTimer >= 3.0f) {
                updateWanderTarget();
                wanderTimer = 0.0f;
                currentState = State::Wander;
            }
        }
       
        // Execute behavior based on state
        switch (currentState) {
            case State::Idle:
                setAnimation("idle");
                break;
               
            case State::Wander:
                moveTowardsPoint(wanderTarget.x, wanderTarget.y, deltaTime, 50.0f);
                if (GameUtils::distance(position.x, position.y, wanderTarget.x, wanderTarget.y) < 10.0f) {
                    currentState = State::Idle;
                }
                break;
               
            case State::Chase:
                moveTowardsPoint(target->getPosition().x, target->getPosition().y, deltaTime, 100.0f);
                break;
               
            case State::Attack:
                if (actionTimer >= 1.0f) {  // Attack every second
                    attack(*target);
                    actionTimer = 0.0f;
                }
                break;
        }
    }
   
    void moveTowardsPoint(float x, float y, float deltaTime, float speed) {
        sf::Vector2f direction(x - position.x, y - position.y);
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
       
        if (length > 0) {
            direction /= length;
            position += direction * speed * deltaTime;
           
            // Update facing direction
            setFacingDirection(direction.x > 0);
            setAnimation("walk");
        }
    }
   
    void updateWanderTarget() {
        // Set a random point within reasonable distance
        float angle = GameUtils::getRandomFloat(0, 2 * 3.14159f);
        float distance = GameUtils::getRandomFloat(50, 150);
        wanderTarget.x = position.x + std::cos(angle) * distance;
        wanderTarget.y = position.y + std::sin(angle) * distance;
    }
   
    void setTarget(Player* player) {
        target = player;
    }
   
    void aggravate() {
        aggravated = true;
    }
   
    int getExperienceValue() const { return experienceValue; }
    int getGoldValue() const { return goldValue; }
};

// Item classes
class Item : public Entity {
protected:
    int value;
    std::string description;
    bool onGround;
   
public:
    Item(const std::string& name, const std::string& type, ResourceManager& resources,
         const std::string& description, int value)
        : Entity(name, type, resources, "items"),
          description(description), value(value), onGround(true) {
       
        // Set texture rect based on item type (for sprite sheet)
        if (type == "weapon") {
            sprite.setTextureRect(sf::IntRect(0, 0, 32, 32));
        } else if (type == "armor") {
            sprite.setTextureRect(sf::IntRect(32, 0, 32, 32));
        } else if (type == "potion") {
            sprite.setTextureRect(sf::IntRect(64, 0, 32, 32));
        } else {
            sprite.setTextureRect(sf::IntRect(96, 0, 32, 32));
        }
    }
   
    virtual ~Item() = default;
   
    virtual bool use(Player& player) {
        return false;  // Base items can't be used
    }
   
    void draw(sf::RenderWindow& window) override {
        if (onGround) {
            Entity::draw(window);
           
            // Add a subtle pulsing effect
            static float pulseTimer = 0.0f;
            pulseTimer += 0.05f;
            float scale = 1.0f + 0.1f * std::sin(pulseTimer);
            sprite.setScale(scale, scale);
        }
    }
   
    void pickUp() {
        onGround = false;
    }
   
    bool isOnGround() const {
        return onGround;
    }
   
    int getValue() const { return value; }
    std::string getDescription() const { return description; }
};

class Weapon : public Item {
private:
    int minDamage;
    int maxDamage;
    int attackBonus;
    std::string weaponType;
   
public:
    Weapon(const std::string& name, ResourceManager& resources,
           const std::string& description, int value,
           int minDamage, int maxDamage, int attackBonus,
           const std::string& weaponType)
        : Item(name, "weapon", resources, description, value),
          minDamage(minDamage), maxDamage(maxDamage),
          attackBonus(attackBonus), weaponType(weaponType) {
       
        // Set specific weapon appearance based on type
        if (weaponType == "Sword") {
            sprite.setTextureRect(sf::IntRect(0, 0, 32, 32));
        } else if (weaponType == "Axe") {
            sprite.setTextureRect(sf::IntRect(32, 0, 32, 32));
        } else if (weaponType == "Mace") {
            sprite.setTextureRect(sf::IntRect(64, 0, 32, 32));
        } else if (weaponType == "Staff") {
            sprite.setTextureRect(sf::IntRect(96, 0, 32, 32));
        }
    }
   
    int rollDamage() const {
        return GameUtils::getRandomInt(minDamage, maxDamage);
    }
   
    int getMinDamage() const { return minDamage; }
    int getMaxDamage() const { return maxDamage; }
    int getAttackBonus() const { return attackBonus; }
    std::string getWeaponType() const { return weaponType; }
};

class Armor : public Item {
private:
    int defense;
    std::string armorType;
   
public:
    Armor(const std::string& name, ResourceManager& resources,
          const std::string& description, int value,
          int defense, const std::string& armorType)
        : Item(name, "armor", resources, description, value),
          defense(defense), armorType(armorType) {
       
        // Set specific armor appearance based on type
        if (armorType == "Leather") {
            sprite.setTextureRect(sf::IntRect(0, 32, 32, 32));
        } else if (armorType == "Chain") {
            sprite.setTextureRect(sf::IntRect(32, 32, 32, 32));
        } else if (armorType == "Plate") {
            sprite.setTextureRect(sf::IntRect(64, 32, 32, 32));
        } else if (armorType == "Shield") {
            sprite.setTextureRect(sf::IntRect(96, 32, 32, 32));
        }
    }
   
    int getDefense() const { return defense; }
    std::string getArmorType() const { return armorType; }
};

class Potion : public Item {
private:
    int healAmount;
   
public:
    Potion(const std::string& name, ResourceManager& resources,
           const std::string& description, int value,
           int healAmount)
        : Item(name, "potion", resources, description, value),
          healAmount(healAmount) {
       
        // Set potion appearance
        sprite.setTextureRect(sf::IntRect(0, 64, 32, 32));
    }
   
    bool use(Player& player) override {
        player.heal(healAmount);
        return true;  // Consumable
    }
   
    int getHealAmount() const { return healAmount; }
};

// Spell class
class Spell {
private:
    std::string name;
    std::string description;
    int manaCost;
    int minimumLevel;
    std::function<bool(Character&, Character&)> effect;
   
public:
    Spell(const std::string& name, const std::string& description,
          int manaCost, int minimumLevel,
          std::function<bool(Character&, Character&)> effect)
        : name(name), description(description),
          manaCost(manaCost), minimumLevel(minimumLevel),
          effect(effect) {}
   
    bool cast(Character& caster, Character& target) {
        return effect(caster, target);
    }
   
    std::string getName() const { return name; }
    std::string getDescription() const { return description; }
    int getManaCost() const { return manaCost; }
    int getMinimumLevel() const { return minimumLevel; }
};

// Projectile class for spells/ranged attacks
class Projectile : public Entity {
private:
    float speed;
    int damage;
    Character* source;
    sf::Vector2f direction;
    float lifespan;
    float lifetime;
   
public:
    Projectile(const std::string& name, ResourceManager& resources,
               Character* source, float x, float y,
               float targetX, float targetY,
               float speed, int damage)
        : Entity(name, "projectile", resources, "fireball"),
          speed(speed), damage(damage), source(source),
          lifespan(2.0f), lifetime(0.0f) {
       
        setPosition(x, y);
       
        // Calculate direction
        direction.x = targetX - x;
        direction.y = targetY - y;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0) {
            direction /= length;
        }
       
        // Rotate sprite to face direction
        float angle = std::atan2(direction.y, direction.x) * 180 / 3.14159f;
        sprite.setRotation(angle);
    }
   
    void update(float deltaTime) override {
        Entity::update(deltaTime);
       
        // Move along direction
        position.x += direction.x * speed * deltaTime;
        position.y += direction.y * speed * deltaTime;
       
        // Update lifetime
        lifetime += deltaTime;
        if (lifetime >= lifespan) {
            setActive(false);
        }
       
        // Add a trail effect
        // In a real implementation, you would add particle effects here
    }
   
    bool hit(Character& target) {
        if (&target == source) return false;
       
        if (intersects(target)) {
            target.takeDamage(damage);
            setActive(false);
            return true;
        }
        return false;
    }
};

// Tile class for the world
class Tile {
public:
    enum class Type {
        Floor,
        Wall,
        Door,
        Chest,
        Water,
        Lava
    };
   
private:
    Type type;
    bool walkable;
    bool explored;
    sf::Sprite sprite;
   
public:
    Tile(Type type, ResourceManager& resources) : type(type), explored(false) {
        switch (type) {
            case Type::Floor:
                sprite.setTexture(resources.getTexture("floor"));
                walkable = true;
                break;
            case Type::Wall:
                sprite.setTexture(resources.getTexture("wall"));
                walkable = false;
                break;
            case Type::Door:
                sprite.setTexture(resources.getTexture("door"));
                walkable = true;
                break;
            case Type::Chest:
                sprite.setTexture(resources.getTexture("chest"));
                walkable = false;
                break;
            case Type::Water:
                sprite.setTexture(resources.getTexture("floor"));
                sprite.setColor(sf::Color(100, 100, 255));
                walkable = false;
                break;
            case Type::Lava:
                sprite.setTexture(resources.getTexture("floor"));
                sprite.setColor(sf::Color(255, 100, 50));
                walkable = false;
                break;
        }
    }
   
    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
    }
   
    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }
   
    Type getType() const { return type; }
    bool isWalkable() const { return walkable; }
   
    void setExplored(bool value) { explored = value; }
    bool isExplored() const { return explored; }
   
    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }
};

// Dungeon class
class Dungeon {
private:
    ResourceManager& resources;
    SoundManager& sounds;
    std::vector<std::vector<Tile>> tiles;
    std::vector<std::shared_ptr<Enemy>> enemies;
    std::vector<std::shared_ptr<Item>> items;
    Player* player;
    int width;
    int height;
   
public:
    Dungeon(ResourceManager& resources, SoundManager& sounds, Player* player, int width, int height)
        : resources(resources), sounds(sounds), player(player), width(width), height(height) {
       
        // Initialize tiles
        tiles.resize(height, std::vector<Tile>(width, Tile(Tile::Type::Floor, resources)));
       
        // Set tile positions
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                tiles[y][x].setPosition(x * TILE_SIZE, y * TILE_SIZE);
            }
        }
    }
   
    // Generate a simple dungeon layout
    void generateDungeon() {
        // Create walls around the edges
        for (int x = 0; x < width; x++) {
            tiles[0][x] = Tile(Tile::Type::Wall, resources);
            tiles[height-1][x] = Tile(Tile::Type::Wall, resources);
        }
        for (int y = 0; y < height; y++) {
            tiles[y][0] = Tile(Tile::Type::Wall, resources);
            tiles[y][width-1] = Tile(Tile::Type::Wall, resources);
        }
       
        // Create some random walls
        for (int i = 0; i < width * height / 20; i++) {
            int x = GameUtils::getRandomInt(2, width - 3);
            int y = GameUtils::getRandomInt(2, height - 3);
            tiles[y][x] = Tile(Tile::Type::Wall, resources);
        }
       
        // Create some random water
        for (int i = 0; i < width * height / 40; i++) {
            int x = GameUtils::getRandomInt(2, width - 3);
            int y = GameUtils::getRandomInt(2, height - 3);
            tiles[y][x] = Tile(Tile::Type::Water, resources);
        }
       
        // Create some random lava
        for (int i = 0; i < width * height / 50; i++) {
            int x = GameUtils::getRandomInt(2, width - 3);
            int y = GameUtils::getRandomInt(2, height - 3);
            tiles[y][x] = Tile(Tile::Type::Lava, resources);
        }
       
        // Add some chests
        for (int i = 0; i < width * height / 100; i++) {
            int x = GameUtils::getRandomInt(2, width - 3);
            int y = GameUtils::getRandomInt(2, height - 3);
            tiles[y][x] = Tile(Tile::Type::Chest, resources);
        }
       
        // Add some doors
        for (int i = 0; i < width * height / 80; i++) {
            int x = GameUtils::getRandomInt(2, width - 3);
            int y = GameUtils::getRandomInt(2, height - 3);
            tiles[y][x] = Tile(Tile::Type::Door, resources);
        }
       
        // Set positions
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                tiles[y][x].setPosition(x * TILE_SIZE, y * TILE_SIZE);
            }
        }
    }
   
    // Add enemies to the dungeon
    void addEnemy(const std::string& name, const std::string& type, int x, int y,
                 int strength, int dexterity, int constitution,
                 int intelligence, int wisdom, int charisma,
                 int experienceValue, int goldValue) {
       
        auto enemy = std::make_shared<Enemy>(name, type, resources, sounds,
                                           strength, dexterity, constitution,
                                           intelligence, wisdom, charisma,
                                           experienceValue, goldValue);
       
        enemy->setPosition(x * TILE_SIZE + TILE_SIZE / 2, y * TILE_SIZE + TILE_SIZE / 2);
        enemy->setTarget(player);
        enemies.push_back(enemy);
    }
   
    // Add item to the dungeon
    template<typename T, typename... Args>
    void addItem(int x, int y, Args&&... args) {
        auto item = std::make_shared<T>(std::forward<Args>(args)...);
        item->setPosition(x * TILE_SIZE + TILE_SIZE / 2, y * TILE_SIZE + TILE_SIZE / 2);
        items.push_back(item);
    }
   
    // Get tile at world position
    Tile* getTileAtPosition(float x, float y) {
        int tileX = static_cast<int>(x) / TILE_SIZE;
        int tileY = static_cast<int>(y) / TILE_SIZE;
       
        if (tileX >= 0 && tileX < width && tileY >= 0 && tileY < height) {
            return &tiles[tileY][tileX];
        }
        return nullptr;
    }
   
    // Check if position is walkable
    bool isWalkable(float x, float y) {
        Tile* tile = getTileAtPosition(x, y);
        return tile && tile->isWalkable();
    }
   
    // Update all entities in the dungeon
    void update(float deltaTime) {
        // Update enemies
        for (auto it = enemies.begin(); it != enemies.end();) {
            auto& enemy = *it;
           
            if (enemy->isActive()) {
                enemy->update(deltaTime);
                ++it;
            } else {
                // Drop loot when enemy dies
                if (GameUtils::getRandomInt(1, 100) <= 30) {
                    // 30% chance to drop an item
                    createRandomLoot(enemy->getPosition().x, enemy->getPosition().y);
                }
                it = enemies.erase(it);
            }
        }
       
        // Update items
        for (auto it = items.begin(); it != items.end();) {
            auto& item = *it;
           
            if (item->isActive() && item->isOnGround()) {
                item->update(deltaTime);
               
                // Check if player is close enough to pick up item
                if (GameUtils::distance(player->getPosition().x, player->getPosition().y,
                                      item->getPosition().x, item->getPosition().y) < 30.0f) {
                   
                    // Determine the item type and add to player inventory
                    if (item->getType() == "weapon") {
                        auto weapon = std::dynamic_pointer_cast<Weapon>(item);
                        if (weapon) {
                            player->addItem(weapon);
                        }
                    } else if (item->getType() == "armor") {
                        auto armor = std::dynamic_pointer_cast<Armor>(item);
                        if (armor) {
                            player->addItem(armor);
                        }
                    } else if (item->getType() == "potion") {
                        auto potion = std::dynamic_pointer_cast<Potion>(item);
                        if (potion) {
                            player->addItem(potion);
                        }
                    }
                   
                    item->pickUp();
                }
               
                ++it;
            } else {
                it = items.erase(it);
            }
        }
    }
   
    // Draw the dungeon
    void draw(sf::RenderWindow& window) {
        // Get the view bounds
        sf::Vector2f viewCenter = window.getView().getCenter();
        sf::Vector2f viewSize = window.getView().getSize();
        sf::FloatRect viewBounds(
            viewCenter.x - viewSize.x / 2,
            viewCenter.y - viewSize.y / 2,
            viewSize.x,
            viewSize.y
        );
       
        // Only draw tiles that are visible
        int startX = std::max(0, static_cast<int>(viewBounds.left) / TILE_SIZE);
        int startY = std::max(0, static_cast<int>(viewBounds.top) / TILE_SIZE);
        int endX = std::min(width, static_cast<int>(viewBounds.left + viewBounds.width) / TILE_SIZE + 1);
        int endY = std::min(height, static_cast<int>(viewBounds.top + viewBounds.height) / TILE_SIZE + 1);
       
        // Draw tiles
        for (int y = startY; y < endY; y++) {
            for (int x = startX; x < endX; x++) {
                tiles[y][x].draw(window);
            }
        }
       
        // Draw items
        for (auto& item : items) {
            item->draw(window);
        }
       
        // Draw enemies
        for (auto& enemy : enemies) {
            enemy->draw(window);
        }
    }
   
    // Create random loot item
    void createRandomLoot(float x, float y) {
        int lootType = GameUtils::getRandomInt(1, 3);
       
        switch (lootType) {
            case 1: {  // Weapon
                std::vector<std::string> weaponTypes = {"Sword", "Axe", "Mace", "Staff"};
                std::string type = weaponTypes[GameUtils::getRandomInt(0, weaponTypes.size() - 1)];
                int minDamage = 1 + player->getLevel() / 2;
                int maxDamage = 3 + player->getLevel();
               
                auto weapon = std::make_shared<Weapon>(
                    type + " of Power",
                    resources,
                    "A well-crafted " + type + " that seems to glow faintly.",
                    player->getLevel() * 10,
                    minDamage,
                    maxDamage,
                    1,
                    type
                );
               
                weapon->setPosition(x, y);
                items.push_back(weapon);
                break;
            }
           
            case 2: {  // Armor
                std::vector<std::string> armorTypes = {"Leather", "Chain", "Plate", "Shield"};
                std::string type = armorTypes[GameUtils::getRandomInt(0, armorTypes.size() - 1)];
                int defense = 1 + player->getLevel() / 2;
               
                auto armor = std::make_shared<Armor>(
                    type + " of Defense",
                    resources,
                    "A sturdy piece of " + type + " armor.",
                    player->getLevel() * 15,
                    defense,
                    type
                );
               
                armor->setPosition(x, y);
                items.push_back(armor);
                break;
            }
           
            case 3: {  // Potion
                int healAmount = 5 + player->getLevel() * 3;
               
                auto potion = std::make_shared<Potion>(
                    "Healing Potion",
                    resources,
                    "A red potion that restores health.",
                    player->getLevel() * 5,
                    healAmount
                );
               
                potion->setPosition(x, y);
                items.push_back(potion);
                break;
            }
        }
    }
   
    // Populate dungeon with Dragonlance-themed enemies
    void populateEnemies() {
        // Add some goblins
        for (int i = 0; i < width * height / 60; i++) {
            int x, y;
            do {
                x = GameUtils::getRandomInt(2, width - 3);
                y = GameUtils::getRandomInt(2, height - 3);
            } while (!isWalkable(x * TILE_SIZE, y * TILE_SIZE));
           
            addEnemy("Goblin", "goblin", x, y, 8, 14, 10, 6, 8, 5, 50, 5);
        }
       
        // Add some skeletons
        for (int i = 0; i < width * height / 80; i++) {
            int x, y;
            do {
                x = GameUtils::getRandomInt(2, width - 3);
                y = GameUtils::getRandomInt(2, height - 3);
            } while (!isWalkable(x * TILE_SIZE, y * TILE_SIZE));
           
            addEnemy("Skeleton", "skeleton", x, y, 10, 12, 12, 8, 8, 5, 75, 10);
        }
       
        // Add boss
        int bossX, bossY;
        do {
            bossX = GameUtils::getRandomInt(width / 2, width - 5);
            bossY = GameUtils::getRandomInt(height / 2, height - 5);
        } while (!isWalkable(bossX * TILE_SIZE, bossY * TILE_SIZE));
       
        addEnemy("Baaz Draconian", "dragon", bossX, bossY, 16, 12, 16, 10, 12, 8, 500, 100);
    }
   
    // Populate dungeon with loot
    void populateItems() {
        // Add some healing potions
        for (int i = 0; i < width * height / 70; i++) {
            int x, y;
            do {
                x = GameUtils::getRandomInt(2, width - 3);
                y = GameUtils::getRandomInt(2, height - 3);
            } while (!isWalkable(x * TILE_SIZE, y * TILE_SIZE));
           
            addItem<Potion>(x, y, "Healing Potion", resources, "A red potion that restores health.", 10, 20);
        }
       
        // Add starter weapon
        int weaponX = GameUtils::getRandomInt(2, 5);
        int weaponY = GameUtils::getRandomInt(2, 5);
        addItem<Weapon>(weaponX, weaponY, "Bronze Sword", resources, "A simple but effective blade.", 15, 2, 5, 1, "Sword");
       
        // Add starter armor
        int armorX = GameUtils::getRandomInt(2, 5);
        int armorY = GameUtils::getRandomInt(2, 5);
        addItem<Armor>(armorX, armorY, "Leather Armor", resources, "Basic protection crafted from tanned hides.", 20, 2, "Leather");
    }
   
    // Get enemies in the dungeon
    const std::vector<std::shared_ptr<Enemy>>& getEnemies() const {
        return enemies;
    }
};

// UI Manager
class UIManager {
private:
    ResourceManager& resources;
    sf::RenderWindow& window;
    Player& player;
   
    sf::RectangleShape inventoryPanel;
    sf::RectangleShape statsPanel;
    sf::RectangleShape minimapPanel;
    sf::Text titleText;
    sf::Text statsText;
    sf::Text inventoryText;
    sf::Text minimapText;
   
    bool inventoryOpen;
   
public:
    UIManager(ResourceManager& resources, sf::RenderWindow& window, Player& player)
        : resources(resources), window(window), player(player), inventoryOpen(false) {
       
        // Initialize UI panels
        inventoryPanel.setSize(sf::Vector2f(300, 400));
        inventoryPanel.setFillColor(sf::Color(30, 30, 30, 220));
        inventoryPanel.setOutlineColor(sf::Color(100, 100, 100));
        inventoryPanel.setOutlineThickness(2);
       
        statsPanel.setSize(sf::Vector2f(window.getSize().x, 50));
        statsPanel.setFillColor(sf::Color(20, 20, 20, 200));
       
        minimapPanel.setSize(sf::Vector2f(150, 150));
        minimapPanel.setFillColor(sf::Color(20, 20, 20, 180));
        minimapPanel.setOutlineColor(sf::Color(80, 80, 80));
        minimapPanel.setOutlineThickness(1);
       
        // Initialize text elements
        titleText.setFont(resources.getFont("main"));
        titleText.setCharacterSize(18);
        titleText.setFillColor(sf::Color::White);
       
        statsText.setFont(resources.getFont("main"));
        statsText.setCharacterSize(14);
        statsText.setFillColor(sf::Color::White);
       
        inventoryText.setFont(resources.getFont("main"));
        inventoryText.setCharacterSize(14);
        inventoryText.setFillColor(sf::Color::White);
       
        minimapText.setFont(resources.getFont("main"));
        minimapText.setCharacterSize(12);
        minimapText.setFillColor(sf::Color::White);
        minimapText.setString("Map");
    }
   
    void update() {
        // Update panel positions
        sf::Vector2f viewCenter = window.getView().getCenter();
        sf::Vector2f viewSize = window.getView().getSize();
       
        statsPanel.setPosition(viewCenter.x - viewSize.x / 2, viewCenter.y - viewSize.y / 2);
       
        minimapPanel.setPosition(
            viewCenter.x + viewSize.x / 2 - minimapPanel.getSize().x - 10,
            viewCenter.y - viewSize.y / 2 + 60
        );
        minimapText.setPosition(
            minimapPanel.getPosition().x + 10,
            minimapPanel.getPosition().y + 5
        );
       
        if (inventoryOpen) {
            inventoryPanel.setPosition(
                viewCenter.x - inventoryPanel.getSize().x / 2,
                viewCenter.y - inventoryPanel.getSize().y / 2
            );
           
            // Update inventory content
            std::stringstream ss;
            ss << "INVENTORY\n\n";
           
            const auto& inventory = player.getInventory();
            if (inventory.empty()) {
                ss << "Empty";
            } else {
                for (size_t i = 0; i < inventory.size(); i++) {
                    ss << i + 1 << ". " << inventory[i]->getName() << " - "
                       << inventory[i]->getDescription() << "\n";
                   
                    if (inventory[i]->getType() == "weapon") {
                        auto weapon = std::dynamic_pointer_cast<Weapon>(inventory[i]);
                        if (weapon) {
                            ss << "   Damage: " << weapon->getMinDamage() << "-"
                               << weapon->getMaxDamage() << ", +" << weapon->getAttackBonus() << " Attack\n";
                        }
                    } else if (inventory[i]->getType() == "armor") {
                        auto armor = std::dynamic_pointer_cast<Armor>(inventory[i]);
                        if (armor) {
                            ss << "   Defense: +" << armor->getDefense() << "\n";
                        }
                    } else if (inventory[i]->getType() == "potion") {
                        auto potion = std::dynamic_pointer_cast<Potion>(inventory[i]);
                        if (potion) {
                            ss << "   Heals: " << potion->getHealAmount() << " HP\n";
                        }
                    }
                   
                    ss << "   Value: " << inventory[i]->getValue() << " gold\n\n";
                }
            }
           
            inventoryText.setString(ss.str());
            inventoryText.setPosition(
                inventoryPanel.getPosition().x + 10,
                inventoryPanel.getPosition().y + 10
            );
        }
       
        // Update stats text
        std::stringstream statsStream;
        statsStream << player.getName() << " | Level " << player.getLevel()
                    << " | HP: " << player.getHealth() << "/" << player.getMaxHealth()
                    << " | Mana: " << player.getMana() << "/" << player.getMaxMana()
                    << " | Gold: " << player.getGold()
                    << " | XP: " << player.getExperience() << "/" << (player.getLevel() * 1000);
       
        statsText.setString(statsStream.str());
        statsText.setPosition(
            statsPanel.getPosition().x + 10,
            statsPanel.getPosition().y + 15
        );
    }
   
    void draw() {
        // Store current view
        sf::View currentView = window.getView();
       
        // Switch to UI view
        sf::View uiView = window.getDefaultView();
        window.setView(uiView);
       
        // Draw stats panel
        window.draw(statsPanel);
        window.draw(statsText);
       
        // Draw minimap
        window.draw(minimapPanel);
        window.draw(minimapText);
       
        // Draw inventory if open
        if (inventoryOpen) {
            window.draw(inventoryPanel);
            window.draw(inventoryText);
        }
       
        // Restore previous view
        window.setView(currentView);
    }
   
    void toggleInventory() {
        inventoryOpen = !inventoryOpen;
    }
   
    bool isInventoryOpen() const {
        return inventoryOpen;
    }
   
    // Show dialog with message
    void showDialog(const std::string& title, const std::string& message) {
        // Store current view
        sf::View currentView = window.getView();
       
        // Switch to UI view
        sf::View uiView = window.getDefaultView();
        window.setView(uiView);
       
        // Create dialog panel
        sf::RectangleShape dialogPanel;
        dialogPanel.setSize(sf::Vector2f(400, 200));
        dialogPanel.setFillColor(sf::Color(40, 40, 40, 230));
        dialogPanel.setOutlineColor(sf::Color(150, 150, 150));
        dialogPanel.setOutlineThickness(2);
        dialogPanel.setPosition(
            window.getSize().x / 2 - dialogPanel.getSize().x / 2,
            window.getSize().y / 2 - dialogPanel.getSize().y / 2
        );
       
        // Create title text
        sf::Text dialogTitle;
        dialogTitle.setFont(resources.getFont("main"));
        dialogTitle.setString(title);
        dialogTitle.setCharacterSize(18);
        dialogTitle.setFillColor(sf::Color::White);
        dialogTitle.setPosition(
            dialogPanel.getPosition().x + 20,
            dialogPanel.getPosition().y + 20
        );
       
        // Create message text
        sf::Text dialogMessage;
        dialogMessage.setFont(resources.getFont("main"));
        dialogMessage.setString(message);
        dialogMessage.setCharacterSize(14);
        dialogMessage.setFillColor(sf::Color::White);
        dialogMessage.setPosition(
            dialogPanel.getPosition().x + 20,
            dialogPanel.getPosition().y + 60
        );
       
        // Create continue button
        sf::RectangleShape continueButton;
        continueButton.setSize(sf::Vector2f(100, 30));
        continueButton.setFillColor(sf::Color(80, 80, 80));
        continueButton.setOutlineColor(sf::Color(150, 150, 150));
        continueButton.setOutlineThickness(1);
        continueButton.setPosition(
            dialogPanel.getPosition().x + dialogPanel.getSize().x / 2 - continueButton.getSize().x / 2,
            dialogPanel.getPosition().y + dialogPanel.getSize().y - 50
        );
       
        sf::Text continueText;
        continueText.setFont(resources.getFont("main"));
        continueText.setString("Continue");
        continueText.setCharacterSize(14);
        continueText.setFillColor(sf::Color::White);
        continueText.setPosition(
            continueButton.getPosition().x + continueButton.getSize().x / 2 - continueText.getGlobalBounds().width / 2,
            continueButton.getPosition().y + continueButton.getSize().y / 2 - continueText.getGlobalBounds().height
        );
       
        // Wait for user to click continue
        bool dialogOpen = true;
        while (dialogOpen) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                    return;
                }
               
                if (event.type == sf::Event::MouseButtonPressed) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(
                        sf::Vector2i(event.mouseButton.x, event.mouseButton.y),
                        uiView
                    );
                   
                    if (continueButton.getGlobalBounds().contains(mousePos)) {
                        dialogOpen = false;
                    }
                }
               
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return) {
                    dialogOpen = false;
                }
            }
           
            // Draw dialog
            window.clear(sf::Color(0, 0, 0));
           
            // Draw game in background (restoring previous view)
            window.setView(currentView);
            // Game rendering would go here
           
            // Draw dialog
            window.setView(uiView);
            window.draw(dialogPanel);
            window.draw(dialogTitle);
            window.draw(dialogMessage);
            window.draw(continueButton);
            window.draw(continueText);
           
            window.display();
        }
       
        // Restore view
        window.setView(currentView);
    }
};

// Game State Manager
class GameState {
public:
    enum class State {
        MainMenu,
        CharacterCreation,
        Playing,
        Inventory,
        Combat,
        Dialogue,
        GameOver,
        Victory
    };
   
private:
    State currentState;
   
public:
    GameState() : currentState(State::MainMenu) {}
   
    void setState(State state) {
        currentState = state;
    }
   
    State getState() const {
        return currentState;
    }
};

// Main game class
class Game {
private:
    sf::RenderWindow window;
    ResourceManager resources;
    SoundManager sounds;
    GameState gameState;
    UIManager* ui;
   
    sf::View gameView;
    sf::Clock gameClock;
   
    std::unique_ptr<Player> player;
    std::unique_ptr<Dungeon> currentDungeon;
   
    // Main menu elements
    sf::Text titleText;
    sf::Text startText;
    sf::Text quitText;
   
    // Character creation elements
    sf::Text creationTitle;
    std::vector<sf::Text> attributeNames;
    std::vector<sf::Text> attributeValues;
    std::vector<sf::RectangleShape> increaseButtons;
    std::vector<sf::RectangleShape> decreaseButtons;
    sf::RectangleShape confirmButton;
    sf::Text confirmText;
    int attributePoints;
    std::vector<int> attributes;
   
    bool showIntro;
   
public:
    Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), GAME_TITLE),
             resources(), sounds(resources), showIntro(true) {
       
        // Setup game view
        gameView.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        gameView.setCenter(sf::Vector2f(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2));
       
        // Setup main menu
        setupMainMenu();
       
        // Setup character creation
        setupCharacterCreation();
       
        // Start with main menu
        gameState.setState(GameState::State::MainMenu);
       
        // Start background music
        sounds.playMusic();
    }
   
    ~Game() {
        delete ui;
    }
   
    void run() {
        float deltaTime;
       
        while (window.isOpen()) {
            // Calculate delta time
            deltaTime = gameClock.restart().asSeconds();
           
            // Process events
            processEvents();
           
            // Update game
            update(deltaTime);
           
            // Render
            render();
        }
    }
   
private:
    void setupMainMenu() {
        titleText.setFont(resources.getFont("main"));
        titleText.setString("Dragonlance: Chronicles of the Lance");
        titleText.setCharacterSize(32);
        titleText.setFillColor(sf::Color::White);
        titleText.setPosition(
            WINDOW_WIDTH / 2 - titleText.getGlobalBounds().width / 2,
            100
        );
       
        startText.setFont(resources.getFont("main"));
        startText.setString("Start Game");
        startText.setCharacterSize(24);
        startText.setFillColor(sf::Color::White);
        startText.setPosition(
            WINDOW_WIDTH / 2 - startText.getGlobalBounds().width / 2,
            300
        );
       
        quitText.setFont(resources.getFont("main"));
        quitText.setString("Quit");
        quitText.setCharacterSize(24);
        quitText.setFillColor(sf::Color::White);
        quitText.setPosition(
            WINDOW_WIDTH / 2 - quitText.getGlobalBounds().width / 2,
            350
        );
    }
   
    void setupCharacterCreation() {
        creationTitle.setFont(resources.getFont("main"));
        creationTitle.setString("Character Creation");
        creationTitle.setCharacterSize(24);
        creationTitle.setFillColor(sf::Color::White);
        creationTitle.setPosition(
            WINDOW_WIDTH / 2 - creationTitle.getGlobalBounds().width / 2,
            50
        );
       
        attributePoints = 10;
        attributes = {10, 10, 10, 10, 10, 10}; // STR, DEX, CON, INT, WIS, CHA
       
        std::vector<std::string> attrNames = {
            "Strength", "Dexterity", "Constitution",
            "Intelligence", "Wisdom", "Charisma"
        };
       
        for (int i = 0; i < 6; i++) {
            sf::Text name;
            name.setFont(resources.getFont("main"));
            name.setString(attrNames[i]);
            name.setCharacterSize(18);
            name.setFillColor(sf::Color::White);
            name.setPosition(WINDOW_WIDTH / 2 - 100, 150 + i * 40);
            attributeNames.push_back(name);
           
            sf::Text value;
            value.setFont(resources.getFont("main"));
            value.setString(std::to_string(attributes[i]));
            value.setCharacterSize(18);
            value.setFillColor(sf::Color::White);
            value.setPosition(WINDOW_WIDTH / 2 + 50, 150 + i * 40);
            attributeValues.push_back(value);
           
            sf::RectangleShape increase;
            increase.setSize(sf::Vector2f(20, 20));
            increase.setFillColor(sf::Color(80, 80, 80));
            increase.setPosition(WINDOW_WIDTH / 2 + 80, 150 + i * 40);
            increaseButtons.push_back(increase);
           
            sf::RectangleShape decrease;
            decrease.setSize(sf::Vector2f(20, 20));
            decrease.setFillColor(sf::Color(80, 80, 80));
            decrease.setPosition(WINDOW_WIDTH / 2 + 20, 150 + i * 40);
            decreaseButtons.push_back(decrease);
        }
       
        confirmButton.setSize(sf::Vector2f(120, 30));
        confirmButton.setFillColor(sf::Color(80, 80, 80));
        confirmButton.setPosition(
            WINDOW_WIDTH / 2 - 60,
            400
        );
       
        confirmText.setFont(resources.getFont("main"));
        confirmText.setString("Start Adventure");
        confirmText.setCharacterSize(16);
        confirmText.setFillColor(sf::Color::White);
        confirmText.setPosition(
            confirmButton.getPosition().x + confirmButton.getSize().x / 2 - confirmText.getGlobalBounds().width / 2,
            confirmButton.getPosition().y + 5
        );
    }
   
    void startGame() {
        // Create player
        player = std::make_unique<Player>("Hero", resources, sounds, gameView,
                                        attributes[0], attributes[1], attributes[2],
                                        attributes[3], attributes[4], attributes[5]);
       
        // Set player position
        player->setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
       
        // Create UI manager
        ui = new UIManager(resources, window, *player);
       
        // Create and generate dungeon
        currentDungeon = std::make_unique<Dungeon>(resources, sounds, player.get(), 50, 50);
        currentDungeon->generateDungeon();
        currentDungeon->populateEnemies();
        currentDungeon->populateItems();
       
        // Set player position to a valid spot
        for (int y = 0; y < 50; y++) {
            for (int x = 0; x < 50; x++) {
                if (currentDungeon->isWalkable(x * TILE_SIZE, y * TILE_SIZE)) {
                    player->setPosition(x * TILE_SIZE + TILE_SIZE / 2, y * TILE_SIZE + TILE_SIZE / 2);
                    break;
                }
            }
        }
       
        // Switch to playing state
        gameState.setState(GameState::State::Playing);
       
        // Show intro if enabled
        if (showIntro) {
            ui->showDialog("Welcome to Krynn",
                          "In the world of Krynn, the evil forces of Queen Takhisis threaten to engulf the land. "
                          "You are a hero who has been called upon by the gods to defend the realm against "
                          "her draconian armies.\n\n"
                          "Your journey begins in the ancient ruins beneath the city of Xak Tsaroth, "
                          "where rumors speak of a powerful artifact that could turn the tide of war...");
            showIntro = false;
        }
    }
   
    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
           
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    if (gameState.getState() == GameState::State::Playing) {
                        window.close();
                    }
                }
               
                if (event.key.code == sf::Keyboard::I &&
                    gameState.getState() == GameState::State::Playing) {
                    ui->toggleInventory();
                }
            }
           
            if (event.type == sf::Event::MouseButtonPressed) {
                handleMouseClick(event.mouseButton.x, event.mouseButton.y);
            }
        }
    }
   
    void handleMouseClick(int x, int y) {
        switch (gameState.getState()) {
            case GameState::State::MainMenu:
                handleMainMenuClick(x, y);
                break;
               
            case GameState::State::CharacterCreation:
                handleCharacterCreationClick(x, y);
                break;
               
            case GameState::State::Playing:
                // Game clicks handled in update
                break;
               
            default:
                break;
        }
    }
   
    void handleMainMenuClick(int x, int y) {
        if (startText.getGlobalBounds().contains(sf::Vector2f(x, y))) {
            gameState.setState(GameState::State::CharacterCreation);
            sounds.playSound("item");
        }
       
        if (quitText.getGlobalBounds().contains(sf::Vector2f(x, y))) {
            window.close();
        }
    }
   
    void handleCharacterCreationClick(int x, int y) {
        for (int i = 0; i < 6; i++) {
            if (increaseButtons[i].getGlobalBounds().contains(sf::Vector2f(x, y))) {
                if (attributePoints > 0 && attributes[i] < 18) {
                    attributes[i]++;
                    attributePoints--;
                    attributeValues[i].setString(std::to_string(attributes[i]));
                    sounds.playSound("item");
                }
            }
           
            if (decreaseButtons[i].getGlobalBounds().contains(sf::Vector2f(x, y))) {
                if (attributes[i] > 8) {
                    attributes[i]--;
                    attributePoints++;
                    attributeValues[i].setString(std::to_string(attributes[i]));
                    sounds.playSound("item");
                }
            }
        }
       
        if (confirmButton.getGlobalBounds().contains(sf::Vector2f(x, y))) {
            startGame();
            sounds.playSound("level_up");
        }
    }
   
    void update(float deltaTime) {
        switch (gameState.getState()) {
            case GameState::State::MainMenu:
                // No updates needed for main menu
                break;
               
            case GameState::State::CharacterCreation:
                // Update character creation display
                break;
               
            case GameState::State::Playing:
                updateGame(deltaTime);
                break;
               
            default:
                break;
        }
    }
   
    void updateGame(float deltaTime) {
        // Update player movement from keyboard
        float moveX = 0, moveY = 0;
       
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            moveY = -1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            moveY = 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            moveX = -1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            moveX = 1;
        }
       
        player->move(moveX, moveY);
       
        // Update player
        player->update(deltaTime);
       
        // Collision detection with walls
        sf::Vector2f pos = player->getPosition();
        sf::FloatRect bounds = player->getBounds();
        float halfWidth = bounds.width / 2;
        float halfHeight = bounds.height / 2;
       
        // Check if player position is valid
        if (!currentDungeon->isWalkable(pos.x - halfWidth, pos.y - halfHeight) ||
            !currentDungeon->isWalkable(pos.x + halfWidth, pos.y - halfHeight) ||
            !currentDungeon->isWalkable(pos.x - halfWidth, pos.y + halfHeight) ||
            !currentDungeon->isWalkable(pos.x + halfWidth, pos.y + halfHeight)) {
            // Move player back if colliding with wall
            player->setPosition(pos.x - player->getPosition().x + pos.x,
                              pos.y - player->getPosition().y + pos.y);
        }
       
        // Update dungeon
        currentDungeon->update(deltaTime);
       
        // Update UI
        ui->update();
       
        // Check for combat
        const auto& enemies = currentDungeon->getEnemies();
        for (const auto& enemy : enemies) {
            float dist = player->distanceTo(*enemy);
            if (dist < 50.0f) {
                // Close enough to attack if clicked
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, gameView);
                   
                    if (enemy->getBounds().contains(worldPos)) {
                        player->attack(*enemy);
                    }
                }
            }
        }
       
        // Check for victory (all enemies defeated)
        if (enemies.empty()) {
            ui->showDialog("Victory!",
                          "You have cleared this dungeon of all enemies and recovered the Dragon Orb, "
                          "a powerful artifact that will help in the fight against Takhisis. "
                          "The heroes of Krynn thank you for your bravery!\n\n"
                          "Continue your journey in the full game...");
           
            gameState.setState(GameState::State::MainMenu);
        }
       
        // Check for game over
        if (!player->isAlive()) {
            ui->showDialog("Game Over",
                          "You have fallen in battle. The forces of Takhisis grow stronger without "
                          "your opposition. Perhaps another hero will rise to take your place...\n\n"
                          "Try again?");
           
            gameState.setState(GameState::State::MainMenu);
        }
    }
   
    void render() {
        window.clear(sf::Color(20, 20, 20));
       
        switch (gameState.getState()) {
            case GameState::State::MainMenu:
                renderMainMenu();
                break;
               
            case GameState::State::CharacterCreation:
                renderCharacterCreation();
                break;
               
            case GameState::State::Playing:
                renderGame();
                break;
               
            default:
                break;
        }
       
        window.display();
    }
   
    void renderMainMenu() {
        window.draw(titleText);
        window.draw(startText);
        window.draw(quitText);
    }
   
    void renderCharacterCreation() {
        window.draw(creationTitle);
       
        for (int i = 0; i < 6; i++) {
            window.draw(attributeNames[i]);
            window.draw(attributeValues[i]);
            window.draw(increaseButtons[i]);
            window.draw(decreaseButtons[i]);
        }
       
        window.draw(confirmButton);
        window.draw(confirmText);
       
        // Draw points remaining
        sf::Text pointsText;
        pointsText.setFont(resources.getFont("main"));
        pointsText.setString("Points remaining: " + std::to_string(attributePoints));
        pointsText.setCharacterSize(18);
        pointsText.setFillColor(sf::Color::White);
        pointsText.setPosition(WINDOW_WIDTH / 2 - 80, 350);
        window.draw(pointsText);
    }
   
    void renderGame() {
        // Set game view
        window.setView(gameView);
       
        // Draw dungeon
        currentDungeon->draw(window);
       
        // Draw player
        player->draw(window);
       
        // Draw UI
        ui->draw();
    }
};

// Entry point
int main() {
    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
   
    return 0;
}
```

In this version, after each combat the defeated monster drops its treasure, which is then added to the player's inventory. The game pauses between each attack so you can follow the action in a text-based console. You can compile this codeFirst we will download and install msys2.
After that we use the series of commands to install packages and update system.

Commands used :
Update the package database and base packages using



Update rest of the base packages 
pacman -Su

Now open up the Msys MinGW terminal
To install gcc and g++ for C and C++ 
For 64 bit
pacman -S mingw-w64-x86_64-gcc
For 32 bit
pacman -S mingw-w64-i686-gcc

To install the debugger ( gdb ) for C and C++
For 64 bit
pacman -S mingw-w64-x86_64-gdb
For 32 bit
pacman -S mingw-w64-i686-gdb

To check
gcc version : gcc --version
g++ version : g++ --version
gdb version : gdb --version

After installing these programs, we need to set the Path environment variable.

Our Website
https://www.LearningLad.com
