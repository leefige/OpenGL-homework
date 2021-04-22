#ifndef CG_SNOW_H_
#define CG_SNOW_H_

#include <deque>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"

namespace cg
{

const float PI = glm::pi<float>();

class Mass
{
protected:
	glm::vec3 speed;
	float fadeSpeed;
	glm::vec3 force;

public:
	float mass;
	float life;
	glm::vec3 position;
	float alpha;
	float scale;

	Mass(float mass, glm::vec3 position, glm::vec3 speed, float life, float scale)
	{
		Init(mass, position, speed, life, scale);
	}

	void Init(float mass, glm::vec3 position, glm::vec3 speed, float life, float scale)
	{
		this->life = life;
		this->force = glm::vec3{0};
		this->speed = speed;
		this->position = position;
		this->scale = scale;
		fadeSpeed = 1.0f / life;
		alpha = 1.0f;
	}

	void ApplyForce(glm::vec3 newForce)
	{
		this->force += newForce;
	}

	virtual void Update(float dt)
	{
		speed += force / GLfloat(mass) * GLfloat(dt);
		position += speed * GLfloat(dt);
		alpha -= fadeSpeed * dt;
		if (alpha < 0) {
			alpha = 0;
		}
		life -= dt;
	}
};

class Snowing
{
	std::deque<Mass*> available;
	std::deque<Mass*> emitted;

	float period;
	float growth;
	float massMass;
	float minScale;
	float maxScale;
	float initLife;
	int height;
	int width;
	glm::vec3 initSpeed;
	glm::vec3 gravity;

	float countdown;

public:
	Snowing(float period, float growth, float m, int width, int height,
			glm::vec3 speed, float life, float minScale, float maxScale, glm::vec3 gravity) :
		period(period),
		growth(growth),
		massMass(m),
		minScale(minScale),
		maxScale(maxScale),
		initLife(life),
		width(width),
		height(height),
		initSpeed(speed),
		gravity(gravity)
	{
		countdown = 0;
	}

	virtual ~Snowing()
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

	const Mass* GetMass(int idx) const
	{
		return emitted[idx];
	}

	void SetPositionRange(int width, int height)
	{
		this->width = width;
		this->height = height;
	}

	void Update(float dt)
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

		// gen new mass
		countdown -= dt;
		if (countdown < 0) {
			auto newOne = newMass();
			newOne->ApplyForce(gravity * newOne->mass);
			emitted.push_back(newOne);
			countdown = float(rand() % int(period * 100)) / 100;
		}

		period -= growth * dt;
		if (period < 0.3f) {
			period = 0.3f;
		}
	}

	virtual void Draw(const Shader& shader, GLuint VAO, GLuint texture, const glm::mat4& view, const glm::mat4& projection) const
	{
		for (auto& mass : emitted) {
			drawMass(mass, shader, VAO, texture, view, projection);
		}
	}

private:
	Mass* newMass()
	{
		auto x = float(rand() % width) - float(width) / 2;
		auto y = float(height / 2) + (minScale + maxScale) / 2;

		auto scale = fmod(float(rand()) / 100.0f, maxScale - minScale) + minScale;

		if (available.size() > 0) {
			auto ret = available.front();
			available.pop_front();

			// reset
			ret->Init(massMass, glm::vec3{x, y, 0}, initSpeed, initLife, scale);

			return ret;
		} else {
			return new Mass(massMass, glm::vec3{x, y, 0}, initSpeed, initLife, scale);
		}
	}

	void drawMass(const Mass* mass, const Shader& shader, GLuint VAO, GLuint texture, const glm::mat4& view, const glm::mat4& projection) const
	{
		shader.Use();

		glUniform2fv(glGetUniformLocation(shader.Program(), "offset"), 1,glm::value_ptr(mass->position));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program(), "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniform1f(glGetUniformLocation(shader.Program(), "scale"), mass->scale);
		glUniform1f(glGetUniformLocation(shader.Program(), "alpha"), mass->alpha);

		glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
};

} /* namespace cg */

#endif /* CG_SNOW_H_ */
