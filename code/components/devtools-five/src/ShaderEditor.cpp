#include <StdInc.h>
#include <CoreConsole.h>
#include <ConsoleHost.h>
#include <imgui.h>
#include <Streaming.h>
#include <atArray.h>
#include <grcTexture.h>

#include <cstring>

namespace
{
enum class grcEffectVarType : uint8_t
{
	VT_NONE = 0,
	VT_INT = 1,
	VT_FLOAT = 2,
	VT_VECTOR2 = 3,
	VT_VECTOR3 = 4,
	VT_VECTOR4 = 5,
	VT_TEXTURE = 6,
	VT_BOOL = 7,
	VT_MATRIX34 = 8,
	VT_MATRIX44 = 9,
	VT_STRING = 10,
	VT_INT1 = 11,
	VT_INT2 = 12,
	VT_INT3 = 13,
	VT_INT4 = 14,
	VT_STRUCTURED_BUFFER = 15,
	VT_SAMPLER_STATE = 16,
};

struct grcInstanceData
{
	struct Entry
	{
		uint8_t Count;
		uint8_t Register;
		uint8_t SamplerStateSet;
		uint8_t SavedSamplerStateSet;
		union
		{
			float* Float;
			void* Any;
		};
	};

	Entry* m_Entries;

	union
	{
		struct grcEffect* ptr;
		uint32_t namehash;
	} m_Effect;

	char m_Count;
	char m_DrawBucket;

	float* GetFloat4(int paramIndex)
	{
		return m_Entries[paramIndex].Float;
	}
};

struct grcParameter
{
	uint8_t Type;
	uint8_t Count;
	uint8_t DataSize;
	uint8_t AnnotationCount;
	const char* Name; // +8
	const char* Semantic;
	uint32_t NameHash;
	uint32_t SemanticHash;
	void* Annotations;
	void* Data;
	uint16_t Register : 6;
	uint16_t TextureType : 2;
	uint16_t Usage : 7;
	uint16_t ComparisonFilter : 1;
	uint8_t SamplerStateSet;
	uint8_t SavedSamplerStateSet;
	uint32_t CBufferOffset;
	uint32_t CBufferNameHash;
	void* CBuf;

