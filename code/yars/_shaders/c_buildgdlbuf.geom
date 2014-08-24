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
	vec3  triCentroid;
} vertexOut;

//uniform const float distMax;
 
void xmain() {
  // euclidean length in Clip Space
  float a = length(gl_in[1].gl_Position - gl_in[0].gl_Position);
  float b = length(gl_in[2].gl_Position - gl_in[1].gl_Position);
  float c = length(gl_in[0].gl_Position - gl_in[2].gl_Position);

  /*
  float a = length(clipPosition[1].xyz - clipPosition[0].xyz);
  float b = length(clipPosition[2].xyz - clipPosition[1].xyz);
  float c = length(clipPosition[0].xyz - clipPosition[2].xyz);
  */

  //semiperimeter
  float p = (a + b + c) / 2.0f;

  //Heron's formula
  float areaTriangle = max(sqrt( p * (p - a) * (p - b) * (p - c) ), 1e-16);	//avoid zero division
  float radius = (a * b * c) / (4.0f * areaTriangle);
  
  //float diskArea = max(sqrt(distMax), radius * radius * 3.14159265);		//guaranteed minumum area
  
  //float diskArea = 1.0;
  //float diskArea = areaTriangle;
  //float diskArea = areaTriangle/3;  
  //float diskArea = radius * radius;
  float diskArea = radius * radius * 3.14159265;    
  //float diskArea = max( radius * radius * 3.14159265, 1.0);
  
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

/*
float calcDiskArea(in const float triangleArea, in const float a, in const float b, in const float c)  {
  //cirscoscritto
  //float radius = (a * b * c) / (4.0f * triangleArea);
  
  //inscritto
  float radius = triangleArea/((a+b+c)*0.5);
  //float diskArea = max(sqrt(distMax), radius * radius * 3.14159265);		//guaranteed minumum area
  
  //float diskArea = 1.0;
  //float diskArea = areaTriangle;
  //float diskArea = areaTriangle/3;  
  //float diskArea = radius * radius;

  //float diskArea = radius * radius * 3.14159265; 
  float diskArea = radius;
  return diskArea;
}
*/

void main() {
  /*
  vec4 p0CS = vertexIn[0].posCS;
  vec4 p1CS = vertexIn[1].posCS;
  vec4 p2CS = vertexIn[2].posCS;

  const float oneThird = 1.0/3.0;
  vec3 triangleCentroid = vec3( oneThird*(p0CS.x + p1CS.x + p2CS.x), 
                                oneThird*(p0CS.y + p1CS.y + p2CS.y),
								oneThird*(p0CS.z + p1CS.z + p2CS.z) );
  */

  /*
  // euclidean length in Clip Space
  float a = length(p1 - p0);
  float b = length(p2 - p1);
  float c = length(p0 - p2);

  float triangleArea = calcTriangleArea(a, b, c);

  float diskArea = calcDiskArea(triangleArea, a, b, c);
  */
     
  vec4 p0 = gl_in[0].gl_Position;
  vec4 p1 = gl_in[1].gl_Position;
  vec4 p2 = gl_in[2].gl_Position;

  vec3 triangleCentroid = calcTriangleCentroid( vertexIn[0].posCS, vertexIn[1].posCS, vertexIn[2].posCS );

  //float diskArea = getArea(p0, p1, p2);
  float diskArea = getArea(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position);
  
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
	//vertexOut.diskArea      = length(vertexIn[i].posCS.xyz-triangleCentroid); //diskArea;
	vertexOut.triCentroid     = triangleCentroid; //(modelViewMatrix * vec4(triangleCentroid, 0.0)).xyz;
	//vertexOut.triCentroid   = length(vertexIn[i].posCS-vec3(triangleCentroid)); //(modelViewMatrix * vec4(triangleCentroid, 0.0)).xyz;

    // done with the vertex
    EmitVertex();
  }
}

#endif