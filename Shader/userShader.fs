#version 330 core
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

struct Material{
    vec3 baseColor;
    sampler2D baseTexture;
    vec3 specular;
    float shininess;
};

struct DirLight{
    vec3 lightcolor;
    vec3 specularcolor;
    vec3 direction;
};

struct PointLight{
    vec3 lightcolor;
    vec3 specularcolor;
    vec3 position;
    vec3 attenuation;//衰减系数 常数项 一次项 二次项
};

struct SpotLight{
    vec3 lightcolor;
    vec3 specularcolor;
    vec3 position;
    vec3 direction;
    float angle;
    float smoothness;
    vec3 attenuation;//衰减系数 常数项 一次项 二次项
};

uniform Material material;

//声明灯光
#define NR_POINT_LIGHTS 4
uniform DirLight dirLight;
uniform PointLight pointLight[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

uniform float ourTime;
uniform vec4 ourColor;// 在OpenGL程序代码中设定这个变量
uniform vec3 viewPos;

//inout关键词会保留输入 out关键词会覆盖输入
vec3 AdjustHSL(vec3 color,float hueShift,float satShift,float lightShift);//色相饱和度明度调整
void CalcDirLight(DirLight light,vec3 normal,vec3 viewDir,inout vec3 diffusecolor,inout vec3 specularcolor);
void CalcPointLight(PointLight light,vec3 normal,vec3 fragPos,vec3 viewDir,inout vec3 diffusecolor,inout vec3 specularcolor);
void CalcSpotLight(SpotLight light,vec3 normal,vec3 fragPos,vec3 viewDir,inout vec3 diffusecolor,inout vec3 specularcolor);

void main()
{
    vec3 viewDir=normalize(viewPos-FragPos);//指向相机
    
    vec3 lightdiff=vec3(0.);
    vec3 lightspec=vec3(0.);
    // directional light
    CalcDirLight(dirLight,Normal,viewDir,lightdiff,lightspec);
    // point lights
    for(int i=0;i<NR_POINT_LIGHTS;i++){
        CalcPointLight(pointLight[i],Normal,FragPos,viewDir,lightdiff,lightspec);
    }
    // spotlight
    CalcSpotLight(spotLight,Normal,FragPos,viewDir,lightdiff,lightspec);
    
    vec3 baseColor=AdjustHSL(texture(material.baseTexture,TexCoord).xyz,ourTime/ourColor.x,0.,0.);
    vec3 ambient=.1*baseColor;
    
    FragColor=vec4(baseColor*lightdiff*material.baseColor+ambient+material.specular*lightspec,1.);
    // FragColor=vec4(lightdiff,1.);
    FragColor=vec4(vec3(gl_FragCoord.z),1.);
}

// 调整色相、饱和度、明度
// color: 输入的RGB颜色 (0-1范围)
// hueShift: 色相偏移量 (-1.0 ~ 1.0, 对应-360°~360°)
// satShift: 饱和度偏移量 (-1.0 ~ 1.0)
// lightShift: 明度偏移量 (-1.0 ~ 1.0)
vec3 AdjustHSL(vec3 color,float hueShift,float satShift,float lightShift)
{
    // --- RGB -> HSL ---
    float maxc=max(max(color.r,color.g),color.b);
    float minc=min(min(color.r,color.g),color.b);
    float delta=maxc-minc;
    
    float h=0.;
    float s=0.;
    float l=(maxc+minc)*.5;
    
    if(delta>.0001){
        // 色相计算
        if(maxc==color.r)
        h=mod((color.g-color.b)/delta,6.);
        else if(maxc==color.g)
        h=(color.b-color.r)/delta+2.;
        else
        h=(color.r-color.g)/delta+4.;
        h/=6.;// 归一化到 0-1
        
        // 饱和度计算
        s=delta/(1.-abs(2.*l-1.));
    }
    
    // --- 调整HSL ---
    h=mod(h+hueShift,1.);// 色相环上循环
    s=clamp(s+satShift,0.,1.);// 饱和度 0~1
    l=clamp(l+lightShift,0.,1.);// 明度 0~1
    
    // --- HSL -> RGB ---
    float c=(1.-abs(2.*l-1.))*s;
    float x=c*(1.-abs(mod(h*6.,2.)-1.));
    float m=l-.5*c;
    
    vec3 rgb;
    if(h<1./6.)rgb=vec3(c,x,0.);
    else if(h<2./6.)rgb=vec3(x,c,0.);
    else if(h<3./6.)rgb=vec3(0.,c,x);
    else if(h<4./6.)rgb=vec3(0.,x,c);
    else if(h<5./6.)rgb=vec3(x,0.,c);
    else rgb=vec3(c,0.,x);
    
    return rgb+vec3(m);
}

// calculates the color when using a directional light.
void CalcDirLight(DirLight light,vec3 normal,vec3 viewDir,inout vec3 diffusecolor,inout vec3 specularcolor)
{
    vec3 lightDir=normalize(-light.direction);
    // diffuse shading
    float diff=max(dot(normal,lightDir),0.);
    // specular shading
    vec3 reflectDir=reflect(-lightDir,normal);
    float spec=pow(max(dot(viewDir,reflectDir),0.),material.shininess);
    diffusecolor+=light.lightcolor*diff;
    specularcolor+=light.specularcolor*spec;
}

// calculates the color when using a point light.
void CalcPointLight(PointLight light,vec3 normal,vec3 fragPos,vec3 viewDir,inout vec3 diffusecolor,inout vec3 specularcolor)
{
    vec3 lightDir=normalize(light.position-fragPos);
    // diffuse shading
    float diff=max(dot(normal,lightDir),0.);
    // specular shading
    vec3 reflectDir=reflect(-lightDir,normal);
    float spec=pow(max(dot(viewDir,reflectDir),0.),material.shininess);
    // attenuation
    float distance=length(light.position-fragPos);
    float attenuation=1./(light.attenuation.x+light.attenuation.y*distance+light.attenuation.z*(distance*distance));
    // combine results
    diffusecolor+=light.lightcolor*diff*attenuation;
    specularcolor+=light.specularcolor*spec*attenuation;
}

// calculates the color when using a spot light.
void CalcSpotLight(SpotLight light,vec3 normal,vec3 fragPos,vec3 viewDir,inout vec3 diffusecolor,inout vec3 specularcolor)
{
    vec3 lightDir=normalize(light.position-fragPos);
    // diffuse shading
    float diff=max(dot(normal,lightDir),0.);
    // specular shading
    vec3 reflectDir=reflect(-lightDir,normal);
    float spec=pow(max(dot(viewDir,reflectDir),0.),material.shininess);
    // attenuation
    float distance=length(light.position-fragPos);
    float attenuation=1./(light.attenuation.x+light.attenuation.y*distance+light.attenuation.z*(distance*distance));
    
    // spotlight intensity
    float theta=dot(lightDir,normalize(-light.direction));
    float intensity=smoothstep(cos(light.angle+light.smoothness),cos(light.angle-light.smoothness),theta);
    // combine results
    diffusecolor+=light.lightcolor*diff*attenuation*intensity;
    specularcolor+=light.specularcolor*spec*attenuation*intensity;
}