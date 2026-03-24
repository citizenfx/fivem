#include <StdInc.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <ConsoleHost.h>
#include <CoreConsole.h>

#include <shellapi.h>

#include <skyr/url.hpp>
#include <fmt/format.h>

#include <UrlConfirmationExport.h>

#if defined(GTA_FIVE)
#define PRODUCT_NAME "FiveM"
#elif defined(IS_RDR3)
#define PRODUCT_NAME "RedM"
#endif

extern ImFont* consoleFontSmall;

namespace
{
struct LocalizedStrings
{
	std::string heading;
	std::string warning;
	std::string visitSite;
	std::string cancel;
};

const std::unordered_map<std::string, LocalizedStrings> g_translations = {
	{ "en-US", { "This link will take you to the website below", "You are now leaving {0} and visiting an unaffiliated external site. This link may not be secure. Only click on links from trusted senders.", "Visit Site", "Cancel" } },
	{ "de-DE", { "Dieser Link führt dich zur folgenden Website", "Du verlässt jetzt {0} und begibst dich auf eine externe Seite eines Drittanbieters. Die Sicherheit des Links kann nicht gewährleistet werden. Klicke nur auf Links, wenn du dem Absender vertraust.", "Website besuchen", "Abbrechen" } },
	{ "es-ES", { "Este enlace te llevará al sitio web a continuación", "Estás abandonando {0} y serás redirigido a un sitio web externo no afiliado con Rockstar Games. Puede que este enlace no sea seguro. Haz clic solamente en enlaces procedentes de fuentes de fiar.", "Visitar sitio", "Cancelar" } },
	{ "es-MX", { "Este enlace te llevará al sitio web a continuación", "Estás por salir de {0} y se te dirigirá a una página externa no afiliada. Este enlace podría no ser seguro. Haz clic solo en enlaces de fuentes seguras.", "Visitar sitio", "Cancelar" } },
	{ "fr-FR", { "Ce lien vous dirigera vers le site ci-dessous", "Vous allez quitter {0} et accéder à un site externe non affilié. Ce lien n'est peut-être pas sûr. Cliquez uniquement sur les liens d'expéditeurs de confiance.", "Visiter le site", "Annuler" } },
	{ "it-IT", { "Questo link ti porterà al sito web qui sotto", "Stai uscendo da {0} per visitare un sito esterno non affiliato. Questo link potrebbe non essere sicuro. Clicca solo su link forniti da mittenti attendibili.", "Visita il sito", "Annulla" } },
	{ "pl-PL", { "Ten link przeniesie Cię na poniższą stronę", "Zamierzasz opuścić {0} i udać się na zewnętrzną stronę sieciową. Ten odnośnik może nie być bezpieczny. Otwieraj odnośniki jedynie z zaufanych źródeł.", "Odwiedź stronę", "Anuluj" } },
	{ "pt-BR", { "Este link levará você ao site abaixo", "Você está saindo do {0} e acessando um site externo não afiliado. O link por não ser seguro. Clique apenas em links de fontes confiáveis.", "Visitar site", "Cancelar" } },
	{ "ru-RU", { "Эта ссылка приведёт вас на сайт ниже", "Вы покидаете {0} и переходите на сторонний сайт. Ссылка может быть ненадежной. Переходите только по ссылкам от доверенных отправителей.", "Перейти на сайт", "Отмена" } }
};

const LocalizedStrings& GetLocalizedStrings()
{
	// Read the user's preferred languages from registry
	HKEY hKey;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\International\\User Profile", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		DWORD bufferSize = 0;
		if (RegGetValueW(hKey, nullptr, L"Languages", RRF_RT_REG_MULTI_SZ, nullptr, nullptr, &bufferSize) == ERROR_SUCCESS && bufferSize > 0)
		{
			std::vector<wchar_t> buffer(bufferSize / sizeof(wchar_t));
			if (RegGetValueW(hKey, nullptr, L"Languages", RRF_RT_REG_MULTI_SZ, nullptr, buffer.data(), &bufferSize) == ERROR_SUCCESS)
			{
				RegCloseKey(hKey);

				// Buffer contains null-separated list of languages in preference order
				const wchar_t* current = buffer.data();
				while (*current)
				{
					std::string locale = ToNarrow(current);

					auto it = g_translations.find(locale);

					// Check for exact match first
					if (it != g_translations.end())
					{
						return it->second;
					}

					// Try language-only fallback (e.g., "en -> "en-US")
					auto dashPos = locale.find('-');
					std::string langOnly = (dashPos != std::string::npos) ? locale.substr(0, dashPos) : locale;

					for (const auto& [key, value] : g_translations)
					{
						if (key.substr(0, key.find('-')) == langOnly)
						{
							return value;
						}
					}

					current += wcslen(current) + 1;
				}
			}
		}
		RegCloseKey(hKey);
	}

