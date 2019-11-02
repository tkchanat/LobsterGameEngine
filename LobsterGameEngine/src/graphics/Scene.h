#pragma once

namespace Lobster
{
    
    class GameObject;
    class CameraComponent;
	class TextureCube;

	//	This class encapsulates all the game objects and elements in the game world.
	//	A scene currently only allows one main camera, and a list of game objects.
	//	Ideally, game objects should form a hierarchy / tree. That means game objects can have children and parents, but the scene act as the root node of all.
    class Scene
    {
    private:
        CameraComponent* m_activeCamera; // non-removable main camera
		TextureCube* m_skybox;
        std::vector<GameObject*> m_gameObjects;
    public:
        Scene();
        ~Scene();
        void OnUpdate(double deltaTime);
		void OnPhysicsUpdate(double deltaTime);
        Scene* AddGameObject(GameObject* gameObject);
		Scene* RemoveGameObject(std::string name);
		Scene* RemoveGameObject(GameObject* gameObject);
		bool IsObjectNameDuplicated(std::string name, std::string except = "");
        inline const std::vector<GameObject*>& GetGameObjects() { return m_gameObjects; }
		inline void SetActiveCamera(CameraComponent* camera) { m_activeCamera = camera; }
        inline CameraComponent* GetActiveCamera() const { return m_activeCamera; }
    };
    
}
