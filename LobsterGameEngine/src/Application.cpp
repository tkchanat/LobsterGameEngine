#include "pch.h"
#include "Application.h"

#include "system/Input.h"

//  Placeholder
#include "audio/AudioSystem.h"
#include "components/AudioComponent.h"
#include "components/ComponentCollection.h"
#include "events/EventDispatcher.h"
#include "events/EventQueue.h"
#include "graphics/meshes/MeshLoader.h"
#include "graphics/Renderer.h"
#include "graphics/Scene.h"
#include "layer/EditorLayer.h"
#include "layer/GUILayer.h"
#include "objects/GameObject.h"
#include "system/FileSystem.h"
#include "system/UndoSystem.h"

namespace Lobster
{
    
    Application* Application::m_instance = nullptr;	

#ifdef LOBSTER_BUILD_DEBUG
	ApplicationMode Application::mode = EDITOR;
#elif LOBSTER_BUILD_RELEASE
	ApplicationMode Application::mode = GAME;
#endif	

    Application::Application() : 
		m_scene(nullptr),
		m_renderer(nullptr)
    {
        //  Don't touch here, please do all initialization work in Initialize() function
        if(m_instance)
        {
            LOG("Application already created! Don't create another one, you stupid.");
            return;
        }
        m_instance = this;
    }
    
    Application::~Application()
    {
		if (m_renderer) delete m_renderer;
		if (m_scene) delete m_scene;
		m_renderer = nullptr;
		m_scene = nullptr;
    }

    void Application::Initialize()
    {
		INFO("Lobster Engine is starting up...");

		//  Initialize File system
		m_fileSystem = new FileSystem();
		bool err = false;
#ifdef LOBSTER_BUILD_DEBUG
		err = m_fileSystem->assignWorkingDirectory("../../LobsterGameEngine/res");
#elif LOBSTER_BUILD_RELEASE
		err = m_fileSystem->createWorkingDirectory("./resources");
#endif
		m_fileSystem->update();
		if (err) throw std::runtime_error("Unable to create directory");
		LOG("Working Directory: " + m_fileSystem->GetCurrentWorkingDirectory());

		// Read from JSON
        struct {
            int width = 1280;
            int height = 760;
            std::string title = "Lobster Engine";
            bool vsync = true;
        } config;

		// Independent system initialization
		ThreadPool::Initialize(16);
		AudioSystem::Initialize();
		Profiler::Initialize();
		EventDispatcher::Initialize();
		EventQueue::Initialize();
		Input::Initialize();

		// OpenGL dependent system initialization (Window class create OpenGL context)
		m_window = new Window(config.width, config.height, config.title, config.vsync);
		TextureLibrary::Initialize();
		ShaderLibrary::Initialize();
		MaterialLibrary::Initialize();
		LightLibrary::Initialize();
		
        //  Initialize Renderer
        m_renderer = new Renderer();
        
		//	Initialize UndoSystem with length = 50
		m_undoSystem = new UndoSystem(50);

        //  Initialize GameObjects
		Timer loadTimer;
		OpenScene("");

//		ThreadPool::Enqueue([]() {
//			// This job should be running in a separate thread without blocking the main thread
//			Sleep(10000); // sleep for 10 seconds
//			LOG("ThreadPool is working! :D");
//		});


		GameObject* barrel = new GameObject("barrel");
		barrel->AddComponent(new MeshComponent(FileSystem::Path("meshes/Barrel_01.obj").c_str(), "materials/barrel.mat"));
		barrel->AddComponent(new AudioSource());
		//barrel->AddComponent(new AABB());
		//barrel->AddComponent(new Rigidbody());
		//barrel->transform.Translate(0, 2, 0);
		barrel->AddChild(new GameObject("child 1"));
		barrel->AddChild(new GameObject("child 2"));
		m_scene->AddGameObject(barrel);

		//for (int i = 0; i < 5; ++i)
		//{
		//	GameObject* cube = (new GameObject(std::to_string(i).c_str()))
		//		->AddComponent<MeshComponent>(FileSystem::Path("meshes/sphere.obj").c_str(), "materials/cube.mat");
		//	cube->transform.WorldPosition = glm::vec3(0, 0, (i - 2.0)*2.5);
		//	m_scene->AddGameObject(cube);
		//}

		GameObject* camera = new GameObject("Main Camera");
		camera->AddComponent(new CameraComponent());
		camera->AddComponent(new AudioListener());
		camera->transform.Translate(0, 2, 10);
		m_scene->AddGameObject(camera);

		GameObject* light = new GameObject("Directional Light");
		light->AddComponent(new LightComponent(LightType::DIRECTIONAL_LIGHT));
		//light->transform.Translate(0, 2, 3);
		m_scene->AddGameObject(light);

        //GameObject* sibenik = (new GameObject("sibenik"))->AddComponent<MeshComponent>(m_fileSystem->Path("meshes/sibenik.obj").c_str(), "materials/sibenik.mat");
		LOG("Model loading spent {} ms", loadTimer.GetElapsedTime());

		// Push layers to layer stack
#ifdef LOBSTER_BUILD_DEBUG
		m_GUILayer = new GUILayer();
		m_editorLayer = new EditorLayer();
#endif
		EventDispatcher::AddCallback(EVENT_KEY_PRESSED, new EventCallback<KeyPressedEvent>([this](KeyPressedEvent* e) {
			if (e->Key == GLFW_KEY_O)
			{
				OpenScene("scenes/test.lobster");
			}
		}));
    }

