#version 330 core
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

uniform float ourTime;
uniform vec4 ourColor;// 在OpenGL程序代码中设定这个变量
uniform sampler2D ourTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;

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

void main()
{
    vec3 baseColor=AdjustHSL(texture(ourTexture,TexCoord).xyz,ourTime/ourColor.x,0.,0.);
    vec3 ambient=.1*baseColor;
    vec3 lightDir=normalize(lightPos-FragPos);//指向灯光
    vec3 viewDir=normalize(viewPos-FragPos);//指向相机
    float diff=max(dot(Normal,lightDir),0.);
    float spec=pow(max(dot(reflect(-lightDir,Normal),viewDir),0.),32.);
    FragColor=vec4(baseColor*diff+ambient+vec3(spec),1.);
    // FragColor=vec4(vec3(spec),1.);
}

