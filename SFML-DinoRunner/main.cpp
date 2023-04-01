#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <array>
#include <vector>
#include <random>
using namespace sf;

const unsigned int windowSize_x = 1000;
const unsigned int windowSize_y = 500;
int gameSpeed = 8;
bool playerDead = false;
bool playDeadSound = false;

/// <summary>
/// A class to manage audio
/// </summary>
class SoundManager {
public:
    SoundBuffer dieBuffer;
    SoundBuffer jumpBuffer;
    SoundBuffer pointBuffer;
    Sound dieSound;
    Sound jumpSound;
    Sound pointSound;

    SoundManager() :dieBuffer(), jumpBuffer(), pointBuffer(), dieSound(), jumpSound(), pointSound() {
        dieBuffer.loadFromFile("E:/iKi/Freelance Stuff/SFML/SFML-DinoRunner/x64/Debug/res/Sounds/die.wav");
        jumpBuffer.loadFromFile("E:/iKi/Freelance Stuff/SFML/SFML-DinoRunner/x64/Debug/res/Sounds/jump.wav");
        pointBuffer.loadFromFile("E:/iKi/Freelance Stuff/SFML/SFML-DinoRunner/x64/Debug/res/Sounds/point.wav");

        dieSound.setBuffer(dieBuffer);
        jumpSound.setBuffer(jumpBuffer);
        pointSound.setBuffer(pointBuffer);
    }

};

/// <summary>
/// A class containing Ground properties
/// </summary>
class Ground {
public:
    Sprite groundSprite;
    Texture groundTexture;
    int offset{ 0 };
    Ground() : groundSprite(), groundTexture() {
        if (groundTexture.loadFromFile("E:/iKi/Freelance Stuff/SFML/SFML-DinoRunner/x64/Debug/res/Images/GroundImage.png")) {
            groundSprite.setTexture(groundTexture);
            groundSprite.setPosition(Vector2f(0.f, windowSize_y - groundTexture.getSize().y - 10));
        }
    }

    void Update() {
        if (playerDead == false) {
            if (offset > groundTexture.getSize().x - windowSize_x)
                offset = 0;

            offset += gameSpeed;
            groundSprite.setTextureRect(IntRect(offset, 0, windowSize_x, windowSize_y));
        }
        else {
            groundSprite.setTextureRect(IntRect(offset, 0, windowSize_x, windowSize_y));
        }
    }

    void Reset() {
        offset = 0;
        groundSprite.setTextureRect(IntRect(offset, 0, windowSize_x, windowSize_y));
    }
};


/// <summary>
/// A class containing Obstacle properties
/// </summary>
class Obstacle {
public:
    Sprite obstacleSprite;
    FloatRect obstacleBounds{ 0.f,0.f,0.f,0.f };
    float offset{ 0.f };
    Obstacle(Texture& texture, int obstacleId) : obstacleSprite(), obstacleBounds() {
        if (obstacleId == 1) {
            offset = windowSize_y - 127.7f;
        }
        if (obstacleId == 2) {
            offset = windowSize_y - 148.f;
        }
        if (obstacleId == 3) {
            offset = windowSize_y - 213.f;
        }

        obstacleSprite.setTexture(texture);
        obstacleSprite.setScale(obstacleSprite.getScale() / 1.65f);
        obstacleSprite.setPosition(Vector2f(windowSize_x, offset));
    }
};

/// <summary>
/// A class contains a group of Obstacle properties
/// </summary>
class Obstacles {
public:
    std::vector<Obstacle> obstacles;

    Time spawnTimer;
    Texture obstacleTexture_1;
    Texture obstacleTexture_2;
    Texture obstacleTexture_3;
    int randomNumber{ 0 };

    Obstacles() : spawnTimer(Time::Zero) {
        obstacles.reserve(5);

        if (obstacleTexture_1.loadFromFile("E:/iKi/Freelance Stuff/SFML/SFML-DinoRunner/x64/Debug/res/Images/Obstacle1.png")) {
            std::cout << "loaded obstacle image 1" << std::endl;
        }

        if (obstacleTexture_2.loadFromFile("E:/iKi/Freelance Stuff/SFML/SFML-DinoRunner/x64/Debug/res/Images/Obstacle2.png")) {
            std::cout << "loaded obstacle image 2" << std::endl;
        }

        if (obstacleTexture_3.loadFromFile("E:/iKi/Freelance Stuff/SFML/SFML-DinoRunner/x64/Debug/res/Images/Obstacle3.png")) {
            std::cout << "loaded obstacle image 3" << std::endl;
        }
    }

