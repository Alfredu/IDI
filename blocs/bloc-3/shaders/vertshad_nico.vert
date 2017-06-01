#version 330 core

in vec3 vertex;
in vec3 normal;

in vec3 matamb;
in vec3 matdiff;
in vec3 matspec;
in float matshin;

out vec3 matamb2;
out vec3 matdiff2;
out vec3 matspec2;
out float matshin2;


uniform mat4 proj;
uniform mat4 view;
uniform mat4 TG;


out vec3 normalSCO;
out vec4 vertexSCO;




void main()
{	
    mat3 NormalMatrix = inverse(transpose(mat3(view*TG)));
	normalSCO = normalize(NormalMatrix * normal);  //Normal en el vertex SCO
	vertexSCO = view * TG * vec4(vertex,1.0);      //Posicio del vertex en SCO
	
	matamb2=matamb;
	matdiff2=matdiff;
	matspec2=matspec;
	matshin2=matshin;
	
    gl_Position = proj * view * TG * vec4 (vertex, 1.0);
}
