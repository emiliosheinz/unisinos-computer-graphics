#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include "Shader.h"
#include "camera.h"
#include "bezier.h"
#include "mesh.h"

#include "./utils/obj-utils.hpp"
#include "./utils/animations-utils.hpp"

const string ASSETS_FOLDER = "./assets/";
const string MOON_OBJ_FILE_PATH = ASSETS_FOLDER + "Moon.obj";
const string EARTH_OBJ_FILE_PATH = ASSETS_FOLDER + "Earth.obj";

struct Geometry
{
  GLuint VAO;
  int verticesCount;
};

Geometry setupGeometry(const std::vector<float> &vertices);
vector <glm::vec3> generateControlPointsSet(string path);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

Camera camera;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  camera.move(window, key, action);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	camera.rotate(window, xpos, ypos);
}

int main()
{
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // Criação da janela GLFW
  GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "3D Cubes", nullptr, nullptr);
  glfwMakeContextCurrent(window);

  // Fazendo o registro da função de callback para a janela GLFW
  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetCursorPos(window, WIDTH / 2, HEIGHT / 2);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // GLAD: carrega todos os ponteiros d funções da OpenGL
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
  }

  // Obtendo as informações de versão
  const GLubyte *renderer = glGetString(GL_RENDERER); /* get renderer string */
  const GLubyte *version = glGetString(GL_VERSION);   /* version as a string */
  cout << "Renderer: " << renderer << endl;
  cout << "OpenGL version supported " << version << endl;

  // Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  Shader shader("./shaders/vertex-shader.vert", "./shaders/fragment-shader.frag");

  glUseProgram(shader.ID);

  camera.initialize(&shader, width, height);

  ParsedObj parsedMoonObj = parseOBJFile(MOON_OBJ_FILE_PATH);
  vector<Material> moonMaterials = readMTLFile(ASSETS_FOLDER, parsedMoonObj.mtlFileName);
  Material moonMaterial = moonMaterials[0];
  GLuint moonTextureId = loadTexture(moonMaterial.texturePath);
  Geometry moonGeometry = setupGeometry(parsedMoonObj.vertices);
  GLuint MOON_VAO = moonGeometry.VAO;
  int moonVerticesCount = moonGeometry.verticesCount;

  Mesh moon;
  moon.initialize(MOON_VAO, moonVerticesCount, &shader, moonTextureId, glm::vec3(-1.0f,0.0f,0.0f), glm::vec3(0.1f,0.1f,0.1f));
  moon.setShouldRotateY(true);

  ParsedObj parsedEarthObj = parseOBJFile(EARTH_OBJ_FILE_PATH);
  vector<Material> earthMaterials = readMTLFile(ASSETS_FOLDER, parsedEarthObj.mtlFileName);
  Material earthMaterial = earthMaterials[0];
  GLuint earthTextureId = loadTexture(earthMaterial.texturePath);
  Geometry earthGeometry = setupGeometry(parsedEarthObj.vertices);
  GLuint EARTH_VAO = earthGeometry.VAO;
  int earthVerticesCount = earthGeometry.verticesCount;

  Mesh earth;
  earth.initialize(EARTH_VAO, earthVerticesCount, &shader, earthTextureId, glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.15f,0.15f,0.15f));
  earth.setShouldRotateY(true);

  // Definindo as propriedades da fonte de luz
  shader.setVec3("lightPosition", 15.0f, 15.0f, 2.0f);
  shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

  std::vector<glm::vec3> controlPoints = generateControlPointsSet("config");

  Bezier bezier;
	bezier.setControlPoints(controlPoints);
	bezier.setShader(&shader);
	bezier.generateCurve(100);
	int nbCurvePoints = bezier.getNbCurvePoints();
	int curentPointOnCurve = 0;

  glEnable(GL_DEPTH_TEST);

  // Loop da aplicação - "game loop"
  while (!glfwWindowShouldClose(window))
  {
    // Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
    glfwPollEvents();

    // Limpa o buffer de cor
    glClearColor(0.08f, 0.08f, 0.08f, 1.0f); // cor de fundo
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLineWidth(10);
    glPointSize(20);

    camera.update();

		glm::vec3 pointOnCurve = bezier.getPointOnCurve(curentPointOnCurve);
		moon.updatePosition(pointOnCurve);
    
		moon.update();
		moon.draw(moonMaterial);

    earth.update();
		earth.draw(earthMaterial);

    curentPointOnCurve = (curentPointOnCurve + 1) % nbCurvePoints;

    glfwSwapBuffers(window);
  }


  glDeleteVertexArrays(1, &MOON_VAO);
  glDeleteVertexArrays(1, &EARTH_VAO);
  glfwTerminate();
  return 0;
}

Geometry setupGeometry(const std::vector<float> &vertices)
{
  GLuint VBO, VAO;

  // Geração do identificador do VBO
  glGenBuffers(1, &VBO);

  // Faz a conexão (vincula) do buffer como um buffer de array
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  // Envia os dados do array de floats para o buffer da OpenGl
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

  // Geração do identificador do VAO (Vertex Array Object)
  glGenVertexArrays(1, &VAO);

  // Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
  // e os ponteiros para os atributos
  glBindVertexArray(VAO);

  // Atributo posição (x, y, z)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid *)0);
  glEnableVertexAttribArray(0);

  // Atributo cor (r, g, b)
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  // Atributo texture (s, t)
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
  glEnableVertexAttribArray(2);

  // Atributo normal (nx, ny, nz)
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid *)(8 * sizeof(GLfloat)));
  glEnableVertexAttribArray(3);

  // Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice
  // atualmente vinculado - para que depois possamos desvincular com segurança
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
  glBindVertexArray(0);

  // Dividimos por 11 pois cada vértice tem 11 floats (3 coordenadas + 3 cores + 2 texturas + 3 normais)
  int verticesCount = vertices.size() / 11;

  return {
      VAO,
      verticesCount,
  };
}

