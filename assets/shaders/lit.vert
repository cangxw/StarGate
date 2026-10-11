	#version 330 core

	layout(location = 0) in vec3 aPosition; // 顶点位置输入
	layout(location = 1) in vec2 aTexCoord; // 纹理输入
	layout(location = 2) in vec3 aNormal; // 法线

	uniform mat4 uTransform;
	uniform mat4 uProjection;
	uniform mat4 uView;

	//GL_MAX_VERTEX_OUTPUT_COMPONENTS查询可以out的分量上限
	// 本机可以传输 128个
	out vec2 vTexCoord;
	out vec3 vNormal;
	out vec3 vWorldPosition;

	void main()
	{
		vec4 worldPosition = uTransform * vec4(aPosition, 1.0);
		gl_Position = uProjection* uView * worldPosition; // 将顶点位置传递给裁剪空间
		
		vWorldPosition = worldPosition.xyz;
		vTexCoord = aTexCoord;

		// 需要将模型局部空间的法线转换到世界空间，保证在模型发生变形后，法线依然垂直于表面
		// 因为光源方向是世界空间中定义的，因此法线也要转换到世界空间，才能正确计算点积
		mat3 normalMatrix = transpose(inverse(mat3(uTransform)));
		vNormal = normalMatrix * aNormal;
	}