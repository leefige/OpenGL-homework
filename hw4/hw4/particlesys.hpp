#ifndef CG_PARTICLESYS_H_
#define CG_PARTICLESYS_H_

#include <stdlib.h>
#include <cmath>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace cg
{

const float PI = glm::pi<float>();

class Mass
{
public:
	double mass;
	glm::vec3 position;
	glm::vec3 speed;
	glm::vec3 force;
	double fadeSpeed;
	double life;

public:
	Mass() : mass(1.0f), position{0}, speed(0), force{0}, fadeSpeed(0), life(0) {}

	Mass(double mass, glm::vec3 position, glm::vec3 speed, double life) :
		mass(mass), force{0}, position{position}, speed(speed),fadeSpeed(0), life(life)
	{
	}

	void Init()
	{
		force = glm::vec3(0.0, 0.0, 0.0);
	}

	void Update(double dt)
	{
		speed += force / GLfloat(mass) * GLfloat(dt);
		position += speed * GLfloat(dt);
	}

	void ApplyForce(glm::vec3 newForce)
	{
		this->force += newForce;
	}

	virtual void Apply() {}

	void Process(double dt)
	{
		Apply();
		Update(dt);
	}
};

class BunchofMass
{
protected:
	int massNum;
	Mass** masses;

public:
	BunchofMass(int massNum, double m, glm::vec3 position, glm::vec3 speed, double life)
	{
		this->massNum = massNum;
		masses = new Mass* [massNum];
		if (masses) {
			for (int i = 0; i < massNum; ++i) {
				masses[i] = new Mass(m, position, speed, life);
			}
		}
	}

	~BunchofMass()
	{
		if (masses) {
			for (int i = 0; i < massNum; ++i)
				delete[] masses[i];
			delete[] masses;
		}
	}

	Mass* GetMass(int idx)
	{
		return masses[idx];
	}

	void Init()
	{
		for (int i = 0; i < massNum; ++i)
			masses[i]->Init(); //set force to zero
	}

	virtual void Apply() {}

	virtual void Update(double dt, glm::vec3 newpos)
	{
		for (int i = 0; i < massNum; ++i)
			masses[i]->Update(dt); //update speed and position
								   // speed += (F/m) * dt
								   // position += speed * dt
	}

	void Process(double dt, glm::vec3 newpos)
	{
		Init(); // set force to zero
		Apply(); // apply force (add force)
		Update(dt, newpos); // update speed and position
	}
};

class FireWork : public BunchofMass
{
	glm::vec3 gravAcceleratio, constGravAcceleratio;
	double lifeLeft;

	glm::vec3 initialSpeed;
	glm::vec3 initialPosition;
	double initialLife;

public:
	bool hasExploded;

public:
	FireWork(int massNum, double m, glm::vec3 position, glm::vec3 speed,
			 glm::vec3 gravity, double lifeValue)
		: BunchofMass(massNum, m, position, speed, lifeValue), hasExploded(false)
	{
		gravAcceleratio = gravity;
		constGravAcceleratio = gravity;
		lifeLeft = lifeValue;

		initialPosition = position;
		initialSpeed = speed;
		initialLife = lifeLeft;

		double r = rand() % 255;
		double g = rand() % 255;
		double b = rand() % 255;
		for (int i = 0; i < massNum; ++i) 		{
			masses[i]->color = glm::vec4(r / 255., g / 255., b / 255., 1.0);
			masses[i]->fadeSpeed = rand() % 2 ? 50 / 255. : 40 / 255.;
			masses[i]->life = lifeLeft;
		}
	}

	~FireWork()
	{
	}

	int GetMassNum()
	{
		return massNum;
	}

	virtual void Apply()
	{
		for (int i = 0; i < massNum; ++i) // apply gravity
		{
			masses[i]->ApplyForce(gravAcceleratio * GLfloat(masses[i]->mass));
		}

		if (!hasExploded && masses[0]->speed[1] < 10) // time to explode
		{
			for (int i = 0; i < massNum; ++i) // simulate uniform spray
			{
				double radius = rand() % 30;
				double angle1 = rand() % 360;
				double angle2 = rand() % 360;

				//a random direction
				glm::vec3 dir = glm::vec3(radius * cos(angle2 * PI / 180) * cos(angle1 * PI / 180),
										  radius * cos(angle2 * PI / 180) * sin(angle1 * PI / 180),
										  radius * sin(angle2 * PI / 180));
				dir = glm::normalize(dir);
				dir *= 10;

				gravAcceleratio = glm::vec3(0, 0, 0);
				masses[i]->speed = glm::vec3(0, 0, 0);
				masses[i]->speed += dir;
				hasExploded = true;
			}
		}
	}

	virtual void Update(double dt, glm::vec3 newPos)
	{
		BunchofMass::Update(dt, newPos); // update speed and position
		lifeLeft -= dt;

		if (hasExploded) 		{
			for (int i = 0; i < massNum; ++i) {
				masses[i]->color.a -= masses[i]->fadeSpeed * dt; // here alpha is transparency
				if (masses[i]->color.a < 0) masses[i]->color.a = 0;
			}
		}
		if (lifeLeft < 1e-2) // revive
		{
			double r = rand() % 255;
			double g = rand() % 255;
			double b = rand() % 255;
			for (int i = 0; i < massNum; ++i) {
				masses[i]->speed = initialSpeed;
				masses[i]->color = glm::vec4(r / 255., g / 255., b / 255., 1.0);
				masses[i]->position = newPos;
			}
			gravAcceleratio = constGravAcceleratio;
			lifeLeft = initialLife;
			hasExploded = false;
		}
	}
};

} /* namespace cg */

#endif /* CG_PARTICLESYS_H_ */
