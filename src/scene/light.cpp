#include <cmath>
#include <iostream>

#include "light.h"
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>


using namespace std;

double DirectionalLight::distanceAttenuation(const glm::dvec3& P) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


glm::dvec3 DirectionalLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.
	ray shadowRay = ray(p, getDirection(p), glm::dvec3(1.0, 1.0, 1.0), ray::SHADOW);
	isect i;
	glm::dvec3 intensity = glm::dvec3(1.0, 1.0, 1.0);
	// Loop for all intersections
	while (scene->intersect(shadowRay, i)) {
		if (!i.getMaterial().Trans()) {
			// Found opaque object on path
			return glm::dvec3(0.0, 0.0, 0.0);
		} else if (glm::dot(shadowRay.getDirection(), i.getN()) > 0) {
			// Entering translucent object on path
			glm::dvec3 kt = i.getMaterial().kt(i);
			intensity[0] *= pow(kt[0], i.getT());
			intensity[1] *= pow(kt[1], i.getT());
			intensity[2] *= pow(kt[2], i.getT());
		shadowRay.setPosition(shadowRay.at(i) + i.getN() * RAY_EPSILON);
		} else {
			// Leaving translucent object on path
			shadowRay.setPosition(shadowRay.at(i) - i.getN() * RAY_EPSILON);
		}
	}
	return intensity;
}

glm::dvec3 DirectionalLight::getColor() const
{
	return color;
}

glm::dvec3 DirectionalLight::getDirection(const glm::dvec3& P) const
{
	return -orientation;
}

double PointLight::distanceAttenuation(const glm::dvec3& P) const
{

	// YOUR CODE HERE

	// You'll need to modify this method to attenuate the intensity
	// of the light based on the distance between the source and the
	// point P.  For now, we assume no attenuation and just return 1.0
	double distance = glm::distance(position, P);
	return min(1.0, 1.0 / (constantTerm + linearTerm * distance + quadraticTerm * (distance * distance)));
}

glm::dvec3 PointLight::getColor() const
{
	return color;
}

glm::dvec3 PointLight::getDirection(const glm::dvec3& P) const
{
	return glm::normalize(position - P);
}


glm::dvec3 PointLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.
	ray shadowRay = ray(p, getDirection(p), glm::dvec3(1.0, 1.0, 1.0), ray::SHADOW);
	isect i;
	glm::dvec3 intensity = glm::dvec3(1.0, 1.0, 1.0);
	double length = glm::distance(p, position);
	// Loop for all intersections
	while (scene->intersect(shadowRay, i)) {
		if (length - i.getT() < 0) {
			return intensity;
		} else if (!i.getMaterial().Trans()) {
			return glm::dvec3(0.0, 0.0, 0.0);
		} else if (glm::dot(shadowRay.getDirection(), i.getN()) > 0) {
            // Leaving translucent object on path
			glm::dvec3 kt = i.getMaterial().kt(i);
			intensity[0] *= pow(kt[0], i.getT());
			intensity[1] *= pow(kt[1], i.getT());
			intensity[2] *= pow(kt[2], i.getT());
		    shadowRay.setPosition(shadowRay.at(i) + RAY_EPSILON * i.getN());
		} else {
			// Entering translucent object on path
			shadowRay.setPosition(shadowRay.at(i) - RAY_EPSILON * i.getN());
		}
	}
	return intensity;
}

#define VERBOSE 0

