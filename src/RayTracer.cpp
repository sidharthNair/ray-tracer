// The main ray tracer.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

#include "ui/TraceUI.h"
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <string.h> // for memset

#include <iostream>
#include <fstream>

using namespace std;
extern TraceUI* traceUI;

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = false;

// Trace a top-level ray through pixel(i,j), i.e. normalized window coordinates (x,y),
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.

glm::dvec3 RayTracer::trace(double x, double y)
{
	// Clear out the ray cache in the scene for debugging purposes,
	if (TraceUI::m_debug)
	{
		scene->clearIntersectCache();
	}

	ray r(glm::dvec3(0,0,0), glm::dvec3(0,0,0), glm::dvec3(1,1,1), ray::VISIBILITY);
	scene->getCamera().rayThrough(x,y,r);
	double dummy;
	glm::dvec3 ret = traceRay(r, glm::dvec3(1.0,1.0,1.0), traceUI->getDepth(), dummy);
	ret = glm::clamp(ret, 0.0, 1.0);
	return ret;
}

glm::dvec3 RayTracer::tracePixel(int i, int j)
{
	glm::dvec3 col(0,0,0);

	if( !sceneLoaded() ) return col;

	double x = double(i) / double(buffer_width);
	double y = double(j) / double(buffer_height);

	unsigned char *pixel = buffer.data() + ( i + j * buffer_width ) * 3;
	col = trace(x, y);

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
	return col;
}

glm::dvec3 RayTracer::tracePixelAA(int i, int j) {
    glm::dvec3 col(0,0,0);

	if( !sceneLoaded() ) return col;

	double x = double(i) / double(buffer_width);
	double y = double(j) / double(buffer_height);
    double d_x = 1.0 / (double(buffer_width) * double(samples)) ;
    double d_y = 1.0 / (double(buffer_height) * double(samples));

    for (int x_s = 0; x_s < samples; x_s++) {
        double x_sample = x + x_s * d_x;
        for (int y_s = 0; y_s < samples; y_s++) {
            double y_sample = y + y_s * d_y;
            col += trace(x_sample, y_sample);
        }
    }
    col /= (samples * samples);

	unsigned char *pixel = buffer.data() + ( i + j * buffer_width ) * 3;
	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
	return col;
}

