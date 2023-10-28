#include "boid.h"
#include "food.h"
#include "evolution.h"
#include <vector>
#include <iostream>
#include "SFML/Graphics.hpp"
#define RANDOM_0_1 ((double)rand()/RAND_MAX)
#define M_PIl 3.141592653589793238462643383279502884L	//precision
#define D_TO_R(X) X * M_PIl/180

/*~~~~~~~~~~~~~~~~~~~~~~~~CONSTRUCTORS~~~~~~~~~~~~~~~~~~~~~~~~*/
bd::boid::boid() {};
bd::boid::boid(int ID, sf::Vector2f screenDim, float maxVelocity) {
	this->_properties.ID = ID;
	this->_TOPSPEED = maxVelocity;
	this->setPosition(sf::Vector2f(
		float((((2 * abs(this->_settings.margin) * ((16 + 9) / 9)) + screenDim.x) * RANDOM_0_1) + this->_settings.margin * ((16 + 9) / 9)),
		((((2 * abs(this->_settings.margin)) + screenDim.y) * RANDOM_0_1) + this->_settings.margin)
	));
	this->setColor(sf::Color::Green);
	//this->_chromosome.range = 250;
	this->_chromosome.init();
	this->_properties.maxVelocity = this->_TOPSPEED;
	this->_properties.screenDim = screenDim;
	this->_health.stamina = this->_chromosome.factors.maxStamina;
	this->_health.health = this->_chromosome.factors.maxHealth;

	this->_settings.keepInBoundsFactor = this->_chromosome.keepInBoundsFactor;
	this->_settings.viewingDistance = this->_chromosome.range;
	this->_settings.flyCentreFactor = this->_chromosome.flyCentreFactor;
	this->_settings.avoidOtherBoidFactor = this->_chromosome.avoidOtherBoidFactor;
	this->_settings.avoidOtherMinDistance = this->_chromosome.avoidOtherMinDistance;
	this->_settings.matchingVelocityFactor = this->_chromosome.matchingVelocityFactor;
	this->_settings.speedUpFactor = this->_chromosome.speedUpFactor;
	this->_settings.steerToFoodFactor = this->_chromosome.steerToFoodFactor;
};
bd::boid::boid(int ID, sf::Vector2f screenDim, float maxVelocity, ai::chromosome chromo, sf::Vector2f parentPos) {
	this->_properties.ID = ID;
	this->_TOPSPEED = maxVelocity;
	this->setPosition(sf::Vector2f(parentPos));
	this->setColor(sf::Color::Green);
	//this->_chromosome.range = 250;
	this->_chromosome = chromo;
	this->_properties.maxVelocity = this->_TOPSPEED;
	this->_properties.screenDim = screenDim;
	this->_health.stamina = this->_chromosome.factors.maxStamina;
	this->_health.health = this->_chromosome.factors.maxHealth;

	this->_settings.keepInBoundsFactor = this->_chromosome.keepInBoundsFactor;
	this->_settings.viewingDistance = this->_chromosome.range;
	this->_settings.flyCentreFactor = this->_chromosome.flyCentreFactor;
	this->_settings.avoidOtherBoidFactor = this->_chromosome.avoidOtherBoidFactor;
	this->_settings.avoidOtherMinDistance = this->_chromosome.avoidOtherMinDistance;
	this->_settings.matchingVelocityFactor = this->_chromosome.matchingVelocityFactor;
	this->_settings.speedUpFactor = this->_chromosome.speedUpFactor;
	this->_settings.steerToFoodFactor = this->_chromosome.steerToFoodFactor;
};

/*~~~~~~~~~~~~~~~~~~~~~~~~UPDATE~~~~~~~~~~~~~~~~~~~~~~~~*/

