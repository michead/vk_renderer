#pragma once

#include <functional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


template <typename T>
class VkWrap {
public:
	VkWrap(std::function<void(T, VkAllocationCallbacks*)> dFunc)
	{
		this->deleteFunc = [=](T obj) { dFunc(obj, nullptr); };
	}

	VkWrap(const VkInstance& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> dFunc)
	{
		this->deleteFunc = [&instance, dFunc](T obj) { dFunc(instance, obj, nullptr); };
	}

	VkWrap(const VkDevice& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> dFunc)
	{
		this->deleteFunc = [&device, dFunc](T obj) { dFunc(device, obj, nullptr); };
	}

	~VkWrap()
	{
		cleanup();
	}

	T* operator &()
	{
		cleanup();
		return &object;
	}

	operator T() const
	{
		return object;
	}

	T& get()
	{
		return object;
	}

private:
	T object { VK_NULL_HANDLE };
	std::function<void(T)> deleteFunc;

	void cleanup()
	{
		if (object != VK_NULL_HANDLE)
		{
			deleteFunc(object);
		}

		object = VK_NULL_HANDLE;
	}
};
