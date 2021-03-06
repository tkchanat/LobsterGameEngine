#include "pch.h"
#include "system/Command.h"
#include "graphics/Scene.h"
#include "layer/EditorLayer.h"
#include "objects/GameObject.h"

namespace Lobster {
	TransformCommand::TransformCommand(GameObject* object, Transform t_original, Transform t_new) :
		m_object(object),
		m_original(t_original),
		m_new(t_new)
	{

	}

	//	Executing a transformation command is to change the matrix to the new one.
	void TransformCommand::Exec() {
		m_object->transform = m_new;
	}

	//	By performing undo on a transformation, we are essentially changing the transforming matrix back to the original one.
	void TransformCommand::Undo() {
		m_object->transform = m_original;
	}

	std::string TransformCommand::ToString() const {
		//	act stores the action, vect stores the vector details.
		std::string act, vect;
		if (m_original.WorldPosition != m_new.WorldPosition) {
			act = "Translated ";
			vect = StringOps::ToString(m_new.WorldPosition);
		} else if (m_original.LocalEulerAngles != m_new.LocalEulerAngles) {
			act = "Rotated ";
			vect = StringOps::ToString(m_new.LocalEulerAngles);
		} else if (m_original.LocalScale != m_new.LocalScale) {
			act = "Scaled ";
			vect = StringOps::ToString(m_new.LocalScale);
		} else {
			act = "Set overall scale of ";
			vect = std::to_string(m_new.OverallScale);
		}
		
		return act + m_object->GetName() + " to " + vect;
	}

	TransformColliderCommand::TransformColliderCommand(Collider* collider, Transform t_original, Transform t_new) :
		m_collider(collider),
		m_original(t_original),
		m_new(t_new)
	{

	}

	//	Executing a transformation command is to change the matrix to the new one.
	void TransformColliderCommand::Exec() {
		m_collider->m_transform = m_new;
	}

	//	By performing undo on a transformation, we are essentially changing the transforming matrix back to the original one.
	void TransformColliderCommand::Undo() {
		m_collider->m_transform = m_original;
	}

	std::string TransformColliderCommand::ToString() const {
		//	act stores the action, vect stores the vector details.
		std::string act, vect;
		if (m_original.WorldPosition != m_new.WorldPosition) {
			act = "Translated ";
			vect = StringOps::ToString(m_new.WorldPosition);
		} else if (m_original.LocalEulerAngles != m_new.LocalEulerAngles) {
			act = "Rotated ";
			vect = StringOps::ToString(m_new.LocalEulerAngles);
		} else {
			act = "Scaled ";
			vect = StringOps::ToString(m_new.LocalScale);
		}

		return act + " a collider in " +  m_collider->GetPhysics()->GetOwner()->GetName() + " by " + vect;
	}

	DestroyObjectCommand::DestroyObjectCommand(GameObject* object, Scene* scene) :
		m_object(object),
		m_scene(scene),
		b_isDeleted(true)
	{

	}

	DestroyObjectCommand::~DestroyObjectCommand() {
		if (b_isDeleted) delete m_object;
		m_object = nullptr;
		m_scene = nullptr;
	}

	//	Executing a destroy command is to soft delete an object.
	void DestroyObjectCommand::Exec() {
		m_scene->RemoveGameObject(m_object);
		b_isDeleted = true;
		m_object->ToggleVirtualDelete();

		if (EditorLayer::s_selectedGameObject == m_object) EditorLayer::s_selectedGameObject = nullptr;
	}

	//	By performing an undo on destroy command, the object should come back to live.
	void DestroyObjectCommand::Undo() {
		m_scene->AddGameObject(m_object);
		b_isDeleted = false;
		m_object->ToggleVirtualDelete();
	}

	std::string DestroyObjectCommand::ToString() const {
		return "Destroyed " + m_object->GetName();
	}

	CreateObjectCommand::CreateObjectCommand(GameObject* object, Scene* scene) :
		m_object(object),
		m_scene(scene),
		b_isDeleted(false)
	{

	}

	CreateObjectCommand::~CreateObjectCommand() {
		if (b_isDeleted) delete m_object;
		m_object = nullptr;
		m_scene = nullptr;
	}

	//	Executing a create command is to bring an object to live.
	void CreateObjectCommand::Exec() {
		m_scene->AddGameObject(m_object);
		b_isDeleted = false;
		m_object->ToggleVirtualDelete();
	}

	//	By performing an undo on create command, the object should be deleted.
	void CreateObjectCommand::Undo() {
		m_scene->RemoveGameObject(m_object);
		b_isDeleted = true;
		m_object->ToggleVirtualDelete();

		if (EditorLayer::s_selectedGameObject == m_object) EditorLayer::s_selectedGameObject = nullptr;
	}

	std::string CreateObjectCommand::ToString() const {
		return "Created " + m_object->GetName();
	}

	DestroyChildCommand::DestroyChildCommand(GameObject* object, GameObject* parent) :
		m_object(object),
		m_parent(parent),
		b_isDeleted(true)
	{

	}

	DestroyChildCommand::~DestroyChildCommand() {
		if (b_isDeleted) delete m_object;
		m_object = nullptr;
		m_parent = nullptr;
	}

