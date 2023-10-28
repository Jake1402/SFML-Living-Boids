#include <SFML/Graphics.hpp>
#include "boid.h"
#include "evolution.h"
#include "food.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <functional>
#include <format>
#include <string>
#include <thread>

/*
This project was very difficult and would've been 100x more
challenging without help from external sources. These sources
include Daniel Shiffman and Ben Eater. Daniel Shiffmans coding
train series was a huge help and Ben Eaters online demo was 
super helpful so huge thanks them
Daniel Shiffman - https://www.youtube.com/watch?v=mhjuuHl6qHM
Ben Eater       - https://eater.net/boids

Some final notes on creation, I kinda wish I created two seperate 
classes for my boid types (predator and prey). I would also like 
to create a way to measure the fitness of each boid and create I
more binary reproduction system instead of a more mitosis like 
reproduction system that I currently have in place.
*/

/*
TODO:
    -SHIFT + ARROW KEYS ALLOWS YOU TO MOVE      #
    -DISPLAY INFORMATION IN TOP LEFT MARGIN SIZE & POPULATION   #
    -COLLISION WITH BOIDS AND FOOD              #
    -ADD STAMINA SYSTEM, BOIDS WILL GO TO FOOD WHEN LOW ON STAMINA          #
    -WHEN BOIDS GO OUT OF BOUNDS THEY TAKE DAMAGE UNTIL DEATH               #
    -MIDDLE CLICKING BOID WILL FOCUS YOU TO IT AND ESC WILL RESET CAMERA VIEW   #
    -PORT OVER GENETIC ALGORITHM CODE FROM YOU FLOATING-NET PROJECT #
    -MAKE PREDATOR BOIDS HUNT PREY BOIDS AND PREY BOIDS WILL TRY TO AVOID PREDATOR BOIDS
    -IMPLEMENT GENETIC ALGORITHM TO EVOLVE PREY AND PREDATORS
    -OPTIMSE, ONLY ONE FOR LOOP INSTEAD OF LIKE 12
*/

#define D_TO_R(X) X * 3.14159265/180

const float maxVel = 150;
int followID = 0; 
bool follow = false;

