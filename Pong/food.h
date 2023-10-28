#include "SFML/Graphics.hpp"
#pragma once
namespace bd {

	typedef struct propertiesFood {
		int foodID;
		sf::Vector2f position;				//Boids position in world
		float staminaGiveBack=0;			//A percentage of stamina to give back 1 = 100% stamina
	}propertiesFood;

	typedef struct settingsFood {
		sf::Vector2f screenDim;
		float margin = -2000;
	}settingsFood;

	class food
	{
	public:
		food();
		food(int foodID, sf::Vector2f screenDim);
		float generateStaminaGiveBack();

	private:
		bd::propertiesFood _properties;
		bd::settingsFood _settings;
	
	public:
		sf::Vector2f getPosition();
		bd::propertiesFood getProperties();
		bd::settingsFood getSettings();

		void setPosition(sf::Vector2f position);
		void setProperties(bd::propertiesFood _properties);
		void setSettings(bd::settingsFood _settings);
	};
};
