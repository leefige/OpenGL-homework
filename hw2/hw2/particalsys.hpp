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

class Mass
{
public:
	float mass;
	glm::vec3 position;
	glm::vec3 speed;
	glm::vec3 force;
	glm::vec4 color;
	float fadeSpeed;
	float life;

public:
	Mass() : mass(0), speed(0), position(0), force(0), color(0), fadeSpeed(0), life(0) { }

	Mass(float mass, glm::vec3 position, glm::vec3 speed, float life) : color(0), force(0), fadeSpeed(0)
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
	void Update(float dt)
	{
		speed += force / GLfloat(mass) * GLfloat(dt);
		position += speed * GLfloat(dt);
		life -= dt;
	}

	void ApplyForce(glm::vec3 newForce)
	{
		this->force += newForce;
	}

	virtual void Apply() { };

	void Process(float dt)
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
	BunchOfMass(int massNum, float m, glm::vec3 position, glm::vec3 speed, float life) : massNum(massNum)
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

	Mass& operator[](int idx)
	{
		return masses[idx];
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

	virtual void Update(float dt, glm::vec3 newpos)
	{
		for (int i = 0; i < massNum; ++i) {
			masses[i].Update(dt); 
		}
	}

	void Process(float dt, glm::vec3 newpos)
	{
		Init(); // set force to zero
		Apply(); // apply force (add force)
		Update(dt, newpos); // update speed and position
	}
};

// =======================================================================================

class ArchimedesRad : public BunchOfMass
{
	glm::vec3 initialPosition;
	glm::vec3 initialSpeed;
	float initialLife;
	float fadeSpeed;
	float period;

	float myLife;
	float myRad;
	float myDegree;
	glm::vec3 myColor;

public:
	/* massNum: each rad consists of massNum particle
	 * position: initial position
	 * speed: initial speed
	 * gravity: gravity is (0, -9.8, 0)
	 * lifeValue: existing time in seconds
	*/
	ArchimedesRad(int massNum, glm::vec3 position, float speed, float degree, float period) :
		BunchOfMass(massNum, 1, position, glm::vec3{0}, period),
		period(period), myRad(glm::radians(degree)), myDegree(degree)
	{
		initialPosition = glm::vec3{position.x, position.y, 0};

		initialLife = period * (massNum - 1);

		fadeSpeed = 1. / initialLife;

		myLife = period * (1 - degree / 360.0f);

		glm::vec3 dir = glm::vec3(cos(myRad), sin(myRad), 0);
		dir = glm::normalize(dir);
		initialSpeed = dir * float(speed);

		float r = cos(myRad) / 2 + 0.5;
		float g = cos(myRad + glm::radians(120.0)) / 2 + 0.5;
		float b = cos(myRad - glm::radians(120.0)) / 2 + 0.5;

		myColor = glm::vec3{r, g, b};

		for (int i = 0; i < massNum; ++i) {
			resetMass(
				masses[i],
				initialPosition + initialSpeed * period * (i + degree / 360.0f),
				initialLife - period * (i + degree / 360.0f + 1)
			);
		}
	}

	int GetMassNum() const
	{
		return massNum;
	}

	virtual void Apply()
	{
		//for (int i = 0; i < massNum; ++i) // apply gravity
		//{
		//	masses[i].ApplyForce(gravAcceleratio * GLfloat(masses[i].mass));
		//}

		//for (int i = 0; i < massNum; ++i) // simulate uniform spray
		//{
		//	
		//}
	}

	virtual void Update(float dt, glm::vec3 newPos)
	{
		// update speed and position
		BunchOfMass::Update(dt, newPos);

		myLife -= dt;

		for (int i = 0; i < massNum; ++i) {
			// here alpha is transparency
			masses[i].color.a -= float(masses[i].fadeSpeed * dt);
			if (masses[i].color.a < 0) {
				masses[i].color.a = 0;
			}
		}

		// revive
		
		if (myLife < 1e-2) {
			for (int i = 0; i < massNum; ++i) {
				std::cout << i << " revive" << std::endl;
				//int j = (i + 1) % massNum;
				resetMass(
					masses[i],
					initialPosition + initialSpeed * (period * i),
					initialLife - period * i
				);
			}
			myLife = period;
		}
	}

	void resetMass(Mass& mass, const glm::vec3& pos, float life)
	{
		mass.color = glm::vec4{myColor.r, myColor.g, myColor.b, life / initialLife};
		mass.fadeSpeed = fadeSpeed;
		mass.life = period;
		mass.speed = initialSpeed;

		mass.position = pos;
		mass.life = life;
	}
};

class Archimedes
{

};

}

#endif // CG_PARTICALSYS_H_

