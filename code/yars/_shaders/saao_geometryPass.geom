#version 420

#include ubo_matrices.inc

#include subroutinesConfig.inc

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

const float PI = 3.1415926535897932384626433832795;

in VertexData {
	vec4 posCS;
	vec2 texCoord;

	vec3 normalCS;
	vec3 tangentCS;
	vec3 bitangentCS;

	vec4 eyeCS;
} vertexIn[3];
 
out VertexData {
	vec4 posCS;
	vec2 texCoord;

	vec3 normalCS;
	vec3 tangentCS;
	vec3 bitangentCS;	

	vec4 eyeCS;

	//new stuff
	float diskArea;	
} vertexOut;

/////////////////////////////////////////////////////////////
 
#include formulas_triangle.inc

/////////////////////////////////////////////////////////////

#ifdef USE_SUBROUTINE_AREACALCULATOR

subroutine float   AreaCalculator(const in vec4 p0, const in vec4 p1, const in vec4 p2);
subroutine uniform AreaCalculator getArea;

#endif

#ifdef USE_SUBROUTINE_AREACALCULATOR
subroutine (AreaCalculator)
#endif
float getAreaUnitValue(const in vec4 p0, const in vec4 p1, const in vec4 p2) {
	return 1.0;
}

#ifdef USE_SUBROUTINE_AREACALCULATOR
subroutine (AreaCalculator)
#endif
float getAreaTriangleArea(const in vec4 p0, const in vec4 p1, const in vec4 p2) {    
	float triangleArea = calcTriangleAreaByVertices(p0, p1, p2);
	return triangleArea;
}

#ifdef USE_SUBROUTINE_AREACALCULATOR
subroutine (AreaCalculator)
#endif
float getAreaInscribedCircleArea(const in vec4 p0, const in vec4 p1, const in vec4 p2) {	
	float radius = calcInscribedCircleRadiusByVertices(p0, p1, p2);
	return radius*radius*PI;	
}

#ifdef USE_SUBROUTINE_AREACALCULATOR
subroutine (AreaCalculator)
#endif
float getAreaCircumscribedCircleArea(const in vec4 p0, const in vec4 p1, const in vec4 p2) {	
	float radius = calcCircumscribedCircleRadiusByVertices(p0, p1, p2);
	return radius*radius*PI;	
}

#ifdef USE_SUBROUTINE_AREACALCULATOR
subroutine (AreaCalculator)
#endif
float getAreaInscribedCircleRadius(const in vec4 p0, const in vec4 p1, const in vec4 p2) {	
	float radius = calcInscribedCircleRadiusByVertices(p0, p1, p2);
	return radius;	
}

#ifdef USE_SUBROUTINE_AREACALCULATOR
subroutine (AreaCalculator)
#endif
float getAreaCircumscribedCircleRadius(const in vec4 p0, const in vec4 p1, const in vec4 p2) {	
	float radius = calcCircumscribedCircleRadiusByVertices(p0, p1, p2);
	return radius;
}

#ifdef USE_SUBROUTINE_AREACALCULATOR
subroutine (AreaCalculator)
#endif
float getAreaInscribedCircleRadiusSquared(const in vec4 p0, const in vec4 p1, const in vec4 p2) {	
	float radius = calcInscribedCircleRadiusByVertices(p0, p1, p2);
	return radius*radius;	
}

#ifdef USE_SUBROUTINE_AREACALCULATOR
subroutine (AreaCalculator)
#endif
float getAreaCircumscribedCircleRadiusSquared(const in vec4 p0, const in vec4 p1, const in vec4 p2) {	
	float radius = calcCircumscribedCircleRadiusByVertices(p0, p1, p2);
	return radius*radius;
}

#ifndef USE_SUBROUTINE_AREACALCULATOR

uniform int getAreaSelector;

float getArea(const in vec4 p0, const in vec4 p1, const in vec4 p2) {
	switch(getAreaSelector) {
		case 0: return getAreaUnitValue(p0, p1, p2);
		case 1: return getAreaTriangleArea(p0, p1, p2);
		case 2: return getAreaInscribedCircleArea(p0, p1, p2);
		case 3: return getAreaCircumscribedCircleArea(p0, p1, p2);
		case 4: return getAreaInscribedCircleRadius(p0, p1, p2);
		case 5: return getAreaCircumscribedCircleRadius(p0, p1, p2);
		case 6: return getAreaInscribedCircleRadiusSquared(p0, p1, p2);
		case 7: return getAreaCircumscribedCircleRadiusSquared(p0, p1, p2);	
	}	
}

/////////////////////////////////////////////////////////////

void main() {
      
  vec4 p0 = gl_in[0].gl_Position;
  vec4 p1 = gl_in[1].gl_Position;
  vec4 p2 = gl_in[2].gl_Position;
    
  float diskArea = getArea(p0, p1, p2);
  
  for(int i = 0; i < gl_in.length(); i++) {
    // copy attributes
    gl_Position = gl_in[i].gl_Position;

	vertexOut.posCS       = vertexIn[i].posCS;
	vertexOut.texCoord    = vertexIn[i].texCoord;

	vertexOut.normalCS    = vertexIn[i].normalCS;
	vertexOut.tangentCS   = vertexIn[i].tangentCS;
	vertexOut.bitangentCS = vertexIn[i].bitangentCS;
	
	vertexOut.eyeCS           = vertexIn[i].eyeCS;

	vertexOut.diskArea        = diskArea;
	
    // done with the vertex
    EmitVertex();
  }
}

#endif