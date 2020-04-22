///////////////
// Main.cpp //
/////////////

#define _CRT_SECURE_NO_WARNINGS
#define GLFW_INCLUDE_VULKAN  // Vulkan Header from LunarG SDK, which provides Vulkan functions, structures and enumerations, is included automatically
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define STB_IMAGE_IMPLEMENTATION

#include <GLFW/glfw3.h> // Include GLFW to render the objects in a window
#include <iostream> // Included for reporting errors
#include <stdexcept> // Included for reporting errors
#include <functional> // Used for Lambda functions in resource management
#include <cstdlib> // Provides EXIT_SUCCESS and EXIT_FAILURE macros
#include <optional> // Provides optional template
#include <set>  // Provides set
#include <cstdint> // Necessary for UINT32_MAX
#include <algorithm> // Necessary for min and max functions
#include <fstream> // Provides functions to read/write a file
#include <string> // String related function
#include <array>
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Vec3
struct Vec3
{
	float x, y, z;
};

// Vec4
struct Vec4 
{ 
	float x, y, z, w; 
};

// Vertex
struct  Vertex
{
	Vec3 position;
	Vec3 color;
	Vec3 tex;
	Vec3 normal;

	// Get Binding Description for the vertex
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};

		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	// Get the attribute descriptions
	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, tex);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, normal);
		return attributeDescriptions;
	}
};

// Uniforms for model, view, projection transformations
struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

// Uniform for Lighting Constants
struct LightingConstants
{
	glm::vec4 lightPosition;
	glm::vec4 lightAmbient;
	glm::vec4 lightDiffuse;
	glm::vec4 lightSpecular;
	float ambientIntensity;
	float specularIntensity;
	float diffuseIntensity;
	float lightSpecularExponent;
	float ambientEnabled = 1;
	float specularEnabled = 1;
	float DiffuseEnabled = 1;
	float textureEnabled = 1;
};

// Mesh
struct Mesh
{
	// Vertices of the mesh
	std::vector<Vertex> vertices;

	// Indices of the faces of the mesh
	std::vector<int> indices;

	// Lighting Constants of the mesh
	LightingConstants lightingConstants;
};

// Maximum no of frames processed concurrently
const int MAX_FRAMES_IN_FLIGHT = 2;

// Width and Height of the Window
const int WIDTH = 800;
const int HEIGHT = 600;

// Validation layers to be enabled
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

// Device extensions to be enabled
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Flag to specify whether Validation layers should be enabled or not
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Function to create VkDebugUtilsMessengerEXT object from Debug Messenger Info structure
// 1st Parameter - Vulkan Instance for the debug messenger to be created
// 2nd Parameter - structure of create info parameters used while creating the Debug Messenger
// 3rd Parameter - optional allocator callback
// 4th Parameter - the Debug Messenger created in this function
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {

	// Look up the address of the extension function to create Debug Utils Messenger EXT
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		// Call the found extension function
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		// Return error as the extension function is not found
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

// Function to destroy VkDebugUtilsMessengerEXT object
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {

	// Look up the address of the extension function to destroy Debug Utils Messenger EXT
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		// Call the found extension function
		func(instance, debugMessenger, pAllocator);
	}
}

// Class to wrap Vulkan objects and functions initiating the Vulkan objects
class HelloTriangleApplication {
public:

	// Function to run the application.
	// This function initializes the Vulkan objects and loops within mainLoop until window is closed. Cleanup is called to free the resources allocated
	void run() {

		// Initialize a Window
		initWindow();

		// Initializes Vulkan resources
		initVulkan();

		// Main Loop where every frame is rendered
		mainLoop();

		// Cleanup and deallocate allocate resources
		cleanup();
	}

private:

	// Mesh loaded
	Mesh m;

	// Instance to GLFW Window
	GLFWwindow* window;

	// Vulkan Instance - Connection between the application and the Vulkan Library
	VkInstance instance;

	// Handle to manage the Debug Callback
	VkDebugUtilsMessengerEXT debugMessenger;

	// Instance of Surface
	VkSurfaceKHR surface;

	// Handle to store the graphics card suitable to the application
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	// Handle to store the logical device to interface with the physical device
	VkDevice device;

	// Handle to graphics queue
	VkQueue graphicsQueue;

	// Handle to presentation queue
	VkQueue presentQueue;

	// Handle to Swap chain
	VkSwapchainKHR swapChain;

	// Handles to swap chain images
	std::vector<VkImage> swapChainImages;

	// Swap chain image format
	VkFormat swapChainImageFormat;

	// Swap chain extent
	VkExtent2D swapChainExtent;

	// Swap chain image views
	std::vector<VkImageView> swapChainImageViews;

	// Render pass
	VkRenderPass renderPass;

	// Descriptor Set Layout
	VkDescriptorSetLayout descriptorSetLayout;

	// Pipeline layout - uniforms
	VkPipelineLayout pipelineLayout;

	// Graphics pipeline
	VkPipeline graphicsPipeline;

	// Swap chain frame buffers
	std::vector<VkFramebuffer> swapChainFramebuffers;

	// Command Pool
	VkCommandPool commandPool;

	// Vertex Buffer
	VkBuffer vertexBuffer;

	// Vertex Buffer Memory
	VkDeviceMemory vertexBufferMemory;

	// Index Buffer
	VkBuffer indexBuffer;

	// Index Buffer Memory
	VkDeviceMemory indexBufferMemory;

	// Uniform Buffers
	std::vector<VkBuffer> uniformBuffers;

	// Uniform Buffers Memory
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	// Lighting Buffers
	std::vector<VkBuffer> lightingBuffers;

	// Lighting Buffers Memory
	std::vector<VkDeviceMemory> lightingBuffersMemory;

	// Descriptor Pool to create descriptor sets
	VkDescriptorPool descriptorPool;

	// Descriptor sets to bind each VkBuffer to the uniform buffer descriptor
	std::vector<VkDescriptorSet> descriptorSets;

	// Texture Image
	VkImage textureImage;

	// Memory for Texture Image
	VkDeviceMemory textureImageMemory;

	// Texture Image View
	VkImageView textureImageView;

	// Texture Sampler
	VkSampler textureSampler;

	// Depth Image
	VkImage depthImage;

	// Memory for depth image
	VkDeviceMemory depthImageMemory;

	// Depth Image view
	VkImageView depthImageView;

	// Command Buffers
	std::vector<VkCommandBuffer> commandBuffers;

	// Semaphores to signal image is acquired for rendering
	std::vector<VkSemaphore> imageAvailableSemaphores;

	// Semaphores to signal rendering is complete and presentation can start
	std::vector<VkSemaphore> renderFinishedSemaphores;

	// Fences to prevent from more than Max frames getting submitted
	std::vector<VkFence> inFlightFences;

	// Fences to prevent images in flight getting rendered
	std::vector<VkFence> imagesInFlight;

	// Index of current frame
	size_t currentFrame = 0;

	// Flag to indicate whether frame buffer is resized due to window resizing
	bool framebufferResized = false;

	// Current translation and last translation values
	GLfloat translate_x, translate_y;
	GLfloat last_x, last_y;

	// Current rotation and last rotation values
	GLfloat rotate_x, rotate_y;
	GLfloat last_rotate_x, last_rotate_y;

	// Current Width and Heigth
	GLfloat width = WIDTH, height = HEIGHT;
	// Structure to  store the indices of the Queue family
	struct QueueFamilyIndices {
		// Graphics family
		std::optional<uint32_t> graphicsFamily;

		// Presentation family
		std::optional<uint32_t> presentFamily;

		// Function to check whether graphicsFamily has a value
		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	// Structure to store the swap chain support properties
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	/////////////////////
	// Run Functions ///
	///////////////////

