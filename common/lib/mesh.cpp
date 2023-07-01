#include "mesh.h"

void Mesh::initialize(GLuint VAO, int nVertices, Shader* shader, GLuint textureID, glm::vec3 position, glm::vec3 scale, float angle, glm::vec3 axis)
{
	this->VAO = VAO;
	this->nVertices = nVertices;
	this->shader = shader;
	this->position = position;
	this->scale = scale;
	this->angle = angle;
	this->axis = axis;
	this->textureID = textureID;
	this->shouldRotateY = false;
}

void Mesh::updatePosition(glm::vec3 position) {
	this->position = position;
}

void Mesh::setShouldRotateY(bool shouldRotateY) {
	this->shouldRotateY = shouldRotateY;
}

void Mesh::update()
{
	glm::mat4 model = glm::mat4(1);
	model = glm::translate(model, position);
	
	if(shouldRotateY) {
		this->angle += 0.001;
		model = glm::rotate(model, this->angle, glm::vec3(0.0, 1.0, 0.0));
	} else {
		model = glm::rotate(model, angle, axis);
	}
		
	model = glm::scale(model, scale);
	shader->setMat4("model", glm::value_ptr(model));
}

void Mesh::draw(Material material)
{
	shader->setVec3("ka", material.ambient.r, material.ambient.g, material.ambient.b);
  shader->setVec3("kd", material.diffuse.r, material.diffuse.g, material.diffuse.b);
  shader->setVec3("ks", material.specular.r, material.specular.g, material.specular.b);
  shader->setFloat("q", material.shininess);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, nVertices);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}