/*~~~~~~~~~~~~~~~~~~~~~~~~CONTROL LOOP~~~~~~~~~~~~~~~~~~~~~~~~*/
void controlLoop(sf::RenderWindow* window, sf::View* view, std::vector<bd::boid*>* boids, std::vector<bd::food*>* foods, sf::RectangleShape* marginBorder, bool* pause) {
    bd::boidBehaviourSettings _settings;
    while (true) {

        /*~~~~~~~~~~~~~~~~~~~~~~~~CHANGE ZOOM~~~~~~~~~~~~~~~~~~~~~~~~*/
        sf::sleep(sf::milliseconds(1000 / 60));
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && !sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
            view->zoom(0.9);
            window->setView(*view);
        }if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && !sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
            view->zoom(1.1);
            window->setView(*view);
        }
        
        /*~~~~~~~~~~~~~~~~~~~~~~~~CHANGE MARGIN SIZE~~~~~~~~~~~~~~~~~~~~~~~~*/
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Add)) {
            _settings.margin = _settings.margin - 50;
            if (_settings.margin > 300) { _settings.margin = 300; }
            for (auto boid : *boids) {
                boid->setSettings(_settings);
            }
            marginBorder->setSize(sf::Vector2f(
                ((2 * abs(_settings.margin)) * ((16 + 9) / 9)) + boids->at(0)->getProperties().screenDim.x,
                ((2 * abs(_settings.margin))) + boids->at(0)->getProperties().screenDim.y
            ));
            marginBorder->setPosition(sf::Vector2f(
                _settings.margin * ((16 + 9) / 9),
                _settings.margin
            ));
        }if (sf::Keyboard::isKeyPressed(sf::Keyboard::Subtract)) {
            if (_settings.margin + 50 >= -100) {
                continue;
            }
            _settings.margin = _settings.margin + 50;
            for (auto boid : *boids) {
                boid->setSettings(_settings);
            }
            marginBorder->setSize(sf::Vector2f(
                ((2 * abs(_settings.margin)) * ((16 + 9) / 9)) + boids->at(0)->getProperties().screenDim.x,
                ((2 * abs(_settings.margin)) + boids->at(0)->getProperties().screenDim.y)
            ));
            marginBorder->setPosition(sf::Vector2f(
                _settings.margin * ((16 + 9) / 9),
                _settings.margin
            ));
        }

        /*~~~~~~~~~~~~~~~~~~~~~~~~PAUSE SIMULATION~~~~~~~~~~~~~~~~~~~~~~~~*/
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            if (*pause) {
                *pause = false;
            }
            else {
                *pause = true;
            }
            sf::sleep(sf::milliseconds(150));
        }

        /*~~~~~~~~~~~~~~~~~~~~~~~~CREATE MORE BOIDS AND FOOD~~~~~~~~~~~~~~~~~~~~~~~~*/
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
            sf::Vector2i mousePos = sf::Mouse::getPosition();
            sf::Vector2f SpawnPos = window->mapPixelToCoords(mousePos);
            boids->push_back(new bd::boid(boids->size() + 1, sf::Vector2f(window->getSize()), boids->at(0)->getMaxVelocity()));
            boids->back()->setPosition(sf::Vector2f(SpawnPos));
            boids->back()->generateVector(maxVel);
            sf::sleep(sf::milliseconds(100));
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
            sf::Vector2i mousePos = sf::Mouse::getPosition();
            sf::Vector2f SpawnPos = window->mapPixelToCoords(mousePos);
            boids->push_back(new bd::boid(boids->size() + 1, sf::Vector2f(window->getSize()), boids->at(0)->getMaxVelocity()));
            boids->back()->setPosition(sf::Vector2f(SpawnPos));
            boids->back()->generateVector(maxVel);
            boids->back()->setColor(sf::Color::Red);
            sf::sleep(sf::milliseconds(100));
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {
            sf::Vector2i mousePos = sf::Mouse::getPosition();
            sf::Vector2f SpawnPos = window->mapPixelToCoords(mousePos);
            foods->push_back(new bd::food(foods->size() + 1, sf::Vector2f(window->getSize())));
            foods->back()->setPosition(sf::Vector2f(SpawnPos));
            sf::sleep(sf::milliseconds(100));
        }

        /*~~~~~~~~~~~~~~~~~~~~~~~~MOVE CAMERA WITH SHIFT + ARROW AND POSITION RESET~~~~~~~~~~~~~~~~~~~~~~~~*/
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
            view->move(-10,0);
            window->setView(*view);
        }if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
            view->move(10, 0);
            window->setView(*view);
        }if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
            view->move(0, -10);
            window->setView(*view);
        }if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
            view->move(0, 10);
            window->setView(*view);
        }if (sf::Keyboard::isKeyPressed(sf::Keyboard::V)) {
            view->setCenter(window->getSize().x/2, window->getSize().y/2);
            window->setView(*view);
        }

        /*~~~~~~~~~~~~~~~~~~~~~~~~FOLLOW SPRITE~~~~~~~~~~~~~~~~~~~~~~~~*/
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
            sf::Vector2f mousePosition(window->mapPixelToCoords(sf::Mouse::getPosition()));
            for (auto boid : *boids) {
                if (boid->getHealth().alive) {
                    float distance = sqrt(
                        (boid->getPosition().x - mousePosition.x) * (boid->getPosition().x - mousePosition.x) +
                        (boid->getPosition().y - mousePosition.y) * (boid->getPosition().y - mousePosition.y)
                    );
                    if (distance < 75) {
                        followID = boid->getID();
                        follow = true;
                        break;
                    }
                }
            }
        }if (sf::Keyboard::isKeyPressed(sf::Keyboard::V) && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
            follow = false;
            view->setCenter(boids->at(followID)->getPosition());
            window->setView(*view);
        }
    }
    delete window, view, boids, marginBorder, pause;
}

