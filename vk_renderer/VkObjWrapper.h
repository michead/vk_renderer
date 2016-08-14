#pragma once

#include <functional>
#include "vulkan\vulkan.h"

template<typename T>
class VkObjWrapper
{
public:
	VkObjWrapper() : VkObjWrapper([](T _) {}) {}

	VkObjWrapper(std::function<void(T, VkAllocationCallbacks*)> deleteFunc_)
	{
		this->deleteFunc = [=](T obj) { deleteFunc_(obj, nullptr); };
	}

	VkObjWrapper(const VkObjWrapper<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deleteFunc_)
	{
		this->deleter = [&instance, deleteFunc_](T obj) { deletef(instance, obj, nullptr); };
	}

	VkObjWrapper(const VkObjWrapper<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deleteFunc_)
	{
		this->deleter = [&device, deleteFunc_](T obj) { deleteFunc_(device, obj, nullptr); };
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
	T vkObject{ VK_NULL_HANDLE };
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