    void Update(Time& deltaTime) {
        spawnTimer += deltaTime;
        if (spawnTimer.asSeconds() > 0.5f + gameSpeed / 8) {
            randomNumber = (rand() % 3) + 1;
            if (randomNumber == 1) {
                obstacles.emplace_back(Obstacle(obstacleTexture_1, 1));
            }
            if (randomNumber == 2) {
                obstacles.emplace_back(Obstacle(obstacleTexture_2, 2));
            }
            if (randomNumber == 3) {
                obstacles.emplace_back(Obstacle(obstacleTexture_3, 3));
            }

            spawnTimer = Time::Zero;
        }

        if (playerDead == false) {
            for (int i = 0; i < obstacles.size(); i++)
            {
                obstacles[i].obstacleBounds = obstacles[i].obstacleSprite.getGlobalBounds();
                obstacles[i].obstacleBounds.width -= 10.f;
                obstacles[i].obstacleSprite.move(-1 * gameSpeed, 0.f);
                if (obstacles[i].obstacleSprite.getPosition().x < -50.f) {
                    std::vector<Obstacle>::iterator obstacleIter = obstacles.begin() + i;
                    obstacles.erase(obstacleIter);
                }
            }
        }
        else {
            for (auto& obstacles : obstacles) {
                obstacles.obstacleSprite.move(0.f, 0.f);
            }
        }
    }

    void drawTo(RenderWindow& window) {
        for (auto& obstacles : obstacles) {
            window.draw(obstacles.obstacleSprite);
        }
    }

    void Reset() {
        obstacles.erase(obstacles.begin(), obstacles.end());
    }
};

/// <summary>
/// A class containing information on Player object
/// </summary>
class Player {
public:
    Sprite player;
    Vector2f playerPos{ 0.f,0.f };
    Vector2f playerMotion{ 0.f,0.f };
    Texture playerTex;
    FloatRect playerBounds;
    SoundManager soundManager;
    /// <summary>
    /// Animation Frames
    /// </summary>
    std::array<IntRect, 7> frames;
    Time timer;
    int animationCounter{ 0 };
    bool dead = false;

    Player() : player(), playerTex(), soundManager(), timer() {
        if (playerTex.loadFromFile("E:/iKi/Freelance Stuff/SFML/SFML-DinoRunner/x64/Debug/res/Images/PlayerSpriteSheet.png")) {
            player.setTexture(playerTex);

            int j = 0;
            // Sprite sheet slicer
            for (int i = 0; i < frames.size(); i++)
            {
                if (i < 4)
                    frames[i] = IntRect(j * 256, 5, 256, 344);
                else
                {
                    if (i == 4) j = 0;
                    frames[i] = IntRect(j * 256, 349, 256, 344);
                }
                j++;
            }

            player.setTextureRect(frames[0]);
            player.setScale(player.getScale() / 3.f);
            playerPos = player.getPosition();
        }
        else {
            std::cout << "Error loading Player Sprites" << std::endl;
        }
    }

    void Update(Time& deltaTime, std::vector<Obstacle>& obstacles) {
        playerPos = player.getPosition();
        playerBounds = player.getGlobalBounds();
        playerBounds.height -= 15.f;
        playerBounds.width -= 10.f;
        timer += deltaTime;

        for (auto& obstacles : obstacles) {
            if (playerBounds.intersects(obstacles.obstacleBounds))
                playerDead = true;
        }

        if (!playerDead) {
            Walk();

            if ((Keyboard::isKeyPressed(Keyboard::Space) == true || Keyboard::isKeyPressed(Keyboard::Up)) && playerPos.y >= windowSize_y - 185.f) {
                animationCounter = 0;
                playerMotion.y = -22.5f;
                player.setTextureRect(frames[0]);
                soundManager.jumpSound.play();
            }

            if (playerPos.y < windowSize_y - 185.f) {
                playerMotion.y += 1.f;
                player.setTextureRect(frames[0]);
            }

            if (playerPos.y > windowSize_y - 185.f) {
                player.setPosition(Vector2f(player.getPosition().x, windowSize_y - 185.f));
                playerMotion.y = 0.f;
            }

            player.move(playerMotion);
        }
        else
        {
            playerMotion.y = 0.f;
            player.setTextureRect(frames[0]);
            if (timer.asMilliseconds() > 1300.f) {
                soundManager.dieSound.stop();
                soundManager.dieSound.setLoop(false);
                timer = Time::Zero;
            }
            else
            {
                if (!dead) {
                    soundManager.dieSound.play();
                    dead = true;
                }
            }
        }
    }