int numberAlive(std::vector<bd::boid*> boids) {
    int population = 0;
    for (auto boid : boids) {
        if (boid->getHealth().alive) {
            population++;
        }
    }
    return population;
}

int main()
{

    /*~~~~~~~~~~~~~~~~~~~~~~~~SETUP WINDOW AND VIEW SETTINGS~~~~~~~~~~~~~~~~~~~~~~~~*/
    srand(time(NULL));
    sf::Vector2f screenDim(1920, 1080);
    sf::RenderWindow window(sf::VideoMode(screenDim.x, screenDim.y), "Evolving Boids");
    window.clear(sf::Color(255,255,255));
    window.setVerticalSyncEnabled(false);
    //window.setFramerateLimit(60);
    sf::View view = window.getDefaultView();
    sf::View guiView = window.getDefaultView();
    bool pause = false;

    /*~~~~~~~~~~~~~~~~~~~~~~~~CREATE SPRITE OBJECTS~~~~~~~~~~~~~~~~~~~~~~~~*/
    sf::Sprite boidSprite;
    sf::Texture boidTexture;
    if (!boidTexture.loadFromFile("textures/boidSprite.png")) { return -1; }
    boidSprite.setTexture(boidTexture);
    boidSprite.setScale(sf::Vector2f(
        1.6,1.6
    ));
    boidSprite.setOrigin(sf::Vector2f(
        boidSprite.getLocalBounds().width/2,
        boidSprite.getLocalBounds().height/2
    ));

    sf::Sprite foodSprite;
    sf::Texture foodTexture;
    if (!foodTexture.loadFromFile("textures/flower.png")) { return -1; }
    foodSprite.setTexture(foodTexture);
    foodSprite.setScale(sf::Vector2f(
        1.0f,1.0f
    ));


    /*~~~~~~~~~~~~~~~~~~~~~~~~CREATE THE BOIDS AND FOODS~~~~~~~~~~~~~~~~~~~~~~~~*/
    std::vector<bd::boid*> boids;
    for (int boidID = 0; boidID < 1000; boidID++) {
        boids.push_back(new bd::boid(boidID, screenDim, maxVel));
        boids.at(boidID)->setVelocity(
            boids.at(boidID)->generateVector(maxVel)
        );
        if ((float)rand() / (float)RAND_MAX <= 0.3) {
            boids.at(boidID)->setType(1);
            boids.at(boidID)->setColor(sf::Color::Red);
            ai::chromosome _tempChromo = boids.at(boidID)->getGene();
            ai::evolution saveChromo;
            saveChromo.saveChromosome(_tempChromo, ".\\boidChromosomes\\predator\\"+std::to_string(boidID) + ".bin");
        }
        else {
            boids.at(boidID)->setType(0);
            boids.at(boidID)->setColor(sf::Color::Green);
            ai::chromosome _tempChromo = boids.at(boidID)->getGene();
            ai::evolution saveChromo;
            saveChromo.saveChromosome(_tempChromo, ".\\boidChromosomes\\prey\\" + std::to_string(boidID) + ".bin");
        }
        //ai::chromosome _tempChromo = boids.at(boidID)->getGene();
        //ai::evolution saveChromo;
        //saveChromo.saveChromosome(_tempChromo, std::to_string(boidID)+".bin");
    }

    std::vector<bd::food*> foods;
    for (int foodID = 0; foodID < 1; foodID++) {
        foods.push_back(new bd::food(foodID, screenDim));
    }


    /*~~~~~~~~~~~~~~~~~~~~~~~~CREATE MARGIN BORDER SHAPE~~~~~~~~~~~~~~~~~~~~~~~~*/
    sf::RectangleShape marginBorder(sf::Vector2f(
        ((2 * abs(boids.at(0)->getSettings().margin)) * ((16 + 9) / 9)) + screenDim.x,
        ((2 * abs(boids.at(0)->getSettings().margin)) + screenDim.y)
    ));
    marginBorder.setPosition(sf::Vector2f(
        -2000 * ((16 + 9) / 9),
        -2000
    ));
    marginBorder.setOutlineColor(sf::Color(214, 116, 17));
    marginBorder.setOutlineThickness(30);

    /*~~~~~~~~~~~~~~~~~~~~~~~~MARGIN TEXT~~~~~~~~~~~~~~~~~~~~~~~~*/
    std::ostringstream ss;
    ss << -2000;
    sf::Font RobotoMono;
    if (!RobotoMono.loadFromFile("resources/RobotoMono-Bold.ttf")) { return -1; }
    sf::Text StatisticText;
    StatisticText.setFont(RobotoMono);
    StatisticText.setCharacterSize(30);
    StatisticText.setPosition(sf::Vector2f(
        screenDim.x * 0.01,
        screenDim.y * 0.01
    ));
    StatisticText.setFillColor(sf::Color::Black);
    StatisticText.setString(ss.str());

    /*~~~~~~~~~~~~~~~~~~~~~~~~CREATE AND LAUNCH CONTROL THREAD~~~~~~~~~~~~~~~~~~~~~~~~*/
    sf::Thread controlThread(std::bind(&controlLoop, &window, &view, &boids, &foods, &marginBorder, &pause));
    controlThread.launch();

    sf::Clock deltaClock;
    sf::Time dt = deltaClock.restart();

    const int num_threads = 4; // number of threads to use
    const int chunk_size = boids.size() / num_threads; // number of boids per thread

    while (window.isOpen())
    {
        dt = deltaClock.restart();
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (follow) {
            view.setCenter(boids.at(followID)->getPosition());
            window.setView(view);
        }

        window.clear(sf::Color(200, 200, 200));
        window.draw(marginBorder);
        /*~~~~~~~~~~~~~~~~~~~~~~~~ITERATE THROUGH EACH BOID AND FOOD UPDATE AND DRAW~~~~~~~~~~~~~~~~~~~~~~~~*/
        for (auto food : foods) {
            foodSprite.setPosition(food->getPosition());
            foodSprite.setScale(sf::Vector2f(
                food->getProperties().staminaGiveBack + 1,
                food->getProperties().staminaGiveBack + 1
            ));
            window.draw(foodSprite);
        }

        //sf::Clock time;
        for (auto boid : boids) {
            if (boid->getHealth().alive) {
                if (!pause) {
                    boid->update(boids, foods, dt);
                }
                boidSprite.setColor(boid->getColor());
                boidSprite.setPosition(boid->getPosition());
                boidSprite.setRotation(boid->alignToVector());
                window.draw(boidSprite);
            }
        }

        /*~~~~~~~~~~~~~~~~~~~~~~~~DRAW THE STATISTICS TO THE SCREEN~~~~~~~~~~~~~~~~~~~~~~~~*/
        window.setView(guiView);
        std::ostringstream ss;
        ss << numberAlive(boids);
        StatisticText.setPosition(sf::Vector2f(
            screenDim.x * 0.01,
            screenDim.y * 0.01
        ));
        StatisticText.setString("Population - " + ss.str());
        window.draw(StatisticText);
        if (follow) {
            StatisticText.setPosition(sf::Vector2f(
                window.getSize().x * (0.01),
                window.getSize().y * 0.05
            ));
            window.setView(guiView);
            std::ostringstream ss1;
            ss1 << int(floor(boids.at(followID)->getHealth().stamina));
            std::string staminaString = "Stamina - " + ss1.str();
            StatisticText.setString(staminaString);
            window.draw(StatisticText);

            StatisticText.setPosition(sf::Vector2f(
                window.getSize().x * (0.01),
                window.getSize().y * 0.09
            ));
            std::ostringstream ss2;
            ss2 << int(floor(boids.at(followID)->getHealth().health));
            std::string healthString = "Health - " + ss2.str();
            StatisticText.setString(healthString);
            window.draw(StatisticText);
        }
        
        std::ostringstream titleTrans;
        titleTrans << floor(float((1 / dt.asSeconds())));
        window.setTitle("Evolving Boids in SFML - " + titleTrans.str());
        window.setView(view);
        window.display();
    }
    controlThread.terminate();
    return 0;
}