#define VERBOSE 0

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
glm::dvec3 RayTracer::traceRay(ray& r, const glm::dvec3& thresh, int depth, double& t )
{
	// Base Case
	if (depth < 0) {
		return glm::dvec3(0.0, 0.0, 0.0);
	}

	isect i;
	glm::dvec3 colorC;
#if VERBOSE
	std::cerr << "== current depth: " << depth << std::endl;
#endif

	if(scene->intersect(r, i)) {
		// YOUR CODE HERE

		// An intersection occurred!  We've got work to do.  For now,
		// this code gets the material for the surface that was intersected,
		// and asks that material to provide a color for the ray.

		// This is a great place to insert code for recursive ray tracing.
		// Instead of just returning the result of shade(), add some
		// more steps: add in the contributions from reflected and refracted
		// rays.

        // Get the intersection time for refraction distance
        t = i.getT();

		const Material& m = i.getMaterial();
		colorC = m.shade(scene.get(), r, i);

		glm::dvec3 normal = i.getN();
		glm::dvec3 reversed = -r.getDirection();

		bool outsideSurface = glm::dot(normal, reversed) > 0;

		if (m.Refl() && outsideSurface) {
			// Reflective surface hit from the front
			glm::dvec3 position = r.at(i) + RAY_EPSILON * normal;
			glm::dvec3 reflected = 2 * glm::dot(reversed, normal) * normal - reversed;
			ray reflectedRay = ray(position, reflected, glm::dvec3(1.0, 1.0, 1.0), ray::REFLECTION);
            double dummy = 0;
			colorC += m.kr(i) * traceRay(reflectedRay, thresh, depth - 1, dummy);
		}

		if (m.Trans()) {
			// Refraction
			double eta = outsideSurface ? 1.0 / m.index(i) : m.index(i);
            if (!outsideSurface) {
                // Inside the surface, so we need to reverse the normal vector
                normal = -normal;
            }
			double cos_i = glm::dot(reversed, normal);
			double temp = 1.0 - eta * eta * (1 - cos_i * cos_i);
			if (temp > 0) {
				// Standard refraction case
				double cos_t = sqrt(temp);
                // Adjust the position to be on the other side of the boundary
				glm::dvec3 position = r.at(i) - RAY_EPSILON * normal;
				glm::dvec3 refracted = (eta * cos_i - cos_t) * normal - eta * reversed;
				ray refractedRay = ray(position, refracted, glm::dvec3(1.0, 1.0, 1.0), ray::REFRACTION);
                double distance = 0;
				glm::dvec3 tempColorC = traceRay(refractedRay, thresh, depth - 1, distance);
                if (outsideSurface) {
                    // Adjust the color based on the distance of refracted ray in the material
                    glm::dvec3 kt = m.kt(i);
                    tempColorC[0] *= pow(kt[0], distance);
                    tempColorC[1] *= pow(kt[1], distance);
                    tempColorC[2] *= pow(kt[2], distance);
                }
                colorC += tempColorC;
			} else if (temp < 0 && m.Refl()) {
				// Total internal reflection
				glm::dvec3 position = r.at(i) + RAY_EPSILON * normal;
				glm::dvec3 reflected = 2 * glm::dot(reversed, normal) * normal - reversed;
				ray reflectedRay = ray(position, reflected, glm::dvec3(1.0, 1.0, 1.0), ray::REFLECTION);
                double dummy = 0;
				colorC += m.kr(i) * traceRay(reflectedRay, thresh, depth - 1, dummy);
			}
		}
	} else {
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.
		//
		// FIXME: Add CubeMap support here.
		// TIPS: CubeMap object can be fetched from traceUI->getCubeMap();
		//       Check traceUI->cubeMap() to see if cubeMap is loaded
		//       and enabled.
		if (traceUI->cubeMap()) {
			colorC = traceUI->getCubeMap()->getColor(r);
		} else {
			colorC = glm::dvec3(0.0, 0.0, 0.0);
		}
	}
#if VERBOSE
	std::cerr << "== depth: " << depth+1 << " done, returning: " << colorC << std::endl;
#endif
	return colorC;
}

RayTracer::RayTracer()
	: scene(nullptr), buffer(0), thresh(0), buffer_width(0), buffer_height(0), m_bBufferReady(false)
{
}