void bd::boid::update(std::vector<bd::boid*> boids, std::vector<bd::food*> foods, sf::Time dt) {
	this->setPosition(sf::Vector2f(
		float(this->_properties.position.x + this->getVelocity().x*dt.asSeconds()),			// * elapsed.asSeconds()
		float(this->_properties.position.y + this->getVelocity().y*dt.asSeconds())			// * elapsed.asSeconds()
	));
	this->touchingFood(foods, dt);
	this->metabolism(dt);
	this->isDead();
	this->flyToCentre(boids);			//JUST PASS A BOID
	this->avoidOtherBoids(boids);		//JUST PASS A BOID
	this->matchVelocityVector(boids);	//JUST PASS A BOID
	this->speedUp();
	this->flyToFood(foods, boids);
	this->limitSpeed();
	this->addNoise(dt);
	this->keepBoidInBounds(dt);
	this->birth(boids);
};

/*~~~~~~~~~~~~~~~~~~~~~~~~ENVIRMENT CHECKS COLLISION, AM ALIVE ETC~~~~~~~~~~~~~~~~~~~~~~~~*/

void bd::boid::touchingFood(std::vector<bd::food*> entities, sf::Time dt) {
	float x = this->getPosition().x, y = this->getPosition().y;
	for (auto entity : entities) {
		float distance = sqrt(
			(entity->getPosition().x - x) * (entity->getPosition().x - x)  +
			(entity->getPosition().y - y) * (entity->getPosition().y - y)
		);
		if (distance <= (entity->getProperties().staminaGiveBack + 1)*30+25) {
			this->_health.stamina = this->_health.stamina + abs(this->_health.stamina * (entity->getProperties().staminaGiveBack)*dt.asSeconds()*5)+1;
			if (this->_health.stamina > this->_chromosome.factors.maxStamina) {
				this->_health.stamina = this->getGene().factors.maxStamina;
			}
		}
	}
};
void bd::boid::metabolism(sf::Time dt) {
	if (this->_health.stamina <= 0) {
		this->_properties.maxVelocity = this->_TOPSPEED / 2;
		this->_health.stamina = 0;
		this->_health.health -= 4 * dt.asSeconds();
	}
	else {
		this->_health.stamina -= 1 * dt.asSeconds();
		this->_properties.maxVelocity = this->_TOPSPEED;
	}
	if (this->_health.health < 100 && this->_health.stamina >= 0.2) {
		this->_health.health += 0.2;
		this->_health.stamina -= 0.2;
		this->_health.reproductionStamina += 5 * dt.asSeconds();
		if (this->_health.reproductionStamina >= 100) {
			this->_health.reproductionStamina += 0.5 * dt.asSeconds();
		}
		if (this->_health.health >= this->_chromosome.factors.maxHealth) {
			this->_health.health = 100;
		}
	}
};
bool bd::boid::isDead() {
	if (this->_health.health <= 0) {
		this->_health.alive = false;
		return true;
	}return false;
};

/*~~~~~~~~~~~~~~~~~~~~~~~~DISTANCE CALCULATIONS~~~~~~~~~~~~~~~~~~~~~~~~*/

float bd::boid::distance(bd::boid _boid) {
	float x = this->getPosition().x, y = this->getPosition().y;
	return sqrt(
		(_boid.getPosition().x - x)*(_boid.getPosition().x - x) +
		(_boid.getPosition().y - y)*(_boid.getPosition().y - y)
	);
};
int bd::boid::closestBoid(std::vector<bd::boid> boids) {
	float _greatestDistance = this->distance(boids.front());
	int counter = 0, bestCounter = 0;
	bd::boid* closestBoid;

	for (auto& boid : boids) {
		counter++;
		auto dis = this->distance(boid);
		if (dis <= _greatestDistance) {
			bestCounter = counter;
			_greatestDistance = dis, closestBoid = &boid;
		}
	}
	return bestCounter-1;	//return position of closest boid in vector boids
};
std::vector<int> bd::boid::boidsInRange(std::vector<bd::boid> boids) {
	std::vector<int> _boidInRange;
	int counter = 0;
	for (auto& boid : boids) {
		counter++;
		auto dis = this->distance(boid);
		if (dis <= this->_chromosome.range) {
			_boidInRange.push_back(counter-1);
		}
	}
	return _boidInRange;	//return positions of boids in range in vector boids
};
sf::Vector2f bd::boid::calculateCentre(std::vector<bd::boid> boids) {
	int x = 0, y = 0, n = 0;
	for (auto boid : boids) {
		x += boid.getPosition().x;
		y += boid.getPosition().y;
		n++;
	}
	return sf::Vector2f(double(x / n), double(y / n));
};

