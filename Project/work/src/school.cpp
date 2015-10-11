//---------------------------------------------------------------------------
//
// Francesco Badraun 2015
//
//----------------------------------------------------------------------------

#include <cmath>
#include <iostream> // input/output streams
#include <string>
#include <stdexcept>
#include <vector>
#include <cstdlib> // for rand and srand
#include <ctime> // for time

#include "comp308.hpp"
#include "school.hpp"
#include "fish.hpp"

using namespace std;
using namespace comp308;

School::School() {
	// init fish
	int i = 0;
	for (; i < fishAmount; i++) {
		Fish fish = Fish();

		schoolOfFish.push_back(fish);
	}

	srand (static_cast <unsigned> (time(0))); // seed random number
	initialisePositions(); // place fish around scene
}

void School::update(bool play) {
	renderSchool();

	if (step) {
		moveAllFishToNewPositions();
		step = false;
	} else if (play) {
		moveAllFishToNewPositions();
	}
}

void School::renderSchool() {
	// render every fish
	for(vector<Fish>::iterator it = schoolOfFish.begin(); it != schoolOfFish.end(); ++it) {
    	it->renderFish();
	}
	
	// render bounds
	renderSphere();
	renderCentreMass();
}

void School::renderSphere() {
	glPushMatrix(); {
		glColor4f(0.3, 0.4, 0.8, 0.5); // transparent blue
		glutWireSphere(sphereRadius, 50, 50);
	} glPopMatrix();
}

void School::renderCentreMass() {
	vec3 centre;
	for (vector<Fish>::iterator it = schoolOfFish.begin(); it != schoolOfFish.end(); ++it) {
		centre += (*it).getPosition();
	}
	centre /= schoolOfFish.size(); // average position of every fish

	glPushMatrix(); {
		glColor3f(0.8, 0.4, 0.4); // bright red
		glTranslatef(centre.x, centre.y, centre.z);

		glutSolidSphere(0.5, 10, 10);
	} glPopMatrix();
}

void School::initialisePositions() {
	// places fish randomly on the surface of the sphere

	// generate random x,y,z values just outside the sphere
	float high = sphereRadius;
	float low = -high;

	for(vector<Fish>::iterator it = schoolOfFish.begin(); it != schoolOfFish.end(); ++it) {

		float x = low + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(high - low)));
		float y = low + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(high - low)));
		float z = low + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(high - low)));

		// create new position vector
		vec3 newPos = vec3(x, y, z);

		newPos /= length(newPos); // normalised

		newPos *= sphereRadius + schoolOfFish.front().fishLength;

	    it->setPosition(newPos);

	    newPos *= 0.01;
	    it->setVelocity(-newPos);
	}
}
/*
	Actual boids algorithm
*/
void School::moveAllFishToNewPositions() {

	vec3 v1, v2, v3; // the 3 main rules for a boid

	for (vector<Fish>::iterator it = schoolOfFish.begin(); it != schoolOfFish.end(); ++it) {

		Fish *fish = &(*it); // &(*it) is an address ('&') to the dereferenced pointer ('(*it)'), which is a pointer

		v1 = rule1(fish);
		v2 = rule2(fish);
		v3 = rule3(fish);

		vec3 velocity = fish->getVelocity() + v1 + v2 + v3;
		vec3 position = fish->getPosition() + velocity;

		fish->setPosition(position);
		fish->setVelocity(velocity);

		// move fish to opposite side of bounds if it goes past bounds
		if (isBoundsCollided(*it)) {
			moveFishToOppositeOfBounds(fish);
		}
	}
}

/*
	Cohesion

	Rule 1: Boids try to fly towards the centre of mass of neighbouring boids.
	
	This uses the 'perceived centre' which is the centre of all the other fish, not including itself.
*/
vec3 School::rule1(Fish *fj) {

	vec3 pcj; // perceived centre, (centre of every fish not including fj)

	for (vector<Fish>::iterator it = schoolOfFish.begin(); it != schoolOfFish.end(); ++it) {
		Fish *f = &(*it);

		if (f != fj) {
			pcj = pcj + f->getPosition();
		}
	}

	pcj = pcj / (schoolOfFish.size() - 1);

	return (pcj - fj->getPosition()) / 1000; // gives a vector which moves fish 1% of the way towards the centre
}

/*
	Separation

	Rule 2: Boids try to keep a small distance away from other objects (including other boids).
*/
vec3 School::rule2(Fish *fj) {
	float minDistance = 1.0;

	vec3 c = vec3();

	for (vector<Fish>::iterator it = schoolOfFish.begin(); it != schoolOfFish.end(); ++it) {
		Fish *f = &(*it);

		if (f != fj) {
			vec3 distanceBetweenFish = f->getPosition() - fj->getPosition();

			if (length(distanceBetweenFish) < minDistance) {
				c = c - distanceBetweenFish;
			}
		}
	}

	return c * 0.1; // 0.1 to lessen the amount of influence the vector has
}

/*
	Alignment

	Rule 3: Boids try to match velocity with near boids.

	Similar to rule 1, this uses the 'perceived velocity' which is the average velocity of all the other fish, not including itself.
*/
vec3 School::rule3(Fish *fj) {

	vec3 pvj; // perceived velocity, (velocity of every fish not including fj)

	for (vector<Fish>::iterator it = schoolOfFish.begin(); it != schoolOfFish.end(); ++it) {
		Fish *f = &(*it);

		if (f != fj) {
			pvj = pvj + f->getVelocity();
		}
	}

	pvj = pvj / (schoolOfFish.size() - 1);

	return (pvj - fj->getVelocity()) / 8;
}

bool School::isBoundsCollided(Fish fish) {
	vec3 position = fish.getPosition();
	vec3 origin = vec3(0, 0, 0);

	vec3 distanceVector = position - origin;
	float distance = length(distanceVector);

	// how far is the fish allowed to go from the origin(center)
	float allowance = sphereRadius + fish.fishLength + 0.001; // 0.001 is so it doesn't collide when in its initialisePositions() position

	if (distance >= allowance) {
		return true;
	}

	return false;
}

void School::moveFishToOppositeOfBounds(Fish* fish) {
	vec3 position = fish->getPosition();
	position /= length(position);
	position *= (sphereRadius + fish->fishLength);

	fish->setPosition(-position); // negative position is mirrored position (opposite)
}
