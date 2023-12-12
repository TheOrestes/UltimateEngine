#version 450

//---------------------------------------------------------------------------------------------------------------------
//-- Input from Vertex shader
layout(location = 0) in vec2 vs_outUV;
layout(location = 1) in vec3 vs_outNormal;

//---------------------------------------------------------------------------------------------------------------------
// -- Final Output color
layout(location = 0) out vec4 outColor;

//---------------------------------------------------------------------------------------------------------------------
//-- Uniforms
layout(set = 0, binding = 0) uniform mvpData
{
    mat4 World;
    mat4 View;
    mat4 Projection;

    vec4 albedoColor;
    vec4 emissionColor;
    vec4 hasTextureAEN;
    vec4 hasTextureRMO;
    float occlusion;
    float roughness;
    float metalness;

}shaderData;

//-- Textures
layout(set = 0, binding = 1) uniform sampler2D samplerAlbedoTexture;

//---------------------------------------------------------------------------------------------------------------------
void main()
{
    vec4 albedoColor = vec4(1.0f);

    //--- Albedo Color
    //if(shaderData.hasTextureAEN.r == 1)
    //{
    //    albedoColor = texture(samplerAlbedoTexture, vs_outUV);    
    //}
    
    albedoColor = texture(samplerAlbedoTexture, vs_outUV); 
    outColor = vec4(shaderData.albedoColor * albedoColor);
    //outColor = vec4(vs_outNormal, 1.0f);
}