/*~~~~~~~~~~~~~~~~~~~~~~~~BOID FLIGHT FUNCTIONS~~~~~~~~~~~~~~~~~~~~~~~~*/

void bd::boid::flyToFood(std::vector<bd::food* > foods, std::vector<bd::boid*> boids) {
	float moveX = 0, moveY = 0;


	
	if (this->_health.type == 1) {
		for(auto prey : boids) {
			float _distance = sqrt(
				(prey->getPosition().x - this->getPosition().x) * (prey->getPosition().x - this->getPosition().x) +
				(prey->getPosition().y - this->getPosition().y) * (prey->getPosition().y - this->getPosition().y)
			);
			if (_distance <= this->_settings.viewingDistance) {
				moveX += this->getPosition().x - prey->getPosition().x;
				moveY += this->getPosition().y - prey->getPosition().y;
			}
		}
	}else {
		for (auto food : foods) {
			float _distance = sqrt(
				(food->getPosition().x - this->getPosition().x) * (food->getPosition().x - this->getPosition().x) +
				(food->getPosition().y - this->getPosition().y) * (food->getPosition().y - this->getPosition().y)
			);
			if (_distance <= this->_settings.viewingDistance) {
				moveX += this->getPosition().x - food->getPosition().x;
				moveY += this->getPosition().y - food->getPosition().y;
			}
		}
	}

	this->setVelocity(sf::Vector2f(
		this->getVelocity().x - moveX * (this->_settings.steerToFoodFactor + (1 - (this->_health.stamina / this->_chromosome.factors.maxStamina))),
		this->getVelocity().y - moveY * (this->_settings.steerToFoodFactor + (1 - (this->_health.stamina / this->_chromosome.factors.maxStamina)))
	));
};
void bd::boid::flyToCentre(std::vector<bd::boid*> boids) {
	float centreX = 0, centreY = 0;
	float numberOfChecks = 0;

	for (auto boid : boids) {
		if (this->distance(*boid) < this->_settings.viewingDistance && this->angleBetweenBoids(*boid) > cos(D_TO_R(this->_chromosome.perception))) {
			centreX += boid->getPosition().x;
			centreY += boid->getPosition().y;
			auto _tChild = this->reproduce(*boid);
			this->_child = _tChild;
			numberOfChecks++;
		}
	}
	if (numberOfChecks != 0) {
		centreX = centreX / numberOfChecks;
		centreY = centreY / numberOfChecks;

		this->setVelocity(sf::Vector2f(
			this->getVelocity().x + (centreX - this->getPosition().x) * this->_settings.flyCentreFactor,
			this->getVelocity().y + (centreY - this->getPosition().y) * this->_settings.flyCentreFactor
		));
	}

};
void bd::boid::keepBoidInBounds(sf::Time dt) {
	if (this->getPosition().x < this->_settings.margin * ((16 + 9) / 9)) {
		if (this->_health.stamina >= 0) {
			this->_health.stamina -= 0.5 * dt.asSeconds();
		}
		this->setVelocity(sf::Vector2f(
			this->getVelocity().x + this->_settings.keepInBoundsFactor,
			this->getVelocity().y
		));
	}if (this->getPosition().x > this->_properties.screenDim.x - this->_settings.margin * ((16+9)/9)) {
		if (this->_health.stamina >= 0) {
			this->_health.stamina -= 0.5 * dt.asSeconds();
		}
		this->setVelocity(sf::Vector2f(
			this->getVelocity().x - this->_settings.keepInBoundsFactor,
			this->getVelocity().y
		));
	}if (this->getPosition().y < this->_settings.margin) {
		if (this->_health.stamina >= 0) {
			this->_health.stamina -= 0.5 * dt.asSeconds();
		}
		this->setVelocity(sf::Vector2f(
			this->getVelocity().x,
			this->getVelocity().y + this->_settings.keepInBoundsFactor
		));
	}if (this->getPosition().y > this->_properties.screenDim.y - this->_settings.margin) {
		if (this->_health.stamina >= 0) {
			this->_health.stamina -= 0.5 * dt.asSeconds();
		}
		this->setVelocity(sf::Vector2f(
			this->getVelocity().x,
			this->getVelocity().y - this->_settings.keepInBoundsFactor
		));
	}
};
void bd::boid::avoidOtherBoids(std::vector<bd::boid*> boids) {
	float moveX = 0, moveY = 0;

	for (auto boid : boids) {
		if (this->distance(*boid) <= this->_settings.avoidOtherMinDistance && this->getID() != boid->getID() && this->angleBetweenBoids(*boid) > cos(D_TO_R(this->_chromosome.perception))) {
			moveX += this->getPosition().x - boid->getPosition().x;
			moveY += this->getPosition().y - boid->getPosition().y;
		}
	}
	this->setVelocity(sf::Vector2f(
		this->getVelocity().x + moveX * this->_settings.avoidOtherBoidFactor,
		this->getVelocity().y + moveY * this->_settings.avoidOtherBoidFactor
	));
};
void bd::boid::matchVelocityVector(std::vector<bd::boid*> boids) {
	float averageX = 0, averageY = 0, numberOfChecks = 0;

	for (auto boid : boids) {
		if (this->distance(*boid) < this->_settings.viewingDistance && this->getID() != boid->getID() && this->angleBetweenBoids(*boid) > cos(D_TO_R(this->_chromosome.perception))) {
			averageX += boid->getVelocity().x;
			averageY += boid->getVelocity().y;
			numberOfChecks+=1;
		}
	}
	if (numberOfChecks != 0) {
		averageX = averageX / numberOfChecks;
		averageY = averageY / numberOfChecks;

		this->setVelocity(sf::Vector2f(
			this->getVelocity().x + (averageX - this->getVelocity().x) * this->_settings.matchingVelocityFactor,
			this->getVelocity().y + (averageY - this->getVelocity().y) * this->_settings.matchingVelocityFactor
		));
	}

};
void bd::boid::addNoise(sf::Time dt) {
	this->setVelocity(sf::Vector2f(
		this->getVelocity().x + 2*(float(rand() / (0.5 * RAND_MAX)) - 1) * dt.asSeconds(),
		this->getVelocity().y + 2*(float(rand() / (0.5 * RAND_MAX)) - 1) * dt.asSeconds()
	));
};
void bd::boid::limitSpeed() {
	float velMagnitude = sqrt(
		(this->getVelocity().x * this->getVelocity().x) + (this->getVelocity().y * this->getVelocity().y)
	);
	if (velMagnitude > this->_properties.maxVelocity) {
		this->setVelocity(sf::Vector2f(
			(this->getVelocity().x / velMagnitude) * this->_properties.maxVelocity,
			(this->getVelocity().y / velMagnitude) * this->_properties.maxVelocity
		));
	}
};
void bd::boid::speedUp() {
	float increaseConsant = this->_settings.speedUpFactor;
	this->setVelocity(sf::Vector2f(
		this->getVelocity().x * increaseConsant,
		this->getVelocity().y * increaseConsant
	));
};

