#define GLFW_INCLUDE_VULKAN //does this and some more in the include glfw3 line: #include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_beta.h> //needed for Mac: includes the PORTABILITY_SUBSET_EXTENSION
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <cstring>
#include <optional>
#include <set>
#include <fstream>


struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentationFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentationFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",

};

std::vector<const char*> requiredDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    #ifdef __APPLE__
    VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
    #endif
};


class HelloTriangleApplication {
    
public:
    static const uint32_t WIDTH = 800;
    static const uint32_t HEIGHT = 600;

    #ifdef NDEBUG
        static const bool enableValidationLayers = false;
    #else
        static const bool enableValidationLayers = true;
    #endif
    
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup(); }
private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, //VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
        VkDebugUtilsMessageTypeFlagsEXT messageType,            //VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;    //Call the Vulkan call True or False!
    }

    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary); //ate seeks to the end of the file. We do that so that we can query the filesize via the read position of the file.

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file to read!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentationQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkImageView> swapChainImageViews;
    
    void initWindow(){
        glfwInit(); //initialize GLFW library
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //do not create OpenGL context
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //disable window resizing
        window = glfwCreateWindow(WIDTH, HEIGHT, "VulkanFirstTriangle", nullptr, nullptr);
        //width, height, title, specify monitor, smt OpenGL
    }
    
    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createGraphicsPipeline();
    }
    
    void createInstance(){
        //get supported Extensions
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());
        
        //set requiered Extensions
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> requiredExtensions;
        
        for(uint32_t i = 0; i < glfwExtensionCount; i++) {
            requiredExtensions.emplace_back(glfwExtensions[i]);
        }
        requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        requiredExtensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

        if (enableValidationLayers) {
            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        
        //check if all requiered Extensions are supported
        for(const char* requiredExtension:requiredExtensions){
            bool extensionPresent=false;
            for(const auto& supportedExtension:supportedExtensions)
            {
                if(std::strcmp(requiredExtension, supportedExtension.extensionName)==0){
                    extensionPresent=true;
                    break;
                }
            }
            
            if(!extensionPresent){
                throw std::runtime_error(std::string("Required Extension missing: ").append(requiredExtension));
            }
        }
        
        //create Instance
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        
        VkInstanceCreateInfo createInfo{};
        createInfo.flags|=VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        createInfo.enabledLayerCount = 0;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;    //2nd Debugger to check CreateInstance and DestroyInstance call
        if (enableValidationLayers && checkValidationLayerSupport()) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }  
        
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        if(result!=VK_SUCCESS){
            std::cout<<result<<std::endl; //-9 -> Incompatible Driver Error
            throw std::runtime_error("Failed to create Vulkan instance!");
        }
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* desiredLayerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(desiredLayerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                throw std::runtime_error(std::string("Required Validation Layer missing: ").append(desiredLayerName));
                return false;
            }
        }
        return true;
    }

    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func == nullptr) {
            throw std::runtime_error("Create Debug Utils Functions is missing!\nCan not setup Debug Messenger");
            return;
        }
        if (VK_SUCCESS != func(instance, &createInfo, nullptr, &debugMessenger)) {
            throw std::runtime_error("Setup Debug Messenger function call failed");
        }
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo={};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }
    }

    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount,nullptr);

        if (deviceCount <= 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        // We can check in this function if the device supports every feature we need.
        // We should also rank the devices and pick the best one according to some score!
        // For now we don't care as all VkPhysicalDevices support Vulkan we are good to go!
        /*VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        return deviceFeatures.geometryShader && deviceProperties.deviceType==VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU*/;

        QueueFamilyIndices indices = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensionSupport(device);
        bool swapChainAdequate = false;

        if (extensionsSupported) {  //only query for SwapChain if the extensions are supported
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            //ensuring that there is at least one supported image format and presentation mode is enough
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            //Apparently it is more performant if the Graphics and presentation queue are on the same queue. Thus, asigning both the same index is a good thing.

            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {   // V K_QUEUE_GRAPHICS_BIT -> we need to be able to do graphics operations!
                indices.graphicsFamily = i;
            }

            VkBool32 presentationSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);

            if (presentationSupport) {
                indices.presentationFamily = i;
            }

            if (indices.isComplete()) {
                //when we found all queues we needed we don't have to change them up with redundant ones!
                break;
            }

            i++;
        }


        return indices;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensionsSet(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensionsSet.erase(extension.extensionName);
        }

        return requiredExtensionsSet.empty();
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount, nullptr);

        if (presentCount != 0) {
            details.presentModes.resize(presentCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount, details.presentModes.data());
        }

        return details;
    }

    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(),indices.presentationFamily.value() };
        
        //this variable is not in the for loop to retain its lifetime
        float queuePriority = 1.0f;

        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            //For now we only want one Queue per queueFamily:
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        //currently we need no VulkanFeature, but we will come back here:
        VkPhysicalDeviceFeatures deviceFeatures{};
        

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = (uint32_t)requiredDeviceExtensions.size();
        createInfo.ppEnabledExtensionNames= requiredDeviceExtensions.data();
        
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device!");
        }

        // 0 is the index of the the Queues we gonna use. We hard code 0 here as we only have one Queue for each family.
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentationFamily.value(), 0, &presentationQueue);
    }

    void createSwapChain(){
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        
        uint32_t imageCount = std::clamp((uint32_t)3, swapChainSupport.capabilities.minImageCount, (uint32_t)3); // for Triple Buffering we need at least 3 images ^^
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent; 
        createInfo.imageArrayLayers = 1; // Amount of layers each image consists of. Would be larger than one for stereoscopic 3D applications.
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // If you would draw an extra image for post processing you may use VK_IMAGE_USAGE_TRANSFER_DST_BIT here.

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),indices.presentationFamily.value() };

        if (indices.graphicsFamily != indices.presentationFamily) {
            // We should use VK_SHARING_MODE_EXCLUSIVE here (better performance)! But we have not done the ownership chapters so we know no better >n<
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else { // the queues are actaully on the same queue ^^:
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // optional when using VK_SHARING_MODE_EXCLUSIVE
            createInfo.pQueueFamilyIndices = nullptr; // optional when using VK_SHARING_MODE_EXCLUSIVE
        }
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // we could rotate or mirrow according to fixed angles if we wanted here: VkSurfaceTransformFlagBitsKHR.
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // We do not want that our application blends with other applications: VkCompositeAlphaFlagBitsKHR.
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE; // this options says we do not care about pixels who might be under a different window. If this is needed we should disable this ^^. But it increases performance of course.
        createInfo.oldSwapchain = VK_NULL_HANDLE; // if the window is resized or moved to a different monitor we might need to create a new swap chain and should state the old swap chain here. For now we will not do that ^^.

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }   

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }


        std::cout << "using random Swap Surface Format as we did not find 24+8Bit Alpha SRGB! Could also be that there isn't even a Format available!" << std::endl;
        //If we do not find what we get we just use whatever, we should score each format and pick the next best one but I and the tutorial are to lazy!
        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) { //this is Gaming mode ^^
                return availablePresentMode;
            }
        }

        //The FIFO Present Mode is garaunted so we use this one if we don't get Triple Buffering. Even though if we do not programm a game FIFO would be preferred to Triple Buffering as we do not need to draw more frames to reduce latency!
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent; //if the width is not max uint32 then Vulkan sets the extent -> widht and height of the window!
        }
        int width, heigth;
        glfwGetFramebufferSize(window, &width, &heigth);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(heigth)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }

    void createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
            // specify that our images will be color targets
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0; // We do not use Mip Mapping
            createInfo.subresourceRange.levelCount = 1; // As we do not use Mip Mapping we only have on level
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1; // We do only use one layer per image

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create image view!");
            }
        }
    }
    
    void createGraphicsPipeline() {
        std::vector<char> vertShaderByteCode = readFile("shaders/vert.spv");
        std::vector<char> fragShaderByteCode = readFile("shaders/frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderByteCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderByteCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";
        vertShaderStageInfo.pSpecializationInfo = nullptr; // We can provide shader constants here for better perfomance. This is nullptr by default

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        vertShaderStageInfo.module = fragShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo,fragShaderStageInfo };

        //The shader modules can be destroyed after the creation of the GraphicsPipeline:
        vkDestroyShaderModule(device,vertShaderModule,nullptr); 
        vkDestroyShaderModule(device,fragShaderModule,nullptr); 
    }

    VkShaderModule createShaderModule(const std::vector<char>& byteCode) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = byteCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(byteCode.data()); //reinterpret_cast does not cast and just treats it as the new type. Thus this line is hella sus!

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module!");
        }
        //the byteCodeBuffer can not be freed!

        return shaderModule;
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }// wait to receive Window close Event
    }

    void cleanup() {
        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        vkDestroyDevice(device, nullptr);
        if (enableValidationLayers) {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr) {
                func(instance, debugMessenger, nullptr);
            }
            else {
                std::cerr << "Could not Cleanup Debug Messenger!" << std::endl;
            }
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance,nullptr);
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
