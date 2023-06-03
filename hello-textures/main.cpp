/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Processamento Gráfico - Jogos Digitais - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 12/05/2023
 *
 */

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

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry(int *verticesCount);
int loadTexture(string path);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = "#version 410\n"
                                   "layout (location = 0) in vec3 position;\n"
                                   "layout (location = 1) in vec3 color;\n"
                                   "layout (location = 2) in vec2 tex_coord;\n"
                                   "out vec4 vertexColor;\n"
                                   "out vec2 texCoord;\n"
                                   "uniform mat4 model;\n"
                                   "void main()\n"
                                   "{\n"
                                   //...pode ter mais linhas de código aqui!
                                   "gl_Position = model * vec4(position, 1.0);\n"
                                   "vertexColor = vec4(color, 1.0);\n"
																	 "texCoord = vec2(tex_coord.x, 1 - tex_coord.y);\n"
                                   "}\0";

// Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = "#version 410\n"
                                     "in vec4 vertexColor;\n"
                                     "in vec2 texCoord;\n"
																		 "out vec4 color;\n"
																		 "uniform sampler2D tex_buffer;\n"
                                     "void main()\n"
                                     "{\n"
                                     "color = texture(tex_buffer, texCoord);\n"
                                     "}\n\0";

bool rotateX, 
     rotateY, 
     rotateZ;

float translateDistance = 0.0f;

enum Direction 
{   
  Increase,
  Decrease
};

glm::mat4 calculateTransformations(glm::mat4 model, float angle) {
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

// Função MAIN
int main()
{
  // Inicialização da GLFW
  glfwInit();

  // Muita atenção aqui: alguns ambientes não aceitam essas configurações
  // Você deve adaptar para a versão do OpenGL suportada por sua placa
  // Sugestão: comente essas linhas de código para desobrir a versão e
  // depois atualize (por exemplo: 4.5 com 4 e 5)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // Essencial para computadores da Apple
  // #ifdef __APPLE__
  //	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  // #endif

  // Criação da janela GLFW
  GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "3D Cubes", nullptr, nullptr);
  glfwMakeContextCurrent(window);

  // Fazendo o registro da função de callback para a janela GLFW
  glfwSetKeyCallback(window, key_callback);

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

  // Compilando e buildando o programa de shader
  GLuint shaderID = setupShader();

	// TODO pegar dinamicamente
	// GLuint textureId = loadTexture("../common/3d-models/suzanne/Cube.png");
	GLuint textureId = loadTexture("../common/3d-models/suzanne/Suzanne.png");

  int verticesCount;
  GLuint VAO = setupGeometry(&verticesCount);

  glUseProgram(shaderID);

	glUniform1i(glGetUniformLocation(shaderID, "tex_buffer"), 0);

  glm::mat4 model = glm::mat4(1); // matriz identidade;
  GLint modelLoc = glGetUniformLocation(shaderID, "model");
  //
  model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/ glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  glUniformMatrix4fv(modelLoc, 1, false, glm::value_ptr(model));

  glEnable(GL_DEPTH_TEST);

  // Loop da aplicação - "game loop"
  while (!glfwWindowShouldClose(window))
  {
    // Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
    glfwPollEvents();

    // Limpa o buffer de cor
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // cor de fundo
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLineWidth(10);
    glPointSize(20);

    float angle = (GLfloat)glfwGetTime();

    model = glm::mat4(1);
		model = glm::scale(model, glm::vec3(0.3, 0.3, 0.3));
    model  = calculateTransformations(model, angle);
    glUniformMatrix4fv(modelLoc, 1, false, glm::value_ptr(model));

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
}

// Esta função está basntante hardcoded - objetivo é compilar e "buildar" um programa de
//  shader simples e único neste exemplo de código
//  O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
//  fragmentShader source no iniçio deste arquivo
//  A função retorna o identificador do programa de shader
int setupShader()
{
  // Vertex shader
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  // Checando erros de compilação (exibição via log no terminal)
  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }
  // Fragment shader
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  // Checando erros de compilação (exibição via log no terminal)
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }
  // Linkando os shaders e criando o identificador do programa de shader
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  // Checando por erros de linkagem
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}

struct Vertex {
    float x, y, z;
};

struct TextureCoord {
    float s, t;
};

struct Face {
    std::vector<int> vertexIndices;
		std::vector<int> textureCoordIndices;
};


std::vector<float> parseOBJFile(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    std::vector<Vertex> vertices;
    std::vector<TextureCoord> textureCoords;
    std::vector<Face> faces;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            Vertex vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        } else if (prefix == "vt") {
            TextureCoord texCoord;
            iss >> texCoord.s >> texCoord.t;
            textureCoords.push_back(texCoord);
        } else if (prefix == "f") {
            Face face;
            std::string vertexString;

            while (iss >> vertexString) {
                std::istringstream vss(vertexString);
                std::string vertexIndexString, texCoordIndexString, normalIndexString;

                std::getline(vss, vertexIndexString, '/');
                std::getline(vss, texCoordIndexString, '/');

                face.vertexIndices.push_back(std::stoi(vertexIndexString) - 1);
                if (!texCoordIndexString.empty())
                    face.textureCoordIndices.push_back(std::stoi(texCoordIndexString) - 1);
            }

            faces.push_back(face);
        }
    }

    std::vector<float> result;

		for (const Face& face : faces) {
        for (size_t i = 0; i < face.vertexIndices.size(); ++i) {
            const Vertex& vertex = vertices[face.vertexIndices[i]];
						const TextureCoord& texture = textureCoords[face.textureCoordIndices[i]];
						result.push_back(vertex.x);
						result.push_back(vertex.y);
						result.push_back(vertex.z);

						result.push_back(0);
						result.push_back(0);
						result.push_back(0);

						result.push_back(texture.s);
						result.push_back(texture.t);
        }
    }

    return result;
}

int setupGeometry(int *verticesCount)
{
  std::vector<float> vertices = parseOBJFile("../common/3d-models/suzanne/SuzanneTriTextured.obj");
	// std::vector<float> vertices = parseOBJFile("../common/3d-models/suzanne/CubeTextured.obj");

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

  // Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando:
  //  Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
  //  Numero de valores que o atributo tem (por ex, 3 coordenadas xyz)
  //  Tipo do dado
  //  Se está normalizado (entre zero e um)
  //  Tamanho em bytes
  //  Deslocamento a partir do byte zero

  // Atributo posição (x, y, z)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
  glEnableVertexAttribArray(0);

  // Atributo cor (r, g, b)
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

	 // Atributo texture (s, t)
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
  glEnableVertexAttribArray(2);

  // Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice
  // atualmente vinculado - para que depois possamos desvincular com segurança
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
  glBindVertexArray(0);

  // Dividimos por 6 pois cada vértice tem 6 floats (3 coordenadas + 3 cores)
  *verticesCount = vertices.size() / 8;
  return VAO;
}

int loadTexture(string path)
{
	GLuint texID;

	// Gera o identificador da textura na memória 
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	//Ajusta os parâmetros de wrapping e filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Carregamento da imagem
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3) //jpg, bmp
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else //png
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