	// Function to initializes a Window to render the objects
	void initWindow() {

		// Initialize the GLFW Library
		glfwInit();

		// Specify GLFW not to create an OpenGL Context
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Restrict Window resizing
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		// Create a Window
		// 1st Parameter - Width of the Window
		// 2nd Parameter - Height of the Window
		// 3rd Parameter - Title of the Window
		// 4th Parameter - Monitor to open the window
		// 5th Parameter - Only relevant to OpenGL
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Duck", nullptr, nullptr);
		// Set the pointer to window in callback functions
		glfwSetWindowUserPointer(window, this);

		// Initialize the callback function for window resize
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

		// Initialize the callback function for mouse events
		glfwSetMouseButtonCallback(window, mouse_callback);

		// Initialize the callback function for keyboard events
		glfwSetKeyCallback(window, key_callback);

		// Set initial translate and rotation
		translate_x = translate_y = 0.0;
		rotate_x = rotate_y = 0.0;
	}

	// Function to initialize the Vulkan objects
	void initVulkan() {
		// Create the Instance
		// An  instance is the connection between the application and Vulkan library
		createInstance();

		// Setup the Debug Messenger
		setupDebugMessenger();

		// Create the surface
		createSurface();

		// Pick up a suitable GPU
		pickPhysicalDevice();

		// Create a logical device
		createLogicalDevice();

		// Create Swap chain
		createSwapChain();

		// Create Image Views
		createImageViews();

		// Create render pass
		createRenderPass();

		// Create Descriptor Set layout
		createDescriptorSetLayout();

		// Create the graphics pipeline
		createGraphicsPipeline();

		// Create Command Pool
		createCommandPool();

		createDepthResources();

		// Create Frame Buffers
		createFramebuffers();

		// Parse the Object file
		m = ParseObjFile("12248_Bird_v1_L2.obj");

		// Create texture image
		createTextureImage("12248_Bird_v1_diff.ppm");

		// Create the texture image view
		createTextureImageView();

		// Create the texture sampler to map the texture based on texture coordinates
		createTextureSampler();

		// Create Vertex Buffer
		createVertexBuffer();

		// Create Index Buffer
		createIndexBuffer();

		// Create the Uniform Buffers
		createUniformBuffers();

		// Create the Descriptor Pool to create descriptor sets
		createDescriptorPool();

		// Create descriptor sets
		createDescriptorSets();

		// Create Command Buffers
		createCommandBuffers();

		// Create the semaphores and fences
		createSyncObjects();
	}

	// Function in which every frame is rendered
	// Iterates until window is closed
	void mainLoop() {

		// Event loop to keep the application running until there is an error or window is closed
		while (!glfwWindowShouldClose(window)) {

			// Checks for events like Window close by the user
			glfwPollEvents();

			// draw the frame
			drawFrame();
		}

		// Wait for the logical device to complete operations
		vkDeviceWaitIdle(device);
	}

	// Function to destroy all Vulkan objects and free allocated resources
	void cleanup() {

		cleanupSwapChain();

		vkDestroySampler(device, textureSampler, nullptr);

		vkDestroyImageView(device, textureImageView, nullptr);

		vkDestroyImage(device, textureImage, nullptr);
		vkFreeMemory(device, textureImageMemory, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		// Destroy the index buffer
		vkDestroyBuffer(device, indexBuffer, nullptr);

		// Free index buffer memory
		vkFreeMemory(device, indexBufferMemory, nullptr);

		// Destroy the vertex buffer
		vkDestroyBuffer(device, vertexBuffer, nullptr);

		// Free vertex buffer memory
		vkFreeMemory(device, vertexBufferMemory, nullptr);

		// Destroy the semaphores and fences
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			// Destroy the render finished semaphore
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);

			// Destroy the image available semaphore
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);

			// Destroy the fence
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}

		// Destroy command pool
		vkDestroyCommandPool(device, commandPool, nullptr);

		// Destroy the logical device
		vkDestroyDevice(device, nullptr);

		// Check whether validation layers are enabled
		if (enableValidationLayers) {
			// Destroy the Debug Utils Messenger Extension
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		// Destroy the surface
		// Using Vulkan function to destroy as GLFW doesn't offer a function to destroy the surface
		vkDestroySurfaceKHR(instance, surface, nullptr);

		// Destroy the Vulkan instance
		// 1st Parameter - instance to be destoyed
		// 2nd Parameter - optional allocator callback
		vkDestroyInstance(instance, nullptr);

		// Destroy and cleanup the GLFW Window
		glfwDestroyWindow(window);

		// Terminate GLFW
		glfwTerminate();
	}

	////////////////////////////
	// Init Window Functions //
	//////////////////////////