	//	Executing a destroy command is to soft delete an object.
	void DestroyChildCommand::Exec() {
		m_parent->RemoveChild(m_object);
		b_isDeleted = true;
		m_object->ToggleVirtualDelete();
	}

	//	By performing an undo on destroy command, the object should come back to live.
	void DestroyChildCommand::Undo() {
		m_parent->AddChild(m_object);
		b_isDeleted = false;
		m_object->ToggleVirtualDelete();
	}

	std::string DestroyChildCommand::ToString() const {
		return "Destroyed " + m_object->GetName() + " inside " + m_parent->GetName();
	}

	CreateChildCommand::CreateChildCommand(GameObject* object, GameObject* parent) :
		m_object(object),
		m_parent(parent),
		b_isDeleted(false)
	{

	}

	CreateChildCommand::~CreateChildCommand() {
		if (b_isDeleted) delete m_object;
		m_object = nullptr;
		m_parent = nullptr;
	}

	//	Executing a create command is to bring an object to live.
	void CreateChildCommand::Exec() {
		m_parent->AddChild(m_object);
		b_isDeleted = false;
		m_object->ToggleVirtualDelete();
	}

	//	By performing an undo on create command, the object should be deleted.
	void CreateChildCommand::Undo() {
		m_parent->RemoveChild(m_object);
		b_isDeleted = true;
		m_object->ToggleVirtualDelete();
	}

	std::string CreateChildCommand::ToString() const {
		return "Created " + m_object->GetName() + " inside " + m_parent->GetName();
	}

	DestroyComponentCommand::DestroyComponentCommand(Component* component, GameObject* object) :
		m_component(component),
		m_object(object),
		b_isDeleted(true)
	{

	}

	DestroyComponentCommand::~DestroyComponentCommand() {
		if (b_isDeleted) delete m_component;
		m_component = nullptr;
		m_object = nullptr;
	}

	//	Executing a destroy command is to soft delete a component.
	void DestroyComponentCommand::Exec() {
		m_object->RemoveComponent(m_component);
		b_isDeleted = true;
		m_component->VirtualDelete();
	}

	//	By performing an undo on destroy command, the component should come back to live.
	void DestroyComponentCommand::Undo() {
		m_object->AddComponent(m_component);
		b_isDeleted = false;
		m_component->VirtualCreate();
	}

	std::string DestroyComponentCommand::ToString() const {
		return "Destroyed " + m_component->GetTypeName() + " in " + m_object->GetName();
	}

	CreateComponentCommand::CreateComponentCommand(Component* component, GameObject* object) :
		m_component(component),
		m_object(object),
		b_isDeleted(false)
	{

	}

	CreateComponentCommand::~CreateComponentCommand() {
		if (b_isDeleted) delete m_component;
		m_component = nullptr;
		m_object = nullptr;
	}

	//	Executing a create command is to bring a component to live.
	void CreateComponentCommand::Exec() {
		m_object->AddComponent(m_component);
		b_isDeleted = false;
		m_component->VirtualCreate();
	}

	//	By performing an undo on create command, the component should be deleted.
	void CreateComponentCommand::Undo() {
		m_object->RemoveComponent(m_component);
		b_isDeleted = true;
		m_component->VirtualDelete();
	}

	std::string CreateComponentCommand::ToString() const {
		return "Created " + m_component->GetTypeName() + " in " + m_object->GetName();
	}

	DestroyColliderCommand::DestroyColliderCommand(Collider* collider, PhysicsComponent* component) :
		m_collider(collider),
		m_component(component),
		b_isDeleted(true)
	{

	}

	DestroyColliderCommand::~DestroyColliderCommand() {
		if (b_isDeleted) delete m_collider;
		m_collider = nullptr;
		m_component = nullptr;
	}

	//	Executing a destroy command is to soft delete a collider.
	void DestroyColliderCommand::Exec() {
		m_component->RemoveCollider(m_collider);
		b_isDeleted = true;
		m_collider->VirtualDelete();
	}

	//	By performing an undo on destroy command, the collider should come back to live.
	void DestroyColliderCommand::Undo() {
		m_component->AddCollider(m_collider);
		b_isDeleted = false;
		m_collider->VirtualCreate();
	}

	std::string DestroyColliderCommand::ToString() const {
		return "Destroyed a collider in " + m_component->GetOwner()->GetName();
	}

	CreateColliderCommand::CreateColliderCommand(Collider* collider, PhysicsComponent* component) :
		m_collider(collider),
		m_component(component),
		b_isDeleted(false)
	{

	}

	CreateColliderCommand::~CreateColliderCommand() {
		if (b_isDeleted) delete m_collider;
		m_collider = nullptr;
		m_component = nullptr;
	}

	//	Executing a create command is to bring a collider to live.
	void CreateColliderCommand::Exec() {
		m_component->AddCollider(m_collider);
		b_isDeleted = false;
		m_collider->VirtualCreate();
	}

	//	By performing an undo on create command, the collider should be deleted.
	void CreateColliderCommand::Undo() {
		m_component->RemoveCollider(m_collider);
		b_isDeleted = true;
		m_collider->VirtualDelete();
	}

	std::string CreateColliderCommand::ToString() const {
		return "Created a collider in " + m_component->GetOwner()->GetName();
	}
}