    void Walk() {
        for (int i = 0; i < frames.size(); i++) {
            if (animationCounter == (i + 1) * 5)
                player.setTextureRect(frames[i]);
        }

        if (animationCounter >= (frames.size() - 1) * 5)
            animationCounter = 0;

        animationCounter++;
    }

    void Reset() {
        dead = false;
        playerMotion.y = 0;
        player.setPosition(Vector2f(player.getPosition().x, windowSize_y - 185.f));
        player.setTextureRect(frames[0]);
    }
};

/// <summary>
/// A class contains of player scores
/// </summary>
class Score {
public:
    Text previousScoreText;
    Text highScoreText;
    Text scoresText;
    Font scoresFont;
    SoundManager soundManager;
    short score{ 0 };
    short previousScore{ 0 };
    short scoresIndex{ 0 };
    short scoresDiff{ 0 };

    short scoresInitial;

    Score() : scoresFont(), scoresText(), previousScoreText(), scoresInitial(), soundManager() {
        if (scoresFont.loadFromFile("E:/iKi/Freelance Stuff/SFML/SFML-DinoRunner/x64/Debug/res/Fonts/Font.ttf")) {
            scoresText.setFont(scoresFont);
            scoresText.setCharacterSize(25);
            scoresText.setPosition((Vector2f(windowSize_x / 2 + windowSize_x / 4 + 185.f, scoresText.getCharacterSize() + 10.f)));
            scoresText.setFillColor(Color(83, 83, 83));

            previousScoreText.setFont(scoresFont);
            previousScoreText.setCharacterSize(25);
            previousScoreText.setPosition(Vector2f(scoresText.getPosition().x - 100.f, scoresText.getPosition().y));
            previousScoreText.setFillColor(Color(83, 83, 83));

            highScoreText.setFont(scoresFont);
            highScoreText.setCharacterSize(25);
            highScoreText.setPosition(Vector2f(previousScoreText.getPosition().x - 50.f, previousScoreText.getPosition().y));
            highScoreText.setFillColor(Color(83, 83, 83));
        }

        highScoreText.setString("HI");
        scoresInitial = 0;
    }

    void Update() {
        if (playerDead == false) {
            scoresIndex++;
            if (scoresIndex >= 5) {
                scoresIndex = 0;
                score++;
            }
            scoresDiff = score - scoresInitial;
            if (scoresDiff > 550) {
                scoresInitial += 550;
                gameSpeed += 1;
                soundManager.pointSound.play();
            }

            scoresText.setString(std::to_string(score));
            previousScoreText.setString(std::to_string(previousScore));
        }
    }

    void Reset() {
        if (score > previousScore)
            previousScore = score;
        if (score < previousScore)
            previousScore = previousScore;

        previousScoreText.setString(std::to_string(previousScore));
        score = 0;
    }
};

class RestartButton {
public:
    Sprite restartButtonSprite;
    FloatRect restartButtonSpriteBounds;
    Texture restartButtonTexture;
    Vector2f mousePos;
    bool checkPressed{ false };

    RestartButton() :restartButtonSprite(), restartButtonTexture(), mousePos(0.f, 0.f), restartButtonSpriteBounds() {
        if (restartButtonTexture.loadFromFile("E:/iKi/Freelance Stuff/SFML/SFML-DinoRunner/x64/Debug/res/Images/RestartButton.png")) {
            restartButtonSprite.setTexture(restartButtonTexture);
            restartButtonSprite.setPosition(Vector2f(windowSize_x / 2 - restartButtonTexture.getSize().x / 2, windowSize_y / 2));
            restartButtonSpriteBounds = restartButtonSprite.getGlobalBounds();
        }
    }
};

class Clouds {
public:
    std::vector<Sprite> clouds;
    Time currTime;
    Texture cloudTexture;
    std::random_device device;
    std::mt19937 rng{ device() };

    Clouds() :cloudTexture(), clouds(), currTime(), device() {
        if (cloudTexture.loadFromFile("E:/iKi/Freelance Stuff/SFML/SFML-DinoRunner/x64/Debug/res/Images/cloud.png")) {
            std::cout << "Cloud Texture is Loaded" << std::endl;
        }

        clouds.reserve(4);
        clouds.emplace_back(Sprite(cloudTexture));
        clouds.back().setPosition(Vector2f(windowSize_x, windowSize_y / 2 - 40.f));
    }

