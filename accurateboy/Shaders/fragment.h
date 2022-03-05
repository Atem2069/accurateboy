R"(
#version 460 core


layout(location=0)in vec2 texcoord;
layout(location=0)out vec4 fragColor;

layout(binding=0)uniform sampler2D texSampler;
layout(binding=1)uniform sampler2D lastFrame;

layout(location=0)uniform int blend;


void main()
{
	vec3 col = texture(texSampler,texcoord).xyz;

	vec3 lastFrameRes = texture(lastFrame,texcoord).xyz;

	if(blend==1)
		col = mix(col,lastFrameRes,0.5);

	fragColor = vec4(col,1.0f);
}


)"