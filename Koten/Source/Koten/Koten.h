#pragma once

// CORE

#include "Koten/Core/Base.h"
#include "Koten/Core/Engine.h"
#include "Koten/Core/Definitions.h"
#include "Koten/Core/Application.h"
#include "Koten/Core/Log.h"
#include "Koten/Core/Assert.h"
#include "Koten/Core/Layer.h"
#include "Koten/Core/LayerStack.h"
#include "Koten/Core/FileSystem.h"
#include "Koten/Core/Time.h"
#include "Koten/Core/UUID.h"
#include "Koten/Core/Buffer.h"

// ASSET

#include "Koten/Asset/Asset.h"
#include "Koten/Asset/AssetImporter.h"
#include "Koten/Asset/AssetManager.h"
#include "Koten/Asset/TextureImporter.h"
#include "Koten/Asset/SceneImporter.h"
#include "Koten/Asset/PhysicsMaterial2DImporter.h"

// EVENTS

#include "Koten/Events/Event.h"
#include "Koten/Events/ApplicationEvent.h"
#include "Koten/Events/WindowEvent.h"
#include "Koten/Events/KeyEvent.h"
#include "Koten/Events/MouseEvent.h"
#include "Koten/Events/JoystickEvent.h"

// GRAPHICS

#include "Koten/Graphics/CommandBuffer.h"
#include "Koten/Graphics/GraphicsContext.h"
#include "Koten/Graphics/Shader.h"
#include "Koten/Graphics/Buffer.h"
#include "Koten/Graphics/VertexArray.h"
#include "Koten/Graphics/RendererAPI.h"
#include "Koten/Graphics/RendererCommand.h"
#include "Koten/Graphics/Texture.h"
#include "Koten/Graphics/UniformBuffer.h"
#include "Koten/Graphics/DescriptorSet.h"
#include "Koten/Graphics/Camera.h"
#include "Koten/Graphics/Framebuffer.h"
#include "Koten/Graphics/Renderpass.h"
#include "Koten/Graphics/Pipeline.h"
#include "Koten/Graphics/Renderer.h"
#include "Koten/Graphics/StorageBuffer.h"
#include "Koten/Graphics/IndirectBuffer.h"
#include "Koten/Graphics/DFFont.h"

// DEBUG

#include "Koten/Debug/Profile.h"

// IMGUI

#include "Koten/ImGui/IconsMaterialDesignIcons.h"
#include "Koten/ImGui/ImGuiLayer.h"
#include "Koten/ImGui/Utils/UI.h"

// OS

#include "Koten/OS/FileDialog.h"
#include "Koten/OS/KeyCodes.h"
#include "Koten/OS/MouseCodes.h"
#include "Koten/OS/Input.h"
#include "Koten/OS/Window.h"

// PHYSICS

#include "Koten/Physics/Box2D/B2Physics.h"
#include "Koten/Physics/PhysicsMaterial2D.h"

// PROJECT

#include "Koten/Project/Project.h"
#include "Koten/Project/ProjectSerializer.h"

// SCRIPT

#include "Koten/Script/ScriptEngine.h"
#include "Koten/Script/ScriptGlue.h"

// SCENE

#include "Koten/Scene/Components.h"
#include "Koten/Scene/Entity.h"
#include "Koten/Scene/Scene.h"
#include "Koten/Scene/SceneGraph.h"
#include "Koten/Scene/SceneSerializer.h"
#include "Koten/Scene/System.h"
#include "Koten/Scene/SystemManager.h"
#include "Koten/Scene/SceneManager.h"

// UTILS

#include "Koten/Utils/HashCombiner.h"
#include "Koten/Utils/Utils.h"
#include "Koten/Utils/StringUtils.h"
#include "Koten/Utils/IniFile.h"