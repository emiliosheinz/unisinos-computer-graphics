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

const string ASSETS_FOLDER = "../common/3d-models/suzanne/";
const string OBJ_FILE_PATH = ASSETS_FOLDER + "SuzanneTriTextured.obj";

struct Geometry
{
  GLuint VAO;
  int verticesCount;
};

struct Vertex
{
  float x, y, z;
};

struct TextureCoord
{
  float s, t;
};

struct Normal
{
  float x, y, z;
};

struct Face
{
  std::vector<int> vertexIndices;
  std::vector<int> textureCoordIndices;
  std::vector<int> normalIndices;
};

struct ParsedObj
{
  std::vector<float> vertices;
  std::string mtlFileName;
};

struct Material
{
  std::string name;
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  string texturePath;
  float shininess;
  int textureId;
};

int loadTexture(string mtlPath);
ParsedObj parseOBJFile(const std::string &mtlPath);
Material readMTLFile(const string &mtlFileName);
Geometry setupGeometry(const std::vector<float> &vertices);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

bool rotateX,
    rotateY,
    rotateZ;

float translateDistance = 0.0f;

enum Direction
{
  Increase,
  Decrease
};

glm::mat4 calculateTransformations(glm::mat4 model, float angle)
{
  if (rotateX)
  {
    return glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
  }

  if (rotateY)
  {
    return glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
  }

  if (rotateZ)
  {
    return glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
  }

  return model;
}

const std::string WHITESPACE = " \n\r\t\f\v";

std::string ltrim(const std::string &s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string &s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string trim(const std::string &s)
{
  return rtrim(ltrim(s));
}

Camera camera;

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

  ParsedObj parsedObj = parseOBJFile(OBJ_FILE_PATH);
  Material material = readMTLFile(parsedObj.mtlFileName);

  Shader shader("vertex-shader.vert", "fragment-shader.frag");

  GLuint textureId = loadTexture(material.texturePath);
  Geometry geometry = setupGeometry(parsedObj.vertices);

  GLuint VAO = geometry.VAO;
  int verticesCount = geometry.verticesCount;

  glUseProgram(shader.ID);

  camera.initialize(&shader, width, height);

  glUniform1i(glGetUniformLocation(shader.ID, "tex_buffer"), 0);

  glm::mat4 model = glm::mat4(1); // matriz identidade;

  glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
  shader.setMat4("view", value_ptr(view));

  glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
  shader.setMat4("projection", glm::value_ptr(projection));

  model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  shader.setMat4("model", glm::value_ptr(model));

  shader.setVec3("ka", material.ambient.r, material.ambient.g, material.ambient.b);
  shader.setVec3("kd", material.diffuse.r, material.diffuse.g, material.diffuse.b);
  shader.setVec3("ks", material.specular.r, material.specular.g, material.specular.b);
  shader.setFloat("q", material.shininess);

  // Definindo as propriedades da fonte de luz
  shader.setVec3("lightPosition", 15.0f, 15.0f, 2.0f);
  shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

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

    float angle = (GLfloat)glfwGetTime();

    model = glm::mat4(1);
    model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
    model = calculateTransformations(model, angle);
    shader.setMat4("model", glm::value_ptr(model));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, verticesCount);
    // glDrawArrays(GL_POINTS, 0, verticesCount);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Troca os buffers da tela
    glfwSwapBuffers(window);
  }
  // Pede pra OpenGL desalocar os buffers
  glDeleteVertexArrays(1, &VAO);
  // Finaliza a execução da GLFW, limpando os recursos alocados por ela
  glfwTerminate();
  return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  if (action == GLFW_PRESS)
  {
    rotateX = key == GLFW_KEY_X;
    rotateY = key == GLFW_KEY_Y;
    rotateZ = key == GLFW_KEY_Z;
  }

  camera.move(window, key, action);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	camera.rotate(window, xpos, ypos);
}


