#include "pch.h"
#include "system/UndoSystem.h"

namespace Lobster {
	UndoSystem* UndoSystem::m_instance = nullptr;

	UndoSystem::UndoSystem(int size) :
		MAX_SIZE(size),
		m_undo(CircularBuffer<Command*>(size)),
		m_undo_str(CircularBuffer<std::string>(size))
	{
		if (m_instance) throw std::runtime_error("Two undo systems will create a great disaster. No please.");
		m_instance = this;
	}

	UndoSystem::~UndoSystem() {
		while (!m_undo.empty()) {
			delete m_undo.pop_front();
		}

		while (!m_redo.empty()) {
			delete m_redo.top();
			m_redo.pop();
		}
	}

	int UndoSystem::UndosRemaining() const {
		return m_undo.size();
	}

	int UndoSystem::RedosRemaining() const {
		return m_redo.size();
	}

	void UndoSystem::Undo(int steps) {
		if (steps > UndosRemaining()) {
			steps = UndosRemaining();
		}

		Command* event = nullptr;
		for (int i = 0; i < steps; i++) {
			event = m_undo.pop_front();
			m_undo_str.pop_front();
			m_redo.push(event);
			event->Undo();
			LOG("Undo performed. Description: {}", event->ToString());
		}
	}

	void UndoSystem::Redo(int steps) {
		if (steps > RedosRemaining()) {
			steps = RedosRemaining();
		}

		Command* event = nullptr;
		for (int i = 0; i < steps; i++) {
			event = m_redo.top();
			m_redo.pop();
			m_undo.push(event);
			m_undo_str.push(event->ToString());
			event->Exec();
			LOG("Redo performed. Description: {}", event->ToString());
		}
	}

	std::vector<std::string> UndoSystem::UndoList() const {
		return m_undo_str.to_vector();
	}

	std::vector<std::string> UndoSystem::RedoList() const {
		std::stack<Command*> redo_command(m_redo);
		std::vector<std::string> redo_strings(RedosRemaining());

		for (int i = 0; i < RedosRemaining(); i++) {
			redo_strings[i] = redo_command.top()->ToString();
			redo_command.pop();
		}
		return redo_strings;
	}

	void UndoSystem::Push(Command* command) {
		while (!m_redo.empty()) {
			delete m_redo.top();
			m_redo.pop();
		}

		//	Push into undo system, free memory if needed due to buffer being full.
		Command* c = m_undo.push(command);
		if (c != nullptr) delete c;

		m_undo_str.push(command->ToString());
		
		// Set the modified flag as true (the asterisk * display in title)
		Application::GetInstance()->SetSaved(false);
	}
}