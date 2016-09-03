#pragma once

#include <functional>
#include "vulkan\vulkan.h"

#define VK_WRAP(a) VkObjWrapper<a>
#define VK_VEC_WRAP(a) std::vector<VkObjWrapper<a>>

template<typename T>
class VkObjWrapper
{
public:
	VkObjWrapper() : VkObjWrapper([](T _) {}) {}

	VkObjWrapper(std::function<void(T, VkAllocationCallbacks*)> deleteFunc_)
	{
		this->deleteFunc = [=](T obj) { deleteFunc_(obj, nullptr); };
	}

	VkObjWrapper(const VK_WRAP(VkInstance)& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deleteFunc_)
	{
		this->deleteFunc = [&instance, deleteFunc_](T obj) { deleteFunc_(instance, obj, nullptr); };
	}

	VkObjWrapper(const VK_WRAP(VkDevice)& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deleteFunc_)
	{
		this->deleteFunc = [&device, deleteFunc_](T obj) { deleteFunc_(device, obj, nullptr); };
	}

	~VkObjWrapper()
	{
		cleanup();
	}

	T* operator &()
	{
		cleanup();
		return &vkObject;
	}

	operator T() const
	{
		return vkObject;
	}

private:
	T vkObject { VK_NULL_HANDLE };
	std::function<void(T)> deleteFunc;

	void cleanup()
	{
		if (vkObject != VK_NULL_HANDLE)
		{
			deleteFunc(vkObject);
		}

		vkObject = VK_NULL_HANDLE;
	}
};
