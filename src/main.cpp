#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "stb_image.h"

#include <user/Shader.h>
#include <user/Camera.h>

#ifdef _WIN32
#include <windows.h>
#endif

void framebuffer_size_callback(GLFWwindow* window, int width, int height);      //屏幕尺寸改变回调函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos);              //鼠标变化回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);       //滚轮变化回调函数
void processInput(GLFWwindow* window);                                          //输入处理函数

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

bool isMouseCaptured = true;

Camera camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));

float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 lightPos(2.0f, 2.0f, 2.0f);

int main()
{
#ifdef _WIN32
    // 在Windows上设置控制台支持UTF-8
    SetConsoleOutputCP(CP_UTF8);
#endif

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);



    //创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "窗口对象创建失败" << std::endl;
        glfwTerminate();
        return -1;
    }
    else
    {
        std::cout << "窗口对象创建成功" << std::endl;
    }
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//设置鼠标为隐藏并捕捉
    glfwSetCursorPosCallback(window, mouse_callback);//注册鼠标监听回调
    glfwSetScrollCallback(window, scroll_callback);//注册滚轮监听回调

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    //加载中文字体
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/simhei.ttf", 12.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    if (!font)
    {
        std::cerr << "加载字体失败" << std::endl;
    }


    //初始化GLAD GLAD负责动态加载函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "初始化GLAD失败" << std::endl;
        return -1;
    }
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);//启用深度测试


    //加载图片
    unsigned int texture;
    glGenTextures(1, &texture);//生成图片对象
    glBindTexture(GL_TEXTURE_2D, texture);//绑定图片对象
    // 为当前绑定的纹理对象设置环绕、过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//水平纹理的环绕方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//竖直纹理的环绕方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//缩小时使用的过滤方式 线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//放大时使用的过滤方式 线性插值

    // 加载并生成纹理
    stbi_set_flip_vertically_on_load(true);//设置图片加载时是否翻转
    int width, height, nrChannels;
    unsigned char* data = stbi_load("Tex/splash.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);//自动生成mipmap
    }
    else
    {
        std::cout << "加载纹理失败" << std::endl;
    }
    stbi_image_free(data);//释放内存

    //创建着色器
    Shader ourShader("Shader/userShader.vs", "Shader/userShader.fs");
    Shader lightShader("Shader/lightShader.vs", "Shader/lightShader.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        //位置，UV，法向量
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   0.0f, 0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f, 1.0f, 0.0f
    };
    // 添加索引数据
    unsigned int indices[] = {
        0, 1, 2, 3, 4, 5,     // 前面
        6, 7, 8, 9, 10, 11,   // 右面
        12, 13, 14, 15, 16, 17, // 后面
        18, 19, 20, 21, 22, 23, // 左面
        24, 25, 26, 27, 28, 29, // 上面
        30, 31, 32, 33, 34, 35  // 下面
    };

    //VBO Vertex Buffer Object
    //VAO Vertex Array Object
    //EBO Element Buffer Object
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);//创建一个VAO对象
    glGenBuffers(1, &VBO);//创建一VBO对象
    glGenBuffers(1, &EBO);//创建一个EBO对象
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);//绑定VAO 设置为激活AVO

    glBindBuffer(GL_ARRAY_BUFFER, VBO);//绑定VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//将顶点数据传输到当前绑定的VBO中

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);//绑定EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //设置第一个三位数据为位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);//告诉OpenGL如何解析顶点数据
    glEnableVertexAttribArray(0);//启用位置为0的顶点属性数组
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));//设置UV的位置
    glEnableVertexAttribArray(1);//启用位置为1的顶点属性数组
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));//设置法向量
    glEnableVertexAttribArray(2);//启用位置为2的顶点属性数组

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);//解绑VBO

    //你可以在之后解除对VAO的绑定，这样其他VAO调用就不会意外地修改这个VAO，但这种情况很少发生。修改其他
    // VAOs无论如何都需要调用glBindVertexArray，所以当不直接需要时，我们通常不会取消绑定VAOs（也不会取消绑定vbo）。
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);//绑定VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//将顶点数据传输到当前绑定的VBO中

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);//绑定EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //设置第一个三位数据为位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);//告诉OpenGL如何解析顶点数据
    glEnableVertexAttribArray(0);//启用位置为0的顶点属性数组
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));//设置UV的位置
    glEnableVertexAttribArray(1);//启用位置为2的顶点属性数组

    glBindVertexArray(0);//解绑VAO

    //查询最大支持的顶点属性数量
    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    std::cout << "最大支持的顶点属性数量 " << nrAttributes << std::endl;

    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    float uniformValue = 0.5f;
    float float3Var[3] = { 0.0f, 0.0f, 0.0f }; // 三个浮点数 背景颜色
    glm::vec3 cubePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- 显示一个窗口控制 uniform ---
        ImGui::Begin("Uniform Controller");
        ImGui::SliderFloat("Uniform 值", &uniformValue, 0.0f, 1.0f);
        ImGui::Text("当前值: %.3f", uniformValue);
        ImGui::ColorEdit3("标签", float3Var);
        ImGui::End();

        // render
        // ------
        glClearColor(float3Var[0], float3Var[1], float3Var[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        // 创建变换矩阵
        // glm::mat4 model = glm::mat4(1.0f);  // 模型矩阵
        glm::mat4 view = glm::mat4(1.0f);   // 视角矩阵
        glm::mat4 projection = glm::mat4(1.0f);//投影矩阵
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);//创建投影矩阵 视场角 宽度/高度 近裁剪面 远裁剪面

        // ourShader.setMat4("model_matrix", model);
        ourShader.setMat4("view_matrix", view);
        ourShader.setMat4("projection_matrix", projection);

        // 更新uniform颜色
        float timeValue = glfwGetTime();
        ourShader.setFloat4("ourColor", uniformValue, 0.0f, 0.0f, 1.0f);
        ourShader.setFloat("ourTime", timeValue);
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::rotate(trans, glm::radians(90.0f * timeValue), glm::vec3(0.0, 0.0, 1.0));
        trans = glm::scale(trans, glm::vec3(1.0, 1.0, 1.0));
        ourShader.setMat4("transform", trans);
        ourShader.setVec3("lightPos", lightPos);
        ourShader.setVec3("viewPos", camera.Position);


        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO); // 因为我们只有一个VAO，所以没有必要每次都绑定它，但是我们这样做是为了让事情更有条理
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        for (unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            // model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f) + i * 50.0f, glm::vec3(1.0f, 0.3f, 0.5f));
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            ourShader.setMat4("model_matrix", model);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0); // 使用索引绘制
            // glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        //绘制灯光
        glBindVertexArray(lightVAO);
        lightShader.use();
        glm::mat4 lightModel = glm::mat4(1.0f);
        // lightPos = glm::vec3(sin(glfwGetTime()) * 2, 1.0f, 2.0f);
        lightModel = glm::translate(lightModel, lightPos);
        lightModel = glm::scale(lightModel, glm::vec3(0.2f));
        lightShader.setMat4("model_matrix", lightModel);
        lightShader.setMat4("view_matrix", view);
        lightShader.setMat4("projection_matrix", projection);

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0); // 使用索引绘制


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();

        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 在文件顶部的全局变量区域添加
        static bool altWasPressed = false;  // 添加static使其具有内部链接性

        // 在渲染循环中
        if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
        {
            if (!altWasPressed)
            {
                // 切换鼠标状态
                isMouseCaptured = !isMouseCaptured;
                glfwSetInputMode(window, GLFW_CURSOR, isMouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
                // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//设置鼠标为隐藏并捕捉
                altWasPressed = true;
            }
        }
        else
        {
            altWasPressed = false;
        }

    }

    // --- 清理 ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    (void)window;//干掉未使用参数警告
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    (void)window;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

bool firstMouse = true;
float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch = 0.0f;//yaw 偏航角
//鼠标监听回调函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    (void)window;
    if (!isMouseCaptured)
    {
        return;
    }
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);

}
//滚轮缩放回调
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    (void)window;
    (void)xoffset;
    camera.ProcessMouseScroll(yoffset);
}