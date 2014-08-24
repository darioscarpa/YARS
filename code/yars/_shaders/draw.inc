
/*
vec3 drawPointSC( const in vec2 p, const in int sizeInPixel, const in vec3 color ) {
	vec2 intersectionPointTC = p.xy;
	vec2 currPixel = vec2(gl_FragCoord.x, gl_FragCoord.y);
	
	float horiz = abs(intersectionPointTC.x - currPixelSC.x);
	float vert  = abs(intersectionPointTC.y - currPixelSC.y);
		
	vec3 ret = vec3(0.0, 0.0, 0.0);
	if (horiz < 5 && vert < 5) {
		ret = color;
	}
	return ret
}
*/

float drawPoint(const in vec2 currPixel, const in vec2 p, const in int size) {
	
	//vec2 uv = currPixel * invScreenSize;
	//p = p * invScreenSize;
	//float horiz = abs(p.x - uv.x);
	//float vert  = abs(p.y - uv.y);

	float d = abs(distance(currPixel, p)) * invScreenSize;

	const float THICKNESS = 0.001 * size;
	return mix(1.0, 0.0, smoothstep(0.5 * THICKNESS, 1.5 * THICKNESS, d));	
}

float drawLine(const in vec2 currPixel, const in vec2 p1, const in vec2 p2, const in float thicknessLevel) {
  const float THICKNESS = 0.001 * thicknessLevel;
  vec2 uv = currPixel / screenSize.xy;

  float a = abs(distance(p1, uv));
  float b = abs(distance(p2, uv));
  float c = abs(distance(p1, p2));

  if ( a >= c || b >=  c ) return 0.0;

  float p = (a + b + c) * 0.5;

  // median to (p1, p2) vector
  float h = 2 / c * sqrt( p * ( p - a) * ( p - b) * ( p - c));

  return mix(1.0, 0.0, smoothstep(0.5 * THICKNESS, 1.5 * THICKNESS, h));
  //val = mix(1.0, 0.0, smoothstep(0.5 * THICKNESS, 1.5 * THICKNESS, h));
  //return color*val;
}
