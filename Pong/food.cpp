#include "food.h"
#include <iostream>

#define RANDOM_0_1 ((double)rand()/RAND_MAX)

bd::food::food() {};
bd::food::food(int foodID, sf::Vector2f screenDim) {
	this->_properties.foodID = foodID;
	this->setPosition(sf::Vector2f(
		float((((2 * abs(this->_settings.margin) * ((16 + 9) / 9)) + screenDim.x) * RANDOM_0_1) + this->_settings.margin * ((16 + 9) / 9)),
		((((2 * abs(this->_settings.margin)) + screenDim.y) * RANDOM_0_1) + this->_settings.margin)
	));
	this->_settings.screenDim = screenDim;
	this->_properties.staminaGiveBack = 0.15;
};
float bd::food::generateStaminaGiveBack() {
	return 0;
};


sf::Vector2f bd::food::getPosition() {
	return this->_properties.position;
};
bd::propertiesFood bd::food::getProperties() {
	return this->_properties;
};
bd::settingsFood bd::food::getSettings() {
	return this->_settings;
};

void bd::food::setPosition(sf::Vector2f position) {
	this->_properties.position = position;
};
void bd::food::setProperties(bd::propertiesFood _properties) {
	this->_properties = _properties;
};
void bd::food::setSettings(bd::settingsFood _settings) {
	this->_settings = _settings;
};