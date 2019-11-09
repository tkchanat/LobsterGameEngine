#pragma once

namespace Lobster
{

	// uniform buffer objects should be a static instance for each class, since each class instances only update one specify UBO.
	class UniformBuffer
	{
		friend struct UniformBufferData;
		friend class UniformBufferLibrary;
	private:
		std::string m_name;
		uint m_id;
		int m_size;
	public:
		~UniformBuffer();
		inline void* Get() const { return (void*)(intptr_t)m_id; }
		inline int GetSize() const { return m_size; }
		inline std::string GetName() const { return m_name; }
	private:
		void SetData(void* data);
		explicit UniformBuffer(const char* name, int size, void* data = nullptr);
	};

	class UniformBufferLibrary
	{
		friend struct UniformBufferData;
	private:
		std::vector<UniformBuffer*> m_uniformBuffers;
		static UniformBufferLibrary* s_instance;
	public:
		static void Initialize();
	private:
		static UniformBuffer* Use(const char* name, int size);
	};

	// ===================================================
	// UniformBufferData =================================
	// ===================================================
	struct UniformBufferDeclaration
	{
	public:
		std::string Name;
		size_t Offset;
		enum DataType : uint
		{
			BOOL,
			INT, IVEC2, IVEC3, IVEC4,
			FLOAT, VEC2, VEC3, VEC4
		} Type;
	public:
		static DataType EnumMap(const std::string& type)
		{
			if (type == "bool")			return BOOL;
			else if (type == "int")		return INT;
			else if (type == "ivec2")	return IVEC2;
			else if (type == "ivec3")	return IVEC3;
			else if (type == "ivec4")	return IVEC4;
			else if (type == "float")	return FLOAT;
			else if (type == "vec2")	return VEC2;
			else if (type == "vec3")	return VEC3;
			else if (type == "vec4")	return VEC4;
			else throw std::runtime_error("What kind of data type did you input!? Our EnumMap doesn't have a record");
		}
		static DataType TypeMap(const std::string& type)
		{
			if (type == "bool")																	return INT;
			else if (type == "int" || type == "ivec2" || type == "ivec3" || type == "ivec4")	return INT;
			else if (type == "float" || type == "vec2" || type == "vec3" || type =="vec4")		return FLOAT;
		}
		static size_t SizeMap(const DataType& type)
		{
			switch (type)
			{
			case BOOL: return 4;
			case INT: case FLOAT: return 4;
			case IVEC2: case VEC2: return 8;
			case IVEC3: case VEC3: return 12;
			case IVEC4: case VEC4: return 16;
			default: throw std::runtime_error("What kind of data type did you input!? Our SizeMap doesn't have a record"); break;
			}
			return 0;
		}
		static int ElementMap(const DataType& type)
		{
			switch (type)
			{
			case BOOL: return 1;
			case INT: case FLOAT: return 1;
			case IVEC2: case VEC2: return 2;
			case IVEC3: case VEC3: return 3;
			case IVEC4: case VEC4: return 4;
			default: throw std::runtime_error("What kind of data type did you input!? Our ElementMap doesn't have a record"); break;
			}
			return 0;
		}
	};

	struct UniformBufferData
	{
	private:
		UniformBuffer* m_buffer;
		byte* m_bufferData;
		std::vector<UniformBufferDeclaration> m_declaration;
	public:
		UniformBufferData() : m_buffer(nullptr), m_bufferData(nullptr) {}
		UniformBufferData(const std::string& blueprintName, const std::vector<std::string>& blueprintElements);
		void BindData();
		void OnImGuiRender(const std::string& materialName, bool& dirty);
		void SetData(const char* name, void* data);
		nlohmann::json Serialize() const;
		inline std::string GetBufferName() const { return m_buffer->GetName(); }
	};

}