/*~~~~~~~~~~~~~~~~~~~~~~~~VECTOR MATHS~~~~~~~~~~~~~~~~~~~~~~~~*/

sf::Vector2f bd::boid::generateVector(float magnitude) {
	sf::Vector2f _vector;
	_vector.x = ((2 * (rand() % 2)) - 1) * ((double)rand() / RAND_MAX) * sqrt(magnitude);
	_vector.y = (sqrt(magnitude - (_vector.x * _vector.x)));
	_vector.y *= (2 * (rand() % 2)) - 1;
	if (_vector.x == 0 || _vector.y == 0) {
		return generateVector(magnitude);
	}
	return _vector;
};
float bd::boid::alignToVector() {
	float dy = this->getVelocity().y;
	float dx = this->getVelocity().x;
	if (dx == 0) { dx = 0.0000001; }
	float dydx = float(dy / dx);
	float degrees = (atan(dydx) * 180 / M_PIl);
	if (this->getVelocity().x >= 0 && this->getVelocity().y >= 0) { return degrees + 90; }
	if (this->getVelocity().x >= 0 && this->getVelocity().y <= 0) { return degrees + 90; }
	if (this->getVelocity().x <= 0 && this->getVelocity().y <= 0) { return degrees + 270; }
	if (this->getVelocity().x <= 0 && this->getVelocity().y >= 0) { return degrees + 270; }
	return degrees;
};
/*this function is my love, I like the formatting it is rather cute*/
float bd::boid::angleBetweenBoids(bd::boid _boid) {
	sf::Vector2f 
		h = this->getVelocity(), 
		p = this->getPosition(), 
		b = _boid.getPosition();

	float 
		dx1 = h.x,
		dy1 = h.y,
		dx2 = b.x - p.x,
		dy2 = b.y - p.y;

	try {
		return double(dx1 * dx2 + dy1 * dy2) / double(sqrt(pow(dx1, 2) + pow(dy1, 2)) * sqrt(pow(dx2, 2) + pow(dy2, 2)));
	}
	catch (int exc) {
		return double(dx1 * dx2 + dy1 * dy2) / 0.0000000001;
	}
};
ai::chromosome bd::boid::reproduce(bd::boid _parentBoid) {
	if (this->distance(_parentBoid) < 10 && this->_health.reproductionStamina > 80 && this->getType() == _parentBoid.getType()) {
		this->_health.reproductionStamina = 0;
		this->breedFactor.setFatherChromosome(this->_chromosome);
		auto _mGene = _parentBoid.getGene();
		this->breedFactor.setMotherChromosome(_mGene);
		this->breedFactor.generateChromosome();
		return this->breedFactor.returnChild();
	}
};
bd::boid* bd::boid::birth(std::vector<bd::boid*> boids) {
	bd::boid baby(boids.size(), this->getProperties().screenDim, this->getMaxVelocity(), this->_child, this->getPosition());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~GETTERS~~~~~~~~~~~~~~~~~~~~~~~~*/


int bd::boid::getID(){
	return  this->_properties.ID;
};
int bd::boid::getType() {
	return  this->_health.type;
};
sf::Vector2f bd::boid::getPosition() {
	return this->_properties.position;
};
sf::Vector2f bd::boid::getVelocity() {
	return this->_properties.velocity;
};
float bd::boid::getMaxVelocity() {
	return this->_properties.maxVelocity;
};
sf::Color bd::boid::getColor() {
	return this->_properties.color;
}
bd::health bd::boid::getHealth() {
	return this->_health;
};
ai::chromosome bd::boid::getGene() {
	return this->_chromosome;
};
bd::properties bd::boid::getProperties() {
	return this->_properties;
};
bd::boidBehaviourSettings bd::boid::getSettings() {
	return this->_settings;
};


/*~~~~~~~~~~~~~~~~~~~~~~~~SETTERS~~~~~~~~~~~~~~~~~~~~~~~~*/

void bd::boid::setID(int ID) {
	this->_properties.ID = ID;
};
void bd::boid::setType(int type) {
	this->_health.type = type;
};
void bd::boid::setPosition(sf::Vector2f pos) {
	this->_properties.position = pos;
};
void bd::boid::setVelocity(sf::Vector2f vel) {
	this->_properties.velocity = vel;
};
void bd::boid::setMaxVelocity(float maxVelocity) {
	this->_properties.maxVelocity = maxVelocity;
};
void bd::boid::setColor(sf::Color color) {
	this->_properties.color = color;
}
void bd::boid::setHealth(bd::health _health) {

};
void bd::boid::setGene(ai::chromosome _gene) {
};
void bd::boid::setProperties(bd::properties _properties) {
	this->_properties = _properties;
};
void bd::boid::setSettings(bd::boidBehaviourSettings _settings) {
	this->_settings = _settings;
};