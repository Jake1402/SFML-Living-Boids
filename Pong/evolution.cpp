#include "evolution.h"
#include <iostream>
#include <fstream>
#include <functional>
#include <utility>

ai::evolution::evolution() {};
void ai::evolution::generateChromosome() {
	this->_childChromosome.init();
};
void ai::evolution::mutate(int index) {
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 gen(seed);
	std::uniform_real_distribution<float> dist(0, 1);
	this->_childChromosome.genes[index] += (this->_childChromosome.maxBounds.maxGenes[index] * (dist(gen) - 0.5)) * 0.1;
};		
void ai::evolution::newGene(int index) {
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 gen(seed);
	std::uniform_real_distribution<float> dist(0, 1);
	this->_childChromosome.genes[index] += (this->_childChromosome.maxBounds.maxGenes[index] * dist(gen));
};
void ai::evolution::crossOver(int index) {
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 gen(seed);
	std::uniform_real_distribution<float> dist(0, 1);
	if (dist(gen) <= 0.5) {
		this->_childChromosome.genes[index] = this->fatherChromosome->genes[index];
	}
	else {
		this->_childChromosome.genes[index] = this->motherChromosome->genes[index];
	}
};
ai::chromosome ai::evolution::returnChild() {
	for (int index = 0; index < sizeof(this->_childChromosome.genes) / sizeof(this->_childChromosome.genes[0]); index++) {
		auto seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::mt19937 gen(seed);
		std::uniform_real_distribution<float> dist(0, 1);
		if (dist(gen) >= 0 && dist(gen) <= this->chance.crossover) {
			this->crossOver(index);
		}
		else if(dist(gen) >= this->chance.crossover && dist(gen) <= this->chance.crossover+this->chance.mutateValue) {
			this->mutate(index);
		}
		else {
			this->newGene(index);
		}
	}
	return this->_childChromosome;
};

// Save the struct to a file
void ai::evolution::saveChromosome(const ai::chromosome& data, const std::string& filename) {
	std::ofstream file(filename, std::ios::binary);
	file.write(reinterpret_cast<const char*>(&data), sizeof(data));
	file.close();
}

// Load the struct from a file
void ai::evolution::loadChromosome(ai::chromosome& data, const std::string& filename) {
	std::ifstream file(filename, std::ios::binary);
	file.read(reinterpret_cast<char*>(&data), sizeof(data));
	file.close();
}

void ai::evolution::setFatherChromosome(ai::chromosome &newFather) {
	this->fatherChromosome = &newFather;
};
void ai::evolution::setMotherChromosome(ai::chromosome &newMother) {
	this->motherChromosome = &newMother;
};
void ai::evolution::setFitness(float fitness) {
};

ai::chromosome ai::evolution::getFatherChromosome() {
	return *this->fatherChromosome;
};
ai::chromosome ai::evolution::getMotherChromosome() {
	return *this->motherChromosome;
};
ai::chromosome ai::evolution::getChildChromosome() {
	return this->_childChromosome;
};