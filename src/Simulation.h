// Simulation.h
#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <glm/glm.hpp>
#include "Object.h"
#include "Constants.h"

// Функции симуляции
std::vector<float> CreateGridVertices(float size, int divisions);
float CalculateVerticalShift(const std::vector<Object>& objs);
std::vector<float> UpdateGridVertices(const std::vector<float>& initialVertices, const std::vector<Object>& objs, float verticalShift, float deformationStrength);
void UpdatePhysics(std::vector<Object>& objs, float dt);

#endif // SIMULATION_H 