	grcEffectVarType GetType() { return (grcEffectVarType)Type; }
};

struct grcEffect
{
	atArray<char> m_Techniques; // grcEffectTechnique
	atArray<grcParameter> m_Locals;
	atArray<void*> m_LocalsCBuf;
	atArray<char> m_VertexPrograms;
	atArray<char> m_FragmentPrograms;
	char m_TechniqueMap[64 * 8];
	char m_Container[16]; // sysMemContainerData
	char m_Name[40];
	grcInstanceData m_InstanceDataTemplate;
};

struct grmShader
{
	grcInstanceData data;
};

struct grmShaderGroup
{
	void* __vftable;
	void* m_TextureDictionary; // +0x08
	atArray<grmShader*> m_Shaders; // +0x10
	// ... more after this, unused here
};

struct rmcDrawableBase
{
	void* __vftable;
	void* m_FirstNode; // pgBase
	grmShaderGroup* m_ShaderGroup; // +0x10
};

rmcDrawableBase* FindDrawable(const char* name)
{
	auto mgr = streaming::Manager::GetInstance();
	if (!mgr)
	{
		return nullptr;
	}

	auto ydrStore = mgr->moduleMgr.GetStreamingModule("ydr");
	if (!ydrStore)
	{
		return nullptr;
	}

	uint32_t slotId = -1;
	if (*ydrStore->FindSlot(&slotId, name) == -1)
	{
		return nullptr;
	}

	return reinterpret_cast<rmcDrawableBase*>(ydrStore->GetPtr(slotId));
}

int GetShaderCount(rmcDrawableBase* drawable)
{
	return drawable->m_ShaderGroup->m_Shaders.GetCount();
}

grmShader* GetShader(rmcDrawableBase* drawable, int index)
{
	return drawable->m_ShaderGroup->m_Shaders.Get(index);
}

int GetShaderParamCount(grmShader* shader)
{
	return shader->data.m_Count;
}

const char* GetParamName(grmShader* shader, int index)
{
	return shader->data.m_Effect.ptr->m_Locals.Get(index).Name;
}

grcEffectVarType GetParamType(grmShader* shader, int index)
{
	return shader->data.m_Effect.ptr->m_Locals.Get(index).GetType();
}

float* GetParamValueFloat4(grmShader* shader, int index)
{
	return shader->data.GetFloat4(index);
}

const char* GetParamTextureName(grmShader* shader, int index)
{
	void* tex = shader->data.m_Entries[index].Any;
	if (!tex)
	{
		return nullptr;
	}

	return *reinterpret_cast<const char**>(reinterpret_cast<char*>(tex) + 0x28);
}

ImTextureID GetParamTextureId(grmShader* shader, int index)
{
	auto& entry = shader->data.m_Entries[index];
	if (!entry.Any)
	{
		return nullptr;
	}

	return reinterpret_cast<ImTextureID>(&entry.Any);
}

const char* ShortTextureLabel(const char* name)
{
	static char buf[64];

	size_t len = strlen(name);
	const char* suffix = "Sampler";
	size_t suffixLen = strlen(suffix);

	if (len > suffixLen && strcmp(name + len - suffixLen, suffix) == 0)
	{
		len -= suffixLen;
	}

	if (len >= sizeof(buf))
	{
		len = sizeof(buf) - 1;
	}

	memcpy(buf, name, len);
	buf[len] = '\0';
	return buf;
}

const char* FitTextToWidth(const char* text, float maxWidth)
{
	if (ImGui::CalcTextSize(text).x <= maxWidth)
	{
		return text;
	}

	static char buf[80];
	size_t n = strlen(text);
	if (n > sizeof(buf) - 3)
	{
		n = sizeof(buf) - 3;
	}

	while (n > 0)
	{
		memcpy(buf, text, n);
		buf[n] = '\0';
		strcat_s(buf, sizeof(buf), "..");
		if (ImGui::CalcTextSize(buf).x <= maxWidth)
		{
			return buf;
		}
		--n;
	}

	buf[0] = '\0';
	return buf;
}

const char* GetShaderName(grmShader* shader)
{
	return shader->data.m_Effect.ptr->m_Name;
}

bool g_materialInspectorAllowed = false;
bool IsMaterialInspectorAllowed()
{
#ifdef _DEBUG
	return true;
#else
	return g_materialInspectorAllowed;
#endif
}
}

