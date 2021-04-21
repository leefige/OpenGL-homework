#ifndef CG_SPIRALS_H_
#define CG_SPIRALS_H_

#include "particalsys.hpp"

namespace cg
{

class SpiralBase : public BunchOfMass
{
protected:
	glm::vec3 initialPosition;
	glm::vec3 initialSpeed;
	float initialLife;
	float fadeSpeed;
	float period;

	float myLife;
	float myRad;
	float myDegree;
	glm::vec3 myDirection;
	glm::vec3 myColor;

public:
	/* massNum: each rad consists of massNum particle
	 * position: initial position
	 * speed: initial speed
	 * gravity: gravity is (0, -9.8, 0)
	 * lifeValue: existing time in seconds
	*/
	SpiralBase(int massNum, glm::vec3 position, float speed, float degree, float period) :
		BunchOfMass(massNum, 1, position, glm::vec3{0}, period),
		period(period), myRad(glm::radians(degree)), myDegree(degree)
	{
		initialPosition = glm::vec3{position.x, position.y, 0};
		initialLife = period * (massNum - 1);
		fadeSpeed = 1. / initialLife;
		myLife = period * (1 - degree / 360.0f);

		float r = cos(myRad) / 2 + 0.5;
		float g = cos(myRad + glm::radians(120.0)) / 2 + 0.5;
		float b = cos(myRad - glm::radians(120.0)) / 2 + 0.5;
		myColor = glm::vec3{r, g, b};

		glm::vec3 dir = glm::vec3(cos(myRad), sin(myRad), 0);
		myDirection = glm::normalize(dir);
		initialSpeed = myDirection * float(speed);
	}

	int GetMassNum() const
	{
		return massNum;
	}

protected:
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

template<typename T>
class Spiral
{
	std::vector<std::unique_ptr<SpiralBase> > rads;
	int numMass;
	int numRad;
	float speed;
	float period;
	float spriteScale;

	Mass origin;

	Spiral() = delete;

public:
	explicit Spiral(int numRad, int numMass, float scale, float speed, float period) :
		numRad(ceil(360.0f / numRad)), speed(speed), period(period), numMass(numMass), spriteScale(scale)
	{
		auto degree = 360.0 / numRad;
		for (int i = 0; i < numRad; i++) {
			rads.emplace_back(std::make_unique<T>(numMass, glm::vec3(0, 0, 0), speed, i * degree, period));
		}

		origin.color = glm::vec4(1.0f);
		origin.position = glm::vec3(0, 0, 400.0f);
	}

	void Process(float dt, glm::vec3 newpos)
	{
		for (auto& spawner : rads) {
			// let particle system update
			spawner->Process(dt, {0, 0, 0}); // apply gravity and update speed, position

		}
	}

	void Draw(const Shader& shader, GLuint VAO, GLuint texture) const
	{
		//drawMass(origin, shader, VAO, texture);
		for (auto& spawner : rads) {
			for (int j = 0; j < spawner->GetMassNum(); ++j) {
				drawMass(spawner->GetMass(j), shader, VAO, texture);
			}
		}
	}

private:
	void drawMass(const Mass& mass, const Shader& shader, GLuint VAO, GLuint texture) const
	{
		glUniform2fv(glGetUniformLocation(shader.Program(), "offset"), 1, glm::value_ptr(mass.position));
		glUniform4fv(glGetUniformLocation(shader.Program(), "color"), 1, glm::value_ptr(mass.color));
		glUniform1f(glGetUniformLocation(shader.Program(), "scale"), spriteScale);

		glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
};

// =====================================================================

class ArchimedesRad : public SpiralBase
{
public:
	/* massNum: each rad consists of massNum particle
	 * position: initial position
	 * speed: initial speed
	 * gravity: gravity is (0, -9.8, 0)
	 * lifeValue: existing time in seconds
	*/
	ArchimedesRad(int massNum, glm::vec3 position, float speed, float degree, float period) :
		SpiralBase(massNum, position, speed, degree, period)
	{
		for (int i = 0; i < massNum; ++i) {
			resetMass(
				masses[i],
				initialPosition + initialSpeed * period * (i + degree / 360.0f),
				initialLife - period * (i + degree / 360.0f + 1)
			);
		}
	}

	virtual void Apply() {}

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
};

}

#endif /* CG_SPIRALS_H_ */
