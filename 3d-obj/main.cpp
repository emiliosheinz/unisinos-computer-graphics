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

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = "#version 410\n"
                                   "layout (location = 0) in vec3 position;\n"
                                   "layout (location = 1) in vec3 color;\n"
                                   "uniform mat4 model;\n"
                                   "out vec4 finalColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   //...pode ter mais linhas de código aqui!
                                   "gl_Position = model * vec4(position, 1.0);\n"
                                   "finalColor = vec4(color, 1.0);\n"
                                   "}\0";

// Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = "#version 410\n"
                                     "in vec4 finalColor;\n"
                                     "out vec4 color;\n"
                                     "void main()\n"
                                     "{\n"
                                     "color = finalColor;\n"
                                     "}\n\0";

bool rotateX, 
     rotateY, 
     rotateZ,
     translateLeft,
     translateRight,
     translateIn,
     translateOut,
     translateUp,
     translateDown,
     scaleUp,
     scaleDown = false;

float translateDistance = 0.0f;
float scaleDistance = 1.0f;

enum Direction 
{   
  Increase,
  Decrease
};

float nextTranslateDistance(Direction dir) {
  if(dir == Direction::Increase) {
    return translateDistance > 1.5f ?  -1.5f : translateDistance + 0.01f;
  } 
  
  if(dir == Direction::Decrease) {
    return translateDistance < -1.5f ? 1.5f : translateDistance - 0.01f;
  }

  return 0.0f;
}

float nextScaleDistance(Direction dir) {
  if(dir == Direction::Increase) {
    return scaleDistance > 2 ?  1.0f : scaleDistance + 0.01f;
  } 
  
  if(dir == Direction::Decrease) {
    return scaleDistance < 0 ? 1.0f : scaleDistance - 0.01f; }

  return 0.0f;
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

  // Gerando um buffer simples, com a geometria de um triângulo
  GLuint VAO = setupGeometry();

  glUseProgram(shaderID);

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
    if (rotateX)
    {
      model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
    }
    else if (rotateY)
    {
      model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else if (rotateZ)
    {
      model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    else if (translateLeft)
    {
      translateDistance = nextTranslateDistance(Direction::Decrease);
      model = glm::translate(model, glm::vec3(translateDistance, 0.0f, 0.0f));
    }
    else if (translateRight)
    {
      translateDistance = nextTranslateDistance(Direction::Increase);
      model = glm::translate(model, glm::vec3(translateDistance, 0.0f, 0.0f));
    }
    else if (translateUp)
    {
      translateDistance = nextTranslateDistance(Direction::Increase);
      model = glm::translate(model, glm::vec3(0.0f, translateDistance, 0.0f));
    }
    else if (translateDown)
    {
      translateDistance = nextTranslateDistance(Direction::Decrease);
      model = glm::translate(model, glm::vec3(0.0f, translateDistance, 0.0f));
    }
    else if (translateIn)
    {
      translateDistance = nextTranslateDistance(Direction::Increase);
      model = glm::translate(model, glm::vec3(0.0f, 0.0f, translateDistance));
    }
    else if (translateOut)
    {
      translateDistance = nextTranslateDistance(Direction::Decrease);
      model = glm::translate(model, glm::vec3(0.0f, 0.0f, translateDistance));
    }
    else if (scaleUp)
    {
      scaleDistance = nextScaleDistance(Direction::Increase);
      model = glm::scale(model, glm::vec3(scaleDistance, scaleDistance, scaleDistance));
    }
    else if (scaleDown)
    {
      scaleDistance = nextScaleDistance(Direction::Decrease);
      model = glm::scale(model, glm::vec3(scaleDistance, scaleDistance, scaleDistance));
    }


    

    glUniformMatrix4fv(modelLoc, 1, false, glm::value_ptr(model));
    // Chamada de desenho - drawcall
    // Poligono Preenchido - GL_TRIANGLES

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Chamada de desenho - drawcall
    // CONTORNO - GL_LINE_LOOP

    glDrawArrays(GL_POINTS, 0, 36);
    glBindVertexArray(0);

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
    translateLeft = key == GLFW_KEY_A;
    translateRight = key == GLFW_KEY_D;
    translateIn = key == GLFW_KEY_W;
    translateOut = key == GLFW_KEY_S;
    translateUp = key == GLFW_KEY_UP;
    translateDown = key == GLFW_KEY_DOWN;
    scaleUp = key == GLFW_KEY_RIGHT_BRACKET;
    scaleDown = key == GLFW_KEY_LEFT_BRACKET;
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



std::vector<float> parseOBJFile(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    std::vector<float> vertices;
    std::vector<float> resultVertices;
    std::vector<int> faces;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            std::cout << "v" << x << " " << y << " " << z << std::endl;
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
        else if (prefix == "f") {
            int v1, v2, v3;
            iss >> v1 >> v2 >> v3;
            std::cout << "f" << v1 << " " << v2 << " " << v3 << std::endl;
            faces.push_back(v1);
            faces.push_back(v2);
            faces.push_back(v3);
        }
    }

    for(int i = 0; i < faces.size(); i++) {
        int index = faces[i] - 1;
        resultVertices.push_back(vertices[index * 3]);
        resultVertices.push_back(vertices[index * 3 + 1]);
        resultVertices.push_back(vertices[index * 3 + 2]);
        resultVertices.push_back(static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
        resultVertices.push_back(static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
        resultVertices.push_back(static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    }

    return resultVertices;
}

int setupGeometry()
{
  std::vector<float> vertices = parseOBJFile("cube.obj");

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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
  glEnableVertexAttribArray(0);

  // Atributo cor (r, g, b)
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  // Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice
  // atualmente vinculado - para que depois possamos desvincular com segurança
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
  glBindVertexArray(0);

  return VAO;
}
