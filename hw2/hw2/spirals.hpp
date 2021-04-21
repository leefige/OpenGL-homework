#ifndef CG_SPIRALS_H_
#define CG_SPIRALS_H_

#include <memory>
#include <deque>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"

namespace cg
{

const float PI = glm::pi<float>();

class Particle
{
protected:
	glm::vec3 speed;
	glm::vec3 direction;
	float fadeSpeed;

public:
	float life;
	glm::vec3 position;
	glm::vec4 color;

	Particle(float offset, float rad, float life):
		speed{0}
	{
		Init(offset, rad, life);
	}

	virtual void Init(float offset, float rad, float life)
	{
		this->life = life;

		float r = cos(rad) / 2 + 0.5;
		float g = cos(rad + glm::radians(120.0)) / 2 + 0.5;
		float b = cos(rad - glm::radians(120.0)) / 2 + 0.5;
		color = glm::vec4{r, g, b, 1.0f};

		fadeSpeed = 1.0f / life;

		direction = glm::normalize(glm::vec3(
			cos(rad),
			sin(rad),
			0
		));

		position = offset * direction;
	}

	virtual void Update(float dt)
	{
		position += speed * GLfloat(dt);
		color.a -= fadeSpeed * dt;
		if (color.a < 0) {
			color.a = 0;
		}
		life -= dt;
	}
};

class Spiral
{
protected:
	std::deque<Particle*> available;
	std::deque<Particle*> emitted;

	int nRad;
	float spriteScale;
	float period;
	float angularVelocity;

	float emitInterval;
	float initLife;

	float countdown;
	float currentRad;

public:
	Spiral(float period, int nRad, float life, float scale) :
		period(period),
		nRad(nRad),
		currentRad(0),
		initLife(life),
		spriteScale(scale)
	{
		angularVelocity = 2 * PI / period;
		emitInterval = period / nRad;
		countdown = emitInterval;
	}

	virtual ~Spiral()
	{
		for (auto& p : emitted) {
			if (p) {
				delete p;
			}
		}

		for (auto& p : available) {
			if (p) {
				delete p;
			}
		}
	}

	virtual const Particle* GetMass(int idx) const
	{
		return emitted[idx];
	}

	virtual void Update(float dt)
	{
		for (auto& m : emitted) {
			m->Update(dt);
		}

		// check dead mass
		while (emitted.size() > 0 && emitted.front()->life < 1e-2) {
			auto& front = emitted.front();
			emitted.pop_front();
			available.push_back(front);
		}

		countdown -= dt;
		currentRad += angularVelocity * dt;
		if (currentRad > 2 * PI) {
			currentRad -= 2 * PI;
		}

		// gen new mass
		if (countdown < 1e-2) {
			emitted.push_back(newMass());
			countdown = emitInterval;
		}
	}

	virtual void Draw(const Shader& shader, GLuint VAO, GLuint texture) const
	{
		for (auto& mass : emitted) {
			drawMass(mass, shader, VAO, texture);
		}
	}

protected:
	virtual Particle* newMass() = 0;

	void drawMass(const Particle* mass, const Shader& shader, GLuint VAO, GLuint texture) const
	{
		glUniform2fv(glGetUniformLocation(shader.Program(), "offset"), 1, glm::value_ptr(mass->position));
		glUniform4fv(glGetUniformLocation(shader.Program(), "color"), 1, glm::value_ptr(mass->color));
		glUniform1f(glGetUniformLocation(shader.Program(), "scale"), spriteScale);

		glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
};

// ===============================================================================

class LogParticle : public Particle
{
	float speedFactor;
public:
	LogParticle(float offset, float rad, float life, float speedFactor) :
		Particle(offset, rad, life),
		speedFactor(speedFactor)
	{
		Init(offset, rad, life);
	}

	virtual void Init(float offset, float rad, float life)
	{
		Particle::Init(offset, rad, life);
		speed = speedFactor * position;
	}

	/*
	 * dr/dt = \alpha*b*r
	 */
	virtual void Update(float dt)
	{
		Particle::Update(dt);
		speed = speedFactor * position;
	}

};

class LogarithmicSpiral : public Spiral
{
	float A;
	float B;

public:
	LogarithmicSpiral(float period, int nRad, float life, float scale, float A, float B) :
		Spiral(period, nRad, life, scale),
		A(A),
		B(B)
	{
	}

private:
	void resetMass(LogParticle* mass, float rad)
	{
		mass->Init(A, rad, initLife);
	}

protected:
	virtual Particle* newMass()
	{
		if (available.size() > 0) {
			auto ret = dynamic_cast<LogParticle*>(available.front());
			available.pop_front();

			// reset
			resetMass(ret, currentRad);

			return ret;
		} else {
			return new LogParticle(A, currentRad, initLife, angularVelocity * B);
		}
	}

	
};

// ===============================================================================

class ArchiParticle : public Particle
{
	float movingSpeed;
public:
	ArchiParticle(float offset, float rad, float life, float speed) :
		Particle(offset, rad, life),
		movingSpeed(speed)
	{
		Init(offset, rad, life);
	}

	virtual void Init(float offset, float rad, float life)
	{
		Particle::Init(offset, rad, life);
		speed = movingSpeed * direction;
	}
};

class ArchimedesSpiral : public Spiral
{
	float speed;

public:
	ArchimedesSpiral(float period, int nRad, float life, float scale, float speed) :
		Spiral(period, nRad, life, scale),
		speed(speed)
	{
	}

private:
	void resetMass(ArchiParticle* mass, float rad)
	{
		mass->Init(0, rad, initLife);
	}

protected:
	virtual Particle* newMass()
	{
		if (available.size() > 0) {
			auto ret = dynamic_cast<ArchiParticle*>(available.front());
			available.pop_front();

			// reset
			resetMass(ret, currentRad);

			return ret;
		} else {
			return new ArchiParticle(0, currentRad, initLife, speed);
		}
	}
};

}


#endif /* CG_SPIRALS_H_ */
