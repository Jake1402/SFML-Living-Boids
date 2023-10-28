#pragma once
#include <chrono>
#include <random>
#include <fstream>
/*
This AI algorithm is designed for the  purpose
of evolving boids, hence naming schemes are for
variables wont be the standard naming schemes
that you'd see on other typical genetic algorithms
projects. If you'd like to know more about the 
algorithm i'm using for this AI look at the wiki 
link below.
	wiki - https://en.wikipedia.org/wiki/Genetic_algorithm#Chromosome_representation

Extra quick note I yonked the code for this genetic
algorithm from a ML JAVA Library i'm currently working
on called floating-net. So expect very poor commenting.
I'm on 4 hours sleep and I've been awake for 13 hours fml.
*/

namespace ai {

	typedef struct chromosome {
		//Tags for use in boids and naming simplicity
		float perception;				//its field of view MAX IS 180
		float range;						//how far it can see
		float keepInBoundsFactor;		//How strong they'll steer to re-enter zone - 2.0
		float flyCentreFactor;			//How much boids will try and steer into local flock
		float avoidOtherBoidFactor;		//How strong they'll steer to avoid hitting other boids
		float matchingVelocityFactor;	//How strong they'll steer to match velocity vectors
		float steerToFoodFactor;		//How strong they'll steer to get food
		float attackOrAvoidFactor;		//How strong they'll turn to flee or hunt
		float avoidOtherMinDistance;		//How close they'll be to other boids before moving away
		float speedUpFactor;				//How quickly they'll speed up to match other boids

		//Contains tags but is used in genetic algorithm
		float genes[10] = {
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0
		};

		//Everything in here is constant
		const struct metabolicFactors {
			const float maxHealth = 100;			//how large it is
			const float maxStamina = 100;			//how much energy it has
			const float reproductionStamina = 100;	//how much stamina to reproduce
		};
		const struct geneBoundingValues {
			float maxPerception = 180;
			float maxRange = 1000;
			float maxKeepInBoundsFactor = 5.0;
			float maxFlyToCentreFactor = 2.0;
			float maxAvoidOtherBoidFactor = 2.0;		//How strong they'll steer to avoid hitting other boids
			float maxMatchingVelocityFactor = 2.0;		//How strong they'll steer to match velocity vectors
			float maxSteerToFoodFactor = 2.0;			//How strong they'll steer to get food
			float maxAttackOrAvoidFactor = 3.0;			//How strong they'll turn to flee or hunt
			float minAvoidOtherMinDistance = 30;		//How close they'll be to other boids before moving away
			float maxSpeedUpFactor = 5.0;				//How quickly they'll speed up to match other boids

			float maxGenes[10] = {
				maxPerception,
				maxRange,
				maxKeepInBoundsFactor,
				maxFlyToCentreFactor,
				maxAvoidOtherBoidFactor,
				maxMatchingVelocityFactor,
				maxSteerToFoodFactor,
				maxAttackOrAvoidFactor,
				minAvoidOtherMinDistance,
				maxSpeedUpFactor
			};
		};
		metabolicFactors factors;				//for boids doesnt change
		geneBoundingValues maxBounds;

		void init() {
			auto seed = std::chrono::system_clock::now().time_since_epoch().count();
			std::mt19937 gen(seed);
			std::uniform_real_distribution<float> dist(0, 1);
			perception = maxBounds.maxPerception * dist(gen);							//its field of view MAX IS 180
			range = maxBounds.maxRange * dist(gen);										//how far it can see
			keepInBoundsFactor = maxBounds.maxKeepInBoundsFactor * dist(gen);			//How strong they'll steer to re-enter zone - 2.0
			flyCentreFactor = maxBounds.maxFlyToCentreFactor * dist(gen);				//How much boids will try and steer into local flock
			avoidOtherBoidFactor = maxBounds.maxAvoidOtherBoidFactor * dist(gen);		//How strong they'll steer to avoid hitting other boids
			matchingVelocityFactor = maxBounds.maxMatchingVelocityFactor * dist(gen);	//How strong they'll steer to match velocity vectors
			steerToFoodFactor = maxBounds.maxSteerToFoodFactor * dist(gen);				//How strong they'll steer to get food
			attackOrAvoidFactor = maxBounds.maxAttackOrAvoidFactor * dist(gen);			//How strong they'll turn to flee or hunt
			avoidOtherMinDistance = maxBounds.minAvoidOtherMinDistance * dist(gen);		//How close they'll be to other boids before moving away
			speedUpFactor = maxBounds.maxSpeedUpFactor * dist(gen);						//How quickly they'll speed up to match other boids
			mapTagToGenes();
		}

		void mapTagToGenes() {
			genes[0] = perception;						//its field of view MAX IS 180
			genes[1] = range;							//how far it can see
			genes[2] = keepInBoundsFactor;				//How strong they'll steer to re-enter zone - 2.0
			genes[3] = flyCentreFactor;					//How much boids will try and steer into local flock
			genes[4] = avoidOtherBoidFactor;			//How strong they'll steer to avoid hitting other boids
			genes[5] = matchingVelocityFactor;			//How strong they'll steer to match velocity vectors
			genes[6] = steerToFoodFactor;				//How strong they'll steer to get food
			genes[7] = attackOrAvoidFactor;				//How strong they'll turn to flee or hunt
			genes[8] = avoidOtherMinDistance;			//How close they'll be to other boids before moving away
			genes[9] = speedUpFactor;					//How quickly they'll speed up to match other boids
		}

		void mapGenesToTag() {
			perception = genes[0];						//its field of view MAX IS 180
			range = genes[1];							//how far it can see
			keepInBoundsFactor = genes[2];				//How strong they'll steer to re-enter zone - 2.0
			flyCentreFactor = genes[3];					//How much boids will try and steer into local flock
			avoidOtherBoidFactor = genes[4];			//How strong they'll steer to avoid hitting other boids
			matchingVelocityFactor = genes[5];			//How strong they'll steer to match velocity vectors
			steerToFoodFactor = genes[6];				//How strong they'll steer to get food
			attackOrAvoidFactor = genes[7];				//How strong they'll turn to flee or hunt
			avoidOtherMinDistance = genes[8];			//How close they'll be to other boids before moving away
			speedUpFactor = genes[9];					//How quickly they'll speed up to match other boids
		}

		
		void setGenes(float _genes[10]) {
			//this->genes = _genes;
		}

		void operator = (const chromosome& _) {
			perception = _.perception;						//its field of view MAX IS 180
			range = _.range;							//how far it can see
			keepInBoundsFactor = _.keepInBoundsFactor;				//How strong they'll steer to re-enter zone - 2.0
			flyCentreFactor = _.flyCentreFactor;					//How much boids will try and steer into local flock
			avoidOtherBoidFactor = _.avoidOtherBoidFactor;			//How strong they'll steer to avoid hitting other boids
			matchingVelocityFactor = _.matchingVelocityFactor;			//How strong they'll steer to match velocity vectors
			steerToFoodFactor = _.steerToFoodFactor;				//How strong they'll steer to get food
			attackOrAvoidFactor = _.attackOrAvoidFactor;				//How strong they'll turn to flee or hunt
			avoidOtherMinDistance = _.avoidOtherMinDistance;			//How close they'll be to other boids before moving away
			speedUpFactor = _.speedUpFactor;
			mapTagToGenes();
		}
		
	}chromosome;

	typedef struct chances {
		float newGene = 0.02;		//Chance of creating a new value for gene
		float crossover = 0.93;		//Chance of crossover between mother/father
		float mutateValue = 0.05;	//Chances of a gene mutating after cross over
	}chances;

	class evolution
	{
	public:
		evolution();
		void generateChromosome();
		void mutate(int index);		//MUTATE A GENE ADD TO IT
		void newGene(int index);
		void crossOver(int index);
		ai::chromosome returnChild();
		void saveChromosome(const ai::chromosome& data, const std::string& filename);
		void loadChromosome(ai::chromosome& data, const std::string& filename);

	private:
		ai::chances chance;

		ai::chromosome 
			*fatherChromosome,
			*motherChromosome,
			_childChromosome;

	public:
		void setFatherChromosome(ai::chromosome &newFather);
		void setMotherChromosome(ai::chromosome &newMother);
		void setFitness(float fitness);

		ai::chromosome getFatherChromosome();
		ai::chromosome getMotherChromosome();
		ai::chromosome getChildChromosome();
	};
}