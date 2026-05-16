#include <core/manager/ressourceManager.h>
#include <factory/meshFactory.h>
#include <graphics/resources/object/mesh.h>
#include <factory/materialFactory.h>
#include <graphics/resources/material.h>

#include <vraktal.h>
#include <graphics/renderer.h>
#include <core/gpu/device.h>

RessourceManager::RessourceManager(core::gpu::Device& _device, graphics::Renderer& _renderer) : m_device(_device)
{
	RegisterRessourceType<graphics::resources::Mesh>(m_device);
    RegisterRessourceType<graphics::resources::Material>(m_device, _renderer);
}

RessourceManager::~RessourceManager()
{
}