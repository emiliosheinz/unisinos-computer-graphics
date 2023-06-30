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