RayTracer::~RayTracer()
{
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer.data();
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene(const char* fn)
{
	ifstream ifs(fn);
	if( !ifs ) {
		string msg( "Error: couldn't read scene file " );
		msg.append( fn );
		traceUI->alert( msg );
		return false;
	}

	// Strip off filename, leaving only the path:
	string path( fn );
	if (path.find_last_of( "\\/" ) == string::npos)
		path = ".";
	else
		path = path.substr(0, path.find_last_of( "\\/" ));

	// Call this with 'true' for debug output from the tokenizer
	Tokenizer tokenizer( ifs, false );
	Parser parser( tokenizer, path );
	try {
		scene.reset(parser.parseScene());
	}
	catch( SyntaxErrorException& pe ) {
		traceUI->alert( pe.formattedMessage() );
		return false;
	} catch( ParserException& pe ) {
		string msg( "Parser: fatal exception " );
		msg.append( pe.message() );
		traceUI->alert( msg );
		return false;
	} catch( TextureMapException e ) {
		string msg( "Texture mapping exception: " );
		msg.append( e.message() );
		traceUI->alert( msg );
		return false;
	}

	if (!sceneLoaded())
		return false;

	return true;
}

void RayTracer::traceSetup(int w, int h)
{
	size_t newBufferSize = w * h * 3;
	if (newBufferSize != buffer.size()) {
		bufferSize = newBufferSize;
		buffer.resize(bufferSize);
	}
	buffer_width = w;
	buffer_height = h;
	std::fill(buffer.begin(), buffer.end(), 0);
	m_bBufferReady = true;

	/*
	 * Sync with TraceUI
	 */

	threads = traceUI->getThreads();
	block_size = traceUI->getBlockSize();
	thresh = traceUI->getThreshold();
	samples = traceUI->getSuperSamples();
	aaThresh = traceUI->getAaThreshold();

	// YOUR CODE HERE
	// FIXME: Additional initializations
    scene->buildBVH();
}

void RayTracer::traceThread(unsigned int id)
{
	int num_elements = buffer_width * buffer_height;
	for (int idx = id; idx < num_elements; idx += threads) {
		int i = idx / buffer_height;
		int j = idx % buffer_height;
		glm::dvec3 color = tracePixel(i, j);
		setPixel(i, j, color);
	}
	threads_done++;
}

void RayTracer::antiAliasingThread(unsigned int id) {
    int num_elements = buffer_width * buffer_height;
	for (int idx = id; idx < num_elements; idx += threads) {
		int i = idx / buffer_height;
		int j = idx % buffer_height;
		glm::dvec3 color = tracePixelAA(i, j);
		setPixel(i, j, color);
	}
	threads_done++;
}

/*
 * RayTracer::traceImage
 *
 *	Trace the image and store the pixel data in RayTracer::buffer.
 *
 *	Arguments:
 *		w:	width of the image buffer
 *		h:	height of the image buffer
 *
 */
void RayTracer::traceImage(int w, int h)
{
	// Always call traceSetup before rendering anything.
	traceSetup(w, h);

	// YOUR CODE HERE
	// FIXME: Start one or more threads for ray tracing
	//
	// TIPS: Ideally, the traceImage should be executed asynchronously,
	//       i.e. returns IMMEDIATELY after working threads are launched.
	//
	//       An asynchronous traceImage lets the GUI update your results
	//       while rendering.
	threads_done = 0;
    for (std::thread &t : thread_list) {
		t.detach();
	}
	thread_list.clear();

	for (unsigned int i = 0; i < threads; i++) {
		thread_list.push_back(std::thread(&RayTracer::traceThread, this, i));
	}
}

int RayTracer::aaImage()
{
	// YOUR CODE HERE
	// FIXME: Implement Anti-aliasing here
	//
	// TIP: samples and aaThresh have been synchronized with TraceUI by
	//      RayTracer::traceSetup() function
    threads_done -= threads;
    for (unsigned int i = 0; i < threads; i++) {
		thread_list.push_back(std::thread(&RayTracer::antiAliasingThread, this, i));
	}
	return 0;
}

bool RayTracer::checkRender()
{
	// YOUR CODE HERE
	// FIXME: Return true if tracing is done.
	//        This is a helper routine for GUI.
	//
	// TIPS: Introduce an array to track the status of each worker thread.
	//       This array is maintained by the worker threads.
	return threads_done == threads;
}

void RayTracer::waitRender()
{
	// YOUR CODE HERE
	// FIXME: Wait until the rendering process is done.
	//        This function is essential if you are using an asynchronous
	//        traceImage implementation.
	//
	// TIPS: Join all worker threads here.
	for (std::thread &t : thread_list) {
		t.join();
	}
}


glm::dvec3 RayTracer::getPixel(int i, int j)
{
	unsigned char *pixel = buffer.data() + ( i + j * buffer_width ) * 3;
	return glm::dvec3((double)pixel[0]/255.0, (double)pixel[1]/255.0, (double)pixel[2]/255.0);
}

void RayTracer::setPixel(int i, int j, glm::dvec3 color)
{
	unsigned char *pixel = buffer.data() + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * color[0]);
	pixel[1] = (int)( 255.0 * color[1]);
	pixel[2] = (int)( 255.0 * color[2]);
}

