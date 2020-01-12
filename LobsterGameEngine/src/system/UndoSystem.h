#pragma once
#include <vector>
#include <stack>
#include "system/CircularBuffer.h"
#include "system/Command.h"

//	 system to keep track of recent undoable activities.
namespace Lobster {
	class UndoSystem {
	public:
		UndoSystem(int size = 50);
		~UndoSystem();

		//	Gets amount of undos / redos left
		int UndosRemaining() const;
		int RedosRemaining() const;

		//	Performs undo / redo action
		void Undo(int steps = 1);
		void Redo(int steps = 1);

		//	Returns a string representation of what is in the undo vector / redo stack
		std::vector<std::string> UndoList() const;
		std::vector<std::string> RedoList() const;

		//	Add an undo command to our undo buffer
		void Push(Command* command);

		inline static UndoSystem* GetInstance() { return m_instance; }

	private:
		static UndoSystem* m_instance;
		CircularBuffer<Command*> m_undo;
		CircularBuffer<std::string> m_undo_str;
		std::stack<Command*> m_redo;
		const int MAX_SIZE = 50;
	};
}