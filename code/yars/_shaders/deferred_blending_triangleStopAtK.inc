
void technique_triangleStopAtK(const in mat3 kernelBasis, const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea, out float ambientOcclusion, out vec3 indirectLight) {
	
	ambientOcclusion = 0.0;
	indirectLight    = vec3(0.0);
		
	int samples   = halfSamplesPerDir * 2 * 4;		
	
	for (int i = 0; i < samples; ++i) {	
		//// CALCULATE SAMPLE POSITION IN SS /////////////////////////////////////////
		//vec2 samplePosSS      = calcScreenSpaceEmisphereSamplePos(i, kernelBasis, receiverPosition, radLength);
		vec2  samplePosSS     = samplesPos[i];
				
		//// GET SAMPLE POINT VALUES FROM GBUFFER /////////////////////////////////////////
		vec3  emitterPosition = gbufGetPosition(samplePosSS);
		
		float coveredSolidAngle = 2*3.14;
		//float coveredSolidAngle = 0;

		if (length(emitterPosition-receiverPosition) <= distMax) {

			vec3  emitterNormal   = gbufGetNormal(samplePosSS);
			float emitterArea     = gbufGetArea(samplePosSS)*areaMultiplier;
		
			//// CALCULATE SOLID ANGLE COVERED BY OCCLUSOR ///////////////////////////////////
			coveredSolidAngle = computeSolidAngle( emitterPosition, emitterNormal, emitterArea,
			                                       receiverPosition, receiverNormal);
					
			float sampleWeight = 1.0;//uKernelWeight[i];			
			//// CALCULATE OCCLUSION/INDIRECT LIGHTING /////////////////////////////////////////
			ambientOcclusion += computeSampleAmbientOcclusion(emitterPosition, emitterNormal, emitterArea, 
													            receiverPosition, receiverNormal, receiverArea, sampleWeight);
			vec3  emitterDirectDiffuseLighting =  dlbufGetScatteredLight(samplePosSS);			
			indirectLight += computeSampleIndirectLight(emitterPosition, emitterNormal, emitterArea, 
													    receiverPosition, receiverNormal, receiverArea, 
														emitterDirectDiffuseLighting, sampleWeight);

		}
	}
	
	ambientOcclusion = ambientOcclusion / samples;	//TODO invSamples
	indirectLight    = indirectLight / samples;    //TODO invSamples
}



vec3 getSampleSquareColor_triangleStopAtK( const in vec2 currPixelSC ) {	
	vec2 mousePixelTC = mousePos * invScreenSize;
	
	vec3  receiverPosition = gbufGetPosition(mousePixelTC);
	vec3  receiverNormal   = gbufGetNormal(mousePixelTC);	
	float receiverArea     = gbufGetArea(mousePixelTC)*areaMultiplier;

	calcSamplesPos(mousePixelTC, halfSamplesPerDir, radLength, receiverPosition, receiverNormal);	

	int samples   = halfSamplesPerDir * 2 * 4;

	
	for(int i = 0; i < samples; ++i) {	    
		vec2 currSamplePos = samplesPos[i] * screenSize;

		float horiz = abs(currSamplePos.x - currPixelSC.x);
		float vert  = abs(currSamplePos.y - currPixelSC.y);
		
		float emitterArea   = gbufGetArea(samplesPos[i])*areaMultiplier;
		
		//float sampleVisSize = sqrt(emitterArea/3.14);
		//float sampleVisSize = 10*sqrt(emitterArea/3.14);
		//float sampleVisSize = gbufGetArea(10*sqrt(emitterArea/3.14);
		
		//float sampleVisSize = 50 * (1 - gbufGetDepth(samplesPos[i])); //100*(gbufGetPosition(samplesPos[i]).z);	
		//float sampleVisSize = 5.0;
		float sampleVisSize = 3.0 +( 16 * uKernelWeight[i]);

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
			
				vec3  emitterNormal = gbufGetNormal(samplesPos[i]);				
				float normalDiff = clamp( dot(receiverNormal, emitterNormal), 0.0, 1.0);					
	
				//sampled = vec3(normalDiff, 1-normalDiff, 0.0);

				float coveredSolidAngle = computeSolidAngle( emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal);								
				
				if (horiz <= sampleVisSize-1 && vert <= sampleVisSize-1) {
					float sampleWeight = uKernelWeight[i];
					float ambientOcclusion = computeSampleAmbientOcclusion(emitterPosition, emitterNormal, emitterArea, 
																	receiverPosition, receiverNormal, receiverArea, sampleWeight);							
					sampled  = vec3(1-ambientOcclusion);

					if (horiz <= sampleVisSize*0.5 && vert <= sampleVisSize*0.5) {
						vec3  emitterDirectDiffuseLighting =  dlbufGetScatteredLight(samplesPos[i]);
						vec3  indirectLight = computeSampleIndirectLight(emitterPosition, emitterNormal, emitterArea, 
																	receiverPosition, receiverNormal, receiverArea, 
																	emitterDirectDiffuseLighting, sampleWeight);
			
						sampled = indirectLight; 
					}
				}
			} else {
				sampled = vec3(1.0, 0.0, 0.0);
			}			
			return sampled;				
		} 
	}
	return vec3(-1.0);
}