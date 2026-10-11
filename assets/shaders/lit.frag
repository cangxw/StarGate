#version 330 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vWorldPosition;

uniform sampler2D uTexture;

uniform vec3 uToLightDirection;
uniform vec3 uLightColor;
uniform float uAmbientStrength;
uniform vec3 uCameraPosition;
uniform float uSpecularStrength;
uniform float uShininess;

out vec4 fragmentColor; // 输出颜色

void main()
{
	// texture ：插值后的uv，在纹理中取色
	vec4 baseColor = texture(uTexture, vTexCoord); 

	vec3 normal = normalize(vNormal);
	vec3 toLight = normalize(uToLightDirection);
	
	float diffuse = max(dot(normal, toLight), 0.0);

	// 环境光暂时使用白色
	vec3 ambientLight = vec3(uAmbientStrength);
	
	// 漫反射使用光源颜色
	vec3 diffuseLight = 0.8 * diffuse * uLightColor;

	// 高光部分
	// 从表面指向相机	
	vec3 toCamera = normalize(uCameraPosition - vWorldPosition);
	// 光线照射到表面后 反射的方向 Phong模型比较相机方向和反射方向的夹角大小
	// vec3 reflectedDirection = reflect(-toLight, normal);

	float specular = 0.0;
	// 光源位于表面正面时，才计算高光
	if(diffuse > 0.0)
	{
		// 半程向量，Blinn-Phong模型比较半程和法线的夹角
		vec3 halfwayDirection = normalize(toLight + toCamera);

		specular = pow(
			max(dot(normal, halfwayDirection), 0.0),
			uShininess
		);
	}

	vec3 specularLight = uSpecularStrength * specular * uLightColor;

	// 光照贡献可以叠加。例如一个表面同时被两盏灯照亮，可以分别计算每盏灯的贡献，再相加
	fragmentColor = vec4(
		baseColor.rgb * (ambientLight + diffuseLight) + specularLight, 
		baseColor.a
	);
}