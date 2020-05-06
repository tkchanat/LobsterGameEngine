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

		GameObject* particle = new GameObject("particle");
		particle->AddComponent(new ParticleComponent());

		particle->AddComponent(new Rigidbody());
		PhysicsComponent* rigitParticle = particle->GetComponent<PhysicsComponent>();
		rigitParticle->SetEnabled(true);
		rigitParticle->SetEnabledCallback();
		particle->transform.WorldPosition = glm::vec3(-0.28, 9.3, 19.35);


		BoxCollider* goal = new BoxCollider(rigitParticle);
		goal->SetOwner(particle);
		goal->SetOwnerTransform(&particle->transform);
		goal->m_transform.WorldPosition = glm::vec3(0, -0.3, 0);
		goal->m_transform.LocalScale = glm::vec3(0.3, 0.01, 0.3);
		rigitParticle->AddCollider(goal);

		m_scene->AddGameObject(particle);

		GameObject* chicken = new GameObject("chicken");
 		chicken->AddComponent(new MeshComponent(FileSystem::Path("meshes/anim_chicken.fbx").c_str(), "materials/chicken.mat"));

		chicken->transform.LocalScale = glm::vec3(0.42, 0.47, 0.47);
		chicken->transform.WorldPosition = glm::vec3(-0.298, 0.465, 0);
		chicken->transform.RotateEuler(180, glm::vec3(0, 1, 0));
		chicken->transform.OverallScale = 0.3;


		chicken->AddComponent(new Rigidbody());

		PhysicsComponent* rigitChicken = chicken->GetComponent<PhysicsComponent>();
		rigitChicken->SetEnabled(true);
		rigitChicken->SetEnabledCallback();

		AudioClip* chickenAudioClip = new AudioClip("Chicken.wav");
		AudioSource* chickenScream = new AudioSource();

		chicken->AddComponent(chickenScream);
		chickenScream->SetSource(chickenAudioClip);
		


		BoxCollider* box = new BoxCollider(rigitChicken);
		box->SetOwner(chicken);
		box->SetOwnerTransform(&chicken->transform);
		rigitChicken->AddCollider(box);

 		m_scene->AddGameObject(chicken);

		GameObject* court = new GameObject("court");
		court->AddComponent(new MeshComponent(FileSystem::Path("meshes/court.obj").c_str(), nullptr));
		court->transform.LocalScale = glm::vec3(0.975, 0.794, 1);

		court->AddComponent(new Rigidbody());
		PhysicsComponent* rigitCourt = court->GetComponent<PhysicsComponent>();
		rigitCourt->SetEnabled(true);
		rigitCourt->SetEnabledCallback();

		BoxCollider* courtBoard = new BoxCollider(rigitCourt);
		courtBoard->SetOwner(court);
		courtBoard->SetOwnerTransform(&court->transform);
		courtBoard->m_transform.WorldPosition = glm::vec3(-0.86, 10.1, 20.6);
		courtBoard->m_transform.LocalScale = glm::vec3(0.069, 0.22, 0.013);
		rigitCourt->AddCollider(courtBoard);

		BoxCollider* hoopStand = new BoxCollider(rigitCourt);
		hoopStand->SetOwner(court);
		hoopStand->SetOwnerTransform(&court->transform);
		hoopStand->m_transform.WorldPosition = glm::vec3(-0.44, 0.92, 20.606);
		hoopStand->m_transform.LocalScale = glm::vec3(0.011, 0.485, 0.013);
		rigitCourt->AddCollider(hoopStand);

		m_scene->AddGameObject(court);

		GameObject* hoop = new GameObject("hoop");
		hoop->transform.LocalScale = glm::vec3(0.97, 0.794, 1);
		hoop->transform.WorldPosition = glm::vec3(-0.298, 9.25, 18.7);
		hoop->transform.OverallScale = 1.5;

		hoop->AddComponent(new MeshComponent(FileSystem::Path("meshes/torus.obj").c_str(), "materials/Material004.mat"));

		hoop->AddComponent(new Rigidbody());
		PhysicsComponent* rigitHoop = hoop->GetComponent<PhysicsComponent>();
		rigitHoop->SetEnabled(true);
		rigitHoop->SetEnabledCallback();

		BoxCollider* hoopCollider = new BoxCollider(rigitHoop);
		BoxCollider* hoopA = new BoxCollider(rigitHoop);
		hoopA->SetOwner(hoop);
		hoopA->SetOwnerTransform(&hoop->transform);
		hoopA->m_transform.WorldPosition = glm::vec3(0.8, 0.0, 0.0);
		hoopA->m_transform.LocalScale = glm::vec3(0.05, 0.3, 0.25);
		rigitHoop->AddCollider(hoopA);

		BoxCollider* hoopB = new BoxCollider(rigitHoop);
		hoopB->SetOwner(hoop);
		hoopB->SetOwnerTransform(&hoop->transform);
		hoopB->m_transform.WorldPosition = glm::vec3(-0.8, 0, 0);
		hoopB->m_transform.LocalScale = glm::vec3(0.05, 0.3, 0.25);
		rigitHoop->AddCollider(hoopB);

		BoxCollider* hoopC = new BoxCollider(rigitHoop);
		hoopC->SetOwner(hoop);
		hoopC->SetOwnerTransform(&hoop->transform);
		hoopC->m_transform.WorldPosition = glm::vec3(-0.5, 0, -0.6);
		hoopC->m_transform.RotateEuler(134, glm::vec3(0, 1, 0));
		hoopC->m_transform.LocalScale = glm::vec3(0.05, 0.3, 0.25);
		rigitHoop->AddCollider(hoopC);

		BoxCollider* hoopD = new BoxCollider(rigitHoop);
		hoopD->SetOwner(hoop);
		hoopD->SetOwnerTransform(&hoop->transform);
		hoopD->m_transform.WorldPosition = glm::vec3(0, 0, -0.8);
		hoopD->m_transform.RotateEuler(90, glm::vec3(0, 1, 0));
		hoopD->m_transform.LocalScale = glm::vec3(0.05, 0.3, 0.25);
		rigitHoop->AddCollider(hoopD);

		BoxCollider* hoopE = new BoxCollider(rigitHoop);
		hoopE->SetOwner(hoop);
		hoopE->SetOwnerTransform(&hoop->transform);
		hoopE->m_transform.WorldPosition = glm::vec3(0.55, 0, -0.6);
		hoopE->m_transform.RotateEuler(47, glm::vec3(0, 1, 0));
		hoopE->m_transform.LocalScale = glm::vec3(0.05, 0.3, 0.25);
		rigitHoop->AddCollider(hoopE);

		BoxCollider* hoopF = new BoxCollider(rigitHoop);
		hoopF->SetOwner(hoop);
		hoopF->SetOwnerTransform(&hoop->transform);
		hoopF->m_transform.WorldPosition = glm::vec3(-0.6, 0, 0.55);
		hoopF->m_transform.RotateEuler(45, glm::vec3(0, 1, 0));
		hoopF->m_transform.LocalScale = glm::vec3(0.05, 0.3, 0.25);
		rigitHoop->AddCollider(hoopF);

		BoxCollider* hoopG = new BoxCollider(rigitHoop);
		hoopG->SetOwner(hoop);
		hoopG->SetOwnerTransform(&hoop->transform);
		hoopG->m_transform.WorldPosition = glm::vec3(0.55, 0, 0.6);
		hoopG->m_transform.RotateEuler(-42, glm::vec3(0, 1, 0));
		hoopG->m_transform.LocalScale = glm::vec3(0.05, 0.3, 0.25);
		rigitHoop->AddCollider(hoopG);

		m_scene->AddGameObject(hoop);

		GameObject* floor = new GameObject("floor");
		floor->transform.LocalScale = glm::vec3(0.975, 0.794, 1);

		floor->AddComponent(new Rigidbody());
		PhysicsComponent* rigitFloor = floor->GetComponent<PhysicsComponent>();
		rigitFloor->SetEnabled(true);
		rigitFloor->SetEnabledCallback();

		BoxCollider* courtFloor = new BoxCollider(rigitFloor);
		courtFloor->SetOwner(floor);
		courtFloor->SetOwnerTransform(&floor->transform);
		courtFloor->m_transform.WorldPosition = glm::vec3(-1.4, -0.98, -4.9);
		courtFloor->m_transform.LocalScale = glm::vec3(80, 1.35, 80);
		rigitFloor->AddCollider(courtFloor);


		BoxCollider* hoopStandBase = new BoxCollider(rigitFloor);
		hoopStandBase->SetOwner(floor);
		hoopStandBase->SetOwnerTransform(&floor->transform);
		hoopStandBase->m_transform.WorldPosition = glm::vec3(-0.43, 0.59, 22.6);
		hoopStandBase->m_transform.LocalScale = glm::vec3(1.185, 0.166, 2.66);
		rigitFloor->AddCollider(hoopStandBase);

		m_scene->AddGameObject(floor);


		GameObject* camera = new GameObject("Main Camera");
		CameraComponent* comp = new CameraComponent();
		camera->AddComponent(comp);
		camera->AddComponent(new AudioListener());
		camera->transform.Translate(0, 2.143, -6.603);
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
		LightLibrary::Update();
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
