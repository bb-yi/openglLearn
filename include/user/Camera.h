#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


// 定义相机移动的几种可能选项。用作抽象，以避免使用特定于窗口系统的输入方法
enum Camera_Movement
{
    FORWARD,  // 前进
    BACKWARD, // 后退
    LEFT,     // 向左
    RIGHT,     // 向右
    UP,     //向上
    DOWN     //向下
};

// 默认相机值
const float YAW = -90.0f;         // 偏航角
const float PITCH = 0.0f;         // 俯仰角
const float SPEED = 2.5f;         // 移动速度
const float SENSITIVITY = 0.1f;   // 鼠标灵敏度
const float ZOOM = 45.0f;         // 缩放


// 一个抽象的相机类，处理输入并计算相应的欧拉角、向量和矩阵供OpenGL使用
class Camera
{
public:
    // 相机属性
    glm::vec3 Position;     // 位置
    glm::vec3 Front;        // 前方向
    glm::vec3 Up;           // 上方向
    glm::vec3 Right;        // 右方向
    glm::vec3 WorldUp;      // 世界上的方向
    // 欧拉角
    float Yaw;              // 偏航角
    float Pitch;            // 俯仰角
    // 相机选项
    float MovementSpeed;    // 移动速度
    float MouseSensitivity; // 鼠标灵敏度
    float Zoom;             // 缩放

    // 使用向量的构造函数
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // 使用标量值的构造函数
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // 使用欧拉角和LookAt矩阵计算返回的视图矩阵
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // 处理来自任何键盘类输入系统的输入。接受以相机定义的枚举形式的输入参数（将其从窗口系统中抽象出来）
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == UP)
            Position += WorldUp * velocity;
        if (direction == DOWN)
            Position -= WorldUp * velocity;

    }

    // 处理来自鼠标输入系统的输入。期望在x和y方向上的偏移值。
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // 确保当俯仰角超出边界时，屏幕不会翻转
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // 使用更新后的欧拉角重新计算前、右和上向量
        updateCameraVectors();
    }

    // 处理来自鼠标滚轮事件的输入。只需要垂直滚轮轴的输入
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 120.0f)
            Zoom = 120.0f;
    }

private:
    // 从相机的（更新后的）欧拉角计算前向量
    void updateCameraVectors()
    {
        // 计算新的前向量
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // 同时重新计算右向量和上向量
        Right = glm::normalize(glm::cross(Front, WorldUp));  // 标准化向量，因为当你向上或向下看时，它们的长度会接近0，导致移动变慢。
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
#endif