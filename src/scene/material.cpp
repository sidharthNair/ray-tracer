#include "material.h"
#include "../ui/TraceUI.h"
#include "light.h"
#include "ray.h"
extern TraceUI* traceUI;

#include <glm/gtx/io.hpp>
#include <iostream>
#include "../fileio/images.h"

using namespace std;
extern bool debugMode;

Material::~Material()
{
}

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
glm::dvec3 Material::shade(Scene* scene, const ray& r, const isect& i) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
	// You will need to call both distanceAttenuation() and
	// shadowAttenuation()
	// somewhere in your code in order to compute shadows and light falloff.
	//	if( debugMode )
	//		std::cout << "Debugging Phong code..." << std::endl;

	// When you're iterating through the lights,
	// you'll want to use code that looks something
	// like this:
	//
	// for ( const auto& pLight : scene->getAllLights() )
	// {
	//              // pLight has type unique_ptr<Light>
	// 		.
	// 		.
	// 		.
	// }

	// Phong Shading Model: K_a*I_scene + (K_d * |l . n| + K_s*(max(v . r, 0))^alpha)*I_in

	// Emissive and Ambient Term
	glm::dvec3 color = ke(i) + ka(i) * scene->ambient();
	glm::dvec3 v = -r.getDirection();
	glm::dvec3 normal = i.getN();
    if (glm::dot(v, normal) < 0) {
        normal = -normal;
    }
	glm::dvec3 position = r.at(i) + RAY_EPSILON * normal;
	for (const auto& lightSource : scene->getAllLights()) {
		glm::dvec3 light = lightSource->getDirection(position);
		double ln = glm::dot(light, normal);
		double vr = glm::dot(v, 2 * ln * normal - light);
		glm::dvec3 I_in = lightSource->distanceAttenuation(r.at(i)) *
						  lightSource->shadowAttenuation(r, position) *
						  lightSource->getColor();
		// Diffuse and Specular Terms
        // For transparent materials we consider diffuse lighting coming from both sides
        // of the surface. Otherwise, we only consider positive terms.
		color += I_in * (kd(i) * (i.getMaterial().Trans() ? abs(ln) : max(ln, 0.0)) +
                         ks(i) * pow(max(vr, 0.0), shininess(i)));
	}
	return color;
}

TextureMap::TextureMap(string filename)
{
	data = readImage(filename.c_str(), width, height);
	if (data.empty()) {
		width = 0;
		height = 0;
		string error("Unable to load texture map '");
		error.append(filename);
		error.append("'.");
		throw TextureMapException(error);
	}
}

glm::dvec3 TextureMap::getMappedValue(const glm::dvec2& coord) const
{
	// YOUR CODE HERE
	//
	// In order to add texture mapping support to the
	// raytracer, you need to implement this function.
	// What this function should do is convert from
	// parametric space which is the unit square
	// [0, 1] x [0, 1] in 2-space to bitmap coordinates,
	// and use these to perform bilinear interpolation
	// of the values.

	double x = coord[0] * double(width - 1);
	double y = coord[1] * double(height - 1);

	int x0 = int(x), x1 = x0 + 1;
	int y0 = int(y), y1 = y0 + 1;
	glm::dvec3 c00 = getPixelAt(x0, y0);
	glm::dvec3 c01 = getPixelAt(x0, y1);
	glm::dvec3 c10 = getPixelAt(x1, y0);
	glm::dvec3 c11 = getPixelAt(x1, y1);
	glm::dvec3 color = c00 * (double(x1) - x) * (double(y1) - y) +
                        c10 * (x - double(x0)) * (double(y1) - y) +
                        c01 * (double(x1) - x) * (y - double(y0)) +
                        c11 * (x - double(x0)) * (y - double(y0));

	return color;
}

glm::dvec3 TextureMap::getPixelAt(int x, int y) const
{
	// YOUR CODE HERE
	//
	// In order to add texture mapping support to the
	// raytracer, you need to implement this function.
    glm::dvec3 color(0, 0, 0);
	if(x < 0 || x >= width || y < 0 || y >= height || data.size() < 3 * width * height)
		return color;
	int offset = y * width + x;
	for(int k = 0; k < 3; k++)
		color[k] = double(data[3 * offset + k]);
	return color / 255.0;
}

glm::dvec3 MaterialParameter::value(const isect& is) const
{
	if (0 != _textureMap)
		return _textureMap->getMappedValue(is.getUVCoordinates());
	else
		return _value;
}

double MaterialParameter::intensityValue(const isect& is) const
{
	if (0 != _textureMap) {
		glm::dvec3 value(
		        _textureMap->getMappedValue(is.getUVCoordinates()));
		return (0.299 * value[0]) + (0.587 * value[1]) +
		       (0.114 * value[2]);
	} else
		return (0.299 * _value[0]) + (0.587 * _value[1]) +
		       (0.114 * _value[2]);
}
