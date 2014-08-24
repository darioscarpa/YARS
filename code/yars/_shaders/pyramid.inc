
#constant const int TRIANGLE_DIVS = #TRIANGLE_DIVS#;
const int TRIANGLE_DIVS_SQUARED = TRIANGLE_DIVS*TRIANGLE_DIVS;
const int PYRAMID_DIVS = TRIANGLE_DIVS_SQUARED * 4;
const float INV_PYRAMID_DIVS = 1.0 / float(PYRAMID_DIVS);

const float PI = 3.1415926535897932384626433832795;
const float PYRAMID_DIVS_SOLID_ANGLE = 2*PI * INV_PYRAMID_DIVS;

const float INV_2PI = 1 / (2*PI);

float pyrSolidAngle[PYRAMID_DIVS];
float pyrOcclusion[PYRAMID_DIVS];

vec3  pyrIndLight[PYRAMID_DIVS];
float pyrDist[PYRAMID_DIVS];

vec3  pyrDirLight[PYRAMID_DIVS];
int   pyrSamplesCont[PYRAMID_DIVS];
int   pyrOcclusorCont[PYRAMID_DIVS];

/*
const vec3 colorid[TRIANGLE_DIVS_SQUARED] = vec3[TRIANGLE_DIVS_SQUARED](
	vec3(1, 0, 0),
	vec3(0, 1, 0),
	vec3(0, 0, 1),
	vec3(1, 1, 0)
);

const vec3 colorid[16] = vec3[16](
	vec3(1, 0, 0),
	vec3(0, 1, 0),
	vec3(0, 0, 1),
	vec3(1, 1, 0),
	
	vec3(1, 0, 0),
	vec3(0, 1, 0),
	vec3(0, 0, 1),
	vec3(1, 1, 0),

	vec3(1, 0, 0),
	vec3(0, 1, 0),
	vec3(0, 0, 1),
	vec3(1, 1, 0),

	vec3(1, 0, 0),
	vec3(0, 1, 0),
	vec3(0, 0, 1),
	vec3(1, 1, 0)
);

*/

/*
//CGA - http://en.wikipedia.org/wiki/Web_colors
const vec3 colorid[16] = vec3[16](
	// octant I
	vec3(1.0 , 0.0 , 0.0 ), // 12 high red
	vec3(0.0 , 1.0 , 0.0 ), // 10 high green
	vec3(0.0 , 0.0 , 1.0 ), //  9 high blue
	vec3(1.0 , 1.0 , 0.0 ), // 14 yellow

	// octant II
	vec3(0.5 , 0.0 , 0.0 ), //  4 low red
	vec3(0.0 , 0.5 , 0.0 ), //  2 low green 	
	vec3(0.0 , 0.0 , 0.5 ), //  1 low blue
	vec3(0.5 , 0.5 , 0.0 ), //  6 brown
		
	// octant III
	vec3(0.0 , 0.5 , 0.5 ), //  3 low cyan	
	vec3(0.0 , 1.0 , 1.0 ), // 11 high cyan
	vec3(0.5 , 0.0 , 0.5 ), //  5 low magenta
	vec3(1.0 , 0.0 , 1.0 ), // 13 high magenta
	
	// octant IV	
	vec3(1.0 , 1.0 , 1.0 ), // 15 white
	vec3(0.75, 0.75, 0.75), //  7 light gray
	vec3(0.5 , 0.5 , 0.5 ), //  8 dark gray
	//vec3(0.0 , 0.0 , 0.0 )  //  0 black	
	vec3(0.25 , 0.25 , 0.25 )  //  0 black	
);
*/

const int quadrantOffset[7] = int[7](
	2*TRIANGLE_DIVS_SQUARED,    // 0: 3rd quadrant
	0,
	3*TRIANGLE_DIVS_SQUARED,    // 2: 4th quadrant
	0,
	TRIANGLE_DIVS_SQUARED,      // 4: 2nd quadrant
	0,
	0                           // 6: 1st quadrant
);
int getQuadrantOffsetByCoords(const in vec3 coords) {
	int i = int(sign(coords.x)) + 1;
	int j = int(sign(coords.z)) + 1;
	int off = i+2*j;
	return quadrantOffset[off];
}

const vec3 colorid_q[4] = vec3[4](
	// octant I
	vec3(1.0 , 0.0 , 0.0 ), // red

	// octant II
	vec3(0.0 , 1.0 , 0.0 ), // green
		
	// octant III
	vec3(0.0 , 0.0 , 1.0 ), // blug
	
	// octant IV	
	vec3(1.0 , 1.0 , 0.0 )  // yellow
);

