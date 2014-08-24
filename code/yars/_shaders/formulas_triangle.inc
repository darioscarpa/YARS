#ifndef FORMULAS_TRIANGLE_INC
#define FORMULAS_TRIANGLE_INC

float calcTriangleArea(const in float a, const in float b, const in float c) {
  //semiperimeter
  float p = (a + b + c) * 0.5;

  //Heron's formula
  float triangleArea = max(sqrt( p * (p - a) * (p - b) * (p - c) ), 1e-16);	//avoid zero division

  return triangleArea;
}

float calcTriangleAreaByVertices(const in vec4 p0, const in vec4 p1, const in vec4 p2) {
	float a = length(p1 - p0);
	float b = length(p2 - p1);
	float c = length(p0 - p2);

	float triangleArea = calcTriangleArea(a, b, c);
	
	return triangleArea;
}

float calcInscribedCircleRadiusByVertices(const in vec4 p0, const in vec4 p1, const in vec4 p2) {
	float a = length(p1 - p0);
	float b = length(p2 - p1);
	float c = length(p0 - p2);

	float triangleArea = calcTriangleArea(a, b, c);

	float radius = triangleArea / ( (a+b+c) * 0.5 );
	return radius;  
}

float calcCircumscribedCircleRadiusByVertices(const in vec4 p0, const in vec4 p1, const in vec4 p2) {
	float a = length(p1 - p0);
	float b = length(p2 - p1);
	float c = length(p0 - p2);

	float triangleArea = calcTriangleArea(a, b, c);

	float radius = (a*b*c) / (4.0f * triangleArea);
	return radius;  
}
 
vec3 calcTriangleCentroid(const in vec4 p0, const in vec4 p1, const in vec4 p2) {
	const float oneThird = 1.0/3.0;
	vec3 triangleCentroid = vec3( oneThird*(p0.x + p1.x + p2.x), 
                                  oneThird*(p0.y + p1.y + p2.y),
								  oneThird*(p0.z + p1.z + p2.z) );
	return triangleCentroid;
}

#endif