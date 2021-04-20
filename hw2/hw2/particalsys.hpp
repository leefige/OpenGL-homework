#ifndef CG_PARTICALSYS_H_
#define CG_PARTICALSYS_H_

#include <cmath>
#include <cstdlib>
#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace cg
{

constexpr const double PI = 3.14159265358979;

class Mass
{
public:
	double mass;
	glm::vec3 position;
	glm::vec3 speed;
	glm::vec3 force;
	glm::vec4 color;
	double fadeSpeed;
	double life;

public:
	Mass() : mass(0), speed(0), position(0), force(0), color(0), fadeSpeed(0), life(0) { }

	Mass(double mass, glm::vec3 position, glm::vec3 speed, double life) : color(0), force(0), fadeSpeed(0)
	{
		this->mass = mass;
		this->position = position;
		this->speed = speed;
		this->life = life;
	}

	void Init()
	{
		force = glm::vec3(0);
	}

	/*
	 * update speed and position
	 * speed += (F/m) * dt
	 * position += speed * dt
	*/
	void Update(double dt)
	{
		speed += force / GLfloat(mass) * GLfloat(dt);
		position += speed * GLfloat(dt);
	}

	void ApplyForce(glm::vec3 newForce)
	{
		this->force += newForce;
	}

	virtual void Apply() { };

	void Process(double dt)
	{
		Init();
		Apply();
		Update(dt);
	}
};

class BunchOfMass
{
protected:
	int massNum;
	std::vector<Mass> masses;

public:
	BunchOfMass(int massNum, double m, glm::vec3 position, glm::vec3 speed, double life) : massNum(massNum)
	{
		masses.reserve(massNum);
		for (int i = 0; i < massNum; ++i) {
			masses.emplace_back(m, position, speed, life);
		}
	}

	virtual ~BunchOfMass()
	{
		masses.clear();
	}

	const Mass& GetMass(int idx) const
	{
		return masses[idx];
	}

	void Init()
	{
		for (int i = 0; i < massNum; ++i) {
			masses[i].Init(); //set force to zero
		}
	}

	virtual void Apply() = 0;

	virtual void Update(double dt, glm::vec3 newpos)
	{
		for (int i = 0; i < massNum; ++i) {
			
			masses[i].Update(dt); 
		}
	}

	void Process(double dt, glm::vec3 newpos)
	{
		Init(); // set force to zero
		Apply(); // apply force (add force)
		Update(dt, newpos); // update speed and position
	}
};

class Archimedes : public BunchOfMass
{
	glm::vec3 gravAcceleratio, constGravAcceleratio;
	double lifeLeft;

	glm::vec3 initialSpeed;
	glm::vec3 initialPosition;
	double initialLife;

public:
	bool hasExploded;

public:
	/* massNum: each firework consists of massNum particle
	 * mass: each particle's mass
	 * position: initial position
	 * speed: initial speed
	 * gravity: gravity is (0, -9.8, 0)
	 * lifeValue: existing time in seconds
	*/
	Archimedes(int massNum, double m, glm::vec3 position, glm::vec3 speed,
			 glm::vec3 gravity, double lifeValue) :
		BunchOfMass(massNum, m, position, speed, lifeValue), hasExploded(false)
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
			masses[i].color = glm::vec4(r / 255., g / 255., b / 255., 1.0);
			masses[i].fadeSpeed = rand() % 2 ? 50 / 255. : 40 / 255.;
			masses[i].life = lifeLeft;
		}
	}

	int GetMassNum() const
	{
		return massNum;
	}

	virtual void Apply()
	{
		for (int i = 0; i < massNum; ++i) // apply gravity
		{
			masses[i].ApplyForce(gravAcceleratio * GLfloat(masses[i].mass));
		}

		if (!hasExploded && masses[0].speed[1] < 10) // time to explode
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
				masses[i].speed = glm::vec3(0, 0, 0);
				masses[i].speed += dir;
				hasExploded = true;
			}
		}
	}

	virtual void Update(double dt, glm::vec3 newPos)
	{
		// update speed and position
		BunchOfMass::Update(dt, newPos);

		lifeLeft -= dt;

		if (hasExploded) {
			for (int i = 0; i < massNum; ++i) {
				// here alpha is transparency
				masses[i].color.a -= float(masses[i].fadeSpeed * dt);
				if (masses[i].color.a < 0) masses[i].color.a = 0;
			}
		}
		if (lifeLeft < 1e-2) // revive
		{
			double r = rand() % 255;
			double g = rand() % 255;
			double b = rand() % 255;
			for (int i = 0; i < massNum; ++i) {
				masses[i].speed = initialSpeed;
				masses[i].color = glm::vec4(r / 255., g / 255., b / 255., 1.0);
				masses[i].position = newPos;
			}
			gravAcceleratio = constGravAcceleratio;
			lifeLeft = initialLife;
			hasExploded = false;
		}
	}
};

}

#endif // CG_PARTICALSYS_H_

