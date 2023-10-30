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
    VkInstance instance;
    
    void initWindow(){
        glfwInit(); //initialize GLFW library
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //do not create OpenGL context
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //disable window resizing
        window = glfwCreateWindow(WIDTH, HEIGHT, "VulkanFirstTriangle", nullptr, nullptr);
        //width, height, title, specify monitor, smt OpenGL
    }
    
    void initVulkan() {
        createInstance();
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
    
    void createInstance(){
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        
        uint32_t glfwExtensionCount = 0; 
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> requiredExtensions;
        
        for(uint32_t i = 0; i < glfwExtensionCount; i++) {
            requiredExtensions.emplace_back(glfwExtensions[i]);
        }
        //requiredExtensions.emplace_back(VK_KHR_surface);
        //requiredExtensions.emplace_back(VK_EXT_metal_surface); //does not exists?
        requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        requiredExtensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME); //necessary?
        
        VkInstanceCreateInfo createInfo{};
        createInfo.flags|=VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        //createInfo.flags|=VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; //useless?
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        createInfo.enabledLayerCount = 0;
        
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        if(result!=VK_SUCCESS){
            std::cout<<result<<std::endl; //-9 -> Incompatible Driver Error
            throw std::runtime_error("Failed to create Vulkan instance!");
        }
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
