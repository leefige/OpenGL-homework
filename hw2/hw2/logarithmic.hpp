#ifndef CG_LOGARITHMIC_H_
#define CG_LOGARITHMIC_H_

#include <memory>
#include <deque>

#include "shader.hpp"

namespace cg
{

const float PI = glm::pi<float>();


class Particle
{
	glm::vec3 speed;
	glm::vec3 direction;

	float fadeSpeed;
	float rad;
	float speedFactor;

public:
	float life;
	glm::vec3 position;
	glm::vec4 color;

	Particle(float offset, float rad, float speedFactor, float life) :
		life(life),
		speedFactor(speedFactor),
		rad(rad)
	{
		direction = glm::normalize(glm::vec3(
			cos(rad),
			sin(rad),
			0
		));

		Reset(offset, rad, life);
	}

	void Reset(float offset, float rad, float life)
	{
		float r = cos(rad) / 2 + 0.5;
		float g = cos(rad + glm::radians(120.0)) / 2 + 0.5;
		float b = cos(rad - glm::radians(120.0)) / 2 + 0.5;
		color = glm::vec4{r, g, b, 1.0f};

		fadeSpeed = 1.0f / life * 2;
		this->life = life;
		position = offset * direction;
		speed = speedFactor * position;
	}

	/*
	 * dr/dt = \alpha*b*r
	 */
	virtual void Update(float dt)
	{
		position += speed * GLfloat(dt);
		speed = speedFactor * position;
		color.a -= fadeSpeed * dt;
		if (color.a < 0) {
			color.a = 0;
		}

		life -= dt;
	}

};

class LogarithmicSpiral
{
	std::deque<Particle*> available;
	std::deque<Particle*> emitted;

	int nRad;
	float spriteScale;

	float period;
	float angularVelocity;
	float A;
	float B;

	float currentRad;
	float emitInterval;
	float initLife;

	float countdown;

public:
	LogarithmicSpiral(float period, float A, float B, int nRad, float life, float scale) :
		period(period),
		A(A),
		B(B),
		nRad(nRad),
		currentRad(0),
		initLife(life),
		spriteScale(scale)
	{
		angularVelocity = 2 * PI / period;
		emitInterval = period / nRad;
		countdown = emitInterval;
	}

	virtual ~LogarithmicSpiral()
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

	virtual const Particle& GetMass(int idx) const
	{
		return *emitted[idx];
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
			drawMass(*mass, shader, VAO, texture);
		}
	}

protected:
	void resetMass(Particle& mass, float rad)
	{
		mass.Reset(A, rad, initLife);
	}

	Particle* newMass()
	{
		if (available.size() > 0) {
			auto& ret = available.front();
			available.pop_front();

			// reset
			resetMass(*ret, currentRad);

			return ret;
		} else {
			return new Particle(A, currentRad, angularVelocity * B, initLife);
		}
	}

	void drawMass(const Particle& mass, const Shader& shader, GLuint VAO, GLuint texture) const
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

}


#endif /* CG_LOGARITHMIC_H_ */