/*
const vec3 colorid[16] = vec3[16](
	// octant I
	vec3(1.0 , 0.0 , 0.0 ),  // red
	vec3(0.75 , 0.0 , 0.0 ),
	vec3(0.5 , 0.0 , 0.0 ), 
	vec3(0.25 , 0.0 , 0.0 ),

	// octant II
	vec3(0.0 , 1.0 , 0.0 ),  // green
	vec3(0.0 , 0.75 , 0.0 ), 
	vec3(0.0 , 0.5 , 0.0 ), 
	vec3(0.0 , 0.25 , 0.0 ),
		
	// octant III
	vec3(0.0 , 0.0 , 1.0 ),  // blue
	vec3(0.0 , 0.0 , 0.75 ), 
	vec3(0.0 , 0.0 , 0.5 ), 
	vec3(0.0 , 0.0 , 0.25 ),
	
	// octant IV	
	vec3(1.0 , 1.0 , 0.0 ),  // yellow
	vec3(0.75, 0.75, 0.0), 
	vec3(0.5 , 0.5 , 0.0 ),
	vec3(0.25 , 0.25 , 0.0 ) 
);
*/

const vec3 colorid[MAX_KERNEL_SIZE] = vec3[MAX_KERNEL_SIZE](
	// octant I
	vec3(1.0 , 0.0 , 0.0 ),  // red
	vec3(0.75 , 0.0 , 0.0 ),
	vec3(0.5 , 0.0 , 0.0 ), 
	vec3(0.25 , 0.0 , 0.0 ),

	// octant II
	vec3(0.0 , 1.0 , 0.0 ),  // green
	vec3(0.0 , 0.75 , 0.0 ), 
	vec3(0.0 , 0.5 , 0.0 ), 
	vec3(0.0 , 0.25 , 0.0 ),
		
	// octant III
	vec3(0.0 , 0.0 , 1.0 ),  // blue
	vec3(0.0 , 0.0 , 0.75 ), 
	vec3(0.0 , 0.0 , 0.5 ), 
	vec3(0.0 , 0.0 , 0.25 ),
	
	// octant IV	
	vec3(1.0 , 1.0 , 0.0 ),  // yellow
	vec3(0.75, 0.75, 0.0), 
	vec3(0.5 , 0.5 , 0.0 ),
	vec3(0.25 , 0.25 , 0.0 ),
	
	// octant I
	vec3(1.0 , 0.0 , 0.0 ),  // red
	vec3(0.75 , 0.0 , 0.0 ),
	vec3(0.5 , 0.0 , 0.0 ), 
	vec3(0.25 , 0.0 , 0.0 ),

	// octant II
	vec3(0.0 , 1.0 , 0.0 ),  // green
	vec3(0.0 , 0.75 , 0.0 ), 
	vec3(0.0 , 0.5 , 0.0 ), 
	vec3(0.0 , 0.25 , 0.0 ),
		
	// octant III
	vec3(0.0 , 0.0 , 1.0 ),  // blue
	vec3(0.0 , 0.0 , 0.75 ), 
	vec3(0.0 , 0.0 , 0.5 ), 
	vec3(0.0 , 0.0 , 0.25 ),
	
	// octant IV	
	vec3(1.0 , 1.0 , 0.0 ),  // yellow
	vec3(0.75, 0.75, 0.0), 
	vec3(0.5 , 0.5 , 0.0 ),
	vec3(0.25 , 0.25 , 0.0 ),

	// octant I
	vec3(1.0 , 0.0 , 0.0 ),  // red
	vec3(0.75 , 0.0 , 0.0 ),
	vec3(0.5 , 0.0 , 0.0 ), 
	vec3(0.25 , 0.0 , 0.0 ),

	// octant II
	vec3(0.0 , 1.0 , 0.0 ),  // green
	vec3(0.0 , 0.75 , 0.0 ), 
	vec3(0.0 , 0.5 , 0.0 ), 
	vec3(0.0 , 0.25 , 0.0 ),
		
	// octant III
	vec3(0.0 , 0.0 , 1.0 ),  // blue
	vec3(0.0 , 0.0 , 0.75 ), 
	vec3(0.0 , 0.0 , 0.5 ), 
	vec3(0.0 , 0.0 , 0.25 ),
	
	// octant IV	
	vec3(1.0 , 1.0 , 0.0 ),  // yellow
	vec3(0.75, 0.75, 0.0), 
	vec3(0.5 , 0.5 , 0.0 ),
	vec3(0.25 , 0.25 , 0.0 ),

	// octant I
	vec3(1.0 , 0.0 , 0.0 ),  // red
	vec3(0.75 , 0.0 , 0.0 ),
	vec3(0.5 , 0.0 , 0.0 ), 
	vec3(0.25 , 0.0 , 0.0 ),

	// octant II
	vec3(0.0 , 1.0 , 0.0 ),  // green
	vec3(0.0 , 0.75 , 0.0 ), 
	vec3(0.0 , 0.5 , 0.0 ), 
	vec3(0.0 , 0.25 , 0.0 ),
		
	// octant III
	vec3(0.0 , 0.0 , 1.0 ),  // blue
	vec3(0.0 , 0.0 , 0.75 ), 
	vec3(0.0 , 0.0 , 0.5 ), 
	vec3(0.0 , 0.0 , 0.25 ),
	
	// octant IV	
	vec3(1.0 , 1.0 , 0.0 ),  // yellow
	vec3(0.75, 0.75, 0.0), 
	vec3(0.5 , 0.5 , 0.0 ),
	vec3(0.25 , 0.25 , 0.0 )  
);