    void Update(Time& deltaTime) {
        currTime += deltaTime;
        if (currTime.asSeconds() > 8.f) {
            clouds.emplace_back(Sprite(cloudTexture));

            std::uniform_int_distribution<std::mt19937::result_type>dist6(windowSize_y / 2 - 200, windowSize_y / 2 - 50);
            clouds.back().setPosition(Vector2f(windowSize_x, dist6(rng)));

            currTime = Time::Zero;
        }

        for (int i = 0; i < clouds.size(); i++)
        {
            if (playerDead == false)
                clouds[i].move(Vector2f(-1.f, 0.f));
            if (playerDead == true)
                clouds[i].move(Vector2f(-0.5f, 0.f));

            if (clouds[i].getPosition().x < 0.f - cloudTexture.getSize().x) {
                std::vector<Sprite>::iterator cloudIter = clouds.begin() + i;
                clouds.erase(cloudIter);
            }
        }
    }

    void drawTo(RenderWindow& window) {
        for (auto& clouds : clouds) {
            window.draw(clouds);
        }
    }
};

class GameState {
public:
    Player player;
    Ground ground;
    Obstacles obstacles;
    Score score;
    Clouds clouds;
    RestartButton restartButton;
    Font gameOverFont;
    Text gameOverText;
    Vector2f mousePos{ 0.f,0.f };

    GameState() : player(), ground(), obstacles(), score(), clouds(), gameOverFont(), gameOverText() {
        gameOverFont.loadFromFile("E:/iKi/Freelance Stuff/SFML/SFML-DinoRunner/x64/Debug/res/Fonts/Font.ttf");
        gameOverText.setFont(gameOverFont);
        gameOverText.setString("Game Over");
        gameOverText.setPosition(Vector2f(restartButton.restartButtonSprite.getPosition().x - gameOverText.getCharacterSize(), restartButton.restartButtonSprite.getPosition().y - 50));
        gameOverText.setFillColor(Color(83, 83, 83));

        player.player.setPosition(Vector2f(windowSize_x / 2 - windowSize_x / 4, windowSize_y - 50.f));
    }

    void SetMousePos(Vector2i p_mousePos) {
        mousePos.x = p_mousePos.x;
        mousePos.y = p_mousePos.y;
    }

    void Update(Time deltaTime) {
        restartButton.checkPressed = Mouse::isButtonPressed(Mouse::Left);
        if (playerDead == true && restartButton.restartButtonSpriteBounds.contains(mousePos) && restartButton.checkPressed == true) {
            ground.Reset();
            obstacles.Reset();
            player.Reset();
            score.Reset();
            playerDead = false;
        }
        else {
            ground.Update();
            obstacles.Update(deltaTime);
            player.Update(deltaTime, obstacles.obstacles);
            clouds.Update(deltaTime);
            score.Update();
        }
    }

    void drawTo(RenderWindow& window) {
        if (playerDead == true) {
            clouds.drawTo(window);
            window.draw(ground.groundSprite);
            obstacles.drawTo(window);
            window.draw(score.scoresText);
            window.draw(score.previousScoreText);
            window.draw(score.highScoreText);
            window.draw(player.player);
            window.draw(gameOverText);
            window.draw(restartButton.restartButtonSprite);
        }
        else {
            clouds.drawTo(window);
            window.draw(ground.groundSprite);
            obstacles.drawTo(window);
            window.draw(score.scoresText);
            window.draw(score.previousScoreText);
            window.draw(score.highScoreText);
            window.draw(player.player);
        }
    }
};

int main()
{
    RenderWindow window(VideoMode(windowSize_x, windowSize_y), "Dino Runner");
    window.setVerticalSyncEnabled(true);

    GameState gameState;
    Event event;
    Time deltaTime;
    Clock deltaTimeClock;

    Image bg;
    Texture bgTextures;
    Sprite bgSprite;

    bg.loadFromFile("E:/iKi/Freelance Stuff/SFML/SFML-DinoRunner/x64/Debug/res/Images/bg.png");
    bgTextures.loadFromImage(bg, IntRect(0, 0, windowSize_x, windowSize_y));
    bgSprite.setTexture(bgTextures);

    while (window.isOpen()) {
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
            gameState.SetMousePos(Mouse::getPosition(window));
        }

        deltaTime = deltaTimeClock.restart();

        gameState.Update(deltaTime);

        window.clear(Color::White);
        window.draw(bgSprite);
        gameState.drawTo(window);
        window.display();
    }
}