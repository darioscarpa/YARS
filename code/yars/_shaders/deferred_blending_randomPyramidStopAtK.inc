
void technique_randomPyramidStopAtK(const in mat3 kernelBasis, const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea, out float ambientOcclusion, out vec3 indirectLight) {
	int samples   = halfSamplesPerDir * 2 * 4;	
	
	for (int i = 0; i < PYRAMID_DIVS; ++i) {
		pyrDist[i] = 10e5;
		pyrOcclusion[i]  = 0;
		pyrIndLight[i]   = vec3(0.0);
		pyrSolidAngle[i] = 0;
	}
	
	float sphereArea = 4 * PI * (radLength * radLength);
	float triArea    = (sphereArea*0.5) * INV_PYRAMID_DIVS;

	for  (int i = 0 ; i < samples ; ++i) {
		//// CALCULATE SAMPLE POSITION IN SS /////////////////////////////////////////
		//vec2  samplePosSS     = calcScreenSpaceEmisphereSamplePos(i, kernelBasis, receiverPosition, radLength);		
		vec2  samplePosSS     = samplesPos[i];
		
		//// GET SAMPLE POINT VALUES FROM GBUFFER /////////////////////////////////////////
		vec3  emitterPosition = gbufGetPosition(samplePosSS);
		
		vec3  recToEmi = emitterPosition-receiverPosition;
		float recToEmiLen = length(recToEmi);
		
		if ( recToEmiLen <= distMax ) {
			
			vec3  emitterNormal   = gbufGetNormal(samplePosSS);
			float emitterArea     = gbufGetArea(samplePosSS)*areaMultiplier;				
		
			vec3 intersection;
			int triId = getGlobalIntersectingTriangleId(recToEmi,
								     kernelBasis,
									 receiverPosition,
									 emitterPosition,
									 intersection);
			if (triId==-1) continue;

			float solidAngle = computeSolidAngle( emitterPosition, emitterNormal, emitterArea, 
			                                      receiverPosition, receiverNormal);

			vec3 dirDiffL = dlbufGetScatteredLight(samplePosSS);
			pyrIndLight[triId] += dirDiffL * formFactor_d2d(emitterPosition, emitterNormal, emitterArea,
				 				receiverPosition, receiverNormal, triArea);

			pyrSolidAngle[triId] += solidAngle;
			pyrSolidAngle[triId] = min(pyrSolidAngle[triId], PYRAMID_DIVS_SOLID_ANGLE);
			//solidAngle = min(solidAngle, PYRAMID_DIVS_SOLID_ANGLE);
			
			/*
			if (solidAngle > pyrSolidAngle[triId]) {				
				pyrSolidAngle[triId] = solidAngle;

				//// CALCULATE OCCLUSION/INDIRECT LIGHTING /////////////////////////////////////////
				float sampleWeight    = 1.0;

				float tmpambientOcclusion = computeSampleAmbientOcclusion(
																			//emitterPosition, 
																			receiverPosition + normalize(recToEmi)*radLength,
																			-recToEmi,
																			//emitterNormal, 
																			
																			triArea*(solidAngle/PYRAMID_DIVS_SOLID_ANGLE),																		    
																			//emitterArea, 
																			receiverPosition, receiverNormal, receiverArea,
																            sampleWeight);
				vec3 emitterDirectDiffuseLighting =  dlbufGetScatteredLight(samplePosSS);			
				vec3 tmpindirectLight = computeSampleIndirectLight(emitterPosition, emitterNormal, emitterArea, 
													    receiverPosition, receiverNormal, receiverArea, 
														emitterDirectDiffuseLighting, sampleWeight);
				pyrOcclusion[triId] = tmpambientOcclusion;
				pyrIndLight[triId] += tmpindirectLight;

			}
			*/
		}	
	}
		
	//////////////////////////////
	
	float sa = 0.0;
	ambientOcclusion = 0.0;
	indirectLight    = vec3(0.0);	
	for (int i = 0; i < PYRAMID_DIVS; ++i) {
		ambientOcclusion += pyrOcclusion[i];
		indirectLight    += pyrIndLight[i];	
		sa += pyrSolidAngle[i];
	}
	
	ambientOcclusion = sa / (2*PI);
	//ambientOcclusion = pow(ambientOcclusion, aoMultiplier);

	//indirectLight    = indirectLight / (2*PI);// / PYRAMID_DIVS;  
	indirectLight    = indirectLight * INV_PYRAMID_DIVS;  

	//ambientOcclusion = ambientOcclusion;// / PYRAMID_DIVS;
	//indirectLight    = indirectLight;// / PYRAMID_DIVS;  
}

