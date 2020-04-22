#version 450
#extension GL_ARB_separate_shader_objects : enable

// Texture sampler uniform
layout(binding = 2) uniform sampler2D texSampler;

// Input values to the fragment
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec4 fragLightVector;
layout(location = 3) in vec4 fragEyeVector;
layout(location = 4) in vec3 fragSpecularLighting;
layout(location = 5) in vec3 fragDiffuseLighting;
layout(location = 6) in vec3 fragAmbientLighting;
layout(location = 7) in float fragSpecularCoefficient;
layout(location = 8) in vec3 fragNormal;
layout(location = 9) in vec4 stagesInfo;
layout(location = 10) in float fragSpecularIntensity;
layout(location = 11) in float fragDiffuseIntensity;
layout(location = 12) in float fragAmbientIntensity;


// Output color of the fragment
layout(location = 0) out vec4 outColor;

void main() {
	
	// Calculate ambient component
	vec4 ambientLight = vec4(0);
	if(stagesInfo.x > 0.5)
	ambientLight = vec4(fragAmbientLighting * fragColor,1.0) *  fragAmbientIntensity;

	// Normalize the vectors
	vec4 normEyeVector = normalize(fragEyeVector);
	vec4 normLightVector = normalize(fragLightVector);
	vec4 normNormal = vec4(normalize(fragNormal),1.0);

	// Calculate the diffuse component
	float diffuseDotProduct = dot(normLightVector, normNormal);
	vec4 diffuseLight = vec4(0);
	if(stagesInfo.y >0.5)
	diffuseLight = vec4(fragAmbientLighting * fragColor * diffuseDotProduct,1.0) *  fragDiffuseIntensity;

	// Calculate the specular component
	vec4 halfAngleVector = normalize((normEyeVector + normLightVector)/2.0);
	float specularDotProduct = dot(halfAngleVector, normNormal);
	float specularPower = pow(max(0.0f,specularDotProduct), fragSpecularCoefficient);
	vec4 specularLight = vec4(0);
	if(stagesInfo.z >0.5)
	
	{
	specularPower = min(max(specularPower, 0.0), 1.0);
	specularLight = vec4( fragSpecularLighting * fragColor * specularPower *  fragSpecularIntensity,1.0) ;
	
	 }
	// Calculate the total lighting
	vec4 lightingColor = ambientLight + diffuseLight + specularLight;

	// Set the lighting color to 1 when there is no lighting to show the texture
	if(lightingColor == vec4(0))
	lightingColor = vec4(1);
	vec4 textureColor = texture(texSampler, fragTexCoord);

	// Set the output color
	if(stagesInfo.w >0.5)
	    outColor =min(lightingColor * textureColor,vec4(1.0));
	else
	outColor =min(lightingColor ,vec4(1.0));

}