	// Updates subsystem chronologically in a fixed timestep, i.e. order does matter
	// * Networking update
	// * Scene fixed update
	// * Physics update
	void Application::FixedUpdate(double deltaTime)
	{ 
		//=========================================================
		// Networking update

		//=========================================================
		// Scene fixed update

		//=========================================================
		// Physics update, not running in editor mode
		if (mode != EDITOR) {
			Timer physicsTimer;
			m_scene->OnPhysicsUpdate(deltaTime);
			Profiler::SubmitData("Physics Update Time", physicsTimer.GetElapsedTime());
		}		
	}

	// Updates subsystem chronologically as much as possible, i.e. order does matter
	// * Input update
	// * Event update
	// * Scene update
	// * Scene late update
	// * Audio update
	// * Renderer update
	// * GUI Renderer update
	// * Window update
	// * Memory update
	void Application::VariableUpdate(double deltaTime)
	{
		//=========================================================
		// Input update
		Input::Update();

		//=========================================================
		// Event update
		Event* event = EventQueue::GetInstance()->Next();
		if (event)
		{
			EventDispatcher::Dispatch(event);
		}

		//=========================================================
		// Scene update
		Timer sceneUpdateTimer;
		m_scene->OnUpdate(deltaTime);	// update game scene
		RenderOverlayCommand ocommand;
		ocommand.UseTexture = TextureLibrary::Use("textures/ui/light.png");
		ocommand.x = 200.0f;
		ocommand.y = 300.0f;
		ocommand.w = 64.0f;
		ocommand.h = 64.0f;
		ocommand.z = 1;
		Renderer::Submit(ocommand);
		Profiler::SubmitData("Scene Update Time", sceneUpdateTimer.GetElapsedTime());

		//=========================================================
		// Scene late update

		//=========================================================
		// Audio update

		//=========================================================
		// Renderer update
		Timer renderTimer;
		m_renderer->Render(CameraComponent::GetActiveCamera());
		Profiler::SubmitData("Render Time", renderTimer.GetElapsedTime());

		//=========================================================
		// GUI Renderer update
		#ifdef LOBSTER_BUILD_DEBUG
		Timer imguiRenderTimer;
		ImGui::GetIO().DeltaTime = deltaTime;
		m_editorLayer->OnUpdate(deltaTime);
		m_renderer->Render(m_editorLayer->GetSceneCamera());
		m_GUILayer->Begin();
		m_editorLayer->OnImGuiRender();
		m_GUILayer->End();
		Profiler::SubmitData("ImGui Render Time", imguiRenderTimer.GetElapsedTime());
		#endif
		m_renderer->ClearAllQueues();

		//=========================================================
		// Window update

		//=========================================================
		// Memory update
		
		m_window->Swap(); // finish the frame and present it
	}

	void Application::Run()
	{
		Timer timer;
		double logTime = 0.0;
		int frames = 0;
		double intervalTime = 1.0 / (double)m_targetFPS;
		double accumulateTime = 0.0;

		//	Loop until the user closes the window 
		while (m_window->IsRunning())
		{
			Timer frameTime;
			//	Get the time difference of executing one game loop
			double deltaTime = timer.GetDeltaTime();

			//	===Display FPS in the window title===
			logTime += deltaTime;
			if (logTime > 1000.0f)
			{
				std::string newTitle = m_window->GetTitle() + " (FPS: " + std::to_string(frames) + ")";
				m_window->SetTitle(newTitle.c_str());
				logTime -= 1000.0f;
				frames = 0;
			}
			//  ===========TO BE REMOVED=============

			accumulateTime += deltaTime;
			for (int i = 0; i < m_maxFixedUpdates && accumulateTime > intervalTime; ++i)
			{
				FixedUpdate(deltaTime);
				accumulateTime -= intervalTime;
			}
			VariableUpdate(deltaTime);

			frames++;
			Profiler::SubmitData("Frame Time", frameTime.GetElapsedTime());
		}
	}

	void Application::SwitchMode(ApplicationMode mode) {
		m_instance->mode = mode;
		// TODO switch tab
		if (mode == EDITOR) {

		}
		else if (mode == SIMULATION) {

		}
	}

	void Application::Shutdown()
	{
	}

	void Application::OpenScene(const char* scenePath)
	{
		if (m_scene) {
			delete m_scene;
            EditorLayer::s_selectedGameObject = nullptr;
		}
		m_scene = new Scene(scenePath);
	}

}
