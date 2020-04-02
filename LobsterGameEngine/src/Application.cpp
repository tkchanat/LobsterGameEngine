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

#ifdef LOBSTER_BUILD_EDITOR
	ApplicationMode Application::mode = EDITOR;
#elif LOBSTER_BUILD_TEMPLATE
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
		if (m_undoSystem) delete m_undoSystem;
		m_renderer = nullptr;
		m_scene = nullptr;
		m_undoSystem = nullptr;
    }

    void Application::Initialize()
    {
		INFO("Lobster Engine is starting up...");

		//  Initialize File system
		m_fileSystem = new FileSystem();
		bool err = false;
#ifdef LOBSTER_BUILD_DEBUG
		err = m_fileSystem->assignWorkingDirectory("../../LobsterGameEngine/res");
#else
		err = m_fileSystem->createWorkingDirectory("./resources");
#endif
		m_fileSystem->update();
		if (err) throw std::runtime_error("Unable to create directory");
		LOG("Working Directory: " + m_fileSystem->GetCurrentWorkingDirectory());

		// Independent system initialization
		ThreadPool::Initialize(16);
		AudioSystem::Initialize();
		PhysicsSystem::Initialize();
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

		//GameObject* dance = new GameObject("dance");
		//dance->AddComponent(new MeshComponent(FileSystem::Path("meshes/dance.fbx").c_str()));
		//dance->transform.LocalScale *= 0.025;
		//dance->AddChild(new GameObject("child 1"));
		//dance->AddChild(new GameObject("child 2"));
		//m_scene->AddGameObject(dance);

// 		GameObject* barrel = new GameObject("barrel");
// 		barrel->AddComponent(new MeshComponent(FileSystem::Path("meshes/Barrel_01.obj").c_str(), "materials/barrel.mat"));
// 		barrel->AddComponent(new AudioSource());
// 		//barrel->AddComponent(new AABB());
// 		//barrel->AddComponent(new Rigidbody());
// 		//barrel->transform.Translate(0, 2, 0);
// 		barrel->AddChild(new GameObject("child 1"));
// 		barrel->AddChild(new GameObject("child 2"));
// 		m_scene->AddGameObject(barrel);

		//GameObject* particle = new GameObject("Particle System");
		//particle->AddComponent(new ParticleComponent());
		//m_scene->AddGameObject(particle);

		//GameObject* sphere = new GameObject("sphere");
		//sphere->AddComponent(new MeshComponent(FileSystem::Path("meshes/sphere.obj").c_str()));
		//m_scene->AddGameObject(sphere);
		//for (int i = 0; i < 5; ++i)
		//{
		//	GameObject* sphere = (new GameObject(std::to_string(i).c_str()));
		//	sphere->AddComponent(new MeshComponent(FileSystem::Path("meshes/sphere.obj").c_str(), "materials/cube.mat"));
		//	sphere->transform.WorldPosition = glm::vec3(0, 0, (i - 2.0)*2.5);
		//	m_scene->AddGameObject(sphere);
		//}

		GameObject* chicken = new GameObject("chicken");
 		chicken->AddComponent(new MeshComponent(FileSystem::Path("meshes/raw_Chicken.fbx").c_str(), "materials/chicken.mat"));

		chicken->transform.LocalScale = glm::vec3(0.42, 0.47, 0.47);
		chicken->transform.WorldPosition = glm::vec3(0, 0.33, 0);
		chicken->transform.RotateEuler(-90, glm::vec3(1, 0, 0));
		chicken->transform.OverallScale = 0.025;

		chicken->AddComponent(new Rigidbody());
		PhysicsComponent* rigitChicken = chicken->GetComponent<PhysicsComponent>();
		rigitChicken->SetEnabled(true);
		rigitChicken->SetEnabledCallback();

		BoxCollider* box = new BoxCollider(rigitChicken);
		box->SetOwner(chicken);
		box->SetOwnerTransform(&chicken->transform);
		rigitChicken->AddCollider(box);

 		m_scene->AddGameObject(chicken);

		GameObject* court = new GameObject("court");
		court->AddComponent(new MeshComponent(FileSystem::Path("meshes/court.obj").c_str(), "materials/Material.mat"));
		court->transform.LocalScale = glm::vec3(0.975, 0.794, 1);

		court->AddComponent(new Rigidbody());
		PhysicsComponent* rigitCourt = court->GetComponent<PhysicsComponent>();
		rigitCourt->SetEnabled(true);
		rigitCourt->SetEnabledCallback();

		BoxCollider* courtFloor = new BoxCollider(rigitCourt);
		courtFloor->SetOwner(court);
		courtFloor->SetOwnerTransform(&court->transform);
		courtFloor->m_transform.WorldPosition = glm::vec3(-1.4, -0.7, -4.9);
		courtFloor->m_transform.LocalScale = glm::vec3(0.725, 0.060, 0.885);
		rigitCourt->AddCollider(courtFloor);

		BoxCollider* courtBoard = new BoxCollider(rigitCourt);
		courtBoard->SetOwner(court);
		courtBoard->SetOwnerTransform(&court->transform);
		courtBoard->m_transform.WorldPosition = glm::vec3(-0.86, 10.1, 20.6);
		courtBoard->m_transform.LocalScale = glm::vec3(0.069, 0.22, 0.013);
		rigitCourt->AddCollider(courtBoard);

		BoxCollider* hoopStand = new BoxCollider(rigitCourt);
		hoopStand->SetOwner(court);
		hoopStand->SetOwnerTransform(&court->transform);
		hoopStand->m_transform.WorldPosition = glm::vec3(-0.44, 0.92, 20.505);
		hoopStand->m_transform.LocalScale = glm::vec3(0.011, 0.485, 0.013);
		rigitCourt->AddCollider(hoopStand);

		BoxCollider* hoopStandBase = new BoxCollider(rigitCourt);
		hoopStandBase->SetOwner(court);
		hoopStandBase->SetOwnerTransform(&court->transform);
		hoopStandBase->m_transform.WorldPosition = glm::vec3(-0.6, 0.44, 22.6);
		hoopStandBase->m_transform.LocalScale = glm::vec3(0.033, 0.016, 0.06);
		rigitCourt->AddCollider(hoopStandBase);

		BoxCollider* hoopA = new BoxCollider(rigitCourt);
		hoopA->SetOwner(court);
		hoopA->SetOwnerTransform(&court->transform);
		hoopA->m_transform.WorldPosition = glm::vec3(-0.4, 11.4, 18.7);
		hoopA->m_transform.LocalScale = glm::vec3(0.005, 0.005, 0.002);
		rigitCourt->AddCollider(hoopA);

		BoxCollider* hoopB = new BoxCollider(rigitCourt);
		hoopB->SetOwner(court);
		hoopB->SetOwnerTransform(&court->transform);
		hoopB->m_transform.WorldPosition = glm::vec3(-0.8, 11.4, 18.85);
		hoopB->m_transform.RotateEuler(40, glm::vec3(0, 1, 0));
		hoopB->m_transform.LocalScale = glm::vec3(0.005, 0.005, 0.002);
		rigitCourt->AddCollider(hoopB);

		BoxCollider* hoopC = new BoxCollider(rigitCourt);
		hoopC->SetOwner(court);
		hoopC->SetOwnerTransform(&court->transform);
		hoopC->m_transform.WorldPosition = glm::vec3(-1, 11.4, 19.24);
		hoopC->m_transform.RotateEuler(80, glm::vec3(0, 1, 0));
		hoopC->m_transform.LocalRotation = glm::vec3(0, 80, 0);
		hoopC->m_transform.LocalScale = glm::vec3(0.005, 0.005, 0.002);
		rigitCourt->AddCollider(hoopC);

		BoxCollider* hoopD = new BoxCollider(rigitCourt);
		hoopD->SetOwner(court);
		hoopD->SetOwnerTransform(&court->transform);
		hoopD->m_transform.WorldPosition = glm::vec3(-0.92, 11.4, 19.66);
		hoopD->m_transform.RotateEuler(115, glm::vec3(0, 1, 0));
		hoopD->m_transform.LocalScale = glm::vec3(0.005, 0.005, 0.002);
		rigitCourt->AddCollider(hoopD);

		BoxCollider* hoopE = new BoxCollider(rigitCourt);
		hoopE->SetOwner(court);
		hoopE->SetOwnerTransform(&court->transform);
		hoopE->m_transform.WorldPosition = glm::vec3(0.07, 11.4, 18.83);
		hoopE->m_transform.RotateEuler(-37, glm::vec3(0, 1, 0));
		hoopE->m_transform.LocalScale = glm::vec3(0.005, 0.005, 0.002);
		rigitCourt->AddCollider(hoopE);

		BoxCollider* hoopF = new BoxCollider(rigitCourt);
		hoopF->SetOwner(court);
		hoopF->SetOwnerTransform(&court->transform);
		hoopF->m_transform.WorldPosition = glm::vec3(0.3, 11.4, 19.18);
		hoopF->m_transform.RotateEuler(-73, glm::vec3(0, 1, 0));
		hoopF->m_transform.LocalScale = glm::vec3(0.005, 0.005, 0.002);
		rigitCourt->AddCollider(hoopF);

		BoxCollider* hoopG = new BoxCollider(rigitCourt);
		hoopG->SetOwner(court);
		hoopG->SetOwnerTransform(&court->transform);
		hoopG->m_transform.WorldPosition = glm::vec3(0.24, 11.4, 19.7);
		hoopG->m_transform.RotateEuler(-115, glm::vec3(0, 1, 0));
		hoopG->m_transform.LocalScale = glm::vec3(0.005, 0.005, 0.002);
		rigitCourt->AddCollider(hoopG);

		m_scene->AddGameObject(court);

		GameObject* camera = new GameObject("Main Camera");
		CameraComponent* comp = new CameraComponent();
		camera->AddComponent(comp);
		camera->AddComponent(new AudioListener());
		camera->transform.Translate(0, 1, -2.5);
		camera->transform.RotateEuler(-175, glm::vec3(1, 0, 0));
		camera->transform.RotateEuler(180, glm::vec3(0, 0, 1));
		camera->GetComponent<CameraComponent>()->SetFar(100);
		m_scene->AddGameObject(camera);
		m_scene->SetGameCamera(comp);

		GameObject* light = new GameObject("Directional Light");
		light->AddComponent(new LightComponent(LightType::DIRECTIONAL_LIGHT));
		light->transform.Translate(0, 2, 3);
		m_scene->AddGameObject(light);

        //GameObject* sibenik = (new GameObject("sibenik"))->AddComponent<MeshComponent>(m_fileSystem->Path("meshes/sibenik.obj").c_str(), "materials/sibenik.mat");
		LOG("Model loading spent {} ms", loadTimer.GetElapsedTime());

		// Push layers to layer stack
#ifdef LOBSTER_BUILD_EDITOR
		m_GUILayer = new GUILayer();
		m_editorLayer = new EditorLayer();
#endif
    }

	// Updates subsystem chronologically in a fixed timestep, i.e. order does matter
	// * Networking update
	// * Scene fixed update
	// * Physics update
	void Application::FixedUpdate(double deltaTime)
	{ 
		// application info (config) update
		int w, h;
		Input::GetWindowSize(&w, &h);
		Application::GetInstance()->GetConfig().width = w;
		Application::GetInstance()->GetConfig().height = h;
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
		m_renderer->ClearOverlayQueue();
		//=========================================================
		// GUI Renderer update
		#ifdef LOBSTER_BUILD_EDITOR
		Timer imguiRenderTimer;
		ImGui::GetIO().DeltaTime = deltaTime;
		m_editorLayer->OnUpdate(deltaTime);
		m_renderer->Render(m_editorLayer->GetSceneCamera(), true);
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

#ifdef LOBSTER_BUILD_EDITOR
			// Display FPS in the window title
			logTime += deltaTime;
			if (logTime > 1000.0f)
			{
				std::string sceneName = (scenePath.empty() ? "New Project" : fs::path(scenePath).filename().string()) + (m_saved ? "" : "*") + " - ";
				std::string newTitle = sceneName + m_window->GetTitle() + " (FPS: " + std::to_string(frames) + ")";
				m_window->SetTitle(newTitle.c_str());
				logTime -= 1000.0f;
				frames = 0;
			}
#endif

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
		if (scenePath && scenePath[0] != '\0') {
			std::string path(scenePath);
			StringOps::ReplaceAll(path, "\\", "/");
			this->scenePath = scenePath;
			m_scene = new Scene(this->scenePath.c_str());
		}
		else {
			m_scene = new Scene();			
		}		
		Application::GetInstance()->SetScenePath(scenePath);
		SetSaved(true);
	}

	void Application::SetScenePath(const char* scenePath) {
		this->scenePath = scenePath;
	}

	void Application::SetSaved(bool saved) {
		m_saved = saved;
	}

}