vec3 getSampleSquareColor_randomPyramidStopAtK( const in vec2 currPixelSC ) {	
	vec2 mousePixelTC = mousePos * invScreenSize;
	
	vec3  receiverPosition = gbufGetPosition(mousePixelTC);
	vec3  receiverNormal   = gbufGetNormal(mousePixelTC);	
	float receiverArea     = gbufGetArea(mousePixelTC)*areaMultiplier;

	calcSamplesPos(mousePixelTC, halfSamplesPerDir, radLength, receiverPosition, receiverNormal);	

	int samples   = halfSamplesPerDir * 2 * 4;
		
	mat3 kernelBasis = getOrthoBasis(mousePixelTC, receiverNormal);
			
	for(int i = 0; i < samples; ++i) {
	    vec2 currSamplePos = samplesPos[i] * screenSize;

		float horiz = abs(currSamplePos.x - currPixelSC.x);
		float vert  = abs(currSamplePos.y - currPixelSC.y);
		
		float emitterArea   = gbufGetArea(samplesPos[i])*areaMultiplier;
		
		//float sampleVisSize = sqrt(emitterArea/3.14);
		//float sampleVisSize = 10*sqrt(emitterArea/3.14);
		//float sampleVisSize = gbufGetArea(10*sqrt(emitterArea/3.14);
		
		//float sampleVisSize = 50 * (1 - gbufGetDepth(samplesPos[i])); //100*(gbufGetPosition(samplesPos[i]).z);	
		float sampleVisSize = 3.0;
		//float sampleVisSize = 3.0 +( 16 * uKernelWeight[i]);



		if ( horiz <= sampleVisSize && vert <= sampleVisSize ) {	
			vec3 sampled = vec3(0.0);
			
			vec3  emitterPosition = gbufGetPosition(samplesPos[i]);	
			vec3  V = emitterPosition - receiverPosition;
				
			float dist = length(V);
			//if (dist > distMax) break;
			//if (dist > distMax) {
			
			//if (dist > radLength) break;
			
			//if (dist <= radLength) {
			if (dist <= distMax) {
			
				vec3  recToEmi = emitterPosition-receiverPosition;
					
				vec3 intersection;
				int triId = getGlobalIntersectingTriangleId(
				//int triId = getIntersectingTriangleId(
									 recToEmi,
								     kernelBasis,
									 receiverPosition,
									 emitterPosition,
									 intersection);

			    if (triId != -1) {
					vec3  emitterNormal = gbufGetNormal(samplesPos[i]);

					float sa_val = computeSolidAngle( emitterPosition, emitterNormal,  emitterArea, 
							  receiverPosition, receiverNormal);
					sampled = vec3(sa_val/PYRAMID_DIVS_SOLID_ANGLE);

				} else {
					sampled = vec3(1,1,0);
				}
				
				//float normalDiff = clamp( dot(receiverNormal, emitterNormal), 0.0, 1.0);	
				//sampled = vec3(normalDiff, 1-normalDiff, 0.0);							
				
				if (horiz <= sampleVisSize-1 && vert <= sampleVisSize-1) {
					float sampleWeight = 1.0;
					
					//float ambientOcclusion = computeSampleAmbientOcclusion(emitterPosition, emitterNormal, emitterArea, 
					//												receiverPosition, receiverNormal, receiverArea, sampleWeight);							
					//sampled  = vec3(1-ambientOcclusion);

					if (triId!=-1) {
						sampled = colorid[triId];
					} else {
						sampled = vec3(-1);
					}
					/*
					if (horiz <= sampleVisSize*0.5 && vert <= sampleVisSize*0.5) {
						vec3  emitterDirectDiffuseLighting =  dlbufGetScatteredLight(samplesPos[i]);
						vec3  indirectLight = computeSampleIndirectLight(emitterPosition, emitterNormal, emitterArea, 
																	receiverPosition, receiverNormal, receiverArea, 
																	emitterDirectDiffuseLighting, sampleWeight);
			
						sampled = indirectLight; 
					}
					*/
				}
			} else {
				sampled = vec3(1.0, 0.0, 0.0);
				if (horiz <= sampleVisSize-1 && vert <= sampleVisSize-1) {
					sampled = vec3(-1);
				}
			}			
			return sampled;				
		} 
	}
	return vec3(-1.0);
}