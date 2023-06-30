#include <iostream>
#include <glad/glad.h>
#include "stb_image.h"

#include "string-utils.hpp"

using namespace std;

struct Face
{
  vector<int> vertexIndices;
  vector<int> textureCoordIndices;
  vector<int> normalIndices;
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

struct ParsedObj
{
  vector<float> vertices;
  string mtlFileName;
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
    cout << "Failed to load texture" << endl;
  }

  stbi_image_free(data);

  glBindTexture(GL_TEXTURE_2D, 0);

  return texID;
}

ParsedObj parseOBJFile(const string &filename)
{
  ifstream file(filename);
  string line;
  vector<Vertex> vertices;
  vector<TextureCoord> textureCoords;
  
  vector<Normal> normals;
  vector<Face> faces;
  string mtlFileName;

  while (getline(file, line))
  {
    istringstream iss(line);
    string prefix;
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
      vector<string> vertexStrings;
      string vertexString;

      while (iss >> vertexString)
      {
        vertexStrings.push_back(vertexString);
      }

      if (vertexStrings.size() >= 3)
      {
        for (size_t i = 1; i < vertexStrings.size() - 1; ++i)
        {
          const string& v0String = vertexStrings[0];
          const string& v1String = vertexStrings[i];
          const string& v2String = vertexStrings[i + 1];

          istringstream vss0(v0String);
          istringstream vss1(v1String);
          istringstream vss2(v2String);

          string v0IndexString, v0TexCoordString, v0NormalIndexString;
          string v1IndexString, v1TexCoordString, v1NormalIndexString;
          string v2IndexString, v2TexCoordString, v2NormalIndexString;

          getline(vss0, v0IndexString, '/');
          getline(vss0, v0TexCoordString, '/');
          getline(vss0, v0NormalIndexString, '/');

          getline(vss1, v1IndexString, '/');
          getline(vss1, v1TexCoordString, '/');
          getline(vss1, v1NormalIndexString, '/');

          getline(vss2, v2IndexString, '/');
          getline(vss2, v2TexCoordString, '/');
          getline(vss2, v2NormalIndexString, '/');

          face.vertexIndices.push_back(stoi(v0IndexString) - 1);
          face.textureCoordIndices.push_back(stoi(v0TexCoordString) - 1);
          face.normalIndices.push_back(stoi(v0NormalIndexString) - 1);

          face.vertexIndices.push_back(stoi(v1IndexString) - 1);
          face.textureCoordIndices.push_back(stoi(v1TexCoordString) - 1);
          face.normalIndices.push_back(stoi(v1NormalIndexString) - 1);

          face.vertexIndices.push_back(stoi(v2IndexString) - 1);
          face.textureCoordIndices.push_back(stoi(v2TexCoordString) - 1);
          face.normalIndices.push_back(stoi(v2NormalIndexString) - 1);
        }

        faces.push_back(face);
      }
    }
  }

  vector<float> result;

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

vector<Material> readMTLFile(const string &assetsFolder, const string &mtlFileName)
{
  vector<Material> materials;
  ifstream file(assetsFolder + mtlFileName);

  if (!file.is_open())
  {
    cout << "Failed to open file: " << mtlFileName << endl;
    return materials;
  }

  Material currentMaterial;
  string line;

  while (getline(file, line))
  {
    istringstream iss(line);
    string keyword;
    iss >> keyword;

    if (keyword == "newmtl")
    {
      // Every time we find a new material, we push the previous one
      if (!currentMaterial.name.empty())
      {
        materials.push_back(currentMaterial);
      }

      currentMaterial = Material();
      iss >> currentMaterial.name;
    }
    else if (keyword == "Ka")
    {
      float r, g, b;
      iss >> r >> g >> b;
      currentMaterial.ambient = glm::vec3(r, g, b);
    }
    else if (keyword == "Ks")
    {
      float r, g, b;
      iss >> r >> g >> b;
      currentMaterial.specular = glm::vec3(r, g, b);
      // Kd and Ke are the same according to the MTL specification
    }
    else if (keyword == "Kd" || keyword == "Ke")
    {
      float r, g, b;
      iss >> r >> g >> b;
      currentMaterial.diffuse = glm::vec3(r, g, b);
    }
    else if (keyword == "map_Kd")
    {
      string fileName;
      iss >> fileName;
      currentMaterial.texturePath = assetsFolder + trim(fileName);
    }
    else if (keyword == "Ns")
    {
      float shininess;
      iss >> shininess;
      currentMaterial.shininess = shininess;
    }
  }

  // Needed to add the last material since no newmtl keyword is found
  if (!currentMaterial.name.empty())
  {
    materials.push_back(currentMaterial);
  }


  file.close();
  return materials;
}