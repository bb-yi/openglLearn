#version 330 core
layout(location=0)in vec3 aPos;//顶点位置为0的数据
layout(location=1)in vec2 aTexCoord;
layout(location=2)in vec3 aNormal;
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 transform;
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main()
{
   gl_Position=projection_matrix*view_matrix*model_matrix*vec4(aPos.x,aPos.y,aPos.z,1.);
   TexCoord=aTexCoord;
   Normal=mat3(transpose(inverse(model_matrix)))*aNormal;//转为世界空间法向量并修复不等比缩放法向量错误 法线矩阵 取反再转置
   FragPos=vec3(model_matrix*vec4(aPos.x,aPos.y,aPos.z,1.));//世界空间位置
}