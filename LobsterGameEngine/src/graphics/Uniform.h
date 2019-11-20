#pragma once

namespace Lobster
{

	struct UniformDeclaration
	{
	public:
		std::string Name;
		enum DataType : uint {
			BOOL, BVEC2, BVEC3, BVEC4,
			UINT, UVEC2, UVEC3, UVEC4,
			INT, IVEC2, IVEC3, IVEC4,
			FLOAT, VEC2, VEC3, VEC4,
			DOUBLE, DVEC2, DVEC3, DVEC4,
			MAT3, MAT4,
			SAMPLER2D, SAMPLER3D, SAMPLERCUBE
		} Type;
	public:
		UniformDeclaration(const std::string_view& name, const std::string_view& type) {
			this->Name = std::string(name);
			if (type == "bool") this->Type = BOOL;
			else if (type == "bvec2") this->Type = BVEC2;
			else if (type == "bvec3") this->Type = BVEC3;
			else if (type == "bvec4") this->Type = BVEC4;
			else if (type == "uint") this->Type = UINT;
			else if (type == "uvec2") this->Type = UVEC2;
			else if (type == "uvec3") this->Type = UVEC3;
			else if (type == "uvec4") this->Type = UVEC4;
			else if (type == "int") this->Type = INT;
			else if (type == "ivec2") this->Type = IVEC2;
			else if (type == "ivec3") this->Type = IVEC3;
			else if (type == "ivec4") this->Type = IVEC4;
			else if (type == "float") this->Type = FLOAT;
			else if (type == "vec2") this->Type = VEC2;
			else if (type == "vec3") this->Type = VEC3;
			else if (type == "vec4") this->Type = VEC4;
			else if (type == "double") this->Type = DOUBLE;
			else if (type == "dvec2") this->Type = DVEC2;
			else if (type == "dvec3") this->Type = DVEC3;
			else if (type == "dvec4") this->Type = DVEC4;
			else if (type == "mat3") this->Type = MAT3;
			else if (type == "mat4") this->Type = MAT4;
			else if (type == "sampler2D") this->Type = SAMPLER2D;
			else if (type == "sampler3D") this->Type = SAMPLER3D;
			else if (type == "samplerCube") this->Type = SAMPLERCUBE;
			else throw std::runtime_error("Unknown uniform type");
		}
		constexpr size_t Size() {
			switch (Type)
			{
			case BOOL: return 1;
			case BVEC2: return 2;
			case BVEC3: return 3;
			case UINT: case INT: case FLOAT: case BVEC4: 
			case SAMPLER2D: case SAMPLER3D: case SAMPLERCUBE: return 4;
			case UVEC2: case IVEC2: case VEC2: case DOUBLE: return 8;
			case UVEC3: case IVEC3: case VEC3: return 12;
			case UVEC4: case IVEC4: case VEC4: case DVEC2: return 16;
			case DVEC3: return 24;
			case DVEC4: return 32;
			case MAT3: return 36;
			case MAT4: return 64;
			default: break;
			}
			return 0;
		}
	};

}