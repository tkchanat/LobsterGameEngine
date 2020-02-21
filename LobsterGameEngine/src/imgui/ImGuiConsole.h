#pragma once
#include "ImGuiComponent.h"
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#ifdef USE_SPDLOG_CONSOLE
#define LOG(...) ::spdlog::get("console")->trace(__VA_ARGS__)
#define INFO(...) ::spdlog::get("console")->info(__VA_ARGS__)
#define WARN(...) ::spdlog::get("console")->warn(__VA_ARGS__)
#define CRITICAL(...) ::spdlog::get("console")->error(__VA_ARGS__)
#define GL_LOG(x) x; ::spdlog::get("console")->trace("GL Error Code: {}", glGetError())
#else
#define LOG(...) ImGuiConsole::log.AddLog((std::string("[LOG] ") + fmt::format(__VA_ARGS__)).c_str())
#define INFO(...) ImGuiConsole::log.AddLog((std::string("[INFO] ") + fmt::format(__VA_ARGS__)).c_str())
#define WARN(...) ImGuiConsole::log.AddLog((std::string("[WARN] ") + fmt::format(__VA_ARGS__)).c_str())
#define CRITICAL(...) ImGuiConsole::log.AddLog((std::string("[CRITICAL] ") + fmt::format(__VA_ARGS__)).c_str())
#define GL_LOG(x) x; ImGuiConsole::log.AddLog(fmt::format("GL Error Code: {}", glGetError()).c_str())
#endif

namespace Lobster
{

	struct SSFLog
	{
		ImGuiTextBuffer     Buf;
		ImGuiTextFilter     Filter;
		ImVector<int>       LineOffsets;
		bool                AutoScroll;
		bool                ScrollToBottom;

		SSFLog()
		{
#ifdef USE_SPDLOG_CONSOLE
			InitConsole();
#endif
			AutoScroll = true;
			ScrollToBottom = false;
			Clear();
		}

		void InitConsole() {
			static std::shared_ptr<spdlog::logger> g_console;
			spdlog::set_pattern("%^[%n %T]: %v%$");
			g_console = spdlog::stdout_color_mt("console");
			g_console->set_level(spdlog::level::trace);
			//g_console->info("SPDLOG is running...");
		}

		void Clear()
		{
			Buf.clear();
			LineOffsets.clear();
			LineOffsets.push_back(0);
		}

		void AddLog(const char* fmt, ...) IM_FMTARGS(2)
		{
			std::string msg = std::string(fmt) + '\n';
			const char* fmtNewLine = msg.c_str();
			int old_size = Buf.size();
			va_list args;
			va_start(args, fmtNewLine);
			Buf.appendfv(fmtNewLine, args);
			va_end(args);
			for (int new_size = Buf.size(); old_size < new_size; old_size++)
				if (Buf[old_size] == '\n')
					LineOffsets.push_back(old_size + 1);
			if (AutoScroll)
				ScrollToBottom = true;
		}

		void Draw(const char* title, bool* p_open = NULL)
		{
			if (!ImGui::Begin(title, p_open))
			{
				ImGui::End();
				return;
			}

			// Options menu
			if (ImGui::BeginPopup("Options"))
			{
				if (ImGui::Checkbox("Auto-scroll", &AutoScroll))
					if (AutoScroll)
						ScrollToBottom = true;
				ImGui::EndPopup();
			}

			// Main window
			if (ImGui::Button("Options"))
				ImGui::OpenPopup("Options");
			ImGui::SameLine();
			bool clear = ImGui::Button("Clear");
			ImGui::SameLine();
			bool copy = ImGui::Button("Copy");
			ImGui::SameLine();
			Filter.Draw("Filter", -100.0f);

			ImGui::Separator();
			ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

			if (clear)
				Clear();
			if (copy)
				ImGui::LogToClipboard();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			const char* buf = Buf.begin();
			const char* buf_end = Buf.end();
			if (Filter.IsActive())
			{
				// In this example we don't use the clipper when Filter is enabled.
				// This is because we don't have a random access on the result on our filter.
				// A real application processing logs with ten of thousands of entries may want to store the result of search/filter.
				// especially if the filtering function is not trivial (e.g. reg-exp).
				for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
				{
					const char* line_start = buf + LineOffsets[line_no];
					const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
					if (Filter.PassFilter(line_start, line_end))
						ImGui::TextUnformatted(line_start, line_end);
				}
			}
			else
			{
				// The simplest and easy way to display the entire buffer:
				//   ImGui::TextUnformatted(buf_begin, buf_end);
				// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward to skip non-visible lines.
				// Here we instead demonstrate using the clipper to only process lines that are within the visible area.
				// If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them on your side is recommended.
				// Using ImGuiListClipper requires A) random access into your data, and B) items all being the  same height,
				// both of which we can handle since we an array pointing to the beginning of each line of text.
				// When using the filter (in the block of code above) we don't have random access into the data to display anymore, which is why we don't use the clipper.
				// Storing or skimming through the search result would make it possible (and would be recommended if you want to search through tens of thousands of entries)
				ImGuiListClipper clipper;
				clipper.Begin(LineOffsets.Size);
				while (clipper.Step())
				{
					for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
					{
						const char* line_start = buf + LineOffsets[line_no];
						const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
						ImGui::TextUnformatted(line_start, line_end);
					}
				}
				clipper.End();
			}
			ImGui::PopStyleVar();

			if (ScrollToBottom)
				ImGui::SetScrollHereY(1.0f);
			ScrollToBottom = false;
			ImGui::EndChild();
			ImGui::End();
		}
	};	

	class ImGuiConsole : public ImGuiComponent
	{
	public: 
		static SSFLog log;
	public:	
		ImGuiConsole() : ImGuiComponent() {}
		virtual void Show(bool* p_open) override
		{
			ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
			ImGui::Begin("Console", nullptr);
			//if (ImGui::SmallButton("[Debug] Add some entries"))
			//{
			//	log.AddLog(fmt::format("[{}] Hello, current time is {}", ImGui::GetFrameCount(), ImGui::GetTime()).c_str());
			//}
			ImGui::End();

			log.Draw("Console", nullptr);
		}
	};

}