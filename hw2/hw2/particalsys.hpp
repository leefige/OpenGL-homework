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

}

#endif /* CG_PARTICALSYS_H_ */