ParsedObj parseOBJFile(const std::string &filename)
{
  std::ifstream file(filename);
  std::string line;
  std::vector<Vertex> vertices;
  std::vector<TextureCoord> textureCoords;
  std::vector<Normal> normals;
  std::vector<Face> faces;
  std::string mtlFileName;

  while (std::getline(file, line))
  {
    std::istringstream iss(line);
    std::string prefix;
    iss >> prefix;

    if (prefix == "mtllib")
    {
      iss >> mtlFileName;
    }
    else if (prefix == "v")
    {
      Vertex vertex;
      iss >> vertex.x >> vertex.y >> vertex.z;
      vertices.push_back(vertex);
    }
    else if (prefix == "vt")
    {
      TextureCoord texCoord;
      iss >> texCoord.s >> texCoord.t;
      textureCoords.push_back(texCoord);
    }
    else if (prefix == "vn")
    {
      Normal normal;
      iss >> normal.x >> normal.y >> normal.z;
      normals.push_back(normal);
    }
    else if (prefix == "f")
    {
      Face face;
      std::string vertexString;

      while (iss >> vertexString)
      {
        std::istringstream vss(vertexString);
        std::string vertexIndexString, texCoordIndexString, normalIndexString;

        std::getline(vss, vertexIndexString, '/');
        std::getline(vss, texCoordIndexString, '/');
        std::getline(vss, normalIndexString, '/');

        face.vertexIndices.push_back(std::stoi(vertexIndexString) - 1);
        if (!texCoordIndexString.empty())
          face.textureCoordIndices.push_back(std::stoi(texCoordIndexString) - 1);
        if (!normalIndexString.empty())
          face.normalIndices.push_back(std::stoi(normalIndexString) - 1);
      }

      faces.push_back(face);
    }
  }

  std::vector<float> result;

  for (const Face &face : faces)
  {
    for (size_t i = 0; i < face.vertexIndices.size(); ++i)
    {
      const Vertex &vertex = vertices[face.vertexIndices[i]];
      const TextureCoord &texture = textureCoords[face.textureCoordIndices[i]];
      const Normal &normal = normals[face.normalIndices[i]];

      result.push_back(vertex.x);
      result.push_back(vertex.y);
      result.push_back(vertex.z);

      result.push_back(1);
      result.push_back(0);
      result.push_back(0);

      result.push_back(texture.s);
      result.push_back(texture.t);

      result.push_back(normal.x);
      result.push_back(normal.y);
      result.push_back(normal.z);
    }
  }

  ParsedObj parsedObj;
  parsedObj.vertices = result;
  parsedObj.mtlFileName = mtlFileName;

  return parsedObj;
}

Material readMTLFile(const string &mtlFileName)
{
  Material material;
  ifstream file(ASSETS_FOLDER + mtlFileName);

  if (!file.is_open())
  {
    cout << "Failed to open file: " << mtlFileName << endl;
    return material;
  }

  string line;
  while (getline(file, line))
  {
    istringstream iss(line);
    string keyword;
    iss >> keyword;
    if (keyword == "newmtl")
    {
      iss >> material.name;
    }
    else if (keyword == "Ka")
    {
      float r, g, b;
      iss >> r >> g >> b;
      material.ambient = glm::vec3(r, g, b);
    }
    else if (keyword == "Ks")
    {
      float r, g, b;
      iss >> r >> g >> b;
      material.specular = glm::vec3(r, g, b);
      // Kd and Ke are the same according to the MTL specification
    }
    else if (keyword == "Kd" || keyword == "Ke")
    {
      float r, g, b;
      iss >> r >> g >> b;
      material.diffuse = glm::vec3(r, g, b);
    }
    else if (keyword == "map_Kd")
    {
      string fileName;
      iss >> fileName;
      material.texturePath = ASSETS_FOLDER + trim(fileName);
    }
    else if (keyword == "Ns")
    {
      float shininess;
      iss >> shininess;
      material.shininess = shininess;
    }
  }

  file.close();
  return material;
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

int loadTexture(string path)
{
  GLuint texID;

  // Gera o identificador da textura na memória
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);

  // Ajusta os parâmetros de wrapping e filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Carregamento da imagem
  int width, height, nrChannels;
  unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

  if (data)
  {
    if (nrChannels == 3) // jpg, bmp
    {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else // png
    {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else
  {
    std::cout << "Failed to load texture" << std::endl;
  }

  stbi_image_free(data);

  glBindTexture(GL_TEXTURE_2D, 0);

  return texID;
}