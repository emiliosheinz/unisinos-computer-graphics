#version 410

// Declara as variáveis de entrada (inputs) do shader
in vec3 finalColor;
in vec3 scaledNormal;
in vec3 fragmentPosition;

// Declara as variáveis uniformes do shader. 
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float q;
// Coeficientes de reflexão
uniform float ka;
// Coeficientes de reflexão difusa
uniform float kd;
// Coeficientes de reflexão especular
uniform float ks;

uniform vec3 cameraPos;

out vec4 color;

void main()
{
	vec3 ambient = ka * lightColor;

	vec3 N = normalize(scaledNormal);
	vec3 L = normalize(lightPosition - fragmentPosition);
	float diff = max(dot(N,L),0.0);
	vec3 diffuse = kd * diff * lightColor;

	vec3 V = normalize(cameraPos - fragmentPosition);
	vec3 R = normalize(reflect(-L,N));
	float spec = max(dot(R,V),0.0);
	spec = pow(spec, q);
	vec3 specular = ks * spec * lightColor;

	vec3 result = (ambient + diffuse) * finalColor + specular;

	color = vec4(result,1.0);
}