static InitFunction initFunction([]()
{
	static bool shaderEditorEnabled;
	static ConVar<bool> shaderEditorVar("shaderEditor", ConVar_Archive | ConVar_UserPref, false, &shaderEditorEnabled);
	static ConVar<bool> materialInspectorAllowVar("sv_materialInspector", ConVar_Replicated, false, &g_materialInspectorAllowed);

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || (shaderEditorEnabled && IsMaterialInspectorAllowed());
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (!shaderEditorEnabled || !IsMaterialInspectorAllowed())
		{
			return;
		}

		static char modelSearchBuffer[256] = "";
		static rmcDrawableBase* selectedDrawable = nullptr;

		ImGui::SetNextWindowSize(ImVec2(400.0f, 700.0f), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Material Inspector", &shaderEditorEnabled))
		{
			ImGui::TextDisabled("Search for a model name (e.g., 'prop_crate_01a')");
			if (ImGui::InputText("##ModelSearch", modelSearchBuffer, IM_ARRAYSIZE(modelSearchBuffer)))
			{
				selectedDrawable = FindDrawable(modelSearchBuffer);
			}
			ImGui::SameLine();
			if (ImGui::Button("Clear"))
			{
				modelSearchBuffer[0] = '\0';
				selectedDrawable = nullptr;
			}

			ImGui::Separator();

			if (!selectedDrawable)
			{
				ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No drawable found or loaded.");
				ImGui::End();
				return;
			}

			ImGui::Text("Model: %s", modelSearchBuffer);
			int shaderCount = GetShaderCount(selectedDrawable);
			ImGui::TextDisabled("%d Shaders found", shaderCount);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 1.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 1.0f));

			for (int i = 0; i < shaderCount; ++i)
			{
				grmShader* shader = GetShader(selectedDrawable, i);
				if (!shader)
				{
					continue;
				}

				const char* shaderName = GetShaderName(shader);

				ImGui::PushID(shader);
				if (ImGui::CollapsingHeader(shaderName, ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Indent();
					int paramCount = GetShaderParamCount(shader);

					const float thumb = 48.0f;
					const float cellPitch = thumb + 40.0f;
					float gridStartX = ImGui::GetCursorPosX();
					int perRow = (int)(ImGui::GetContentRegionAvail().x / cellPitch);
					if (perRow < 1)
					{
						perRow = 1;
					}
					int texCol = 0;
					bool anyTextures = false;

					for (int p = 0; p < paramCount; ++p)
					{
						if (GetParamType(shader, p) != grcEffectVarType::VT_TEXTURE)
						{
							continue;
						}

						anyTextures = true;
						ImGui::PushID(p);

						if (texCol > 0)
						{
							ImGui::SameLine(gridStartX + texCol * cellPitch);
						}

						ImGui::BeginGroup();
						ImTextureID texId = GetParamTextureId(shader, p);
						if (texId)
						{
							auto tex = reinterpret_cast<rage::grcTexture*>(shader->data.m_Entries[p].Any);
							float w = (float)tex->GetWidth();
							float h = (float)tex->GetHeight();

							ImVec2 size(thumb, thumb);
							if (w > 0.0f && h > 0.0f)
							{
								float scale = thumb / (w > h ? w : h);
								size = ImVec2(w * scale, h * scale);
							}
							ImGui::Image(texId, size);
						}
						else
						{
							ImGui::Dummy(ImVec2(thumb, thumb));
						}

						const char* texparamName = GetParamName(shader, p);
						ImGui::TextDisabled("%s", FitTextToWidth(ShortTextureLabel(texparamName), cellPitch - 6.0f));
						ImGui::EndGroup();

						if (ImGui::IsItemHovered())
						{
							const char* texName = GetParamTextureName(shader, p);
							ImGui::SetTooltip("%s\n%s", texparamName, texName ? texName : "(none)");
						}

						ImGui::PopID();

						if (++texCol >= perRow)
						{
							texCol = 0;
						}
					}

					if (anyTextures)
					{
						ImGui::Separator();
					}

					for (int p = 0; p < paramCount; ++p)
					{
						const char* paramName = GetParamName(shader, p);
						grcEffectVarType type = GetParamType(shader, p);

						if (type == grcEffectVarType::VT_TEXTURE)
						{
							continue;
						}

						ImGui::PushID(p);

						ImGui::Columns(2, "ParamColumns", false);
						ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.4f);
						ImGui::Text("%s", paramName);
						ImGui::NextColumn();

						switch (type)
						{
							case grcEffectVarType::VT_FLOAT:
							{
								float* val = GetParamValueFloat4(shader, p);
								ImGui::DragFloat("##v", val, 0.01f);
								break;
							}

							case grcEffectVarType::VT_VECTOR4:
							{
								float* val = GetParamValueFloat4(shader, p);
								if (strstr(paramName, "Color") || strstr(paramName, "Tint"))
									ImGui::ColorEdit4("##v", val, ImGuiColorEditFlags_NoInputs);
								else
									ImGui::DragFloat4("##v", val, 0.01f);
								break;
							}

							default:
								ImGui::TextDisabled("Type: %d", (int)type);
								break;
						}

						ImGui::Columns(1);
						ImGui::PopID();
					}
					ImGui::Unindent();
				}
				ImGui::PopID();
			}

			ImGui::PopStyleVar(2);
		}

		ImGui::End();
	});
});
