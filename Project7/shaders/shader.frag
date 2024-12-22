#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform TransformInfo {
    mat4 MVP;
    mat4 ViewMatrix;
    mat4 ModelViewMatrix;
} Transform;

layout(binding = 1) uniform LightInfo 
{
	vec3 Ldir;
	vec3 La;
	vec3 Ld;
	vec3 Ls;
} Light;

layout(binding = 2) uniform MaterialInfo 
{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Shininess;
} Material;

 vec3 ads() 
 {
  	vec4 s4 = Transform.ViewMatrix*vec4(Light.Ldir, 0.0);
	vec3 n = normalize(Normal);
	vec3 v = normalize(-Position);
	vec3 s = normalize(-vec3(s4));
	vec3 r = reflect(-s, n);
	float dRate = max(dot(s, n), 0.0);
	float sRate = pow(max(dot(r, v), 0.0), Material.Shininess);
	vec3 ambient = Light.La * Material.Ka;
	vec3 difusse = Light.Ld * Material.Kd * dRate;
	vec3 specular = Light.Ls * Material.Ks * sRate;
	return ambient + difusse + specular;
 }

void main() 
{
    vec3 Color = ads();
    outColor = vec4(Color, 1.0);
}