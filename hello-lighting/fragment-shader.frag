#version 410

// Declara as variáveis de entrada (inputs) do shader
in vec3 finalColor;
in vec3 scaledNormal;
in vec2 textureCoord;
in vec3 fragmentPosition;

// Declara as variáveis uniformes do shader. 
uniform vec3 lightColor;
uniform vec3 lightPosition;
// Coeficientes de reflexão
uniform vec3 ka;
// Coeficientes de reflexão difusa
uniform vec3 kd;
// Coeficientes de reflexão especular
uniform vec3 ks;
// Expoente de reflexão especular
uniform float q;

uniform vec3 cameraPos;
uniform sampler2D tex_buffer;

out vec4 color;

void main()
{
	// Cálculo da parcela de iluminação ambiente
	vec3 ambient = ka * lightColor;
	
	// Cálculo da parcela de iluminação difusa
	vec3 N = normalize(scaledNormal);
	vec3 L = normalize(lightPosition - fragmentPosition);
	float diff = max(dot(N,L),0.0);
	vec3 diffuse = kd * diff * lightColor;

	vec3 V = normalize(cameraPos - fragmentPosition);
	vec3 R = normalize(reflect(-L,N));
	float spec = max(dot(R,V),0.0);
	spec = pow(spec, q);
	vec3 specular = ks * spec * lightColor;

	vec3 texColor = texture(tex_buffer, textureCoord).xyz;
	vec3 result = (ambient + diffuse) * texColor + specular;

	color = vec4(result, 1.0f);
}