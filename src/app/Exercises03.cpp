#include "student/VKSetup.hpp"
#include <iostream>
#include <string> 

using namespace std;
using namespace student;

const char* getDeviceTypeString(vk::PhysicalDeviceType t) {
    switch (t) {
        case vk::PhysicalDeviceType::eIntegratedGpu: return "Integrated GPU";
        case vk::PhysicalDeviceType::eDiscreteGpu: return "Discrete GPU";
        case vk::PhysicalDeviceType::eVirtualGpu: return "Virtual GPU";
        case vk::PhysicalDeviceType::eCpu: return "CPU";
        default: return "Other";
    }
}

void printPhysicalDeviceProperties(vk::PhysicalDevice &pd) {
    vk::PhysicalDeviceProperties props = pd.getProperties();
    uint32_t ver = props.apiVersion;

    cout << "Name: " << props.deviceName.data() << endl;
    cout << "Type: " << getDeviceTypeString(props.deviceType) << endl;
    cout << "API Version: "
                    << VK_VERSION_MAJOR(ver) << "."
                    << VK_VERSION_MINOR(ver) << "."
                    << VK_VERSION_PATCH(ver) << endl;
}

void listAvailablePhysicalDevices(VulkanInitData &vkInitData) {
    vector<vk::PhysicalDevice> phys
        = vkInitData.instance.enumeratePhysicalDevices();
    cout << "Found " << phys.size() << " physical device(s):" << endl;
    for (int i = 0; i < phys.size(); i++) {
        cout << "** Device " << i << " ***********" << endl;
        printPhysicalDeviceProperties(phys[i]);
    }
}

int main(int argc, char **argv) {
    cout << "BEGIN VULKAN EXERCISE" << endl;

    string appName = "Exercises03";
    string windowTitle = appName;
    int windowWidth = 640;
    int windowHeight = 480;

    GLFWwindow* window = createGLFWWindow(
        windowTitle, windowWidth, windowHeight);

    if (!window) {
        glfwTerminate();
        exit(1);
    }

    VulkanInitData vkInitData {};
    vkInitData.appName = appName;
    vkInitData.window = window;
    if(!createVulkanSetup(vkInitData)) {
        exit(1);
    }

    listAvailablePhysicalDevices(vkInitData);

    uint32_t apiVersion = vk::enumerateInstanceVersion();
    cout << "My loader supports Vulkan "
            << VK_VERSION_MAJOR(apiVersion) << "."
            << VK_VERSION_MINOR(apiVersion) << "."
            << VK_VERSION_PATCH(apiVersion) << endl;


    //while(!glfwWindowShouldClose(window)) {
    //    glfwPollEvents();
    //}        
    // TODO
    // Cleanup
    cleanupVulkanSetup(vkInitData);
    cleanupGLFWWindow(window);
    return 0;
}
