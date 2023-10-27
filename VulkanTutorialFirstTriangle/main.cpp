#define GLFW_INCLUDE_VULKAN //#include <vulkan/vulkan.h> is not needed with this and the following line
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

class HelloTriangleApplication {
    
public:
    static const uint32_t WIDTH = 800;
    static const uint32_t HEIGHT = 600;
    
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup(); }
private:
    GLFWwindow* window;
    
    void initWindow(){
        glfwInit(); //initialize GLFW library
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //do not create OpenGL context
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //disable window resizing
        window = glfwCreateWindow(WIDTH, HEIGHT, "VulkanFirstTriangle", nullptr, nullptr);
        //width, height, title, specify monitor, smt OpenGL
    }
    
    void initVulkan() {
    }
    
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }// wait to receive Window close Event
    }
    
    void cleanup() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main() {
    std::cout<<"START main\n";
    HelloTriangleApplication app;
    try {
        app.run();
    } catch (const std::exception& e) { 
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