	// Fallback to English if no match found
	return g_translations.at("en-US");
}
}

void DrawUrlConfirmationModal()
{
	if (!nui::g_showUrlConfirmModal.load())
	{
		return;
	}

	ImGui::OpenPopup("##UrlConfirmModal");

	std::lock_guard<std::mutex> lock(nui::g_urlModalMutex);

	const auto& strings = GetLocalizedStrings();

	ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(26, 26, 26, 255));
	ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(26, 26, 26, 255));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(45.0f, 50.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(960, 0), ImGuiCond_Always);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

	if (ImGui::BeginPopupModal("##UrlConfirmModal", nullptr, flags))
	{
		// Main heading
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(235, 235, 235, 255));
		ImGui::Text("%s", strings.heading.c_str());
		ImGui::PopStyleColor();

		ImGui::Dummy(ImVec2(0, 8.0f));

		std::string hostDisplay;

		auto parsed = skyr::make_url(nui::g_pendingUrl);
		if (parsed && !parsed->host().empty())
		{
			hostDisplay = parsed->host();
		}
		else
		{
			hostDisplay = nui::g_pendingUrl;
		}

		size_t hostPos = nui::g_pendingUrl.find(hostDisplay);
		if (hostPos != std::string::npos && hostDisplay != nui::g_pendingUrl)
		{
			std::string beforeHost = nui::g_pendingUrl.substr(0, hostPos);
			std::string afterHost = nui::g_pendingUrl.substr(hostPos + hostDisplay.length());

			// Display scheme
			if (!beforeHost.empty())
			{
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(235, 235, 235, 255));
				ImGui::Text("%s", beforeHost.c_str());
				ImGui::PopStyleColor();
				ImGui::SameLine(0, 0);
			}

			// Display host in accent color
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(248, 171, 19, 255));
			ImGui::Text("%s", hostDisplay.c_str());
			ImGui::PopStyleColor();

			// Display path/query
			if (!afterHost.empty())
			{
				ImGui::SameLine(0, 0);
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(235, 235, 235, 255));
				ImGui::Text("%s", afterHost.c_str());
				ImGui::PopStyleColor();
			}
		}
		else
		{
			// Fallback: display entire URL in accent color if host not found
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(248, 171, 19, 255));
			ImGui::Text("%s", nui::g_pendingUrl.c_str());
			ImGui::PopStyleColor();
		}

		ImGui::Dummy(ImVec2(0, 12.0f));

		// Separator
		ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(44, 44, 44, 255));
		ImGui::Separator();
		ImGui::PopStyleColor();

		ImGui::Dummy(ImVec2(0, 12.0f));

		// Warning text
		ImGui::PushFont(consoleFontSmall);
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(152, 152, 152, 255));
		auto warningText = fmt::format(fmt::runtime(strings.warning), PRODUCT_NAME);
		ImGui::TextWrapped("%s", warningText.c_str());
		ImGui::PopStyleColor();
		ImGui::PopFont();

		ImGui::Dummy(ImVec2(0, 20.0f));

		float buttonWidth = ImGui::GetContentRegionAvail().x;

		// Visit Site button
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(235, 235, 235, 255));
		ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(23, 73, 138, 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(35, 95, 170, 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(18, 58, 110, 255));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

		if (ImGui::Button(strings.visitSite.c_str(), ImVec2(buttonWidth, 90.0f)))
		{
			ShellExecuteW(nullptr, L"open", ToWide(nui::g_pendingUrl).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
			nui::g_pendingUrl.clear();
			nui::g_showUrlConfirmModal.store(false);
			ImGui::CloseCurrentPopup();
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);

		ImGui::Dummy(ImVec2(0, 30.0f));

		// Cancel button
		ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(26, 26, 26, 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(50, 50, 53, 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(35, 35, 38, 255));
		ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(63, 63, 63, 255));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);

		if (ImGui::Button(strings.cancel.c_str(), ImVec2(buttonWidth, 80.0f)))
		{
			nui::g_pendingUrl.clear();
			nui::g_showUrlConfirmModal.store(false);
			ImGui::CloseCurrentPopup();
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(5);

		ImGui::Dummy(ImVec2(0, 5.0f));

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(2);
}
