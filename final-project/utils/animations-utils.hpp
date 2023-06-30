#include <fstream>
#include <sstream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

vector<glm::vec3> generateControlPointsSet(string fileName)
{
	vector <glm::vec3> controlPoints;
	string line;
	ifstream configFile("./animations/" + fileName + ".txt");

	while (getline(configFile, line))
	{
		istringstream iss(line);

		float x, y, z;
		iss >> x >> y >> z;
		controlPoints.push_back(glm::vec3(x, y, z));
	}

	configFile.close();

	return controlPoints;
}