	// Callback function called when window is resized
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		// Get the pointer to the application
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		// Set the frame buffer resized flag to true
		app->framebufferResized = true;
		app->width = width;
		app->height = height;
	}
	
	// Mouse callback function
	static void mouse_callback(GLFWwindow* window, int button, int action, int mods)
	{
		double x;
		double y;
		glfwGetCursorPos(window, &x, &y);
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		Vec4 vNow;
		float size = (app->width > app->height) ? app->height : app->width;
		vNow.x = (2.0 * x - size) / size;
		vNow.y = (size - 2.0 * y) / size;
		if (button == GLFW_MOUSE_BUTTON_LEFT) {

			if (GLFW_PRESS == action)
			{
				app->last_x = vNow.x;
				app->last_y = vNow.y;
			}
			else if (GLFW_RELEASE == action)
			{
				app->translate_x += vNow.x - app->last_x;
				app->translate_y += vNow.y - app->last_y;
				app->last_x = vNow.x;
				app->last_y = vNow.y;
			}

		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (GLFW_PRESS == action)
			{
				app->last_rotate_x = vNow.x;
				app->last_rotate_y = vNow.y;
			}
			else if (GLFW_RELEASE == action)
			{
				app->rotate_x += vNow.x - app->last_rotate_x;
				app->rotate_y += vNow.y - app->last_rotate_y;
				app->last_rotate_x = vNow.x;
				app->last_rotate_y = vNow.y;
			}

		}
	}

	// Keyboard callback functions
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
			app->m.lightingConstants.ambientEnabled = !app->m.lightingConstants.ambientEnabled;
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
			app->m.lightingConstants.DiffuseEnabled = !app->m.lightingConstants.DiffuseEnabled;
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
			app->m.lightingConstants.specularEnabled = !app->m.lightingConstants.specularEnabled;
		if (key == GLFW_KEY_T && action == GLFW_PRESS)
			app->m.lightingConstants.textureEnabled = !app->m.lightingConstants.textureEnabled;
	}


	////////////////////////////
	// Init Vulkan Functions //
	//////////////////////////

	// Function to create an  instance - the connection between the application and the Vulkan library
	void createInstance()
	{
		// Check whether validation layers requested are supported
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		// Structure to specify optional information about the application.
		// This provides useful information to the driver to optimize for the application
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // specifies the type of the information stored in the structure
		appInfo.pApplicationName = "Hello Triangle"; // Name of the application
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // Version of the application
		appInfo.pEngineName = "No Engine"; // Name of the engine used to create the engine
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); // Version of engine used to create the engine
		appInfo.apiVersion = VK_API_VERSION_1_0; // Highest version of Vulkan that the application must use

		// Structure to specify which global extensions and validation layers should be used in the application
		// This is applied to the entire program and not to specific device
		VkInstanceCreateInfo createInfo = {};
		// Type of inforamtion stored in the structure
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		// Specify the application info
		createInfo.pApplicationInfo = &appInfo;

		// Retrieve the number of supported extensions and store in extensionCount
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		// An array to hold the supported extension details
		std::vector<VkExtensionProperties> extensions(extensionCount);

		// Query the supported extension details
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		// Print the supported extension details
		std::cout << "available extensions:" << std::endl;

		// Loop through the extensions and print
		for (const auto& extension : extensions) {
			std::cout << "\t" << extension.extensionName << std::endl;
		}

		// Stores the reference to the extensions
		auto glfwExtensions = getRequiredExtensions();

		// Specifies the no of extensions and the extensions to use in the application
		createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
		createInfo.ppEnabledExtensionNames = glfwExtensions.data();

		// Stores the Debug Create Info
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;

		// Check whether Validation layers are enabled
		if (enableValidationLayers)
		{
			// Specifies the no of validation layers to use in the application
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());

			// Specifies the validation layers to use in the application
			createInfo.ppEnabledLayerNames = validationLayers.data();

			// Populate the Debug Messenger Create Info
			populateDebugMessengerCreateInfo(debugCreateInfo);

			// Set the Debug Messenger to pNext
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			// Set the no of validation layers as zero
			createInfo.enabledLayerCount = 0;

			// Set pNext to null
			createInfo.pNext = nullptr;
		}
		// Creates an Vulkan instance
		// 1st parameter - pointer to creation info
		// 2nd parameter - pointer to custom allocator callbacks
		// 3rd parameter - pointer to Vulkan instance
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {

			// Throw runtime error exception if the Vulkan instance creation fails
			throw std::runtime_error("failed to create instance!");
		}
	}

	// Function to set up the Debug Messenger
	// Debug Messender - provides an explicit callback to print debug message in standard output
	void setupDebugMessenger() {

		// Return if the validation layers are disabled
		if (!enableValidationLayers) return;

		// Create and populate the Debug Messenger Create Info
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		// Create the Debug Messenger Extension
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			// Throw runtime error if the extension creation fails
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	// Function to create surface
	// Surface - to present rendered images
	void createSurface() {
		// Create the surface using glfwCreateWindowSurface
		// glfwCreateWindowSurface takes care of platform specific implementation of creating the surface
		// 1st Parameter - Vulkan instance
		// 2nd Parameter - Window
		// 3rd Parameter - Custom allocator
		// 4th Parameter - pointer to the surface
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			// Surface creation failed. Throw runtime error exception
			throw std::runtime_error("failed to create window surface!");
		}
	}

	// Function to pick the graphics card in the system that supports the features required in the application
	void pickPhysicalDevice() {

		// Stores the number of supporting GPUs in the system
		uint32_t deviceCount = 0;

		// Fetches the number of supporting GPUs in the system
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		// Check whether the number of supporting GPUs is zero
		if (deviceCount == 0) {
			// There is no supporting GPU. Throw runtime error exception
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		// Stores the list of all supporting GPUs
		std::vector<VkPhysicalDevice> devices(deviceCount);

		// Fetches the list of all supporting GPUs
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		// Loop through the GPUs
		for (const auto& device : devices) {

			// Check whether this GPU is suitable to application
			if (isDeviceSuitable(device)) {
				// Set the current GPU and break so that the current GPU is not assigned with a different value
				physicalDevice = device;
				break;
			}
		}

		// Check whether there is no suitable GPU found
		if (physicalDevice == VK_NULL_HANDLE) {
			// Throw runtime error exception as there is no suitable GPU
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	// Function to create logical device to interface with physical device
	// Logical device - represents logical connections to physical device
	void createLogicalDevice() {
		// Fetch the Queue families for the GPU
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		// vector to store the queue create informations
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;

		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			// structure to store the queue create information for current queue
			VkDeviceQueueCreateInfo queueCreateInfo = {};

			// Type of information stored in struture
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

			// Queue family index
			queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();

			// Number of queue families
			queueCreateInfo.queueCount = 1;

			// Set the Queue Priority
			queueCreateInfo.pQueuePriorities = &queuePriority;

			// Push this queue to the vector

			queueCreateInfos.push_back(queueCreateInfo);
		}


		// Set of device features used in the application
		VkPhysicalDeviceFeatures deviceFeatures = {};

		deviceFeatures.samplerAnisotropy = VK_TRUE;

		// Information for creating the logical device
		VkDeviceCreateInfo createInfo = {};

		// Type of information stored in the structure
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		// Set the Queue create information
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		// Set the device features information
		createInfo.pEnabledFeatures = &deviceFeatures;

		// Set the extensions used
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		// Check whether validation layers are enabled
		if (enableValidationLayers) {
			// Set the validation layers in the create information
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			// Set the validation layers as zero
			createInfo.enabledLayerCount = 0;
		}

		// Create the logical device
		// 1st Parameter - the GPU to interface with
		// 2nd Parameter - Queue and usage information
		// 3rd Parameter - optional allocator callback
		// 4th Parameter - a pointer to variable to  store the logical device
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			// Logical device creation failed. So, throw runtime error exception
			throw std::runtime_error("failed to create logical device!");
		}

		// Create the graphics queue
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);

		// Create the presentaion queue
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	// Function to create Swap chain
	// Swap chain - A queue of images waiting to be presented in the screen
	void createSwapChain() {
		// Query swap chain support properties
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		// Choose surface format
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);

		// Choose presentation mode
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);

		// Choose swap extent
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		// No of images in swap chain
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

		// Check whether the image count exceeds the max image count
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			// Set the image count to max image count
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		// Information to create swap chain
		VkSwapchainCreateInfoKHR createInfo = {};

		// Type of imformation stored in the structure
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

		// Assign Surface
		createInfo.surface = surface;

		// Assign Minimum Image count
		createInfo.minImageCount = imageCount;

		// Assign image format
		createInfo.imageFormat = surfaceFormat.format;

		// Assign color space
		createInfo.imageColorSpace = surfaceFormat.colorSpace;

		// Assign swap chain extent
		createInfo.imageExtent = extent;

		// Assign the amount of layers each image has
		createInfo.imageArrayLayers = 1;

		// Specify the usage of the image in the swap chain
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// Fetch the Queue families
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		// Store the queue family indices in an array
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		// Check whether the graphics queue family and presentation queue family are different
		if (indices.graphicsFamily != indices.presentFamily) {
			// Use concurrent sharing - Images can be used across multiple queue families without explicit owner transfer
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			// Use exclusive sharing - Image is owned by one queue family at a time and ownership must be explicitly transferred
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		// Specify the transform to be applied to the image in the swap chain
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

		// Specify if the alpha channel should be used for blending with other windows
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		// Set the presentation mode
		createInfo.presentMode = presentMode;

		// Set clipping to true
		createInfo.clipped = VK_TRUE;

		// Specify null as old swap chain
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		// Create the swap chain
		// 1st Parameter - GPU
		// 2nd Parameter - Create info
		// 3rd Parameter - optional custom allocator
		// 4th Parameter - pointer to swap chain
		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			// Swap chain creation failed. Throw runtime error exception
			throw std::runtime_error("failed to create swap chain!");
		}

		// Get the number of swap chain images
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);

		// Resize the collection of swap chain images
		swapChainImages.resize(imageCount);

		// Fetch the swap chain images
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		// Store the image format
		swapChainImageFormat = surfaceFormat.format;

		// store the extent
		swapChainExtent = extent;
	}

	// Function to create Image Views
	// Image View - specifies a way to access the image and part of image to access
	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());

		for (uint32_t i = 0; i < swapChainImages.size(); i++) {
			swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	// Function to create render pass
	// Render Pass - Information on no of color and depth buffers, no of samples to use and how the contents should be handled
	void createRenderPass() {
		// Color buffer attachment information
		VkAttachmentDescription colorAttachment = {};
		// format of colour buffer attachment
		colorAttachment.format = swapChainImageFormat;
		// Number of samples
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		// Specify what to do with the data in the attachment before rendering
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		// Specify what to do with the data in the attachment after rendering
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		// Specify what to do with the stencil data in the attachment before rendering
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		// Specify what to do with the stencil data in the attachment after rendering
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		// Specify layout the image will have before the render pass
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// Specify layout the image should transition to after render pass
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Attachment Reference for subpass
		VkAttachmentReference colorAttachmentRef = {};
		// Specify the attachment to refer
		colorAttachmentRef.attachment = 0;
		// Specify the layout to transition to when this subpass is started
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Subpass desciption
		VkSubpassDescription subpass = {};
		// Specify where the subpass has to be executed
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		// Specify the number of color attachment references
		subpass.colorAttachmentCount = 1;
		// Specify the pointer to the color attachment reference
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		// Subpass dependency to make the render pass wait for the color attachment output bit stage
		VkSubpassDependency dependency = {};
		// Specify the dependency and dependent subpasses
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		// Specify the operation to wait for. i.e, wait for the swap chain to read the image
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		// Specify the operation that should wait
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

		// Render pass create info
		VkRenderPassCreateInfo renderPassInfo = {};
		// Type of information stored in the structure
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		// No of attachments in the render pass
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		// Pointer to the attachment
		renderPassInfo.pAttachments = attachments.data();
		// No of subpasses
		renderPassInfo.subpassCount = 1;
		// Pointer to the subpass
		renderPassInfo.pSubpasses = &subpass;
		// No of subpass dependencies
		renderPassInfo.dependencyCount = 1;
		// Pointer to the subpass dependency
		renderPassInfo.pDependencies = &dependency;

		// Create render pass
		// 1st Parameter - GPU
		// 2nd Parameter - Render pass create info
		// 3rd Parameter - Custom Allocator
		// 4th Parameter - Pointer to the created render pass
		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			// Throw runtime error exception as render pass creation failed
			throw std::runtime_error("failed to create render pass!");
		}
	}

	// Function to create the Graphics pipeline
	// Graphics Pipeline - Sequence of operations with vertices & textures as input and pixels to render as output
	void createGraphicsPipeline() {
		// Fetch the byte code of vertex shader
		auto vertShaderCode = readFile("shaders/vert.spv");

		// Fetch the byte code of fragment shader
		auto fragShaderCode = readFile("shaders/frag.spv");

		// Create Vertex shader module
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);

		// Create Fragment shader module
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		// Vertex shader stage create info
		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		// Type of information stored in the structure
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		// Specify the vertex shader stage in the pipeline
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		// Specify the vertex shader module
		vertShaderStageInfo.module = vertShaderModule;
		// Specify the entry point
		vertShaderStageInfo.pName = "main";

		// Fragment shader stage create info
		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		// Type of information stored in the structure
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		// Specify the fragment shader stage in the pipeline
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		// Specify the fragment shader module
		fragShaderStageInfo.module = fragShaderModule;
		// Specify the entry point
		fragShaderStageInfo.pName = "main";

		// Create an array to store vertex shader stage create info and fragment shader stage create info
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// Binding description
		auto bindingDescription = Vertex::getBindingDescription();

		// Attribute description
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		// Information of format of the vertex data passed to the vertex shader
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		// Type of information stored in the structure
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		// Details for loading vertex data
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		// Information of kind of geometry drawn
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		// Type of information stored in the structure
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		// Specify kind of geometry drawn
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		// Specify that it is not possible to break lines and triangles by using special index
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Viewport information
		// Viewport - region of framebuffer where the output will be rendereds
		VkViewport viewport = {};
		// starting x of viewport
		viewport.x = 0.0f;
		// starting y of viewport
		viewport.y = 0.0f;
		// width of viewport
		viewport.width = (float)swapChainExtent.width;
		// height of viewport
		viewport.height = (float)swapChainExtent.height;
		// min depth of viewport
		viewport.minDepth = 0.0f;
		// max depth of viewport
		viewport.maxDepth = 1.0f;

		// Scissor information
		// Scissor - a specification of the pixels that will be stored
		VkRect2D scissor = {};
		// Scissor offset
		scissor.offset = { 0, 0 };
		// Scissor extent
		scissor.extent = swapChainExtent;

		// Viewport State create info
		VkPipelineViewportStateCreateInfo viewportState = {};
		// Type of information stored in the structure
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		// No of viewports
		viewportState.viewportCount = 1;
		// Specify the pointer to the viewports
		viewportState.pViewports = &viewport;
		// No of scissors
		viewportState.scissorCount = 1;
		// Specify the pointer to the scissors
		viewportState.pScissors = &scissor;

		// Rasterization State create info
		// Rasterizer - turns the geometry into fragments
		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		// Type of information stored in the structure
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		// Specify whether the fragments beyond the near and far planes has to be clamped or discarded
		rasterizer.depthClampEnable = VK_FALSE;
		// Specify whether the geometry should pass through the rasterizer stage
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		// Specify how fragments are generated
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		// Specify the thickness of the lines
		rasterizer.lineWidth = 1.0f;
		// Specify the type of culling to use
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		// Specify the vertex order
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		// Should the rasterizer alter the depth values by adding a constant value or biasing them
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		// Information to configure multisampling to perform anti-aliasing
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		// Type of information stored in the structure
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		// Colour blending configuration per attached framebuffer
		// Colour blending - way to combine with colour already in framebuffer
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		// Global colour blending setting information
		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		// Type of information stored in the structure
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		// Dynamic states which can be changed without recreating the pipeline
		VkDynamicState dynamicStates[] = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_LINE_WIDTH
		};

		// Dynamic state create info
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		// Type of information stored in the structure
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		// No of dynamic states
		dynamicState.dynamicStateCount = 2;
		// Specify the array of dynamic states
		dynamicState.pDynamicStates = dynamicStates;

		// Pipeline layout create info
		// Pipeline layout - Uniform values/Push constants specified during pipeline creation
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		// Type of information stored in the structure
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		// Number of uniforms
		pipelineLayoutInfo.setLayoutCount = 1;

		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		// Number of push constants
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		// Create the pipeline layout
		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			// Throw runtime error exception as pipeline layout creation failed
			throw std::runtime_error("failed to create pipeline layout!");
		}

		// Graphics pipeline create info
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		// Type of information stored in the structure
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		// No of shader stages
		pipelineInfo.stageCount = 2;
		// Pointer to shader stages
		pipelineInfo.pStages = shaderStages;
		// Specify the vertex input state
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		// Specify the input assembly state
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		// Specify the viewport state
		pipelineInfo.pViewportState = &viewportState;
		// Specify the rasterization state
		pipelineInfo.pRasterizationState = &rasterizer;
		// Specify the Multisampling state
		pipelineInfo.pMultisampleState = &multisampling;
		// Specify Depth stencil state
		pipelineInfo.pDepthStencilState = &depthStencil;
		// Specify the color blend state
		pipelineInfo.pColorBlendState = &colorBlending;
		// Specify the dynamic state
		pipelineInfo.pDynamicState = nullptr; // Optional
		// Specify the pipeline layout
		pipelineInfo.layout = pipelineLayout;
		// Specify the render pass
		pipelineInfo.renderPass = renderPass;
		// Specify the index of the render pass where this graphics pipeline will be used
		pipelineInfo.subpass = 0;
		// Specify the base pipeline to derive from
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		// Specify the index of base pipeline to derive from
		pipelineInfo.basePipelineIndex = -1; // Optional

		// Create the graphics pipeline
		// 1st Parameter - GPU
		// 2nd Parameter - Pipeline cache to store and reuse data for pipeline creation
		// 3rd Parameter - Pipeline create info
		// 4th Parameter - Custom allocator
		// 5th Parameter - Pointer to the created graphics pipeline
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			// Throw runtime error exception as graphics pipeline creation failed
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		// Destroy the fragment shader module
		vkDestroyShaderModule(device, fragShaderModule, nullptr);

		// Destroy the vertex shader module
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	// Function to create Frame buffers
	// Frame buffers - Frame buffers represent a group of memory attachments used by a render pass instance
	void createFramebuffers() {
		// Resize the collection of frame buffers
		swapChainFramebuffers.resize(swapChainImageViews.size());

		// Loop through the image views
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {

			// Attachments for current image view
			std::array<VkImageView, 2> attachments = {
	swapChainImageViews[i],
	depthImageView
			};

			// Frame buffer create info
			VkFramebufferCreateInfo framebufferInfo = {};
			// Type of information stored in the structure
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			// Specify the render pass
			framebufferInfo.renderPass = renderPass;
			// Specify the no of attachments for image view
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			// Specify the attachment for the image view
			framebufferInfo.pAttachments = attachments.data();
			// Specify width
			framebufferInfo.width = swapChainExtent.width;
			// Specify height
			framebufferInfo.height = swapChainExtent.height;
			// Specify the no of layers in the image
			framebufferInfo.layers = 1;

			// Create the frame buffer
			// 1st Parameter - GPU
			// 2nd Parameter - Frame buffer create info
			// 3rd Parameter - Custom Allocator
			// 4th Parameter - Pointer to the created frame buffer
			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				// Throw runtime error exception as framebuffer creation failed
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	// Function to create command pool
	// Command Pool - Manage the memory used to store the buffers. Command buffers are allocated from Command Pools
	void createCommandPool() {
		// Query the queue family indices
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		// Command pool create info
		VkCommandPoolCreateInfo poolInfo = {};
		// Type of information stored in the structure
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		// Specify the graphics queue family index as the commands are for drawing
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		// Set the flag to zero as command buffers will be recorded only at the program start
		poolInfo.flags = 0; // Optional

		// Create command pool
		// 1st Parameter - GPU
		// 2nd Parameter - command pool create info
		// 3rd Parameter - Custom allocator
		// 4th Parameter - pointer to created command pool
		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			// Throw runtime error exception as command pool creation failed
			throw std::runtime_error("failed to create command pool!");
		}
	}
	
	// Function to create a buffer
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	// Function to create a image view
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	// Function to create depth resources for depth buffer
	void createDepthResources() {
		VkFormat depthFormat = findDepthFormat();
		createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
		depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	}
		
	// Function to find the suitable depth format
	VkFormat findDepthFormat() {
		return findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	// Function to find the supported depth format
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {

		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}
	
	// Function to create descriptor pool to create descriptor sets
	void createDescriptorPool() {
		std::array<VkDescriptorPoolSize, 3> poolSizes = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[2].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	// Function to create descriptor sets for each Vk Buffer
	void createDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(swapChainImages.size());
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorBufferInfo lightingBufferInfo = {};
			lightingBufferInfo.buffer = lightingBuffers[i];
			lightingBufferInfo.offset = 0;
			lightingBufferInfo.range = sizeof(LightingConstants);

			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = textureImageView;
			imageInfo.sampler = textureSampler;

			std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &lightingBufferInfo;

			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = descriptorSets[i];
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pImageInfo = &imageInfo;

			try
			{
				vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
			catch (...)
			{

			}
		}
	}

	// Function to create Uniform Buffers
	void createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(swapChainImages.size());
		uniformBuffersMemory.resize(swapChainImages.size());

		VkDeviceSize lightingBufferSize = sizeof(LightingConstants);

		lightingBuffers.resize(swapChainImages.size());
		lightingBuffersMemory.resize(swapChainImages.size());


		for (size_t i = 0; i < swapChainImages.size(); i++) {
			createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
			createBuffer(lightingBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, lightingBuffers[i], lightingBuffersMemory[i]);
		}
	}

	// Function to create Index Buffer
	void createIndexBuffer() {
		VkDeviceSize bufferSize = sizeof(m.indices[0]) * m.indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, m.indices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	// Function to create Vertex Buffer
	void createVertexBuffer() {

		VkDeviceSize bufferSize = sizeof(m.vertices[0]) * m.vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, m.vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	// Function to parse obj file and generate a mesh
	Mesh ParseObjFile(const char* filename)
	{
		std::vector<Vec3> positions;
		std::vector<Vec3> normals;
		std::vector<Vec3> texCoords;
		Mesh mesh;
		std::ifstream inputFile;
		inputFile.open(filename);
		if (!inputFile.is_open())
		{
			throw std::runtime_error("failed to open obj file!");
		}


		std::string tempString = "";
		int i;
		while (true)
		{
			inputFile >> tempString;
			if (tempString == "mtllib")
			{
				std::string materialFilename;
				inputFile >> materialFilename;
				mesh.lightingConstants = LoadMaterial(materialFilename.c_str());
			}
			if (tempString == "v")
			{
				float x, y, z;
				inputFile >> x >> y >> z;
				Vec3 v;
				v.x = x;
				v.y = y;
				v.z = z;

				positions.push_back(v);
			}
			else if (tempString == "vn")
			{
				float x, y, z;
				inputFile >> x >> y >> z;
				Vec3 v;
				v.x = x;
				v.y = y;
				v.z = z;

				normals.push_back(v);
			}
			else if (tempString == "vt")
			{
				float x, y, z;
				inputFile >> x >> y >> z;
				Vec3 v;
				v.x = x;
				v.y = 1 - y;
				v.z = z;
				texCoords.push_back(v);
			}

			else if (tempString == "f")
			{
				for (int j = 0; j < 4; j++)
				{
					int position_index, tex_index, normal_index;
					inputFile >> tempString;
					position_index = atoi(strtok(&tempString[0], "/"));
					tex_index = atoi(strtok(NULL, "/"));
					normal_index = atoi(strtok(NULL, "/"));
					Vertex v;
					v.position = positions[position_index - 1];
					Vec3 color;
					color.x = 1.0f;
					color.y = 1.0f;
					color.z = 1.0f;

					v.color = color;
					v.tex = texCoords[tex_index - 1];
					v.normal = normals[normal_index - 1];

					mesh.vertices.push_back(v);
				}
				int vertex_index = mesh.vertices.size();
				mesh.indices.push_back(vertex_index - 4);
				mesh.indices.push_back(vertex_index - 3);
				mesh.indices.push_back(vertex_index - 2);
				mesh.indices.push_back(vertex_index - 4);
				mesh.indices.push_back(vertex_index - 2);
				mesh.indices.push_back(vertex_index - 1);

			}
			else if (inputFile.eof())
				break;
		}

		return mesh;
	}

	// Function to load material
	LightingConstants LoadMaterial(const char* filename)
	{
		LightingConstants lightingConstants;
		std::ifstream inputFile;
		inputFile.open(filename);
		if (!inputFile.is_open())
		{
			throw std::runtime_error("failed to open obj file!");
		}
		std::string tempString = "";
		while (true)
		{
			inputFile >> tempString;
			if (tempString == "Ns")
			{
				inputFile >> lightingConstants.lightSpecularExponent;
			}
			else if (tempString == "Ka")
			{
				float r, g, b;
				inputFile >> r >> g >> b;
				lightingConstants.lightAmbient = glm::vec4(r, g, b, 1.0); 
			}
			else if (tempString == "Ks")
			{
				float r, g, b;
				inputFile >> r >> g >> b;
				lightingConstants.lightSpecular =  glm::vec4(r, g, b, 1.0); 
			}
			else if (tempString == "Kd")
			{
				float r, g, b;
				inputFile >> r >> g >> b;
				lightingConstants.lightDiffuse = glm::vec4(r, g, b, 1.0); 
			}
			else if (inputFile.eof())
				break;
		}

		// Set the intensities of the light
		lightingConstants.ambientIntensity = 0.2;
		lightingConstants.specularIntensity = 5.3;
		lightingConstants.diffuseIntensity = 0.7;

		// Set the position of the light
		lightingConstants.lightPosition = glm::vec4(0.0f, -200.0f, 260.0f, 1.0f);
		return lightingConstants;
	}

	// Function to create texture image
	void createTextureImage(const char* filename) {

		int texWidth, texHeight, texChannels;

		std::ifstream inputFile;
		inputFile.open(filename);
		if (!inputFile.is_open())
		{
			throw std::runtime_error("failed to open obj file!");
		}


		std::string tempString = "";

		//Read the header
		int tempInt = 0;

		inputFile >> tempString;

		// TO DO: Reading Comments

		// Read the height and width of the texture
		//int texWidth, texHeight;
		inputFile >> texWidth >> texHeight;

		// Read max colours
		inputFile >> tempString;
		int size = texWidth * texHeight;

		inputFile.seekg(0, inputFile.end);
		int length = inputFile.tellg();
		inputFile.seekg(0, inputFile.beg);

		inputFile >> tempString;
		inputFile >> tempString;
		inputFile >> tempString;
		inputFile >> tempString;

		unsigned char* fileContents = new unsigned char[length];

		int j = 0;
		inputFile.read((char*)fileContents, length);

		//// Array of texels
		//uint32_t* texels = new uint32_t[size];
		unsigned char* texels = new unsigned char[size * 4];
		unsigned char maxr = 0;
		VkDeviceSize imageSize = texWidth * texHeight * 4;
		int i = 0;
		while (i < size)
		{
			unsigned char r, g, b;

			r = fileContents[i * 3 + 1];
			g = fileContents[i * 3 + 2];
			b = fileContents[i * 3 + 3];

			texels[i * 4] = r;
			texels[i * 4 + 1] = g;
			texels[i * 4 + 2] = b;
			texels[i * 4 + 3] = 255;
			i++;
		}

		inputFile.close();
		if (!texels) {
			throw std::runtime_error("failed to load texture image!");
		}



		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, texels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device, stagingBufferMemory);

		//inputFile.close();

		createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

	}

	// Function to create texture image view
	void createTextureImageView() {
		textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	// Function to create texture sampler
	void createTextureSampler() {

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	// Function to copy buffer to image
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);
		endSingleTimeCommands(commandBuffer);
	}

	// Function to create image
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device, image, imageMemory, 0);
	}

	// Function to start single time commands in command buffer
	VkCommandBuffer beginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	// Function to end single time commands in command buffer
	void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	// Function to find the memory type to create a buffer
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {

		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	// Function to copy the contents from one buffer to another
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}

	// Function to transition image layout
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands(commandBuffer);

	}

	// function to create descriptor set layout to  provide the details about descriptor bindings in every shader
	void createDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		VkDescriptorSetLayoutBinding lightingLayoutBinding = {};
		lightingLayoutBinding.binding = 1;
		lightingLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lightingLayoutBinding.descriptorCount = 1;
		lightingLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		lightingLayoutBinding.pImmutableSamplers = nullptr; // Optional

		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 2;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 3> bindings = { uboLayoutBinding, lightingLayoutBinding, samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	// Function to create command buffers
	// Command Buffers - All drawing operations are recorded in a command buffer
	void createCommandBuffers() {
		// resize the command buffers collection
		commandBuffers.resize(swapChainFramebuffers.size());

		// Command Buffer Allocate Info
		VkCommandBufferAllocateInfo allocInfo = {};
		// Type of information stored in the structure
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		// Specify the command pool
		allocInfo.commandPool = commandPool;
		// Specify the command buffer is primary command buffer - can be submitted to queue for execution, but cannot be called from another other command buffer
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		// Specify the no of buffers to allocate
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		// Allocate command buffers
		// 1st Parameter - GPU
		// 2nd Parameter - Command Buffer allocate info
		// 3rd Parameter - Pointer to Command Buffers
		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			// Throw runtime error exception as allocation for command buffers failed
			throw std::runtime_error("failed to allocate command buffers!");
		}

		// Loop through the command buffers
		for (size_t i = 0; i < commandBuffers.size(); i++) {
			// Command buffer begin info to start command buffer recording
			VkCommandBufferBeginInfo beginInfo = {};
			// Type of information stored in the structure
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			// Specify how the command buffer is used
			beginInfo.flags = 0; // Optional
			// Specify which state to inherit from, in case of secondary command buffer
			beginInfo.pInheritanceInfo = nullptr; // Optional

			// Start record the command buffer
			// 1st Parameter - command buffer to start recording
			// 2nd Parameter - Begin info
			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				// Throw runtime error exception
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			// Render pass begin info to start the render pass
			VkRenderPassBeginInfo renderPassInfo = {};
			// Type of information stored in the structure
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			// Specify the render pass to start
			renderPassInfo.renderPass = renderPass;
			// Specify the swap chain frame buffers
			renderPassInfo.framebuffer = swapChainFramebuffers[i];
			// Specify the area where shader loads and stores take place
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChainExtent;
			// Set the clear colour for the background
			// Set the number of clear colours
			// Specify the pointer to the clear colour

			std::array<VkClearValue, 2> clearValues = {};
			clearValues[0].color = { 0.8f, 0.6f, 0.0f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			// Start the render pass
			// 1st Parameter - command buffer to record the commands to
			// 2nd Parameter - render pass begin info
			// 3rd Parameter - whether the drawing commands are executed inline or executed from a secondary command buffers
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Bind the graphics pipeline
			// 1st Parameter - command buffer 
			// 2nd Parameter - whether the pipeline object is graphics pipeline or compute pipeline
			// 3rd Parameter - graphics pipeline
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

			VkBuffer vertexBuffers[] = { vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			// Draw the polygon - triangle
			// 1st Parameter - command buffer
			// 2nd Parameter - vertex count
			// 3rd Parameter - instance count
			// 4th Parameter - first vertex
			// 5th Parameter - first instance
			//vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);

			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(m.indices.size()), 1, 0, 0, 0);

			// End the render pass recording
			vkCmdEndRenderPass(commandBuffers[i]);

			// End the command buffer recording
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				// Throw runtime error exception as command buffer recording cannot be ended
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}


	// Function to create semaphores and fences
	// Semaphores - A synchronization method where operations are synchronized within or across command queues
	// Fences - A synchronization method where the entire application is synchronized with the rendering operation
	void createSyncObjects() {

		// Resize the collection of image available semaphores
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

		// Resize the collection of render finished semaphores
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

		// Resize the collection of in-flight fences
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		// Resize the collection of images in-flight fences
		imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);


		// create info for semaphores
		VkSemaphoreCreateInfo semaphoreInfo = {};
		// Type of information stored in the structure
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		// Create info for fences
		VkFenceCreateInfo fenceInfo = {};
		// Type of information stored in the strucuture
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		// Specify the flag to start the fences with signaled state
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		// Create all the semaphores and fences
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			// Create semaphores and fences

			// Create the semaphores
			// 1st Parameter - GPU
			// 2nd Parameter - create info for semaphores
			// 3rd Parameter - Custom allocator
			// 4th Parameter - pointer to the created semaphore

			// Create the fence
			// 1st Parameter - GPU
			// 2nd Parameter - create info for fence
			// 3rd Parameter - Custom allocator
			// 4th Parameter - pointer to the created fence
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				// Any one or both the semaphore creation failed, so throw runtime error exception
				throw std::runtime_error("failed to create semaphores!");
			}
		}
	}

	// Function to check whether all the requested validation layers are available
	// Validation layer - optional components hooked into Vulkan function calls to add validation check operations
	bool checkValidationLayerSupport() {

		// Find the number of available layers
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		// Find the list of all available layers
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// Loop through the requested validation layers
		for (const char* layerName : validationLayers) {

			// flag to set when the requested validation layer is there in the list of available validation layers
			bool layerFound = false;

			// Loop through all the available validation layers
			for (const auto& layerProperties : availableLayers) {

				// Compare the layer name of the requested layer and layer in available layers 
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					// The requested layer is there in the available layers list
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				// The requested layer is not there in the available layers list
				return false;
			}
		}

		return true;
	}

	// Function that returns the list of extenstions based on whether validation layers are enabled or not
	std::vector<const char*> getRequiredExtensions() {

		// Stores the no of extensions
		uint32_t glfwExtensionCount = 0;

		// Stores the reference to the extensions
		const char** glfwExtensions;

		// GLFW function to fetch the extensions required
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// A list of extensions
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		// Check whether validation layers are enabled
		if (enableValidationLayers) {
			// Add Debug Utils Extension
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		// Return the extensions
		return extensions;
	}

	// Function to populate the Debug Messenger Create Info
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		// Specify the structure type
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		// Specify all types of severities for the callback
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		// Specify the types of messages for the callback to be called
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		// Pointer to the callback function to be called
		createInfo.pfnUserCallback = debugCallback;
	}

	// Function to check whether the device is suitable for the application
	bool isDeviceSuitable(VkPhysicalDevice device) {

		// Stores the properties of a device
		VkPhysicalDeviceProperties deviceProperties;

		// Stores the optional features supported by the device
		VkPhysicalDeviceFeatures deviceFeatures;

		// Fetch the properties of the device
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		// Fetch the features supported by the device
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		// Fetch the queue family indices supported
		QueueFamilyIndices indices = findQueueFamilies(device);

		// Flag to check Device extension support
		bool extensionsSupported = checkDeviceExtensionSupport(device);

		// Flag to check swap chain properties are supported
		bool swapChainAdequate = false;

		// Check whether device extension is supported
		if (extensionsSupported) {

			// Query the swap chain support properties
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);

			// Check whether all swap chain properties are supported
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		// Check whether the device type, device features, queue families, device extensions and swap chain properties satisfy the conditions
		// Return the value evaluated
		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader &&
			indices.isComplete() &&
			extensionsSupported &&
			swapChainAdequate &&
			supportedFeatures.samplerAnisotropy;
	}

	// Function to  find the suitable Queue families
	// Queue family - A family of queues only allow a subset of commands
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {

		// Stores the Queue family indices
		QueueFamilyIndices indices;

		// The no of queue families
		uint32_t queueFamilyCount = 0;

		// Fetch the number of queue families
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		// List of all queue families
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

		// Fetch the list of all queue families
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;

		// Loop through the queue families
		for (const auto& queueFamily : queueFamilies) {
			// Check whether queue family supports VK_QUEUE_GRAPHICS_BIT
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				// Assign the index of the graphics family
				indices.graphicsFamily = i;
			}

			// Check whether queue family supports presentation to the surface
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport) {
				// Assign the index of the presentation family
				indices.presentFamily = i;
			}

			// Break if there is a value assigned to indices
			if (indices.isComplete())
				break;
			i++;
		}

		// Return the Queue family indices
		return indices;
	}

	// Function to query the swap chain support details
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {

		// Swap chain support properties
		SwapChainSupportDetails details;

		// Query the device surface capabilities
		// 1st Parameter - GPU
		// 2nd Parameter - surface
		// 3rd Parameter - pointer to device surface capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		// Fetch the number of surface formats supported
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		// Check whether the number of supported surface formats is not zero
		if (formatCount != 0) {
			// Resize the collection to store surface formats
			details.formats.resize(formatCount);

			// Fetch the collection of surface formats
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		// Fetch the number of presentation modes available
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		// Check whether the number of available presentation modes is not zero
		if (presentModeCount != 0) {
			// Resize the collection of presentation modes
			details.presentModes.resize(presentModeCount);

			// Fetch the collection of presentation modes
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		// Return the swap chain support details
		return details;
	}

	// Function to choose the surface format for the swap chain
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		// Loop through the available surface formats
		for (const auto& availableFormat : availableFormats) {
			// Check whether the current iterate of the formats stores in order BGRA with 8 bit unsigned integer and whether SRGB color space is supported
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				// Return the format satifying the condition
				return availableFormat;
			}
		}

		// Return the first format if none of the formats satisfy the conditions
		return availableFormats[0];
	}

	// Function to choose the presentation mode for the swap chain
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		// Loop through the available presentation modes
		for (const auto& availablePresentMode : availablePresentModes) {
			// Check if the presentation mode supports triple buffering
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				// Return the presentation mode supporting triple buffering
				return availablePresentMode;
			}
		}

		// Return FIFO presentation mode when no presentation mode supports triple buffering
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	// Function to choose swap extent for the swap chain
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		// Check whether the current extent width is not maximum
		if (capabilities.currentExtent.width != UINT32_MAX) {
			// return the current extent
			return capabilities.currentExtent;
		}
		else {
			// Get the current window width and height
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			// Choose the extent that best matches the window within minImageExtent and maxImageExtentBounds
			VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
			};

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	// Debug callback function
	// VKAPI_ATTR and VKAPI_CALL ensures the right signature for Vulkan to call it
	// 1st Parameter - Severity of message - Diagnostic, Information, Warning or Error
	// 2nd Parameter - Message Type - General, Validation or Performance
	// 3rd Parameter - Contains the details of the message with members pMessage,pObjects and objectsCount
	// 4th Parameter - Used to pass data
	// Return value - Should the Vulkan call that triggered the validation layer message should be aborted
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		// Print the validation error
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		// Do not abort the Vulkan call that triggered the validation layer message
		return VK_FALSE;
	}

	// Function to read a file
	static std::vector<char> readFile(const std::string& filename) {
		// Open the file as a binary and start reading at the end of file
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		// Check whether file opening failed
		if (!file.is_open()) {
			// Throw runtime error exception if file opening failed
			throw std::runtime_error("failed to open file!");
		}

		// Find the file size
		size_t fileSize = (size_t)file.tellg();

		// Initialize a buffer with file size
		std::vector<char> buffer(fileSize);

		// Seek to the beginning of the file
		file.seekg(0);

		// Read all the bytes
		file.read(buffer.data(), fileSize);

		// Close the file
		file.close();

		// Return the buffer
		return buffer;
	}

	// Function to create shader module
	// This function takes byte code of the shader as paramter and returns the shader module
	VkShaderModule createShaderModule(const std::vector<char>& code) {
		// Create info for shader module
		VkShaderModuleCreateInfo createInfo = {};
		// Type of information stored in the structure
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		// Size of byte code
		createInfo.codeSize = code.size();
		// Assign the byte code. Cast it uint32_t pointer from char pointer
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		// Create the shader module
		// 1st Parameter - GPU
		// 2nd Parameter - Create Info
		// 3rd Parameter - optional custom allocator
		// 4th Parameter - pointer to shader module
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			// Throw runtime error exception as the shader module creation failed
			throw std::runtime_error("failed to create shader module!");
		}

		// Return shader module
		return shaderModule;
	}

	// Function to check whether the device supports all required extensions
	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {

		// Fetch the number of extensions supported by the device
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		// Fetch the list of extensions supported by the device
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		// Set of strings used to represent the unconfirmed required extensions
		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			// Remove the current iterate of available extensions from the set
			requiredExtensions.erase(extension.extensionName);
		}

		// If the set is empty, then all the required extensions are available in the device
		return requiredExtensions.empty();
	}

	//////////////////////////
	// Main Loop Functions //
	////////////////////////

	// Function to draw the frame on the screen
	// Acquires the image from the swap chain and executes command buffer and returns the image to swap chain for presentation
	void drawFrame() {

		// Wait for frame to finish
		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		// index of swap chain image
		uint32_t imageIndex;
		// Acquire image from swap chain
		// 1st Parameter - GPU
		// 2nd Parameter - Swap chain
		// 3rd Parameter - timeout in nanoseconds for an image to become available
		// 4th & 5th Parameter - synchronization object to signal when presentation engine used this image
		// 6th Parameter - index of swap chain image
		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		// Check whether swap chain has become incompatible with the surface
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			// Recreate swap chain
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			// Throw runtime error exception as acquiring image from swap chain failed
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		// Check if a previous frame is using this image (i.e. there is its fence to wait on)
		if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
			// Wait for the fences
			vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}
		// Mark the image as now being in use by this frame
		imagesInFlight[imageIndex] = inFlightFences[currentFrame];

		// Update the uniform buffer to have the current model view projection matrices
		updateUniformBuffer(imageIndex);

		// Update the uniform buffer to have the current ambient, specular, diffuse values
		updateLightingConstants(imageIndex);

		// Submit info to submit to command buffer
		VkSubmitInfo submitInfo = {};
		// Type of information stored in the structure
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		// Semaphores to wait for
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		// Pipeline stage to wait
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		// Set the no of semaphores
		submitInfo.waitSemaphoreCount = 1;
		// Set the semaphores to wait for
		submitInfo.pWaitSemaphores = waitSemaphores;
		// Set the stages to wait
		submitInfo.pWaitDstStageMask = waitStages;
		// Set the no of command buffers to submit
		submitInfo.commandBufferCount = 1;
		// Set the pointer to command buffers to submit
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
		// Semaphores to signal after command buffer execution
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		// Set the no of semaphores to signal
		submitInfo.signalSemaphoreCount = 1;
		// Set the semaphores to signal
		submitInfo.pSignalSemaphores = signalSemaphores;

		// Reset the fence to unsignaled state
		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		// Submit the command buffer to graphics queue
		// 1st Parameter - graphics queue
		// 2nd Parameter - No of submit infos
		// 3rd Parameter - pointer to submit infos
		// 4th Parameter - optional fence signaled when command buffer is executed
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			// Throw runtime error exception as the submission of command buffer to graphics queue failed
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		// Configuring the presentation using present info
		VkPresentInfoKHR presentInfo = {};
		// Type of information stored in the structure
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		// no of semaphores to wait for before presentation
		presentInfo.waitSemaphoreCount = 1;
		// semaphores to wait for before presentation
		presentInfo.pWaitSemaphores = signalSemaphores;
		// Array of swap chains
		VkSwapchainKHR swapChains[] = { swapChain };
		// Set the no of swap chains
		presentInfo.swapchainCount = 1;
		// Specify the swap chains
		presentInfo.pSwapchains = swapChains;
		// Specify the index of image for each swap chain	
		presentInfo.pImageIndices = &imageIndex;
		// Result values to check in each swap chain
		presentInfo.pResults = nullptr;

		// Submit a request to present the image to swap chain
		result = vkQueuePresentKHR(presentQueue, &presentInfo);

		// Check whether the swap chain has become incompatible with the surface or the surface properties no longer match or frame buffer is resized
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {

			// Set frame buffer resized flag to false
			framebufferResized = false;

			// Recreate swap chain
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			// Throw runtime error exception as request to present the image to swap chain failed
			throw std::runtime_error("failed to present swap chain image!");
		}

		// Update the current frame index
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	// Function to update uniform buffer values
	void updateUniformBuffer(uint32_t currentImage) {
		
		UniformBufferObject ubo = {};
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f , 0.0f + translate_x * 2, -15.0f + translate_y * 2)) * glm::rotate(glm::mat4(1.0f), glm::radians(10.0f) * rotate_y, glm::vec3(0.0f,1.0f , 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(10.0f) * rotate_x, glm::vec3(0.0f, 0.0f, 1.0f));
		//ubo.view = glm::lookAt(glm::vec3(0.0f, -100.0f, 100.0f), glm::vec3(0.0f, 0.0f, 40.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(85.0f, 2.0f, 100.0f), glm::vec3(0.0f, 0.0f, 40.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 1000.0f);
		ubo.proj[1][1] *= -1;

		void* data;
		vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
	}

	// Function to update lighting constant values
	void updateLightingConstants(uint32_t currentImage) {
		
		void* lightData;
		vkMapMemory(device, lightingBuffersMemory[currentImage], 0, sizeof(m.lightingConstants), 0, &lightData);
		memcpy(lightData, &m.lightingConstants, sizeof(m.lightingConstants));
		vkUnmapMemory(device, lightingBuffersMemory[currentImage]);
	}

	// Function to recreate swap chain and other components when window is resized
	void recreateSwapChain() {

		// Get the width and height of the window to check whether it is minimized
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		// Check whether the window is minimized
		while (width == 0 || height == 0) {
			// Update the width and height
			glfwGetFramebufferSize(window, &width, &height);
			// Wait for the window to be maximized
			glfwWaitEvents();
		}
		// Wait for the device to complete its operations
		vkDeviceWaitIdle(device);

		// Cleanup existing swap chain and dependent components
		cleanupSwapChain();

		// Create swap chain
		createSwapChain();
		// Create Image Views
		createImageViews();
		// Create Render Pass
		createRenderPass();
		// Create Graphics pipeline
		createGraphicsPipeline();

		createDepthResources();

		// Create frame buffers
		createFramebuffers();


		createUniformBuffers();

		createDescriptorPool();

		createDescriptorSets();

		// Create command buffers
		createCommandBuffers();
	}

	////////////////////////
	// Cleanup Functions //
	//////////////////////

	// Cleanup function to destroy swap chain and related components
	void cleanupSwapChain() {
		
		// Destroy the depth image view, depth image and depth memory
		vkDestroyImageView(device, depthImageView, nullptr);
		vkDestroyImage(device, depthImage, nullptr);
		vkFreeMemory(device, depthImageMemory, nullptr);

		// Destroy the frame buffers
		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		// Destroy the graphics pipeline
		vkDestroyPipeline(device, graphicsPipeline, nullptr);

		// Destroy the pipeline layout
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

		// Destroy the render pass
		vkDestroyRenderPass(device, renderPass, nullptr);

		// Destroy the image views
		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}

		// Destroy the swap chain
		vkDestroySwapchainKHR(device, swapChain, nullptr);

		// Cleanup the uniform buffer and lighting constants buffer
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);

			vkDestroyBuffer(device, lightingBuffers[i], nullptr);
			vkFreeMemory(device, lightingBuffersMemory[i], nullptr);
		}

		// Destroy the descriptor pool
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	}
};

// Main function
int main() {

	// Instance to Vulkan Application
	HelloTriangleApplication app;

	try {
		// Run the application to initialize and run the Vulkan objects
		app.run();
	}
	catch (const std::exception& e) {

		// Print the exception description to command prompt
		std::cerr << e.what() << std::endl;

		// return EXIT_FAILURE as the program has failed
		return EXIT_FAILURE;
	}

	// return EXIT_SUCCESS as the program has run successfully
	return EXIT_SUCCESS;
}