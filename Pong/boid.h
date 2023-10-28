#include <vector>
#include "evolution.h"
#include "SFML/Graphics.hpp"
#include "food.h"
#pragma once

namespace bd {

	/*
		~~~~~RULES OF BOIDS AND GENETICS~~~~~
		BOID RULES:
			What is seperation? Seperation is steer to avoid flocking local mates.
			What is alignment? Align your self with the average heading of local 
			flock mates.
			What is cohesion? Steer to the average position of all the flock mates.

		GENETIC RULES:
			~What is speed? Speed is the maximum magnitude of its velocity
			highers speeds mean it'll consume its stamina quickly.
			~What is perception? Perception is the range of view/FOV higher Perception
			will not have any effects.
			~What is range? Range is the viewing distance of the boid. A higher viewing
			range will have a small impact on stamina consumption.
			~What is agility? Agility is the boids ability to align itself and 
			steer with the flock. Higher agilitys will consume more stamina when moving
			quickly.
			~What is maxHealth? Max health is the amount of health the boid has, health will 
			regenerate but doing this will consume stamina. If health is zero or stamina is zero 
			the boid dies and will not reproduce.
			~What is maxStamina? Max stamina is how long a boid will move for before it dies.
			Stamina is effected by speed, agility, and regenerating health. Stamina will also
			be effected by reproducing.
			~What is reproductionStamina? Reproduction stamina is the amount of stamina required to reproduce.
			Reproducing will consume 50% of the boids stamina. If the RS is met boids will begin
			reproducing.
			~What is numOffSpring? numOffSpring is the number boids that'll be created after reproducing.
			1 = 50% stamina consumption, 2 = 60%, 3 = 75%, 4 = 90%
	*/

	struct boidBehaviourSettings {
		float margin = -2000;					//How far boids can go off screen before turning
		float keepInBoundsFactor = 2.0;			//How strong they'll steer to re-enter zone - 0.2
		float viewingDistance = 100;			//How far boids can see other boids, this is tied to evolution
		float flyCentreFactor = 0.05;			//How much boids will try and steer into local flock
		float avoidOtherBoidFactor = 0.3;		//How strong they'll steer to avoid hitting other boids
		float avoidOtherMinDistance = 40;		//The minimum distance they'll be towards other boids before steering away
		float matchingVelocityFactor = 0.05;	//How string they'll steer to match velocity vectors
		float speedUpFactor = 1.05;				//Speed up factor to prevent boids from "stalling"
		float steerToFoodFactor = 0.01;			//Steer towards food
	};

	typedef struct health {
		int type;					//Is it prey or predator
		float health;				//What is current health
		float stamina;				//What is it current stamina
		float reproductionStamina;
		bool alive = true;			//true for alive
	}health;

	typedef struct properties {
		int ID;							//Type 0 = prey, 1 = predator
		sf::Color color;				//Color of the boid G||R
		sf::Vector2f position;			//Boids position in world
		sf::Vector2f velocity;			//Boids current velocity
		sf::Vector2f boidSize = sf::Vector2f(float(25*1.6), float(25*1.6));
		float maxVelocity;
		sf::Vector2f screenDim;
	}properties;

	class boid
	{
	public:
		boid();
		boid(int ID, sf::Vector2f screenDim, float maxVelocity);
		boid(int ID, sf::Vector2f screenDim, float maxVelocity, ai::chromosome chromo, sf::Vector2f parentPos);
		void update(std::vector<bd::boid*> boids, std::vector<bd::food*> foods, sf::Time dt);
		float alignToVector();
		void touchingFood(std::vector<bd::food*> entities, sf::Time dt);
		void metabolism(sf::Time dt);
		bool isDead();
		float distance(bd::boid _boid);
		int closestBoid(std::vector<bd::boid> boids);
		std::vector<int> boidsInRange(std::vector<bd::boid> boids);
		sf::Vector2f calculateCentre(std::vector<bd::boid> boids);
		void flyToFood(std::vector<bd::food* > foods, std::vector<bd::boid*> boids);
		void flyToCentre(std::vector<bd::boid*> boids);
		void keepBoidInBounds(sf::Time dt);
		void avoidOtherBoids(std::vector<bd::boid*> boids);
		void matchVelocityVector(std::vector<bd::boid*> boids);
		void addNoise(sf::Time dt);
		void limitSpeed();
		void speedUp();
		sf::Vector2f generateVector(float magnitude);
		float angleBetweenBoids(bd::boid _boid);
		ai::chromosome reproduce(bd::boid _parentBoid);
		bd::boid* birth(std::vector<bd::boid*> boids);

	private:
		bd::health _health;
		ai::chromosome _chromosome;
		bd::properties _properties;
		bd::boidBehaviourSettings _settings;
		ai::evolution breedFactor;
		ai::chromosome _child;
		int _TOPSPEED;

	public:
		int getID();
		int getType();
		sf::Vector2f getPosition();
		sf::Vector2f getVelocity();
		float getMaxVelocity();
		sf::Color getColor();
		bd::health getHealth();
		ai::chromosome getGene();
		bd::properties getProperties();
		bd::boidBehaviourSettings getSettings();

		void setID(int ID);
		void setType(int type);
		void setPosition(sf::Vector2f pos);
		void setVelocity(sf::Vector2f vel);
		void setMaxVelocity(float maxVelocity);
		void setColor(sf::Color color);
		void setHealth(bd::health _health);
		void setGene(ai::chromosome _gene);
		void setProperties(bd::properties _properties);
		void setSettings(bd::boidBehaviourSettings _settings);
	};
};
