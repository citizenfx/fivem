//- Common Code For All Addons needed just to ease inclusion as separate files in user code ----------------------
#include "StdInc.h"

#define IMGUI_API DLL_IMPORT
#include <imgui.h>
#undef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
//-----------------------------------------------------------------------------------------------------------------

#include "imguivariouscontrols.h"

#ifndef NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE
#ifndef IMGUI_USE_AUTO_BINDING
#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../imguibindings/stb_image.h"
#endif //STBI_INCLUDE_STB_IMAGE_H
#endif //IMGUI_USE_AUTO_BINDING
//#define DEBUG_OUT_TEXTURE
#ifdef DEBUG_OUT_TEXTURE
#ifndef STBI_INCLUDE_STB_IMAGE_WRITE_H
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#include "./addons/imguiyesaddons/imguiimageeditor_plugins/stb_image_write.h"
#endif //DEBUG_OUT_TEXTURE
#endif //STBI_INCLUDE_STB_IMAGE_WRITE_H
#endif //NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE


namespace ImGui {

#ifndef IMGUIHELPER_H_
// Posted by Omar in one post. It might turn useful...
bool IsItemActiveLastFrame()    {
#if 0
    ImGuiContext& g = *ImGui::GetCurrentContext();
    if (g.ActiveIdPreviousFrame)
	return g.ActiveIdPreviousFrame== ImGui::GetCurrentContext()->CurrentWindow->DC.LastItemId;
#endif
    return false;
}
bool IsItemJustReleased()   {
    return IsItemActiveLastFrame() && !ImGui::IsItemActive();
}
#endif //IMGUIHELPER_H_

static float GetWindowFontScale() {
    //ImGuiContext& g = *ImGui::GetCurrentContext();
    ImGuiWindow* window = GetCurrentWindow();
    return window->FontWindowScale;
}

static bool CheckButton(const char* label,bool* pvalue,bool useSmallButton,float checkedStateAlphaMult=0.5f) {
    bool rv = false;
    const bool tmp = pvalue ? *pvalue : false;
    if (tmp) {
        const ImGuiStyle& style(ImGui::GetStyle());
        ImVec4 CheckButtonColor = style.Colors[ImGuiCol_Button];                CheckButtonColor.w*=checkedStateAlphaMult;
        ImVec4 CheckButtonHoveredColor = style.Colors[ImGuiCol_ButtonHovered];  CheckButtonHoveredColor.w*=checkedStateAlphaMult;
        ImVec4 CheckButtonActiveColor = style.Colors[ImGuiCol_ButtonActive];    CheckButtonActiveColor.w*=checkedStateAlphaMult;

        ImGui::PushStyleColor(ImGuiCol_Button,CheckButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,CheckButtonHoveredColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,CheckButtonActiveColor);
    }
    if (useSmallButton) {if (ImGui::SmallButton(label)) {if (pvalue) *pvalue=!(*pvalue);rv=true;}}
    else if (ImGui::Button(label)) {if (pvalue) *pvalue=!(*pvalue);rv=true;}
    if (tmp) ImGui::PopStyleColor(3);
    return rv;
}
bool CheckButton(const char* label,bool* pvalue) {return CheckButton(label,pvalue,false);}
bool SmallCheckButton(const char* label,bool* pvalue) {return CheckButton(label,pvalue,true);}

float ProgressBar(const char *optionalPrefixText, float value, const float minValue, const float maxValue, const char *format, const ImVec2 &sizeOfBarWithoutTextInPixels, const ImVec4 &colorLeft, const ImVec4 &colorRight, const ImVec4 &colorBorder)    {
    if (value<minValue) value=minValue;
    else if (value>maxValue) value = maxValue;
    const float valueFraction = (maxValue==minValue) ? 1.0f : ((value-minValue)/(maxValue-minValue));
    const bool needsPercConversion = strstr(format,"%%")!=NULL;

    ImVec2 size = sizeOfBarWithoutTextInPixels;
    if (size.x<=0) size.x = ImGui::GetWindowWidth()*0.25f;
    if (size.y<=0) size.y = ImGui::GetTextLineHeightWithSpacing(); // or without

    const ImFontAtlas* fontAtlas = ImGui::GetIO().Fonts;

    if (optionalPrefixText && strlen(optionalPrefixText)>0) {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s",optionalPrefixText);
        ImGui::SameLine();
    }

    if (valueFraction>0)   {
        ImGui::Image(fontAtlas->TexID,ImVec2(size.x*valueFraction,size.y), fontAtlas->TexUvWhitePixel,fontAtlas->TexUvWhitePixel,colorLeft,colorBorder);
    }
    if (valueFraction<1)   {
        if (valueFraction>0) ImGui::SameLine(0,0);
        ImGui::Image(fontAtlas->TexID,ImVec2(size.x*(1.f-valueFraction),size.y), fontAtlas->TexUvWhitePixel,fontAtlas->TexUvWhitePixel,colorRight,colorBorder);
    }
    ImGui::SameLine();

    ImGui::Text(format,needsPercConversion ? (valueFraction*100.f+0.0001f) : value);
    return valueFraction;
}

void TestProgressBar()  {    
    const float time = ((float)(((unsigned int) (ImGui::GetTime()*1000.f))%50000)-25000.f)/25000.f;
    float progress=(time>0?time:-time);
    // No IDs needed for ProgressBars:
    ImGui::ProgressBar("ProgressBar",progress);
    ImGui::ProgressBar("ProgressBar",1.f-progress);
    ImGui::ProgressBar("",500+progress*1000,500,1500,"%4.0f (absolute value in [500,1500] and fixed bar size)",ImVec2(150,-1));
    ImGui::ProgressBar("",500+progress*1000,500,1500,"%3.0f%% (same as above, but with percentage and new colors)",ImVec2(150,-1),ImVec4(0.7,0.7,1,1),ImVec4(0.05,0.15,0.5,0.8),ImVec4(0.8,0.8,0,1));
    // This one has just been added to ImGui:
    //char txt[48]="";sprintf(txt,"%3d%% (ImGui default progress bar)",(int)(progress*100));
    //ImGui::ProgressBar(progress,ImVec2(0,0),txt);
}


int PopupMenuSimple(bool &open, const char **pEntries, int numEntries, const char *optionalTitle, int *pOptionalHoveredEntryOut, int startIndex, int endIndex, bool reverseItems, const char *scrollUpEntryText, const char *scrollDownEntryText)   {
    int selectedEntry = -1;
    if (pOptionalHoveredEntryOut) *pOptionalHoveredEntryOut=-1;
    if (!open) return selectedEntry;
    if (numEntries==0 || !pEntries) {
        open = false;
        return selectedEntry;
    }

    float fs = 1.f;
#   ifdef IMGUI_INCLUDE_IMGUI_USER_INL
    fs = ImGui::GetWindowFontScale();   // Internal to <imgui.cpp>
#   endif //   IMGUI_INCLUDE_IMGUI_USER_INL

    if (!open) return selectedEntry;
    ImGui::PushID(&open);   // or pEntries ??
    //ImGui::BeginPopup(&open);
    ImGui::OpenPopup("MyOwnPopupSimpleMenu");
    if (ImGui::BeginPopup("MyOwnPopupSimpleMenu"))  {
        if (optionalTitle) {ImGui::Text("%s",optionalTitle);ImGui::Separator();}
        if (startIndex<0) startIndex=0;
        if (endIndex<0) endIndex = numEntries-1;
        if (endIndex>=numEntries) endIndex = numEntries-1;
        const bool needsScrolling = (endIndex-startIndex+1)<numEntries;
        if (scrollUpEntryText && needsScrolling) {
            ImGui::SetWindowFontScale(fs*0.75f);
            if (reverseItems ? (endIndex+1<numEntries) : (startIndex>0))    {
                const int entryIndex = reverseItems ? -3 : -2;
                if (ImGui::Selectable(scrollUpEntryText, false))  {
                    selectedEntry = entryIndex;//open = false;    // Hide menu
                }
                else if (pOptionalHoveredEntryOut && ImGui::IsItemHovered()) *pOptionalHoveredEntryOut = entryIndex;
            }
            else ImGui::Text(" ");
            ImGui::SetWindowFontScale(fs);
        }
        if (!reverseItems)  {
            for (int i = startIndex; i <= endIndex; i++)    {
                const char* entry = pEntries[i];
                if (!entry || strlen(entry)==0) ImGui::Separator();
                else {
                    if (ImGui::Selectable(entry, false))  {
                        selectedEntry = i;open = false;    // Hide menu
                    }
                    else if (pOptionalHoveredEntryOut && ImGui::IsItemHovered()) *pOptionalHoveredEntryOut = i;
                }
            }
        }
        else {
            for (int i = endIndex; i >= startIndex; i--)    {
                const char* entry = pEntries[i];
                if (!entry || strlen(entry)==0) ImGui::Separator();
                else {
                    if (ImGui::Selectable(entry, false))  {
                        selectedEntry = i;open = false;    // Hide menu
                    }
                    else if (pOptionalHoveredEntryOut && ImGui::IsItemHovered()) *pOptionalHoveredEntryOut = i;
                }

            }
        }
        if (scrollDownEntryText && needsScrolling) {
            const float fs = ImGui::GetWindowFontScale();      // Internal to <imgui.cpp>
            ImGui::SetWindowFontScale(fs*0.75f);
            if (reverseItems ? (startIndex>0) : (endIndex+1<numEntries))    {
                const int entryIndex = reverseItems ? -2 : -3;
                if (ImGui::Selectable(scrollDownEntryText, false))  {
                    selectedEntry = entryIndex;//open = false;    // Hide menu
                }
                else if (pOptionalHoveredEntryOut && ImGui::IsItemHovered()) *pOptionalHoveredEntryOut = entryIndex;
            }
            else ImGui::Text(" ");
            ImGui::SetWindowFontScale(fs);
        }
        if (open)   // close menu when mouse goes away
        {
            const float d = 10;
            ImVec2 pos = ImGui::GetWindowPos();pos.x-=d;pos.y-=d;
            ImVec2 size = ImGui::GetWindowSize();size.x+=2.f*d;size.y+=2.f*d;
            const ImVec2& mousePos = ImGui::GetIO().MousePos;
            if (mousePos.x<pos.x || mousePos.y<pos.y || mousePos.x>pos.x+size.x || mousePos.y>pos.y+size.y) open = false;
        }
    }
    ImGui::EndPopup();
    ImGui::PopID();

    return selectedEntry;    
}

int PopupMenuSimpleCopyCutPasteOnLastItem(bool readOnly) {
    static bool open = false;
    static const char* entries[] = {"Copy","Cut","","Paste"};   // "" is separator
    //open|=ImGui::Button("Show Popup Menu Simple");                    // BUTTON
    open|= ImGui::GetIO().MouseClicked[1] && ImGui::IsItemHovered(); // RIGHT CLICK
    int selectedEntry = PopupMenuSimple(open,entries,readOnly?1:4);
    if (selectedEntry>2) selectedEntry = 2; // Normally separator takes out one space
    return selectedEntry;
    // About "open": when user exits popup-menu, "open" becomes "false".
    // Please set it to "true" to display it again (we do it using open|=[...])
}


int PopupMenuSimple(PopupMenuSimpleParams &params, const char **pTotalEntries, int numTotalEntries, int numAllowedEntries, bool reverseItems, const char *optionalTitle, const char *scrollUpEntryText, const char *scrollDownEntryText)    {
    if (numAllowedEntries<1 || numTotalEntries==0) {params.open=false;return -1;}
    if (params.endIndex==-1) params.endIndex=reverseItems ? numTotalEntries-1 : numAllowedEntries-1;
    if (params.startIndex==-1) params.startIndex=params.endIndex-numAllowedEntries+1;

    const int oldHoveredEntry = params.hoveredEntry;
    params.selectedEntry = PopupMenuSimple(params.open,pTotalEntries,numTotalEntries,optionalTitle,&params.hoveredEntry,params.startIndex,params.endIndex,reverseItems,scrollUpEntryText,scrollDownEntryText);

    if (params.hoveredEntry<=-2 || params.selectedEntry<=-2)   {
        if (oldHoveredEntry!=params.hoveredEntry) params.scrollTimer = ImGui::GetTime();
        const float newTime = ImGui::GetTime();
        if (params.selectedEntry<=-2 || (newTime - params.scrollTimer > 0.4f))    {
            params.scrollTimer = newTime;
            if (params.hoveredEntry==-2 || params.selectedEntry==-2)   {if (params.startIndex>0) {--params.startIndex;--params.endIndex;}}
            else if (params.hoveredEntry==-3 || params.selectedEntry==-3) {if (params.endIndex<numTotalEntries-1) {++params.startIndex;++params.endIndex;}}

        }
    }
    if (!params.open && params.resetScrollingWhenRestart) {
        params.endIndex=reverseItems ? numTotalEntries-1 : numAllowedEntries-1;
        params.startIndex=params.endIndex-numAllowedEntries+1;
    }
    return params.selectedEntry;
}

void TestPopupMenuSimple(const char *scrollUpEntryText, const char *scrollDownEntryText) {
    // Recent Files-like menu
    static const char* recentFileList[] = {"filename01","filename02","filename03","filename04","filename05","filename06","filename07","filename08","filename09","filename10"};

    static PopupMenuSimpleParams pmsParams;
    pmsParams.open|= ImGui::GetIO().MouseClicked[1];// RIGHT CLICK
    const int selectedEntry = PopupMenuSimple(pmsParams,recentFileList,(int) sizeof(recentFileList)/sizeof(recentFileList[0]),5,true,"RECENT FILES",scrollUpEntryText,scrollDownEntryText);
    if (selectedEntry>=0) {
        // Do something: clicked entries[selectedEntry]
    }
}

inline static void ClampColor(ImVec4& color)    {
    float* pf;
    pf = &color.x;if (*pf<0) *pf=0;if (*pf>1) *pf=1;
    pf = &color.y;if (*pf<0) *pf=0;if (*pf>1) *pf=1;
    pf = &color.z;if (*pf<0) *pf=0;if (*pf>1) *pf=1;
    pf = &color.w;if (*pf<0) *pf=0;if (*pf>1) *pf=1;
}

// Based on the code from: https://github.com/benoitjacquier/imgui
inline static bool ColorChooserInternal(ImVec4 *pColorOut,bool supportsAlpha,bool showSliders,ImGuiWindowFlags extra_flags=0,bool* pisAnyItemActive=NULL,float windowWidth = 180/*,bool isCombo = false*/)    {
    bool colorSelected = false;
    if (pisAnyItemActive) *pisAnyItemActive=false;
    //const bool isCombo = (extra_flags&ImGuiWindowFlags_ComboBox);

    ImVec4 color = pColorOut ? *pColorOut : ImVec4(0,0,0,1);
    if (!supportsAlpha) color.w=1.f;

    const float smallWidth = windowWidth/9.f;

    static const ImU32 black = ColorConvertFloat4ToU32(ImVec4(0,0,0,1));
    static const ImU32 white = ColorConvertFloat4ToU32(ImVec4(1,1,1,1));
    static float hue, sat, val;

    ImGui::ColorConvertRGBtoHSV( color.x, color.y, color.z, hue, sat, val );


    ImGuiWindow* colorWindow = GetCurrentWindow();

    const float quadSize = windowWidth - smallWidth - colorWindow->WindowPadding.x*2;
    //if (isCombo) ImGui::SetCursorPosX(ImGui::GetCursorPos().x+colorWindow->WindowPadding.x);
    // Hue Saturation Value
    if (ImGui::BeginChild("ValueSaturationQuad##ValueSaturationQuadColorChooser", ImVec2(quadSize, quadSize), false,extra_flags ))
    //ImGui::BeginGroup();
    {
        const int step = 5;
        ImVec2 pos = ImVec2(0, 0);
        ImGuiWindow* window = GetCurrentWindow();

        ImVec4 c00(1, 1, 1, 1);
        ImVec4 c10(1, 1, 1, 1);
        ImVec4 c01(1, 1, 1, 1);
        ImVec4 c11(1, 1, 1, 1);
        for (int y = 0; y < step; y++) {
            for (int x = 0; x < step; x++) {
                float s0 = (float)x / (float)step;
                float s1 = (float)(x + 1) / (float)step;
                float v0 = 1.0 - (float)(y) / (float)step;
                float v1 = 1.0 - (float)(y + 1) / (float)step;


                ImGui::ColorConvertHSVtoRGB(hue, s0, v0, c00.x, c00.y, c00.z);
                ImGui::ColorConvertHSVtoRGB(hue, s1, v0, c10.x, c10.y, c10.z);
                ImGui::ColorConvertHSVtoRGB(hue, s0, v1, c01.x, c01.y, c01.z);
                ImGui::ColorConvertHSVtoRGB(hue, s1, v1, c11.x, c11.y, c11.z);

                window->DrawList->AddRectFilledMultiColor(window->Pos + pos, window->Pos + pos + ImVec2(quadSize / step, quadSize / step),
                                                          ImGui::ColorConvertFloat4ToU32(c00),
                                                          ImGui::ColorConvertFloat4ToU32(c10),
                                                          ImGui::ColorConvertFloat4ToU32(c11),
                                                          ImGui::ColorConvertFloat4ToU32(c01));

                pos.x += quadSize / step;
            }
            pos.x = 0;
            pos.y += quadSize / step;
        }

        window->DrawList->AddCircle(window->Pos + ImVec2(sat, 1-val)*quadSize, 4, val<0.5f?white:black, 4);

        const ImGuiID id = window->GetID("ValueSaturationQuad");
        ImRect bb(window->Pos, window->Pos + window->Size);
        bool hovered, held;
        /*bool pressed = */ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_NoKeyModifiers);///*false,*/ false);
        if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if (held)   {
            ImVec2 pos = ImGui::GetIO().MousePos - window->Pos;
            sat = ImSaturate(pos.x / (float)quadSize);
            val = 1 - ImSaturate(pos.y / (float)quadSize);
            ImGui::ColorConvertHSVtoRGB(hue, sat, val, color.x, color.y, color.z);
            colorSelected = true;
        }

    }
    ImGui::EndChild();	// ValueSaturationQuad
    //ImGui::EndGroup();

    ImGui::SameLine();

    //if (isCombo) ImGui::SetCursorPosX(ImGui::GetCursorPos().x+colorWindow->WindowPadding.x+quadSize);

    //Vertical tint
    if (ImGui::BeginChild("Tint##TintColorChooser", ImVec2(smallWidth, quadSize), false,extra_flags))
    //ImGui::BeginGroup();
    {
        const int step = 8;
        const int width = (int)smallWidth;
        ImGuiWindow* window = GetCurrentWindow();
        ImVec2 pos(0, 0);
        ImVec4 c0(1, 1, 1, 1);
        ImVec4 c1(1, 1, 1, 1);
        for (int y = 0; y < step; y++) {
            float tint0 = (float)(y) / (float)step;
            float tint1 = (float)(y + 1) / (float)step;
            ImGui::ColorConvertHSVtoRGB(tint0, 1.0, 1.0, c0.x, c0.y, c0.z);
            ImGui::ColorConvertHSVtoRGB(tint1, 1.0, 1.0, c1.x, c1.y, c1.z);

            window->DrawList->AddRectFilledMultiColor(window->Pos + pos, window->Pos + pos + ImVec2(width, quadSize / step),
                                                      ColorConvertFloat4ToU32(c0),
                                                      ColorConvertFloat4ToU32(c0),
                                                      ColorConvertFloat4ToU32(c1),
                                                      ColorConvertFloat4ToU32(c1));

            pos.y += quadSize / step;
        }

        window->DrawList->AddCircle(window->Pos + ImVec2(smallWidth*0.5f, hue*quadSize), 4, black, 4);
        //window->DrawList->AddLine(window->Pos + ImVec2(0, hue*quadSize), window->Pos + ImVec2(width, hue*quadSize), ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)));
        bool hovered, held;
        const ImGuiID id = window->GetID("Tint");
        ImRect bb(window->Pos, window->Pos + window->Size);
        /*bool pressed = */ButtonBehavior(bb, id, &hovered, &held,ImGuiButtonFlags_NoKeyModifiers);// /*false,*/ false);
        if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if (held)
        {

            ImVec2 pos = ImGui::GetIO().MousePos - window->Pos;
            hue = ImClamp( pos.y / (float)quadSize, 0.0f, 1.0f );
            ImGui::ColorConvertHSVtoRGB( hue, sat, val, color.x, color.y, color.z );
            colorSelected = true;
        }

    }
    ImGui::EndChild(); // "Tint"
    //ImGui::EndGroup();

    if (showSliders)
    {
        //Sliders
        //ImGui::PushItemHeight();
        //if (isCombo) ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x+colorWindow->WindowPadding.x,ImGui::GetCursorPos().y+colorWindow->WindowPadding.y+quadSize));
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Sliders");
        static bool useHsvSliders = false;
        static const char* btnNames[2] = {"to HSV","to RGB"};
        const int index = useHsvSliders?1:0;
        ImGui::SameLine();
        if (ImGui::SmallButton(btnNames[index])) useHsvSliders=!useHsvSliders;

        ImGui::Separator();
        const ImVec2 sliderSize = /*isCombo ? ImVec2(-1,quadSize) : */ImVec2(-1,-1);
        if (ImGui::BeginChild("Sliders##SliderColorChooser", sliderSize, false,extra_flags))
        {


            {
                int r = ImSaturate( useHsvSliders ? hue : color.x )*255.f;
                int g = ImSaturate( useHsvSliders ? sat : color.y )*255.f;
                int b = ImSaturate( useHsvSliders ? val : color.z )*255.f;
                int a = ImSaturate( color.w )*255.f;

                static const char* names[2][3]={{"R","G","B"},{"H","S","V"}};
                bool sliderMoved = false;
                sliderMoved|= ImGui::SliderInt(names[index][0], &r, 0, 255);
                sliderMoved|= ImGui::SliderInt(names[index][1], &g, 0, 255);
                sliderMoved|= ImGui::SliderInt(names[index][2], &b, 0, 255);
                sliderMoved|= (supportsAlpha && ImGui::SliderInt("A", &a, 0, 255));
                if (sliderMoved)
                {
                    colorSelected = true;
                    color.x = (float)r/255.f;
                    color.y = (float)g/255.f;
                    color.z = (float)b/255.f;
                    if (useHsvSliders)  ImGui::ColorConvertHSVtoRGB(color.x,color.y,color.z,color.x,color.y,color.z);
                    if (supportsAlpha) color.w = (float)a/255.f;
                }
                //ColorConvertRGBtoHSV(s_color.x, s_color.y, s_color.z, tint, sat, val);*/
                if (pisAnyItemActive) *pisAnyItemActive|=sliderMoved;
            }


        }
        ImGui::EndChild();
    }

    if (colorSelected && pColorOut) *pColorOut = color;

    return colorSelected;
}

// Based on the code from: https://github.com/benoitjacquier/imgui
bool ColorChooser(bool* open,ImVec4 *pColorOut,bool supportsAlpha)   {
    static bool lastOpen = false;
    static const ImVec2 windowSize(175,285);

    if (open && !*open) {lastOpen=false;return false;}
    if (open && *open && *open!=lastOpen) {
        ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
        ImGui::SetNextWindowSize(windowSize);
        lastOpen=*open;
    }

    //ImGui::OpenPopup("Color Chooser##myColorChoserPrivate");

    bool colorSelected = false;

    ImGuiWindowFlags WindowFlags = 0;
    //WindowFlags |= ImGuiWindowFlags_NoTitleBar;
    WindowFlags |= ImGuiWindowFlags_NoResize;
    //WindowFlags |= ImGuiWindowFlags_NoMove;
    WindowFlags |= ImGuiWindowFlags_NoScrollbar;
    WindowFlags |= ImGuiWindowFlags_NoCollapse;
    WindowFlags |= ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,2));

    if (open) ImGui::SetNextWindowFocus();
    //if (ImGui::BeginPopupModal("Color Chooser##myColorChoserPrivate",open,WindowFlags))
    //if (ImGui::Begin("Color Chooser##myColorChoserPrivate",open,windowSize,-1.f,WindowFlags)) // Old API
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Color Chooser##myColorChoserPrivate",open,WindowFlags))
    {
        colorSelected = ColorChooserInternal(pColorOut,supportsAlpha,true);

        //ImGui::EndPopup();
    }
    ImGui::End();

    ImGui::PopStyleVar(2);

    return colorSelected;

}

// Based on the code from: https://github.com/benoitjacquier/imgui
bool ColorCombo(const char* label,ImVec4 *pColorOut,bool supportsAlpha,float width,bool closeWhenMouseLeavesIt)    {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    /*typedef struct _ImGuiCppDuply {
        // The meaning of this struct is to expose some internal imgui.cpp methods (not exposed by imgui_internal.h),
        // so that this .cpp file can be used directly (even without the IMGUI_INCLUDE_IMGUI_USER_H / IMGUI_INCLUDE_IMGUI_USER_INL mechanism).

        // I know it's a bit crazy... but this addon is huge and it's a pity to prevent free usage just because of ColorCombo()...

        static bool IsPopupOpen(ImGuiID id) {
            ImGuiContext& g = *ImGui::GetCurrentContext();
            return g.OpenPopupStack.Size > g.BeginPopupStack.Size && g.OpenPopupStack[g.BeginPopupStack.Size].PopupId == id;
        }
        static void ClosePopupToLevel(int remaining)    {
            ImGuiContext& g = *ImGui::GetCurrentContext();
            if (remaining > 0)
                ImGui::FocusWindow(g.OpenPopupStack[remaining-1].Window);
            else
                ImGui::FocusWindow(g.OpenPopupStack[0].ParentWindow);
            g.OpenPopupStack.resize(remaining);
        }
        static void ClosePopup(ImGuiID id)  {
            if (!IsPopupOpen(id))
                return;
            ImGuiContext& g = *ImGui::GetCurrentContext();
            ClosePopupToLevel(g.OpenPopupStack.Size - 1);
        }
        static bool BeginPopupEx(const char* str_id, ImGuiWindowFlags extra_flags)  {
            ImGuiContext& g = *ImGui::GetCurrentContext();
            if (g.OpenPopupStack.Size <= g.BeginPopupStack.Size) // Early out for performance
            {
                g.NextWindowData.Clear(); // We behave like Begin() and need to consume those values
                return false;
            }
            ImGuiWindowFlags flags = extra_flags|ImGuiWindowFlags_Popup|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_AlwaysAutoResize;
            return ImGui::BeginPopupEx(g.CurrentWindow->GetID(str_id), flags);
        }
    } ImGuiCppDuply;*/

#if 0
    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const float itemWidth = width>=0 ? width : ImGui::CalcItemWidth();
    const ImVec2 label_size = ImGui::CalcTextSize(label);
    const float color_quad_size = (g.FontSize + style.FramePadding.x);
    const float arrow_size = (g.FontSize + style.FramePadding.x * 2.0f);
    ImVec2 totalSize = ImVec2(label_size.x+color_quad_size+arrow_size, label_size.y) + style.FramePadding*2.0f;
    if (totalSize.x < itemWidth) totalSize.x = itemWidth;
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + totalSize);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max);// + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id)) return false;
    const float windowWidth = frame_bb.Max.x - frame_bb.Min.x - style.FramePadding.x;


    ImVec4 color = pColorOut ? *pColorOut : ImVec4(0,0,0,1);
    if (!supportsAlpha) color.w=1.f;

    const bool hovered = ItemHoverable(frame_bb, id);

    const ImRect value_bb(frame_bb.Min, frame_bb.Max - ImVec2(arrow_size, 0.0f));
    RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
    RenderFrame(frame_bb.Min,ImVec2(frame_bb.Min.x+color_quad_size,frame_bb.Max.y), ImColor(style.Colors[ImGuiCol_Text]), true, style.FrameRounding);
    RenderFrame(ImVec2(frame_bb.Min.x+1,frame_bb.Min.y+1), ImVec2(frame_bb.Min.x+color_quad_size-1,frame_bb.Max.y-1),
                ImGui::ColorConvertFloat4ToU32(ImVec4(color.x,color.y,color.z,1.f)),
                true, style.FrameRounding);

    RenderFrame(ImVec2(frame_bb.Max.x-arrow_size, frame_bb.Min.y), frame_bb.Max, GetColorU32(hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button), true, style.FrameRounding); // FIXME-ROUNDING
    RenderArrow(ImVec2(frame_bb.Max.x-arrow_size, frame_bb.Min.y) + style.FramePadding, ImGuiDir_Down);

    RenderTextClipped(ImVec2(frame_bb.Min.x+color_quad_size,frame_bb.Min.y) + style.FramePadding, value_bb.Max, label, NULL, NULL);

    if (hovered)
    {
        SetHoveredID(id);
        if (g.IO.MouseClicked[0])
        {
            ClearActiveID();
            if (ImGui::IsPopupOpen(id))
            {
                ClosePopupToLevel(g.OpenPopupStack.Size - 1,true);
            }
            else
            {
                FocusWindow(window);
                ImGui::OpenPopup(label);
            }
        }
        static ImVec4 copiedColor(1,1,1,1);
        static const ImVec4* pCopiedColor = NULL;
        if (g.IO.MouseClicked[1]) { // right-click (copy color)
            copiedColor = color;
            pCopiedColor = &copiedColor;
            //fprintf(stderr,"Copied\n");
        }
        else if (g.IO.MouseClicked[2] && pCopiedColor && pColorOut) { // middle-click (paste color)
            pColorOut->x = pCopiedColor->x;
            pColorOut->y = pCopiedColor->y;
            pColorOut->z = pCopiedColor->z;
            if (supportsAlpha) pColorOut->w = pCopiedColor->w;
            color = *pColorOut;
            //fprintf(stderr,"Pasted\n");
        }
    }

    bool value_changed = false;
    if (ImGui::IsPopupOpen(id))
    {
        ImRect popup_rect(ImVec2(frame_bb.Min.x, frame_bb.Max.y), ImVec2(frame_bb.Max.x, frame_bb.Max.y));
        //popup_rect.Max.y = ImMin(popup_rect.Max.y, g.IO.DisplaySize.y - style.DisplaySafeAreaPadding.y); // Adhoc height limit for Combo. Ideally should be handled in Begin() along with other popups size, we want to have the possibility of moving the popup above as well.
        ImGui::SetNextWindowPos(popup_rect.Min);
        ImGui::SetNextWindowSize(ImVec2(windowWidth,-1));//popup_rect.GetSize());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.FramePadding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,2));

        bool mustCloseCombo = false;
        const ImGuiWindowFlags flags =  0;//ImGuiWindowFlags_Modal;//ImGuiWindowFlags_ComboBox;  // ImGuiWindowFlags_ComboBox is no more available... what now ?
        if (ImGui::BeginPopup(label, flags))
        {
            bool comboItemActive = false;
            value_changed = ColorChooserInternal(pColorOut,supportsAlpha,false,flags,&comboItemActive,windowWidth/*,true*/);
            if (closeWhenMouseLeavesIt && !comboItemActive)
            {
                const float distance = g.FontSize*1.75f;//1.3334f;//24;
                //fprintf(stderr,"%1.f",distance);
                ImVec2 pos = ImGui::GetWindowPos();pos.x-=distance;pos.y-=distance;
                ImVec2 size = ImGui::GetWindowSize();
                size.x+=2.f*distance;
                size.y+=2.f*distance+windowWidth*8.f/9.f;   // problem: is seems that ImGuiWindowFlags_ComboBox does not return the full window height
                const ImVec2& mousePos = ImGui::GetIO().MousePos;
                if (mousePos.x<pos.x || mousePos.y<pos.y || mousePos.x>pos.x+size.x || mousePos.y>pos.y+size.y) {
                    mustCloseCombo = true;
                    //fprintf(stderr,"Leaving ColorCombo: pos(%1f,%1f) size(%1f,%1f)\n",pos.x,pos.y,size.x,size.y);
                }
            }
            ImGui::EndPopup();
        }
        if (mustCloseCombo && ImGui::IsPopupOpen(id)) {
            ClosePopupToLevel(g.OpenPopupStack.Size - 1,true);
        }
        ImGui::PopStyleVar(3);
    }
    return value_changed;
#endif

	return false;
}


// Based on the code from: https://github.com/Roflraging (see https://github.com/ocornut/imgui/issues/383)
struct MultilineScrollState {
    // Input.
    float scrollRegionX;
    float scrollX;
    ImGuiStorage *storage;
    const char* textToPasteInto;
    int actionToPerformCopyCutSelectAllFrom1To3;

    // Output.
    bool newScrollPositionAvailable;
    float newScrollX;
    int CursorPos;
    int SelectionStart; //                                      // Read (== to SelectionEnd when no selection)
    int SelectionEnd;   //                                      // Read
};
// Based on the code from: https://github.com/Roflraging (see https://github.com/ocornut/imgui/issues/383)
static int MultilineScrollCallback(ImGuiInputTextCallbackData *data) {
    //static int cnt=0;fprintf(stderr,"MultilineScrollCallback (%d)\n",++cnt);
    MultilineScrollState *scrollState = (MultilineScrollState *)data->UserData;

    ImGuiID cursorId = ImGui::GetID("cursor");
    int oldCursorIndex = scrollState->storage->GetInt(cursorId, 0);

    if (oldCursorIndex != data->CursorPos)  {
        int begin = data->CursorPos;

        while ((begin > 0) && (data->Buf[begin - 1] != '\n'))   {
            --begin;
        }

        float cursorOffset = ImGui::CalcTextSize(data->Buf + begin, data->Buf + data->CursorPos).x;
        float SCROLL_INCREMENT = scrollState->scrollRegionX * 0.25f;

        if (cursorOffset < scrollState->scrollX)    {
            scrollState->newScrollPositionAvailable = true;
            scrollState->newScrollX = cursorOffset - SCROLL_INCREMENT; if (scrollState->newScrollX<0) scrollState->newScrollX=0;
        }
        else if ((cursorOffset - scrollState->scrollRegionX) >= scrollState->scrollX)   {
            scrollState->newScrollPositionAvailable = true;
            scrollState->newScrollX = cursorOffset - scrollState->scrollRegionX + SCROLL_INCREMENT;
        }
    }

    scrollState->storage->SetInt(cursorId, data->CursorPos);

    scrollState->CursorPos = data->CursorPos;
    if (data->SelectionStart<=data->SelectionEnd) {scrollState->SelectionStart = data->SelectionStart;scrollState->SelectionEnd = data->SelectionEnd;}
    else {scrollState->SelectionStart = data->SelectionEnd;scrollState->SelectionEnd = data->SelectionStart;}

    return 0;
}
// Based on the code from: https://github.com/Roflraging (see https://github.com/ocornut/imgui/issues/383)
bool InputTextMultilineWithHorizontalScrolling(const char* label, char* buf, size_t buf_size, float height, ImGuiInputTextFlags flags, bool* pOptionalIsHoveredOut, int *pOptionalCursorPosOut, int *pOptionalSelectionStartOut, int *pOptionalSelectionEndOut,float SCROLL_WIDTH)  {
    float scrollbarSize = ImGui::GetStyle().ScrollbarSize;
    //float labelWidth = ImGui::CalcTextSize(label).x + scrollbarSize;
    MultilineScrollState scrollState = {};

    // Set up child region for horizontal scrolling of the text box.
    ImGui::BeginChild(label, ImVec2(0/*-labelWidth*/, height), false, ImGuiWindowFlags_HorizontalScrollbar);
    scrollState.scrollRegionX = ImGui::GetWindowWidth() - scrollbarSize; if (scrollState.scrollRegionX<0) scrollState.scrollRegionX = 0;
    scrollState.scrollX = ImGui::GetScrollX();
    scrollState.storage = ImGui::GetStateStorage();
    bool changed = ImGui::InputTextMultiline(label, buf, buf_size, ImVec2(SCROLL_WIDTH-scrollbarSize, (height - scrollbarSize)>0?(height - scrollbarSize):0),
                                             flags | ImGuiInputTextFlags_CallbackAlways, MultilineScrollCallback, &scrollState);
    if (pOptionalIsHoveredOut) *pOptionalIsHoveredOut = ImGui::IsItemHovered();

    if (scrollState.newScrollPositionAvailable) {
        ImGui::SetScrollX(scrollState.newScrollX);
    }

    ImGui::EndChild();
    //ImGui::SameLine();
    //ImGui::Text("%s",label);

    if (pOptionalCursorPosOut) *pOptionalCursorPosOut = scrollState.CursorPos;
    if (pOptionalSelectionStartOut) *pOptionalSelectionStartOut = scrollState.SelectionStart;
    if (pOptionalSelectionEndOut)   *pOptionalSelectionEndOut = scrollState.SelectionEnd;

    return changed;
}

// Based on the code from: https://github.com/Roflraging (see https://github.com/ocornut/imgui/issues/383)
bool InputTextMultilineWithHorizontalScrollingAndCopyCutPasteMenu(const char *label, char *buf, int buf_size, float height,bool& staticBoolVar,int *staticArrayOfThreeIntegersHere, ImGuiInputTextFlags flags, bool *pOptionalHoveredOut,float SCROLL_WIDTH, const char *copyName, const char *cutName, const char* pasteName)   {
    bool isHovered=false;
    int& cursorPos=staticArrayOfThreeIntegersHere[0];
    int& selectionStart=staticArrayOfThreeIntegersHere[1];
    int& selectionEnd=staticArrayOfThreeIntegersHere[2];
    bool& popup_open = staticBoolVar;
    const bool changed = InputTextMultilineWithHorizontalScrolling(label,buf,(size_t)buf_size,height,flags,&isHovered,popup_open ? NULL : &cursorPos,popup_open ? NULL : &selectionStart,popup_open ? NULL : &selectionEnd,SCROLL_WIDTH);
    if (pOptionalHoveredOut) *pOptionalHoveredOut=isHovered;
    // Popup Menu ------------------------------------------

    const bool readOnly = flags&ImGuiInputTextFlags_ReadOnly;       // "Cut","","Paste" not available
    const bool hasSelectedText = selectionStart != selectionEnd;	// "Copy","Cut" available

    if (hasSelectedText || !readOnly)	{
        const bool onlyPaste = !readOnly && !hasSelectedText;
        const char* clipboardText = ImGui::GetIO().GetClipboardTextFn(NULL);
        const bool canPaste = clipboardText && strlen(clipboardText)>0;
        if (onlyPaste && !canPaste) popup_open = false;
        else {
            static const char* entries[] = {"Copy","Cut","","Paste"};   // "" is separator
            const char* pEntries[4]={copyName?copyName:entries[0],cutName?cutName:entries[1],entries[2],pasteName?pasteName:entries[3]};
            popup_open|= ImGui::GetIO().MouseClicked[1] && isHovered; // RIGHT CLICK
            int sel = ImGui::PopupMenuSimple(popup_open,onlyPaste ? &pEntries[3] : pEntries,(readOnly||onlyPaste)?1:canPaste? 4:2);
            if (sel==3) sel = 2; // Normally separator takes out one space
            const bool mustCopy = sel==0 && !onlyPaste;
            const bool mustCut = !mustCopy && sel==1;
            const bool mustPaste = !mustCopy && !mustCut && (sel==2 || (sel==0 && onlyPaste));
            if (mustCopy || mustCut || (mustPaste && (selectionStart<selectionEnd))) {
                // Copy to clipboard
                if (!mustPaste)	{
                    const char tmp = buf[selectionEnd];buf[selectionEnd]='\0';
                    ImGui::GetIO().SetClipboardTextFn(NULL,&buf[selectionStart]);
                    buf[selectionEnd]=tmp;
                }
                // Delete chars
                if (!mustCopy) {
                    //if (mustPaste) {fprintf(stderr,"Deleting before pasting: %d  %d.\n",selectionStart,selectionEnd);}

		    //strncpy(&buf[selectionStart],&buf[selectionEnd],buf_size-selectionEnd);				// Valgrind complains here, but I KNOW that source and destination overlap: I just want to shift chars to the left!
		    for (int i=0,isz=buf_size-selectionEnd;i<isz;i++) buf[i+selectionStart]=buf[i+selectionEnd];// I do it manually, so Valgrind is happy

		    for (int i=selectionStart+buf_size-selectionEnd;i<buf_size;i++) buf[i]='\0';		// This is mandatory at the end
                }
                popup_open = false;
            }
            if (mustPaste)  {
                // This is VERY HARD to make it work as expected...
                const int cursorPosition = (selectionStart<selectionEnd) ? selectionStart : cursorPos;
                const int clipboardTextSize = strlen(clipboardText);
                int buf_len = strlen(buf);if (buf_len>buf_size) buf_len=buf_size;

                // Step 1- Shift [cursorPosition] to [cursorPosition+clipboardTextSize]
                const int numCharsToShiftRight = buf_len - cursorPosition;
                //fprintf(stderr,"Pasting: \"%s\"(%d) at %d. buf_len=%d buf_size=%d numCharsToShiftRight=%d\n",clipboardText,clipboardTextSize,cursorPosition,buf_len,buf_size,numCharsToShiftRight);

                for (int i=cursorPosition+numCharsToShiftRight>buf_size?buf_size-1:cursorPosition+numCharsToShiftRight-1;i>=cursorPosition;i--) {
                    if (i+clipboardTextSize<buf_size) {
                        //fprintf(stderr,"moving to the right char (%d): '%c' (%d)\n",i,buf[i],(int)buf[i]);
                        buf[i+clipboardTextSize] = buf[i];
                    }
                }
                // Step 2- Overwrite [cursorPosition] o [cursorPosition+clipboardTextSize]
                for (int i=cursorPosition,isz=cursorPosition+clipboardTextSize>=buf_size?buf_size:cursorPosition+clipboardTextSize;i<isz;i++) buf[i]=clipboardText[i-cursorPosition];

                popup_open = false;
            }
        }	 
    }
    else popup_open = false;
    //------------------------------------------------------------------
    return changed;
}


bool ImageButtonWithText(ImTextureID texId,const char* label,const ImVec2& imageSize, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col) {
#if 0
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
    return false;

    ImVec2 size = imageSize;
    if (size.x<=0 && size.y<=0) {size.x=size.y=ImGui::GetTextLineHeightWithSpacing();}
    else {
        if (size.x<=0)          size.x=size.y;
        else if (size.y<=0)     size.y=size.x;
        size*=window->FontWindowScale*ImGui::GetIO().FontGlobalScale;
    }

    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;

    const ImGuiID id = window->GetID(label);
    const ImVec2 textSize = ImGui::CalcTextSize(label,NULL,true);
    const bool hasText = textSize.x>0;

    const float innerSpacing = hasText ? ((frame_padding >= 0) ? (float)frame_padding : (style.ItemInnerSpacing.x)) : 0.f;
    const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
    const ImVec2 totalSizeWithoutPadding(size.x+innerSpacing+textSize.x,size.y>textSize.y ? size.y : textSize.y);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + totalSizeWithoutPadding + padding*2);
    ImVec2 start(0,0);
    start = window->DC.CursorPos + padding;if (size.y<textSize.y) start.y+=(textSize.y-size.y)*.5f;
    const ImRect image_bb(start, start + size);
    start = window->DC.CursorPos + padding;start.x+=size.x+innerSpacing;if (size.y>textSize.y) start.y+=(size.y-textSize.y)*.5f;
    ItemSize(bb);
    if (!ItemAdd(bb, id))
    return false;

    bool hovered=false, held=false;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    // Render
    const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
    if (bg_col.w > 0.0f)
    window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));

    window->DrawList->AddImage(texId, image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(tint_col));

    if (textSize.x>0) ImGui::RenderText(start,label);
    return pressed;
#endif
	return false;
}

#ifndef NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE
// Now this struct cannot be used without stb_image.h anymore, even if no gif support is required,
// because it uses STBI_MALLOC and STBI_FREE
struct AnimatedImageInternal {
    protected:

    int w,h,frames;
    unsigned char* buffer;                    // Allocated with STBI_MALLOC: thus stb_image.h is always required now
    ImVector<float> delays;                   // Currently in cs (but now stb_image.h gives us ms)
    ImTextureID persistentTexId;              // This will be used when all frames can fit into a single texture (very good for performance and memory)
    int numFramesPerRowInPersistentTexture,numFramesPerColInPersistentTexture;
    bool hoverModeIfSupported;
    bool persistentTexIdIsNotOwned;
    mutable bool isAtLeastOneWidgetInHoverMode;  // internal

    mutable int lastFrameNum;
    mutable float delay;
    mutable float timer;
    mutable ImTextureID texId;
    mutable ImVec2 uvFrame0,uvFrame1;   // used by persistentTexId
    mutable int lastImGuiFrameUpdate;

    inline void updateTexture() const   {
        // fix updateTexture() to use persistentTexID when necessary
        IM_ASSERT(AnimatedImage::GenerateOrUpdateTextureCb!=NULL);	// Please use ImGui::AnimatedImage::SetGenerateOrUpdateTextureCallback(...) before calling this method
        if (frames<=0) return;
        else if (frames==1) {
            if (!texId) AnimatedImage::GenerateOrUpdateTextureCb(texId,w,h,4,buffer,false,false,false);
            return;
        }

        // These two lines sync animation in multiple items:
        if (texId && lastImGuiFrameUpdate==ImGui::GetFrameCount()) return;
        lastImGuiFrameUpdate=ImGui::GetFrameCount();
        if (hoverModeIfSupported && !isAtLeastOneWidgetInHoverMode) {
            // reset animation here:
            timer=-1.f;lastFrameNum=-1;delay=0;
            //calculateTexCoordsForFrame(0,uvFrame0,uvFrame1);
        }
        isAtLeastOneWidgetInHoverMode = false;

        float lastDelay = delay;
        if (timer>0) {
            delay = ImGui::GetTime()*100.f-timer;
            if (delay<0) timer = -1.f;
        }
        if (timer<0) {timer = ImGui::GetTime()*100.f;delay=0.f;}

        const int imageSz = 4 * w * h;
        IM_ASSERT(sizeof(unsigned short)==2*sizeof(unsigned char));
        bool changed = false;
        float frameTime=0.f;
        bool forceUpdate = false;
        if (lastFrameNum<0) {forceUpdate=true;lastFrameNum=0;}
        for (int i=lastFrameNum;i<frames;i++)   {
            frameTime = delays[i];
            //fprintf(stderr,"%d/%d) %1.2f\n",i,frames,frameTime);
            if (delay <= lastDelay+frameTime) {
                changed = (i!=lastFrameNum || !texId);
                lastFrameNum = i;
                if (changed || forceUpdate)    {
                    if (!persistentTexId) AnimatedImage::GenerateOrUpdateTextureCb(texId,w,h,4,&buffer[imageSz*i],false,false,false);
                    else {
                        texId = persistentTexId;
                        // calculate uvFrame0 and uvFrame1 here based on 'i' and numFramesPerRowInPersistentTexture,numFramesPerColInPersistentTexture
                        calculateTexCoordsForFrame(i,uvFrame0,uvFrame1);
                    }
                }
                //fprintf(stderr,"%d/%d) %1.2f %1.2f %1.2f\n",i,frames,frameTime,delay,lastDelay);
                delay = lastDelay;
                return;
            }
            lastDelay+=frameTime;
            if (i==frames-1) i=-1;
        }

    }

#   ifndef IMGUIVARIOUSCONTROLS_NO_STDIO
    struct ScopedFileContent {
        stbi_uc* gif_buffer;
        int gif_buffer_size;
        static stbi_uc* GetFileContent(const char *filePath,int* size_out)   {
            stbi_uc* f_data = NULL;FILE* f=NULL;long f_size=-1;size_t f_size_read=0;*size_out=0;
            if (!filePath || (f = ImFileOpen(filePath, "rb")) == NULL) return NULL;
            if (fseek(f, 0, SEEK_END) ||  (f_size = ftell(f)) == -1 || fseek(f, 0, SEEK_SET))  {fclose(f);return NULL;}
            f_data = (stbi_uc*) STBI_MALLOC(f_size);
            f_size_read = f_size>0 ? fread(f_data, 1, f_size, f) : 0;
            fclose(f);
            if (f_size_read == 0 || f_size_read!=(size_t)f_size)  {STBI_FREE(f_data);return NULL;}
            *size_out=(int)f_size;
            return f_data;
        }
        ScopedFileContent(const char* filePath) {gif_buffer=GetFileContent(filePath,&gif_buffer_size);}
        ~ScopedFileContent() {if (gif_buffer) {STBI_FREE(gif_buffer);gif_buffer=NULL;} gif_buffer_size=0;}
    };
#   endif //IMGUIVARIOUSCONTROLS_NO_STDIO

    public:
    AnimatedImageInternal()  {buffer=NULL;persistentTexIdIsNotOwned=false;texId=persistentTexId=NULL;clear();}
    ~AnimatedImageInternal()  {clear();persistentTexIdIsNotOwned=false;}
#	ifndef STBI_NO_GIF
#   ifndef IMGUIVARIOUSCONTROLS_NO_STDIO
    AnimatedImageInternal(char const *filename,bool useHoverModeIfSupported=false)  {buffer=NULL;persistentTexIdIsNotOwned = false;texId=persistentTexId=NULL;load(filename,useHoverModeIfSupported);}
#   endif //IMGUIVARIOUSCONTROLS_NO_STDIO
    AnimatedImageInternal(const unsigned char* memory_gif,int memory_gif_size,bool useHoverModeIfSupported=false)  {buffer=NULL;persistentTexIdIsNotOwned = false;texId=persistentTexId=NULL;load_from_memory(memory_gif,memory_gif_size,useHoverModeIfSupported);}
#	endif //STBI_NO_GIF
    AnimatedImageInternal(ImTextureID myTexId,int animationImageWidth,int animationImageHeight,int numFrames,int numFramesPerRowInTexture,int numFramesPerColumnInTexture,float delayDetweenFramesInCs,bool useHoverMode=false) {
        buffer=NULL;persistentTexIdIsNotOwned = false;texId=persistentTexId=NULL;
        create(myTexId,animationImageWidth,animationImageHeight,numFrames,numFramesPerRowInTexture,numFramesPerColumnInTexture,delayDetweenFramesInCs,useHoverMode);
    }
    void clear() {
        w=h=frames=lastFrameNum=0;delay=0.f;timer=-1.f;
        if (buffer) {STBI_FREE(buffer);buffer=NULL;} delays.clear();
        numFramesPerRowInPersistentTexture = numFramesPerColInPersistentTexture = 0;
        uvFrame0.x=uvFrame0.y=0;uvFrame1.x=uvFrame1.y=1;
        lastImGuiFrameUpdate = -1;hoverModeIfSupported=isAtLeastOneWidgetInHoverMode = false;
        if (texId || persistentTexId) IM_ASSERT(AnimatedImage::FreeTextureCb!=NULL);   // Please use ImGui::AnimatedImage::SetFreeTextureCallback(...)
        if (texId) {if (texId!=persistentTexId) AnimatedImage::FreeTextureCb(texId);texId=NULL;}
        if (persistentTexId)  {if (!persistentTexIdIsNotOwned) AnimatedImage::FreeTextureCb(persistentTexId);persistentTexId=NULL;}
    }
#	ifndef STBI_NO_GIF
#   ifndef IMGUIVARIOUSCONTROLS_NO_STDIO
    bool load(char const *filename,bool useHoverModeIfSupported=false)  {
        ScopedFileContent fc(filename);
        return (fc.gif_buffer && load_from_memory(fc.gif_buffer,fc.gif_buffer_size,useHoverModeIfSupported));
    }
#   endif //ifndef IMGUIVARIOUSCONTROLS_NO_STDIO
    bool load_from_memory(const unsigned char* gif_buffer,int gif_buffer_size,bool useHoverModeIfSupported=false)  {
        clear();hoverModeIfSupported = false;

        int c=0, *int_delays=NULL;
        buffer = stbi_load_gif_from_memory(gif_buffer,gif_buffer_size,&int_delays,&w,&h,&frames,&c,4);
        if (!buffer || frames<=0 || !int_delays) {clear();return false;}
        //fprintf(stderr,"w=%d h=%d z=%d c=%d\n",w,h,frames,c);

        // copy int_delays into delays
        delays.resize(frames);
        for (int i=0;i<frames;i++) {
            delays[i] = ((float) int_delays[i])*0.1f;   // cs, whereas int_delays is ms
            //fprintf(stderr,"int_delays[%d] = %d;\n",i,int_delays[i]);
        }
        STBI_FREE(int_delays);int_delays=NULL;

        if (AnimatedImage::MaxPersistentTextureSize.x>0 && AnimatedImage::MaxPersistentTextureSize.y>0)	{
            // code path that checks 'MaxPersistentTextureSize' and puts all into a single texture (rearranging the buffer)
            ImVec2 textureSize = AnimatedImage::MaxPersistentTextureSize;
            int maxNumFramesPerRow = (int)textureSize.x/(int) w;
            int maxNumFramesPerCol = (int)textureSize.y/(int) h;
            int maxNumFramesInATexture = maxNumFramesPerRow * maxNumFramesPerCol;
            int cnt = 0;
            ImVec2 lastValidTextureSize(0,0);
            while (maxNumFramesInATexture>=frames)	{
                // Here we just halve the 'textureSize', so that, if it fits, we save further texture space
                lastValidTextureSize = textureSize;
                if (cnt%2==0) textureSize.y = textureSize.y/2;
                else textureSize.x = textureSize.x/2;
                maxNumFramesPerRow = (int)textureSize.x/(int)w;
                maxNumFramesPerCol = (int)textureSize.y/(int)h;
                maxNumFramesInATexture = maxNumFramesPerRow * maxNumFramesPerCol;
                ++cnt;
            }
            if (cnt>0)  {
                textureSize=lastValidTextureSize;
                maxNumFramesPerRow = (int)textureSize.x/(int)w;
                maxNumFramesPerCol = (int)textureSize.y/(int)h;
                maxNumFramesInATexture = maxNumFramesPerRow * maxNumFramesPerCol;
            }

            if (maxNumFramesInATexture>=frames)	{
                numFramesPerRowInPersistentTexture = maxNumFramesPerRow;
                numFramesPerColInPersistentTexture = maxNumFramesPerCol;

                rearrangeBufferForPersistentTexture();

                // generate persistentTexture,delete buffer
                IM_ASSERT(AnimatedImage::GenerateOrUpdateTextureCb!=NULL);	// Please use ImGui::AnimatedImage::SetGenerateOrUpdateTextureCallback(...) before calling this method
                AnimatedImage::GenerateOrUpdateTextureCb(persistentTexId,w*maxNumFramesPerRow,h*maxNumFramesPerCol,4,buffer,false,false,false);
                STBI_FREE(buffer);buffer=NULL;

                hoverModeIfSupported = useHoverModeIfSupported;
                //fprintf(stderr,"%d x %d (%d x %d)\n",numFramesPerRowInPersistentTexture,numFramesPerColInPersistentTexture,(int)textureSize.x,(int)textureSize.y);

                if (hoverModeIfSupported) delays[0] = 0.f;  // Otherwise when we start hovering, we usually get an unwanted delay
            }
        }

        return true;
    }
#	endif //STBI_NO_GIF
    bool create(ImTextureID myTexId,int animationImageWidth,int animationImageHeight,int numFrames,int numFramesPerRowInTexture,int numFramesPerColumnInTexture,float delayDetweenFramesInCs,bool useHoverMode=false)   {
        clear();
        persistentTexIdIsNotOwned = false;
        IM_ASSERT(myTexId);
        IM_ASSERT(animationImageWidth>0 && animationImageHeight>0);
        IM_ASSERT(numFrames>0);
        IM_ASSERT(delayDetweenFramesInCs>0);
        IM_ASSERT(numFramesPerRowInTexture*numFramesPerColumnInTexture>=numFrames);
        if (!myTexId || animationImageWidth<=0 || animationImageHeight<=0
                || numFrames<=0 || delayDetweenFramesInCs<=0 || (numFramesPerRowInTexture*numFramesPerColumnInTexture<numFrames))
            return false;
        persistentTexId = myTexId;
        persistentTexIdIsNotOwned = true;
        w = animationImageWidth;
        h = animationImageHeight;
        frames = numFrames;
        numFramesPerRowInPersistentTexture = numFramesPerRowInTexture;
        numFramesPerColInPersistentTexture = numFramesPerColumnInTexture;
        delays.resize(frames);
        for (int i=0;i<frames;i++) delays[i] = delayDetweenFramesInCs;
        hoverModeIfSupported = useHoverMode;
        return true;
    }

    inline bool areAllFramesInASingleTexture() const {return persistentTexId!=NULL;}
    void render(ImVec2 size=ImVec2(0,0), const ImVec2& uv0=ImVec2(0,0), const ImVec2& uv1=ImVec2(1,1), const ImVec4& tint_col=ImVec4(1,1,1,1), const ImVec4& border_col=ImVec4(0,0,0,0)) const  {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return;
        if (size.x==0) size.x=w;
        else if (size.x<0) size.x=-size.x*w;
        if (size.y==0) size.y=h;
        else if (size.y<0) size.y=-size.y*h;
        size*=window->FontWindowScale*ImGui::GetIO().FontGlobalScale;

        ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        if (border_col.w > 0.0f)
            bb.Max += ImVec2(2,2);
        ItemSize(bb);
        if (!ItemAdd(bb, 0))
            return;

        updateTexture();

        ImVec2 uv_0 = uv0;
        ImVec2 uv_1 = uv1;
        if (persistentTexId) {
            bool hovered = true;	// to fall back when useHoverModeIfSupported == false;
            if (hoverModeIfSupported) {
                hovered = ImGui::IsItemHovered();
                if (hovered) isAtLeastOneWidgetInHoverMode = true;
            }
            if (hovered)	{
                const ImVec2 uvFrameDelta = uvFrame1 - uvFrame0;
                uv_0 = uvFrame0 + uv0*uvFrameDelta;
                uv_1 = uvFrame0 + uv1*uvFrameDelta;
            }
            else {
                // We must use frame zero here:
                ImVec2 uvFrame0,uvFrame1;
                calculateTexCoordsForFrame(0,uvFrame0,uvFrame1);
                const ImVec2 uvFrameDelta = uvFrame1 - uvFrame0;
                uv_0 = uvFrame0 + uv0*uvFrameDelta;
                uv_1 = uvFrame0 + uv1*uvFrameDelta;
            }
        }
        if (border_col.w > 0.0f)
        {
            window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), 0.0f);
            window->DrawList->AddImage(texId, bb.Min+ImVec2(1,1), bb.Max-ImVec2(1,1), uv_0, uv_1, GetColorU32(tint_col));
        }
        else
        {
            window->DrawList->AddImage(texId, bb.Min, bb.Max, uv_0, uv_1, GetColorU32(tint_col));
        }
    }
    bool renderAsButton(const char* label,ImVec2 size=ImVec2(0,0), const ImVec2& uv0 = ImVec2(0,0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1)) const {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        if (size.x==0) size.x=w;
        else if (size.x<0) size.x=-size.x*w;
        if (size.y==0) size.y=h;
        else if (size.y<0) size.y=-size.y*h;
        size*=window->FontWindowScale*ImGui::GetIO().FontGlobalScale;

        ImGuiContext& g = *ImGui::GetCurrentContext();
        const ImGuiStyle& style = g.Style;

        // Default to using texture ID as ID. User can still push string/integer prefixes.
        // We could hash the size/uv to create a unique ID but that would prevent the user from animating UV.
        ImGui::PushID((void *)this);
        const ImGuiID id = window->GetID(label);
        ImGui::PopID();

        const ImVec2 textSize = ImGui::CalcTextSize(label,NULL,true);
        const bool hasText = textSize.x>0;

        const float innerSpacing = hasText ? ((frame_padding >= 0) ? (float)frame_padding : (style.ItemInnerSpacing.x)) : 0.f;
        const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
        const ImVec2 totalSizeWithoutPadding(size.x+innerSpacing+textSize.x,size.y>textSize.y ? size.y : textSize.y);
        const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + totalSizeWithoutPadding + padding*2);
        ImVec2 start(0,0);
        start = window->DC.CursorPos + padding;if (size.y<textSize.y) start.y+=(textSize.y-size.y)*.5f;
        const ImRect image_bb(start, start + size);
        start = window->DC.CursorPos + padding;start.x+=size.x+innerSpacing;if (size.y>textSize.y) start.y+=(size.y-textSize.y)*.5f;
        ItemSize(bb);
        if (!ItemAdd(bb, id))
            return false;

        bool hovered=false, held=false;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held);

        updateTexture();

        ImVec2 uv_0 = uv0;
        ImVec2 uv_1 = uv1;
        if (hovered && hoverModeIfSupported) isAtLeastOneWidgetInHoverMode = true;
        if ((persistentTexId && hoverModeIfSupported && hovered) || !persistentTexId || !hoverModeIfSupported) {
            const ImVec2 uvFrameDelta = uvFrame1 - uvFrame0;
            uv_0 = uvFrame0 + uv0*uvFrameDelta;
            uv_1 = uvFrame0 + uv1*uvFrameDelta;
        }
        else {
            // We must use frame zero here:
            ImVec2 uvFrame0,uvFrame1;
            calculateTexCoordsForFrame(0,uvFrame0,uvFrame1);
            const ImVec2 uvFrameDelta = uvFrame1 - uvFrame0;
            uv_0 = uvFrame0 + uv0*uvFrameDelta;
            uv_1 = uvFrame0 + uv1*uvFrameDelta;
        }


        // Render
        const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
        RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
        if (bg_col.w > 0.0f)
            window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));

        window->DrawList->AddImage(texId, image_bb.Min, image_bb.Max, uv_0, uv_1, GetColorU32(tint_col));

        if (hasText) ImGui::RenderText(start,label);
        return pressed;
    }


    inline int getWidth() const {return w;}
    inline int getHeight() const {return h;}
    inline int getNumFrames() const {return frames;}
    inline ImTextureID getTexture() const {return texId;}

    private:
    AnimatedImageInternal(const AnimatedImageInternal& ) {}
    void operator=(const AnimatedImageInternal& ) {}
    void rearrangeBufferForPersistentTexture()  {
        const int newBufferSize = w*numFramesPerRowInPersistentTexture*h*numFramesPerColInPersistentTexture*4;

        // BUFFER: frames images one below each other: size: 4*w x (h*frames)
        // TMP:    frames images numFramesPerRowInPersistentTexture * (4*w) x (h*numFramesPerColInPersistentTexture)

        const int strideSz = w*4;
        const int frameSz = strideSz*h;
        unsigned char* tmp = (unsigned char*) STBI_MALLOC(newBufferSize);
        IM_ASSERT(tmp);

        unsigned char*          pw = tmp;
        const unsigned char*    pr = buffer;

        int frm=0,colSz=0;
        while (frm<frames)	{
            for (int y = 0; y<h;y++)    {
                pr=&buffer[frm*frameSz + y*strideSz];
                colSz = numFramesPerRowInPersistentTexture>(frames-frm)?(frames-frm):numFramesPerRowInPersistentTexture;
                for (int col = 0; col<colSz;col++)    {
                    memcpy(pw,pr,strideSz);
                    pr+=frameSz;
                    pw+=strideSz;
                }
                if (colSz<numFramesPerRowInPersistentTexture) {
                    for (int col = colSz;col<numFramesPerRowInPersistentTexture;col++)    {
                        memset(pw,0,strideSz);
                        pw+=strideSz;
                    }
                }
            }
            frm+=colSz;
        }

        //-----------------------------------------------------------------------
        STBI_FREE(buffer);buffer=tmp;tmp=NULL;

#       ifdef DEBUG_OUT_TEXTURE
        stbi_write_png("testOutputPng.png", w*numFramesPerRowInPersistentTexture,h*numFramesPerColInPersistentTexture, 4, &buffer[0], w*numFramesPerRowInPersistentTexture*4);
#       undef DEBUG_OUT_TEXTURE
#       endif //DEBUG_OUT_TEXTURE
    }
    void calculateTexCoordsForFrame(int frm,ImVec2& uv0Out,ImVec2& uv1Out) const    {
        uv0Out=ImVec2((float)(frm%numFramesPerRowInPersistentTexture)/(float)numFramesPerRowInPersistentTexture,(float)(frm/numFramesPerRowInPersistentTexture)/(float)numFramesPerColInPersistentTexture);
        uv1Out=ImVec2(uv0Out.x+1.f/(float)numFramesPerRowInPersistentTexture,uv0Out.y+1.f/(float)numFramesPerColInPersistentTexture);
    }

};

AnimatedImage::FreeTextureDelegate AnimatedImage::FreeTextureCb =
#ifdef IMGUI_USE_AUTO_BINDING
    &ImImpl_FreeTexture;
#else //IMGUI_USE_AUTO_BINDING
    NULL;
#endif //IMGUI_USE_AUTO_BINDING
AnimatedImage::GenerateOrUpdateTextureDelegate AnimatedImage::GenerateOrUpdateTextureCb =
#ifdef IMGUI_USE_AUTO_BINDING
    &ImImpl_GenerateOrUpdateTexture;
#else //IMGUI_USE_AUTO_BINDING
    NULL;
#endif //IMGUI_USE_AUTO_BINDING

ImVec2 AnimatedImage::MaxPersistentTextureSize(2048,2048);

#ifndef STBI_NO_GIF
#ifndef IMGUIVARIOUSCONTROLS_NO_STDIO
AnimatedImage::AnimatedImage(const char *gif_filepath, bool useHoverModeIfSupported)    {
    ptr = (AnimatedImageInternal*) ImGui::MemAlloc(sizeof(AnimatedImageInternal));
    IM_PLACEMENT_NEW(ptr) AnimatedImageInternal(gif_filepath,useHoverModeIfSupported);
}
#endif //IMGUIVARIOUSCONTROLS_NO_STDIO
AnimatedImage::AnimatedImage(const unsigned char* gif_buffer,int gif_buffer_size,bool useHoverModeIfSupported)  {
    ptr = (AnimatedImageInternal*) ImGui::MemAlloc(sizeof(AnimatedImageInternal));
    IM_PLACEMENT_NEW(ptr) AnimatedImageInternal(gif_buffer,gif_buffer_size,useHoverModeIfSupported);
}
#endif //STBI_NO_GIF
AnimatedImage::AnimatedImage(ImTextureID myTexId, int animationImageWidth, int animationImageHeight, int numFrames, int numFramesPerRowInTexture, int numFramesPerColumnInTexture, float delayBetweenFramesInCs, bool useHoverMode) {
    ptr = (AnimatedImageInternal*) ImGui::MemAlloc(sizeof(AnimatedImageInternal));
    IM_PLACEMENT_NEW(ptr) AnimatedImageInternal(myTexId,animationImageWidth,animationImageHeight,numFrames,numFramesPerRowInTexture,numFramesPerColumnInTexture,delayBetweenFramesInCs,useHoverMode);
}
AnimatedImage::AnimatedImage()  {
    ptr = (AnimatedImageInternal*) ImGui::MemAlloc(sizeof(AnimatedImageInternal));
    IM_PLACEMENT_NEW(ptr) AnimatedImageInternal();
}
AnimatedImage::~AnimatedImage() {
    clear();
    ptr->~AnimatedImageInternal();
    ImGui::MemFree(ptr);ptr=NULL;
}
void AnimatedImage::clear() {ptr->clear();}
void AnimatedImage::render(ImVec2 size, const ImVec2 &uv0, const ImVec2 &uv1, const ImVec4 &tint_col, const ImVec4 &border_col) const   {ptr->render(size,uv0,uv1,tint_col,border_col);}
bool AnimatedImage::renderAsButton(const char *label, ImVec2 size, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col)   {return ptr->renderAsButton(label,size,uv0,uv1,frame_padding,bg_col,tint_col);}
#ifndef STBI_NO_GIF
#ifndef IMGUIVARIOUSCONTROLS_NO_STDIO
bool AnimatedImage::load(const char *gif_filepath, bool useHoverModeIfSupported)    {return ptr->load(gif_filepath,useHoverModeIfSupported);}
#endif //IMGUIVARIOUSCONTROLS_NO_STDIO
bool AnimatedImage::load_from_memory(const unsigned char* gif_buffer,int gif_buffer_size,bool useHoverModeIfSupported)  {return ptr->load_from_memory(gif_buffer,gif_buffer_size,useHoverModeIfSupported);}
#endif //STBI_NO_GIF
bool AnimatedImage::create(ImTextureID myTexId, int animationImageWidth, int animationImageHeight, int numFrames, int numFramesPerRowInTexture, int numFramesPerColumnInTexture, float delayBetweenFramesInCs, bool useHoverMode)   {return ptr->create(myTexId,animationImageWidth,animationImageHeight,numFrames,numFramesPerRowInTexture,numFramesPerColumnInTexture,delayBetweenFramesInCs,useHoverMode);}
int AnimatedImage::getWidth() const {return ptr->getWidth();}
int AnimatedImage::getHeight() const    {return ptr->getHeight();}
int AnimatedImage::getNumFrames() const {return ptr->getNumFrames();}
bool AnimatedImage::areAllFramesInASingleTexture() const    {return ptr->areAllFramesInASingleTexture();}
#endif //NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE


/*
    inline ImVec2 mouseToPdfRelativeCoords(const ImVec2 &mp) const {
       return ImVec2((mp.x+cursorPosAtStart.x-startPos.x)*(uv1.x-uv0.x)/zoomedImageSize.x+uv0.x,
               (mp.y+cursorPosAtStart.y-startPos.y)*(uv1.y-uv0.y)/zoomedImageSize.y+uv0.y);
    }
    inline ImVec2 pdfRelativeToMouseCoords(const ImVec2 &mp) const {
        return ImVec2((mp.x-uv0.x)*(zoomedImageSize.x)/(uv1.x-uv0.x)+startPos.x-cursorPosAtStart.x,(mp.y-uv0.y)*(zoomedImageSize.y)/(uv1.y-uv0.y)+startPos.y-cursorPosAtStart.y);
    }
*/
bool ImageZoomAndPan(ImTextureID user_texture_id, const ImVec2& size,float aspectRatio,float& zoom,ImVec2& zoomCenter,int panMouseButtonDrag,int resetZoomAndPanMouseButton,const ImVec2& zoomMaxAndZoomStep)
{
    bool rv = false;
    ImGuiWindow* window = GetCurrentWindow();
    if (!window || window->SkipItems) return rv;
    ImVec2 curPos = ImGui::GetCursorPos();
    const ImVec2 wndSz(size.x>0 ? size.x : ImGui::GetWindowSize().x-curPos.x,size.y>0 ? size.y : ImGui::GetWindowSize().y-curPos.y);

    IM_ASSERT(wndSz.x!=0 && wndSz.y!=0 && zoom!=0);

    // Here we use the whole size (although it can be partially empty)
    ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + wndSz.x,window->DC.CursorPos.y + wndSz.y));
    ItemSize(bb);
    if (!ItemAdd(bb, 0)) return rv;

    ImVec2 imageSz = wndSz;
    ImVec2 remainingWndSize(0,0);
    if (aspectRatio!=0) {
        const float wndAspectRatio = wndSz.x/wndSz.y;
        if (aspectRatio >= wndAspectRatio) {imageSz.y = imageSz.x/aspectRatio;remainingWndSize.y = wndSz.y - imageSz.y;}
        else {imageSz.x = imageSz.y*aspectRatio;remainingWndSize.x = wndSz.x - imageSz.x;}
    }

    if (ImGui::IsItemHovered()) {
        const ImGuiIO& io = ImGui::GetIO();
        if (io.MouseWheel!=0) {
            if (io.KeyCtrl) {
                const float zoomStep = zoomMaxAndZoomStep.y;
                const float zoomMin = 1.f;
                const float zoomMax = zoomMaxAndZoomStep.x;
                if (io.MouseWheel < 0) {zoom/=zoomStep;if (zoom<zoomMin) zoom=zoomMin;}
                else {zoom*=zoomStep;if (zoom>zoomMax) zoom=zoomMax;}
                rv = true;
                /*if (io.FontAllowUserScaling) {
                    // invert effect:
                    // Zoom / Scale window
                    ImGuiContext& g = *ImGui::GetCurrentContext();
                    ImGuiWindow* window = g.HoveredWindow;
                    float new_font_scale = ImClamp(window->FontWindowScale - g.IO.MouseWheel * 0.10f, 0.50f, 2.50f);
                    float scale = new_font_scale / window->FontWindowScale;
                    window->FontWindowScale = new_font_scale;

                    const ImVec2 offset = window->Size * (1.0f - scale) * (g.IO.MousePos - window->Pos) / window->Size;
                    window->Pos += offset;
                    window->PosFloat += offset;
                    window->Size *= scale;
                    window->SizeFull *= scale;
                }*/
            }
            else  {
                const bool scrollDown = io.MouseWheel <= 0;
                const float zoomFactor = .5/zoom;
                if ((!scrollDown && zoomCenter.y > zoomFactor) || (scrollDown && zoomCenter.y <  1.f - zoomFactor))  {
                    const float slideFactor = zoomMaxAndZoomStep.y*0.1f*zoomFactor;
                    if (scrollDown) {
                        zoomCenter.y+=slideFactor;///(imageSz.y*zoom);
                        if (zoomCenter.y >  1.f - zoomFactor) zoomCenter.y =  1.f - zoomFactor;
                    }
                    else {
                        zoomCenter.y-=slideFactor;///(imageSz.y*zoom);
                        if (zoomCenter.y < zoomFactor) zoomCenter.y = zoomFactor;
                    }
                    rv = true;
                }
            }
        }
        if (io.MouseClicked[resetZoomAndPanMouseButton]) {zoom=1.f;zoomCenter.x=zoomCenter.y=.5f;rv = true;}
        if (ImGui::IsMouseDragging(panMouseButtonDrag,1.f))   {
            zoomCenter.x-=io.MouseDelta.x/(imageSz.x*zoom);
            zoomCenter.y-=io.MouseDelta.y/(imageSz.y*zoom);
            rv = true;
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
    }

    const float zoomFactor = .5/zoom;
    if (rv) {
        if (zoomCenter.x < zoomFactor) zoomCenter.x = zoomFactor;
        else if (zoomCenter.x > 1.f - zoomFactor) zoomCenter.x = 1.f - zoomFactor;
        if (zoomCenter.y < zoomFactor) zoomCenter.y = zoomFactor;
        else if (zoomCenter.y > 1.f - zoomFactor) zoomCenter.y = 1.f - zoomFactor;
    }

    ImVec2 uvExtension(2.f*zoomFactor,2.f*zoomFactor);
    if (remainingWndSize.x > 0) {
        const float remainingSizeInUVSpace = 2.f*zoomFactor*(remainingWndSize.x/imageSz.x);
        const float deltaUV = uvExtension.x;
        const float remainingUV = 1.f-deltaUV;
        if (deltaUV<1) {
            float adder = (remainingUV < remainingSizeInUVSpace ? remainingUV : remainingSizeInUVSpace);
            uvExtension.x+=adder;
            remainingWndSize.x-= adder * zoom * imageSz.x;
            imageSz.x+=adder * zoom * imageSz.x;

            if (zoomCenter.x < uvExtension.x*.5f) zoomCenter.x = uvExtension.x*.5f;
            else if (zoomCenter.x > 1.f - uvExtension.x*.5f) zoomCenter.x = 1.f - uvExtension.x*.5f;
        }
    }
    if (remainingWndSize.y > 0) {
        const float remainingSizeInUVSpace = 2.f*zoomFactor*(remainingWndSize.y/imageSz.y);
        const float deltaUV = uvExtension.y;
        const float remainingUV = 1.f-deltaUV;
        if (deltaUV<1) {
            float adder = (remainingUV < remainingSizeInUVSpace ? remainingUV : remainingSizeInUVSpace);
            uvExtension.y+=adder;
            remainingWndSize.y-= adder * zoom * imageSz.y;
            imageSz.y+=adder * zoom * imageSz.y;

            if (zoomCenter.y < uvExtension.y*.5f) zoomCenter.y = uvExtension.y*.5f;
            else if (zoomCenter.y > 1.f - uvExtension.y*.5f) zoomCenter.y = 1.f - uvExtension.y*.5f;
        }
    }

    ImVec2 uv0((zoomCenter.x-uvExtension.x*.5f),(zoomCenter.y-uvExtension.y*.5f));
    ImVec2 uv1((zoomCenter.x+uvExtension.x*.5f),(zoomCenter.y+uvExtension.y*.5f));


    /* // Here we use just the window size, but then ImGui::IsItemHovered() should be moved below this block. How to do it?
    ImVec2 startPos=window->DC.CursorPos;
    startPos.x+= remainingWndSize.x*.5f;
    startPos.y+= remainingWndSize.y*.5f;
    ImVec2 endPos(startPos.x+imageSz.x,startPos.y+imageSz.y);
    ImRect bb(startPos, endPos);
    ItemSize(bb);
    if (!ItemAdd(bb, NULL)) return rv;*/

    ImVec2 startPos=bb.Min,endPos=bb.Max;
    startPos.x+= remainingWndSize.x*.5f;
    startPos.y+= remainingWndSize.y*.5f;
    endPos.x = startPos.x + imageSz.x;
    endPos.y = startPos.y + imageSz.y;

    window->DrawList->AddImage(user_texture_id, startPos, endPos, uv0, uv1);

    return rv;
}

inline static bool GlyphButton(ImGuiID id, const ImVec2& pos,const ImVec2& halfSize,const char* text,bool* toggleButtonState=NULL,bool *pHovered = NULL)    {
    ImGuiWindow* window = GetCurrentWindow();

    const ImRect bb(pos - halfSize, pos + halfSize);

    bool hovered=false, held=false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held,ImGuiButtonFlags_PressedOnRelease);
    if (pHovered) *pHovered = hovered;
    //const bool isACheckedToggleButton = (toggleButtonState && *toggleButtonState);
    //const bool useNormalButtonStyle = (text && text[0]!='\0' && !isACheckedToggleButton);   // Otherwise use CloseButtonStyle

    // Render    
    //ImU32 col = GetColorU32((held && hovered) ? (useNormalButtonStyle ? ImGuiCol_ButtonActive : ImGuiCol_CloseButtonActive) : hovered ? (useNormalButtonStyle ? ImGuiCol_ButtonHovered : ImGuiCol_CloseButtonHovered) : (useNormalButtonStyle ? ImGuiCol_Button : ImGuiCol_CloseButton));
    ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImU32 textCol = GetColorU32(ImGuiCol_Text);
    if (!hovered) {
        col = (((col>>24)/2)<<24)|(col&0x00FFFFFF);
        textCol = (((textCol>>24)/2)<<24)|(textCol&0x00FFFFFF);
    }
    window->DrawList->AddRectFilled(bb.GetTL(),bb.GetBR(), col, text ? 2 : 6);

    if (text) {
        const ImVec2 textSize = ImGui::CalcTextSize(text);
        window->DrawList->AddText(
                    ImVec2(bb.GetTL().x+(bb.GetWidth()-textSize.x)*0.5f,bb.GetTL().y+(bb.GetHeight()-textSize.y)*0.5f),
                    textCol,
                    text);
    }
    else {  // fallback to the close button
        const ImVec2 center = bb.GetCenter();
        const float cross_extent = ((halfSize.x * 0.7071f) - 1.0f)*0.75f;
        window->DrawList->AddLine(center + ImVec2(+cross_extent,+cross_extent), center + ImVec2(-cross_extent,-cross_extent), textCol ,2.f);
        window->DrawList->AddLine(center + ImVec2(+cross_extent,-cross_extent), center + ImVec2(-cross_extent,+cross_extent), textCol, 2.f);
    }

    if (toggleButtonState && pressed) *toggleButtonState=!(*toggleButtonState);
    return (pressed);
}

static int AppendTreeNodeHeaderButtonsV(const void* ptr_id,float startWindowCursorXForClipping,int numButtons,va_list args)   {
#if 0
    if (!ImGui::IsItemVisible()) return -1; // We don't reset non-togglable buttons to false for performance reasons

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImGuiID id = window->GetID(ptr_id);

    ImGuiContext& g = *ImGui::GetCurrentContext();
    const float buttonSz = g.FontSize;
    const ImVec2 glyphHalfSize(buttonSz*0.5f,buttonSz*0.5f);
    ImVec2 pos(ImMin(window->DC.LastItemRect.Max.x, window->ClipRect.Max.x) - g.Style.FramePadding.x -buttonSz*0.5f, window->DC.LastItemRect.Min.y + g.Style.FramePadding.y+buttonSz*0.5f);
    bool pressed = false,hovered = false;

    bool* pPressed = NULL;
    const char* tooltip = NULL;
    const char* glyph = NULL;
    int isToggleButton = 0, rv = -1;

    for (int i=0;i<numButtons;i++)  {
        pPressed = va_arg(args, bool*);tooltip = va_arg(args, const char*);
        glyph = va_arg(args, const char*);isToggleButton = va_arg(args, int);

        if (pPressed)	{
            //fprintf(stderr,"btn:%d pos.x=%1.0f startWindowCursorXForClipping=%1.0f\n",i,pos.x,startWindowCursorXForClipping);
            if (pos.x>startWindowCursorXForClipping+(4.0f*buttonSz))   {
                id = window->GetID((void*)(intptr_t)(id+1));
                if ((pressed = ImGui::GlyphButton(id, pos, glyphHalfSize,glyph,isToggleButton ? pPressed : NULL,&hovered))) rv = i;
                if (!isToggleButton) *pPressed = pressed;
                pos.x-=buttonSz-2;
                if (tooltip && hovered && strlen(tooltip)>0) ImGui::SetTooltip("%s",tooltip);
                if (hovered && rv==-1) rv = numButtons+i;
            }
            else {
                pressed = false;
                if (!isToggleButton) *pPressed = false; // Just in case we pass a static bool
            }
            //if (pressed) fprintf(stderr,"Pressed button %d: '%s'\n",i,glyph?glyph:"close"); // TO REMOVE
        }
        else pos.x-=buttonSz-2; // separator mode
    }
    return rv;
#endif
	return 0;
}

int AppendTreeNodeHeaderButtons(const void* ptr_id, float startWindowCursorXForClipping, int numButtons, ...) {
    va_list args;
    va_start(args, numButtons);
    const int rv = AppendTreeNodeHeaderButtonsV(ptr_id, startWindowCursorXForClipping, numButtons, args);
    va_end(args);
    return rv;
}

// Start PlotHistogram(...) implementation -------------------------------
struct ImGuiPlotMultiArrayGetterData    {
    const float** Values;int Stride;
    ImGuiPlotMultiArrayGetterData(const float** values, int stride) { Values = values; Stride = stride; }

    // Some static helper methods stuffed in this struct fo convenience
    inline static void GetVerticalGradientTopAndBottomColors(ImU32 c,float fillColorGradientDeltaIn0_05,ImU32& tc,ImU32& bc)  {
        if (fillColorGradientDeltaIn0_05==0) {tc=bc=c;return;}

        const bool negative = (fillColorGradientDeltaIn0_05<0);
        if (negative) fillColorGradientDeltaIn0_05=-fillColorGradientDeltaIn0_05;
        if (fillColorGradientDeltaIn0_05>0.5f) fillColorGradientDeltaIn0_05=0.5f;

        // New code:
        //#define IM_COL32(R,G,B,A)    (((ImU32)(A)<<IM_COL32_A_SHIFT) | ((ImU32)(B)<<IM_COL32_B_SHIFT) | ((ImU32)(G)<<IM_COL32_G_SHIFT) | ((ImU32)(R)<<IM_COL32_R_SHIFT))
        const int fcgi = fillColorGradientDeltaIn0_05*255.0f;
        const int R = (unsigned char) (c>>IM_COL32_R_SHIFT);    // The cast should reset upper bits (as far as I hope)
        const int G = (unsigned char) (c>>IM_COL32_G_SHIFT);
        const int B = (unsigned char) (c>>IM_COL32_B_SHIFT);
        const int A = (unsigned char) (c>>IM_COL32_A_SHIFT);

        int r = R+fcgi, g = G+fcgi, b = B+fcgi;
        if (r>255) r=255;
        if (g>255) g=255;
        if (b>255) b=255;
        if (negative) bc = IM_COL32(r,g,b,A); else tc = IM_COL32(r,g,b,A);

        r = R-fcgi; g = G-fcgi; b = B-fcgi;
        if (r<0) r=0;
        if (g<0) g=0;
        if (b<0) b=0;
        if (negative) tc = IM_COL32(r,g,b,A); else bc = IM_COL32(r,g,b,A);

        /* // Old legacy code (to remove)... [However here we lerp alpha too...]
        // Can we do it without the double conversion ImU32 -> ImVec4 -> ImU32 ?
        const ImVec4 cf = ColorConvertU32ToFloat4(c);
        ImVec4 tmp(cf.x+fillColorGradientDeltaIn0_05,cf.y+fillColorGradientDeltaIn0_05,cf.z+fillColorGradientDeltaIn0_05,cf.w+fillColorGradientDeltaIn0_05);
        if (tmp.x>1.f) tmp.x=1.f;if (tmp.y>1.f) tmp.y=1.f;if (tmp.z>1.f) tmp.z=1.f;if (tmp.w>1.f) tmp.w=1.f;
        if (negative) bc = ColorConvertFloat4ToU32(tmp); else tc = ColorConvertFloat4ToU32(tmp);
        tmp=ImVec4(cf.x-fillColorGradientDeltaIn0_05,cf.y-fillColorGradientDeltaIn0_05,cf.z-fillColorGradientDeltaIn0_05,cf.w-fillColorGradientDeltaIn0_05);
        if (tmp.x<0.f) tmp.x=0.f;if (tmp.y<0.f) tmp.y=0.f;if (tmp.z<0.f) tmp.z=0.f;if (tmp.w<0.f) tmp.w=0.f;
        if (negative) tc = ColorConvertFloat4ToU32(tmp); else bc = ColorConvertFloat4ToU32(tmp);*/
    }
    // Same as default ImSaturate, but overflowOut can be -1,0 or 1 in case of clamping:
    inline static float ImSaturate(float f,short& overflowOut)	{
        if (f < 0.0f) {overflowOut=-1;return 0.0f;}
        if (f > 1.0f) {overflowOut=1;return 1.0f;}
        overflowOut=0;return f;
    }
    // Same as IsMouseHoveringRect, but only for the X axis (we already know if the whole item has been hovered or not)
    inline static bool IsMouseBetweenXValues(float x_min, float x_max,float *pOptionalXDeltaOut=NULL, bool clip=true, bool expandForTouchInput=false)   {
        ImGuiContext& g = *ImGui::GetCurrentContext();
        ImGuiWindow* window = GetCurrentWindowRead();

        if (clip) {
            if (x_min < window->ClipRect.Min.x) x_min = window->ClipRect.Min.x;
            if (x_max > window->ClipRect.Max.x) x_max = window->ClipRect.Max.x;
        }
        if (expandForTouchInput)    {x_min-=g.Style.TouchExtraPadding.x;x_max+=g.Style.TouchExtraPadding.x;}

        if (pOptionalXDeltaOut) *pOptionalXDeltaOut = g.IO.MousePos.x - x_min;
        return (g.IO.MousePos.x >= x_min && g.IO.MousePos.x < x_max);
    }
};
static float Plot_MultiArrayGetter(void* data, int idx,int histogramIdx)    {
    ImGuiPlotMultiArrayGetterData* plot_data = (ImGuiPlotMultiArrayGetterData*)data;
    const float v = *(float*)(void*)((unsigned char*)(&plot_data->Values[histogramIdx][0]) + (size_t)idx * plot_data->Stride);
    return v;
}
int PlotHistogram(const char* label, const float** values,int num_histograms,int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride,float histogramGroupSpacingInPixels,int* pOptionalHoveredHistogramIndexOut,float fillColorGradientDeltaIn0_05,const ImU32* pColorsOverride,int numColorsOverride)   {
    ImGuiPlotMultiArrayGetterData data(values, stride);
    return PlotHistogram(label, &Plot_MultiArrayGetter, (void*)&data, num_histograms, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size,histogramGroupSpacingInPixels,pOptionalHoveredHistogramIndexOut,fillColorGradientDeltaIn0_05,pColorsOverride,numColorsOverride);
}
int PlotHistogram(const char* label, float (*values_getter)(void* data, int idx,int histogramIdx), void* data,int num_histograms, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size,float histogramGroupSpacingInPixels,int* pOptionalHoveredHistogramIndexOut,float fillColorGradientDeltaIn0_05,const ImU32* pColorsOverride,int numColorsOverride)  {
    ImGuiWindow* window = GetCurrentWindow();
    if (pOptionalHoveredHistogramIndexOut) *pOptionalHoveredHistogramIndexOut=-1;
    if (window->SkipItems) return -1;

    static const float minSingleHistogramWidth = 5.f;   // in pixels
    static const float maxSingleHistogramWidth = 100.f;   // in pixels

    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    if (graph_size.x == 0.0f) graph_size.x = CalcItemWidth();
    if (graph_size.y == 0.0f) graph_size.y = label_size.y + (style.FramePadding.y * 2);

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(graph_size.x, graph_size.y));
    const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, 0)) return -1;

    // Determine scale from values if not specified
    if (scale_min == FLT_MAX || scale_max == FLT_MAX || scale_min==scale_max)   {
        float v_min = FLT_MAX;
        float v_max = -FLT_MAX;
        for (int i = 0; i < values_count; i++)  {
            for (int h=0;h<num_histograms;h++)  {
                const float v = values_getter(data, (i + values_offset) % values_count, h);
                if (v != v) // Ignore NaN values
                    continue;
                v_min = ImMin(v_min, v);
                v_max = ImMax(v_max, v);
            }
        }
        if (scale_min == FLT_MAX  || scale_min>=scale_max) scale_min = v_min;
        if (scale_max == FLT_MAX  || scale_min>=scale_max) scale_max = v_max;
    }
    if (scale_min>scale_max) {float tmp=scale_min;scale_min=scale_max;scale_max=tmp;}

    RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

    static ImU32 col_base_embedded[] = {0,IM_COL32(100,100,225,255),IM_COL32(100,225,100,255),IM_COL32(225,100,100,255),IM_COL32(150,75,225,255)};
    static const int num_colors_embedded = sizeof(col_base_embedded)/sizeof(col_base_embedded[0]);


    int v_hovered = -1, h_hovered = -1;
    if (values_count > 0 && num_histograms > 0)
    {
        const bool mustOverrideColors = (pColorsOverride && numColorsOverride>0);
        const ImU32* col_base = mustOverrideColors ? pColorsOverride : col_base_embedded;
        const int num_colors = mustOverrideColors ? numColorsOverride : num_colors_embedded;

        const int total_histograms = values_count * num_histograms;

        const bool isItemHovered = ItemHoverable(inner_bb, id);

        if (!mustOverrideColors) col_base_embedded[0] = GetColorU32(ImGuiCol_PlotHistogram);
        const ImU32 lineCol = GetColorU32(ImGuiCol_WindowBg);
        const float lineThick = 1.f;
        const ImU32 overflowCol = lineCol;

        const ImVec2 inner_bb_extension(inner_bb.Max.x-inner_bb.Min.x,inner_bb.Max.y-inner_bb.Min.y);
        const float scale_extension = scale_max - scale_min;
        const bool hasXAxis = scale_max>=0 && scale_min<=0;
        const float xAxisSat = ImSaturate((0.0f - scale_min) / (scale_max - scale_min));
        const float xAxisSatComp = 1.f-xAxisSat;
        const float posXAxis = inner_bb.Max.y-inner_bb_extension.y*xAxisSat-0.5f;
        const bool isAlwaysNegative = scale_max<0 && scale_min<0;

        float t_step = (inner_bb.Max.x-inner_bb.Min.x-(float)(values_count-1)*histogramGroupSpacingInPixels)/(float)total_histograms;
        if (t_step<minSingleHistogramWidth) t_step = minSingleHistogramWidth;
        else if (t_step>maxSingleHistogramWidth) t_step = maxSingleHistogramWidth;

        float t1 = 0.f;
        float posY=0.f;ImVec2 pos0(0.f,0.f),pos1(0.f,0.f);
        ImU32 rectCol=0,topRectCol=0,bottomRectCol=0;float gradient = 0.f;
        short overflow = 0;bool mustSkipBorder = false;int iWithOffset=0;
        for (int i = 0; i < values_count; i++)  {
            if (i!=0) t1+=histogramGroupSpacingInPixels;
            if (t1>inner_bb.Max.x) break;
            iWithOffset = (i + values_offset) % values_count;
            for (int h=0;h<num_histograms;h++)  {
                const float v1 = values_getter(data, iWithOffset, h);

                pos0.x = inner_bb.Min.x+t1;
                pos1.x = pos0.x+t_step;

                const int col_num = h%num_colors;
                rectCol = col_base[col_num];
                if (isItemHovered &&
                //ImGui::IsMouseHoveringRect(ImVec2(pos0.x,inner_bb.Min.y),ImVec2(pos1.x,inner_bb.Max.y))
                ImGuiPlotMultiArrayGetterData::IsMouseBetweenXValues(pos0.x,pos1.x)
                ) {
                    h_hovered = h;
                    v_hovered = iWithOffset; // iWithOffset or just i ?
                    if (pOptionalHoveredHistogramIndexOut) *pOptionalHoveredHistogramIndexOut=h_hovered;
                    SetTooltip("%d: %8.4g", i, v1); // Tooltip on hover

                    if (h==0 && !mustOverrideColors) rectCol = GetColorU32(ImGuiCol_PlotHistogramHovered); // because: col_base[0] = GetColorU32(ImGuiCol_PlotHistogram);
                    else {
                        // We don't have any hover color ready, but we can calculate it based on the same logic used between ImGuiCol_PlotHistogramHovered and ImGuiCol_PlotHistogram.
                        // Note that this code is executed only once, when a histogram is hovered.
                        const ImGuiStyle& style = ImGui::GetStyle();
                        ImVec4 diff = style.Colors[ImGuiCol_PlotHistogramHovered];
                        ImVec4 base = style.Colors[ImGuiCol_PlotHistogram];
                        diff.x-=base.x;diff.y-=base.y;diff.z-=base.z;diff.w-=base.w;
                        base = ImGui::ColorConvertU32ToFloat4(rectCol);
                        if (style.Alpha!=0.f) base.w /= style.Alpha;	// See GetColorU32(...) for this
                        base.x+=diff.x;base.y+=diff.y;base.z+=diff.z;base.w+=diff.w;
                        base = ImVec4(ImSaturate(base.x),ImSaturate(base.y),ImSaturate(base.z),ImSaturate(base.w));
                        rectCol = GetColorU32(base);
                    }
                }
                gradient = fillColorGradientDeltaIn0_05;
                mustSkipBorder = false;

                if (!hasXAxis) {
                    if (isAlwaysNegative)   {
                        posY = inner_bb_extension.y*ImGuiPlotMultiArrayGetterData::ImSaturate((scale_max - v1) / scale_extension,overflow);
                        overflow=-overflow;

                        pos0.y = inner_bb.Min.y;
                        pos1.y = inner_bb.Min.y+posY;

                        gradient = -fillColorGradientDeltaIn0_05;
                        mustSkipBorder = (overflow==1);
                    }
                    else {
                        posY = inner_bb_extension.y*ImGuiPlotMultiArrayGetterData::ImSaturate((v1 - scale_min) / scale_extension,overflow);

                        pos0.y = inner_bb.Max.y-posY;
                        pos1.y = inner_bb.Max.y;
                        mustSkipBorder = (overflow==-1);
                    }
                }
                else if (v1>=0){
                    posY = ImGuiPlotMultiArrayGetterData::ImSaturate(v1/scale_max,overflow);
                    pos1.y = posXAxis;
                    pos0.y = posXAxis-inner_bb_extension.y*xAxisSatComp*posY;
                }
                else {
                    posY = ImGuiPlotMultiArrayGetterData::ImSaturate(v1/scale_min,overflow);
                    overflow = -overflow;	// Probably redundant
                    pos0.y = posXAxis;
                    pos1.y = posXAxis+inner_bb_extension.y*xAxisSat*posY;
                    gradient = -fillColorGradientDeltaIn0_05;
                }

                ImGuiPlotMultiArrayGetterData::GetVerticalGradientTopAndBottomColors(rectCol,gradient,topRectCol,bottomRectCol);

                window->DrawList->AddRectFilledMultiColor(pos0, pos1,topRectCol,topRectCol,bottomRectCol,bottomRectCol); // Gradient for free!

                if (overflow!=0)    {
                    // Here we draw the small arrow that indicates that the histogram is out of scale
                    const float spacing = lineThick+1;
                    const float height = inner_bb_extension.y*0.075f;
                    // Using CW order here... but I'm not sure this is correct in Dear ImGui (we should enable GL_CULL_FACE and see if it's the same output)
                    if (overflow>0)	    window->DrawList->AddTriangleFilled(ImVec2((pos0.x+pos1.x)*0.5f,inner_bb.Min.y+spacing),ImVec2(pos1.x-spacing,inner_bb.Min.y+spacing+height), ImVec2(pos0.x+spacing,inner_bb.Min.y+spacing+height),overflowCol);
                    else if (overflow<0)    window->DrawList->AddTriangleFilled(ImVec2((pos0.x+pos1.x)*0.5f,inner_bb.Max.y-spacing),ImVec2(pos0.x+spacing,inner_bb.Max.y-spacing-height), ImVec2(pos1.x-spacing,inner_bb.Max.y-spacing-height),overflowCol);
                }

                if (!mustSkipBorder) window->DrawList->AddRect(pos0, pos1,lineCol,0.f,0,lineThick);

                t1+=t_step; if (t1>inner_bb.Max.x) break;
            }
        }

        if (hasXAxis) {
            // Draw x Axis:
            const ImU32 axisCol = GetColorU32(ImGuiCol_Text);
            const float axisThick = 1.f;
            window->DrawList->AddLine(ImVec2(inner_bb.Min.x,posXAxis),ImVec2(inner_bb.Max.x,posXAxis),axisCol,axisThick);
        }
    }

    // Text overlay
    if (overlay_text)
        RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, overlay_text, NULL, NULL, ImVec2(0.5f,0.0f));

    if (label_size.x > 0.0f)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);

    return v_hovered;
}
int PlotHistogram2(const char* label, const float* values,int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride,float fillColorGradientDeltaIn0_05,const ImU32* pColorsOverride,int numColorsOverride) {
    const float* pValues[1] = {values};
    return PlotHistogram(label,pValues,1,values_count,values_offset,overlay_text,scale_min,scale_max,graph_size,stride,0.f,NULL,fillColorGradientDeltaIn0_05,pColorsOverride,numColorsOverride);
}
// End PlotHistogram(...) implementation ----------------------------------
// Start PlotCurve(...) implementation ------------------------------------
int PlotCurve(const char* label, float (*values_getter)(void* data, float x,int numCurve), void* data,int num_curves,const char* overlay_text,const ImVec2 rangeY,const ImVec2 rangeX, ImVec2 graph_size,ImVec2* pOptionalHoveredValueOut,float precisionInPixels,float numGridLinesHint,const ImU32* pColorsOverride,int numColorsOverride)  {
    ImGuiWindow* window = GetCurrentWindow();
    if (pOptionalHoveredValueOut) *pOptionalHoveredValueOut=ImVec2(0,0);    // Not sure how to initialize this...
    if (window->SkipItems || !values_getter) return -1;

    if (precisionInPixels<=1.f) precisionInPixels = 1.f;

    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    if (graph_size.x == 0.0f) graph_size.x = CalcItemWidth();
    if (graph_size.y == 0.0f) graph_size.y = label_size.y + (style.FramePadding.y * 2);

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(graph_size.x, graph_size.y));
    const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, 0)) return -1;
    const ImVec2 inner_bb_extension(inner_bb.Max.x-inner_bb.Min.x,inner_bb.Max.y-inner_bb.Min.y);

    ImVec2 xRange = rangeX,yRange = rangeY,rangeExtension(0,0);

    if (yRange.x>yRange.y) {float tmp=yRange.x;yRange.x=yRange.y;yRange.y=tmp;}
    else if (yRange.x==yRange.y) {yRange.x-=5.f;yRange.y+=5.f;}
    rangeExtension.y = yRange.y-yRange.x;

    if (xRange.x>xRange.y) {float tmp=xRange.x;xRange.x=xRange.y;xRange.y=tmp;}
    else if (xRange.x==xRange.y) xRange.x-=1.f;
    if (xRange.y==FLT_MAX || xRange.x==xRange.y) xRange.y = xRange.x + (rangeExtension.y/inner_bb_extension.y)*inner_bb_extension.x;
    rangeExtension.x = xRange.y-xRange.x;

    RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

    static ImU32 col_base_embedded[] = {0,IM_COL32(150,150,225,255),IM_COL32(150,225,150,255),IM_COL32(225,150,150,255),IM_COL32(150,100,225,255)};
    static const int num_colors_embedded = sizeof(col_base_embedded)/sizeof(col_base_embedded[0]);


    int v_hovered=-1;int h_hovered = -1;ImVec2 hoveredValue(FLT_MAX,FLT_MAX);
    {
        const bool mustOverrideColors = (pColorsOverride && numColorsOverride>0);
        const ImU32* col_base = mustOverrideColors ? pColorsOverride : col_base_embedded;
        const int num_colors = mustOverrideColors ? numColorsOverride : num_colors_embedded;

        const bool isItemHovered = ItemHoverable(inner_bb, id);

        if (!mustOverrideColors) col_base_embedded[0] = GetColorU32(ImGuiCol_PlotLines);

        const bool hasXAxis = yRange.y>=0 && yRange.x<=0;
        const float xAxisSat = ImSaturate((0.0f - yRange.x) / rangeExtension.y);
        const float posXAxis = inner_bb.Max.y-inner_bb_extension.y*xAxisSat;

        const bool hasYAxis = xRange.y>=0 && xRange.x<=0;
        const float yAxisSat = ImSaturate((0.0f-xRange.x) / rangeExtension.x);
        const float posYAxis = inner_bb.Min.x+inner_bb_extension.x*yAxisSat;

        // Draw additinal horizontal and vertical lines: TODO: Fix this it's wrong
        if (numGridLinesHint>=1.f)   {
            ImU32 lineCol = GetColorU32(ImGuiCol_WindowBg);
            lineCol = (((lineCol>>24)/2)<<24)|(lineCol&0x00FFFFFF);	// Halve alpha
            const float lineThick = 1.f;
            float lineSpacing = (rangeExtension.x < rangeExtension.y) ? rangeExtension.x : rangeExtension.y;
            lineSpacing/=numGridLinesHint;      // We draw 'numGridLinesHint' lines (or so) on the shortest axis
            lineSpacing = floor(lineSpacing);   // We keep integral delta
            if (lineSpacing>0)  {
                float pos=0.f;
                // Draw horizontal lines:
                float rangeCoord = floor(yRange.x);if (rangeCoord<yRange.x) rangeCoord+=lineSpacing;
                while (rangeCoord<=yRange.y) {
                    pos = inner_bb.Max.y-inner_bb_extension.y*(yRange.y-rangeCoord)/rangeExtension.y;
                    window->DrawList->AddLine(ImVec2(inner_bb.Min.x,pos),ImVec2(inner_bb.Max.x,pos),lineCol,lineThick);
                    rangeCoord+=lineSpacing;
                }
                // Draw vertical lines:
                rangeCoord = floor(xRange.x);if (rangeCoord<xRange.x) rangeCoord+=lineSpacing;
                while (rangeCoord<=xRange.y) {
                    pos = inner_bb.Max.x-inner_bb_extension.x*(xRange.y-rangeCoord)/rangeExtension.x;
                    window->DrawList->AddLine(ImVec2(pos,inner_bb.Min.y),ImVec2(pos,inner_bb.Max.y),lineCol,lineThick);
                    rangeCoord+=lineSpacing;
                }
            }
        }

        const ImU32 axisCol = GetColorU32(ImGuiCol_Text);
        const float axisThick = 1.f;
        if (hasXAxis) {
            // Draw x Axis:
            window->DrawList->AddLine(ImVec2(inner_bb.Min.x,posXAxis),ImVec2(inner_bb.Max.x,posXAxis),axisCol,axisThick);
        }
        if (hasYAxis) {
            // Draw y Axis:
            window->DrawList->AddLine(ImVec2(posYAxis,inner_bb.Min.y),ImVec2(posYAxis,inner_bb.Max.y),axisCol,axisThick);
        }

        float mouseHoverDeltaX = 0.f;float minDistanceValue=FLT_MAX;float hoveredValueXInBBCoords=-1.f;
        ImU32 curveCol=0;const float curveThick=2.f;
        ImVec2 pos0(0.f,0.f),pos1(0.f,0.f);
        float yValue=0.f,lastYValue=0.f;
        const float t_step_xScale = precisionInPixels*rangeExtension.x/inner_bb_extension.x;
        const float t1Start = xRange.x+t_step_xScale;
        const float t1Max = xRange.x + rangeExtension.x;
        for (int h=0;h<num_curves;h++)  {
            const int col_num = h%num_colors;
            curveCol = col_base[col_num];
            lastYValue = values_getter(data, xRange.x, h);
            pos0.x = inner_bb.Min.x;
            pos0.y = inner_bb.Max.y - inner_bb_extension.y*((lastYValue-yRange.x)/rangeExtension.y);

            int v_idx = 0;
            for (float t1 = t1Start; t1 < t1Max; t1+=t_step_xScale)  {
                yValue = values_getter(data, t1, h);

                pos1.x = pos0.x+precisionInPixels;
                pos1.y = inner_bb.Max.y - inner_bb_extension.y*((yValue-yRange.x)/rangeExtension.y);

                if (pos0.y>=inner_bb.Min.y && pos0.y<=inner_bb.Max.y && pos1.y>=inner_bb.Min.y && pos1.y<=inner_bb.Max.y)
                {
                    // Draw this curve segment
                    window->DrawList->AddLine(pos0, pos1,curveCol,curveThick);
                }

                if (isItemHovered && h==0 && v_hovered==-1 && ImGuiPlotMultiArrayGetterData::IsMouseBetweenXValues(pos0.x,pos1.x,&mouseHoverDeltaX)) {
                    v_hovered = v_idx;
                    hoveredValueXInBBCoords = pos0.x+mouseHoverDeltaX;
                    hoveredValue.x = xRange.x+(hoveredValueXInBBCoords - inner_bb.Min.x)*rangeExtension.x/inner_bb_extension.x;
                }

                if (v_hovered == v_idx) {
                    const float value = values_getter(data, hoveredValue.x, h);
                    float deltaYMouse = inner_bb.Min.y + (yRange.y-value)*inner_bb_extension.y/rangeExtension.y - ImGui::GetIO().MousePos.y;
                    if (deltaYMouse<0) deltaYMouse=-deltaYMouse;
                    if (deltaYMouse<minDistanceValue) {
                        minDistanceValue = deltaYMouse;

                        h_hovered = h;
                        hoveredValue.y = value;
                    }
                }


                // Mandatory
                lastYValue = yValue;
                pos0 = pos1;
                ++v_idx;
            }
        }


        if (v_hovered>=0 && h_hovered>=0)   {
            if (pOptionalHoveredValueOut) *pOptionalHoveredValueOut=hoveredValue;

	    // Tooltip on hover
	    if (num_curves==1)	SetTooltip("P (%1.4f , %1.4f)", hoveredValue.x, hoveredValue.y);
	    else		SetTooltip("P%d (%1.4f , %1.4f)",h_hovered, hoveredValue.x, hoveredValue.y);

            const float circleRadius = curveThick*3.f;
            const float hoveredValueYInBBCoords = inner_bb.Min.y+(yRange.y-hoveredValue.y)*inner_bb_extension.y/rangeExtension.y;

            // We must draw a circle in (hoveredValueXInBBCoords,hoveredValueYInBBCoords)
	    if (hoveredValueYInBBCoords>=inner_bb.Min.y && hoveredValueYInBBCoords<=inner_bb.Max.y) {
                const int col_num = h_hovered%num_colors;
                curveCol = col_base[col_num];

                window->DrawList->AddCircle(ImVec2(hoveredValueXInBBCoords,hoveredValueYInBBCoords),circleRadius,curveCol,12,curveThick);
            }

        }

    }



    // Text overlay
    if (overlay_text)
        RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, overlay_text, NULL, NULL, ImVec2(0.5f,0.0f));

    if (label_size.x > 0.0f)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);

    return h_hovered;
}
// End PlotCurve(...) implementation --------------------------------------

// Start PlotMultiLines(...) and PlotMultiHistograms(...)------------------------
// by @JaapSuter and @maxint (please see: https://github.com/ocornut/imgui/issues/632)
static inline ImU32 InvertColorU32(ImU32 in)
{
    ImVec4 in4 = ColorConvertU32ToFloat4(in);
    in4.x = 1.f - in4.x;
    in4.y = 1.f - in4.y;
    in4.z = 1.f - in4.z;
    return GetColorU32(in4);
}

static void PlotMultiEx(
    ImGuiPlotType plot_type,
    const char* label,
    int num_datas,
    const char** names,
    const ImColor* colors,
    float(*getter)(const void* data, int idx),
    const void * const * datas,
    int values_count,
    float scale_min,
    float scale_max,
    ImVec2 graph_size)
{
    const int values_offset = 0;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    if (graph_size.x == 0.0f)
        graph_size.x = CalcItemWidth();
    if (graph_size.y == 0.0f)
        graph_size.y = label_size.y + (style.FramePadding.y * 2);

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(graph_size.x, graph_size.y));
    const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, 0))
        return;

    // Determine scale from values if not specified
    if (scale_min == FLT_MAX || scale_max == FLT_MAX)
    {
        float v_min = FLT_MAX;
        float v_max = -FLT_MAX;
        for (int data_idx = 0; data_idx < num_datas; ++data_idx)
        {
            for (int i = 0; i < values_count; i++)
            {
                const float v = getter(datas[data_idx], i);
                v_min = ImMin(v_min, v);
                v_max = ImMax(v_max, v);
            }
        }
        if (scale_min == FLT_MAX)
            scale_min = v_min;
        if (scale_max == FLT_MAX)
            scale_max = v_max;
    }

    RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

    int res_w = ImMin((int) graph_size.x, values_count) + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0);
    int item_count = values_count + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0);

    // Tooltip on hover
    int v_hovered = -1;
    if (ItemHoverable(inner_bb, id) && inner_bb.Contains(g.IO.MousePos))
    {
        const float t = ImClamp((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0.0f, 0.9999f);
        const int v_idx = (int) (t * item_count);
        IM_ASSERT(v_idx >= 0 && v_idx < values_count);

        // std::string toolTip;
        ImGui::BeginTooltip();
        const int idx0 = (v_idx + values_offset) % values_count;
        if (plot_type == ImGuiPlotType_Lines)
        {
            const int idx1 = (v_idx + 1 + values_offset) % values_count;
            Text("%8d %8d | Name", v_idx, v_idx+1);
            for (int dataIdx = 0; dataIdx < num_datas; ++dataIdx)
            {
                const float v0 = getter(datas[dataIdx], idx0);
                const float v1 = getter(datas[dataIdx], idx1);
                TextColored(colors[dataIdx], "%8.4g %8.4g | %s", v0, v1, names[dataIdx]);
            }
        }
        else if (plot_type == ImGuiPlotType_Histogram)
        {
            for (int dataIdx = 0; dataIdx < num_datas; ++dataIdx)
            {
                const float v0 = getter(datas[dataIdx], idx0);
                TextColored(colors[dataIdx], "%d: %8.4g | %s", v_idx, v0, names[dataIdx]);
            }
        }
        ImGui::EndTooltip();
        v_hovered = v_idx;
    }

    for (int data_idx = 0; data_idx < num_datas; ++data_idx)
    {
        const float t_step = 1.0f / (float) res_w;

        float v0 = getter(datas[data_idx], (0 + values_offset) % values_count);
        float t0 = 0.0f;
        ImVec2 tp0 = ImVec2(t0, 1.0f - ImSaturate((v0 - scale_min) / (scale_max - scale_min)));    // Point in the normalized space of our target rectangle

        const ImU32 col_base = colors[data_idx];
        const ImU32 col_hovered = InvertColorU32(colors[data_idx]);

        //const ImU32 col_base = GetColorU32((plot_type == ImGuiPlotType_Lines) ? ImGuiCol_PlotLines : ImGuiCol_PlotHistogram);
        //const ImU32 col_hovered = GetColorU32((plot_type == ImGuiPlotType_Lines) ? ImGuiCol_PlotLinesHovered : ImGuiCol_PlotHistogramHovered);

        for (int n = 0; n < res_w; n++)
        {
            const float t1 = t0 + t_step;
            const int v1_idx = (int) (t0 * item_count + 0.5f);
            IM_ASSERT(v1_idx >= 0 && v1_idx < values_count);
            const float v1 = getter(datas[data_idx], (v1_idx + values_offset + 1) % values_count);
            const ImVec2 tp1 = ImVec2(t1, 1.0f - ImSaturate((v1 - scale_min) / (scale_max - scale_min)));

            // NB: Draw calls are merged together by the DrawList system. Still, we should render our batch are lower level to save a bit of CPU.
            ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max, tp0);
            ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max, (plot_type == ImGuiPlotType_Lines) ? tp1 : ImVec2(tp1.x, 1.0f));
            if (plot_type == ImGuiPlotType_Lines)
            {
                window->DrawList->AddLine(pos0, pos1, v_hovered == v1_idx ? col_hovered : col_base);
            }
            else if (plot_type == ImGuiPlotType_Histogram)
            {
                if (pos1.x >= pos0.x + 2.0f)
                    pos1.x -= 1.0f;
                window->DrawList->AddRectFilled(pos0, pos1, v_hovered == v1_idx ? col_hovered : col_base);
            }

            t0 = t1;
            tp0 = tp1;
        }
    }

    RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);
}

void PlotMultiLines(
    const char* label,
    int num_datas,
    const char** names,
    const ImColor* colors,
    float(*getter)(const void* data, int idx),
    const void * const * datas,
    int values_count,
    float scale_min,
    float scale_max,
    ImVec2 graph_size)
{
    PlotMultiEx(ImGuiPlotType_Lines, label, num_datas, names, colors, getter, datas, values_count, scale_min, scale_max, graph_size);
}

void PlotMultiHistograms(
    const char* label,
    int num_hists,
    const char** names,
    const ImColor* colors,
    float(*getter)(const void* data, int idx),
    const void * const * datas,
    int values_count,
    float scale_min,
    float scale_max,
    ImVec2 graph_size)
{
    PlotMultiEx(ImGuiPlotType_Histogram, label, num_hists, names, colors, getter, datas, values_count, scale_min, scale_max, graph_size);
}
// End PlotMultiLines(...) and PlotMultiHistograms(...)--------------------------

int DefaultInputTextAutoCompletionCallback(ImGuiInputTextCallbackData *data) {
    InputTextWithAutoCompletionData& mad = *((InputTextWithAutoCompletionData*) data->UserData);
    if (mad.newTextToSet.size()>0 && mad.newTextToSet[0]!='\0') {
        data->DeleteChars(0,data->BufTextLen);
        data->InsertChars(0,&mad.newTextToSet[0]);
        mad.newTextToSet[0]='\0';
    }
    if      (data->EventKey==ImGuiKey_DownArrow) ++mad.deltaTTItems;
    else if (data->EventKey==ImGuiKey_UpArrow) --mad.deltaTTItems;
    else if (data->EventKey==ImGuiKey_Tab) {mad.tabPressed=true;}
    return 0;
}
float InputTextWithAutoCompletionData::Opacity = 0.6f;
int InputTextWithAutoCompletionData::HelperGetItemInsertionPosition(const char* txt,bool (*items_getter)(void*, int, const char**), int items_count, void* user_data,bool* item_is_already_present_out) {
    if (item_is_already_present_out) *item_is_already_present_out=false;
    if (!txt || txt[0]=='\0' || !items_getter || items_count<0) return -1;
    const char* itxt = NULL;int cmp = 0;
    for (int i=0;i<items_count;i++) {
        if (items_getter(user_data,i,&itxt))   {
            if ((cmp=strcmp(itxt,txt))>=0)  {
                if (item_is_already_present_out && cmp==0) *item_is_already_present_out=true;
                return i;
            }
        }
    }
    return items_count;
}
int InputTextWithAutoCompletionData::HelperInsertItem(const char* txt,bool (*items_getter)(void*, int, const char**),bool (*items_inserter)(void*, int,const char*), int items_count, void* user_data,bool* item_is_already_present_out) {
    if (!txt || txt[0]=='\0' || !items_getter || !items_inserter || items_count<0) return -1;
    bool alreadyPresent=false;
    if (!item_is_already_present_out) item_is_already_present_out=&alreadyPresent;
    const int itemPosition = HelperGetItemInsertionPosition(txt,items_getter,items_count,user_data,item_is_already_present_out);
    if (!(*item_is_already_present_out) && itemPosition>=0) {
        if (items_inserter(user_data,itemPosition,txt)) return itemPosition;
        else return -1;
    }
    return itemPosition;
}
bool InputTextWithAutoCompletion(const char* label, char* buf, size_t buf_size, InputTextWithAutoCompletionData* pAutocompletion_data, bool (*autocompletion_items_getter)(void*, int, const char**), int autocompletion_items_size, void* autocompletion_user_data, int num_visible_autocompletion_items) {
    IM_ASSERT(pAutocompletion_data);
    IM_ASSERT(autocompletion_items_getter);
    InputTextWithAutoCompletionData& ad = *pAutocompletion_data;
    ad.inited = true;
    const ImGuiInputTextFlags itFlags = (!(ad.newTextToSet.size()>0 && ad.newTextToSet[0]!='\0')) ?
                (ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_AutoSelectAll) :
                (ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackAlways);
    const ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
    const bool rv = ImGui::InputText(label,buf,buf_size,itFlags|ad.additionalFlags,DefaultInputTextAutoCompletionCallback,(void*)&ad);
    if (rv) {
        // return pressed
        ad.itemPositionOfReturnedText=ad.itemIndexOfReturnedText=-1;
        if (strlen(buf)>0)  {
            const char* txt=NULL;
            int itemPlacement = 0,comp = 0, alreadyPresentIndex = -1;
            for (int i=0;i<autocompletion_items_size;i++) {
                if (autocompletion_items_getter(autocompletion_user_data,i,&txt))   {
                    comp = strcmp(buf,txt);
                    if (comp>0) ++itemPlacement;
                    else if (comp==0) {
                        alreadyPresentIndex=i;
                        break;    // already present
                    }
                }
            }
            if (alreadyPresentIndex>=0)	{ad.itemIndexOfReturnedText=alreadyPresentIndex;}
            else {ad.itemPositionOfReturnedText=itemPlacement;}
        }
        return rv;
    }
    bool inputTextActive = ImGui::IsItemActive();
    //ImGui::SameLine();ImGui::Text(label);
    if (inputTextActive && autocompletion_items_size>0) {
        const int numItems = autocompletion_items_size;
        if (buf[0]!='\0' && numItems>0) {
            const int buffersize = strlen(buf);
            if (ad.bufTextLen!=buffersize) {
                ad.bufTextLen=buffersize;
                ad.deltaTTItems = 0;    // We reset the UP/DOWN offset whe text changes
            }

            int selectedTTItemIndex = numItems-1;
            const char* txt=NULL;
            // We need to fetch the selectedTTItemIndex here
            if (ad.lastSelectedTTItemIndex>=0 && ad.lastSelectedTTItemIndex<numItems && autocompletion_items_getter(autocompletion_user_data,ad.lastSelectedTTItemIndex,&txt))   {
                // Speed up branch (we start our search from previous frame: ad.lastSelectedTTItemIndex
                int i = ad.lastSelectedTTItemIndex;
                //int cnt = 0;
                if (strcmp(txt,buf)<0)  {
                    while (i<numItems) {
                        if (autocompletion_items_getter(autocompletion_user_data,++i,&txt))   {
                            if (strcmp(txt,buf)>=0)  {selectedTTItemIndex=i;break;}
                        }
                        //++cnt;
                    }
                    if (i>=numItems) selectedTTItemIndex=numItems-1;
                }
                else {
                    while (i>=0) {
                        if (autocompletion_items_getter(autocompletion_user_data,i-1,&txt))   {
                            if (strcmp(txt,buf)<0)  {selectedTTItemIndex=i;break;}
                        }
                        --i;
                        //++cnt;
                    }
                    if (i<0) selectedTTItemIndex=0;
                }
                //static int oldCnt=10000000;if (cnt!=oldCnt) {fprintf(stderr,"cnt=%d\n",cnt);oldCnt=cnt;}
            }
            else {
                // Normal (slow) branch
                for (int i=0;i<numItems;i++) {
                    if (autocompletion_items_getter(autocompletion_user_data,i,&txt))   {
                        if (strcmp(txt,buf)>=0)  {selectedTTItemIndex=i;break;}
                    }
                }
            }
            if (selectedTTItemIndex + ad.deltaTTItems>=numItems) ad.deltaTTItems=numItems-selectedTTItemIndex-1;
            else if (selectedTTItemIndex + ad.deltaTTItems<0) ad.deltaTTItems=-selectedTTItemIndex;
            ad.lastSelectedTTItemIndex=selectedTTItemIndex+=ad.deltaTTItems;
            if (ad.tabPressed)  {
                if (selectedTTItemIndex<numItems) {
                    const char* selectedTTItemText=NULL;
                    if (!autocompletion_items_getter(autocompletion_user_data,selectedTTItemIndex,&selectedTTItemText))   {
                        IM_ASSERT(true);
                    }
                    IM_ASSERT(selectedTTItemText && strlen(selectedTTItemText)>0);
                    const size_t len = strlen(selectedTTItemText);
                    ad.newTextToSet.resize(len+1);
                    strcpy(&ad.newTextToSet[0],selectedTTItemText);
                }
                ad.deltaTTItems=0;
            }

            const int MaxNumTooltipItems = num_visible_autocompletion_items>0 ? num_visible_autocompletion_items : 7;
            const float textLineHeightWithSpacing = ImGui::GetTextLineHeightWithSpacing();
            const ImVec2 storedCursorScreenPos = ImGui::GetCursorScreenPos();
            const int MaxNumItemsBelowInputText = (ImGui::GetIO().DisplaySize.y - (storedCursorScreenPos.y+textLineHeightWithSpacing))/textLineHeightWithSpacing;
            const int MaxNumItemsAboveInputText = storedCursorScreenPos.y/textLineHeightWithSpacing;
            int numTTItems = numItems>MaxNumTooltipItems?MaxNumTooltipItems:numItems;
            bool useUpperScreen = false;
            if (numTTItems>MaxNumItemsBelowInputText && MaxNumItemsBelowInputText<MaxNumItemsAboveInputText)    {
                useUpperScreen = true;
                if (numTTItems>MaxNumItemsAboveInputText) numTTItems = MaxNumItemsAboveInputText;
            }
            else if (numTTItems>MaxNumItemsBelowInputText) numTTItems=MaxNumItemsBelowInputText;

            const int numTTItemsHalf = numTTItems/2;
            int firstTTItemIndex = selectedTTItemIndex-numTTItemsHalf;
            if (selectedTTItemIndex+numTTItemsHalf>=numItems) firstTTItemIndex = numItems-numTTItems;
            if (firstTTItemIndex<0) firstTTItemIndex=0;

            const ImVec2 inputTextBoxSize = ImGui::GetItemRectSize();
            float labelWidth = ImGui::CalcTextSize(label,NULL,true).x;
            if (labelWidth>0.0f) labelWidth+=ImGui::GetStyle().ItemInnerSpacing.x;            
            const ImVec2 ttWindowSize(ImVec2(inputTextBoxSize.x-labelWidth,numTTItems*textLineHeightWithSpacing));
            const ImVec2 newCursorScreenPos(cursorScreenPos.x,cursorScreenPos.y+(useUpperScreen ? (-numTTItems*textLineHeightWithSpacing) : (textLineHeightWithSpacing)));
            ImGui::SetCursorScreenPos(newCursorScreenPos);
            ImGui::SetNextWindowPos(newCursorScreenPos);
            ImGui::SetNextWindowSize(ttWindowSize);
            const float xPadding = textLineHeightWithSpacing*0.5f;
            ImGuiWindow* window = ImGui::GetCurrentWindowRead();
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,0);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));

            //if (ImGui::Begin("##TooltipAutocomplete", NULL,ttWindowSize,InputTextWithAutoCompletionData::Opacity,ImGuiWindowFlags_Tooltip|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse))  // Old API
            ImGui::SetNextWindowSize(ttWindowSize, ImGuiCond_FirstUseEver);
            if (InputTextWithAutoCompletionData::Opacity>=0.f) ImGui::SetNextWindowBgAlpha(InputTextWithAutoCompletionData::Opacity);
            if (ImGui::Begin("##TooltipAutocomplete", NULL,ImGuiWindowFlags_Tooltip|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse))
            {
                // We must always use newCursorScreenPos when mnually drawing inside this window
                if (!window) window = ImGui::GetCurrentWindowRead();
                for (int i=firstTTItemIndex,iSz=firstTTItemIndex+numTTItems;i<iSz;i++) {
                    const ImVec2 start(newCursorScreenPos.x,newCursorScreenPos.y+(i-firstTTItemIndex)*textLineHeightWithSpacing);
                    if (i==ad.currentAutocompletionItemIndex) {
                        ImVec2 end(start.x+ttWindowSize.x-1,start.y+textLineHeightWithSpacing);
                        ImU32 col = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Header]);
                        ImGui::GetWindowDrawList()->AddRectFilled(start,end,col,0,0);
                        //ImGui::GetWindowDrawList()->AddRect(start,end,ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]),0,0,1.f);

                        ImGui::PushStyleColor(ImGuiCol_Text,ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                    }
                    if (i==selectedTTItemIndex) {
                        ImVec2 end(start.x+ttWindowSize.x-1,start.y+textLineHeightWithSpacing);
                        ImU32 col = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Button]);
                        ImGui::GetWindowDrawList()->AddRectFilled(start,end,col,0,0);
                        ImGui::GetWindowDrawList()->AddRect(start,end,ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]),0,0,1.f);

                    }
                    const bool ok = autocompletion_items_getter(autocompletion_user_data,i,&txt);
                    ImGui::RenderText(ImVec2(start.x+xPadding,start.y+window->DC.PrevLineTextBaseOffset*0.5f),ok ? txt : "unknown");    // Not sure about +window->DC.PrevLineTextBaseOffset*0.5f
                    if (i==ad.currentAutocompletionItemIndex) ImGui::PopStyleColor();
                }
            }
            ImGui::End();
            ImGui::PopStyleVar(2);
            ImGui::SetCursorScreenPos(storedCursorScreenPos);   // restore


            // Debug:
            //ImGui::SetTooltip("autocompletionEntries.size()=%d\nbufData.currentAutocompletionItemIndex=%d\nad.deltaTTItems=%d\nfirstTTItemIndex=%d\nnumTTItems=%d",autocompletion_items_size,ad.currentAutocompletionItemIndex,ad.deltaTTItems,firstTTItemIndex,numTTItems);

        }
        else ad.deltaTTItems = 0;
    }
    ad.tabPressed=false;
    return rv;
}

char InputComboWithAutoCompletionData::ButtonCharcters[3][5] = {"+","r","x"};
char InputComboWithAutoCompletionData::ButtonTooltips[3][128] = {"add","rename","delete"};

bool InputComboWithAutoCompletion(const char* label, int *current_item, size_t autocompletion_buffer_size, InputComboWithAutoCompletionData* pAutocompletion_data,
                                  bool (*items_getter)(void*, int, const char**),       // gets item at position ... (cannot be NULL)
                                  bool (*items_inserter)(void*, int,const char*),       // inserts item at position ... (cannot be NULL)
                                  bool (*items_deleter)(void*, int),                    // deletes item at position ... (can be NULL)
                                  bool (*items_renamer)(void *, int, int, const char *),// deletes item at position, and inserts renamed item at new position  ... (can be NULL)
                                  int items_count, void* user_data, int num_visible_items)   {

    IM_ASSERT(pAutocompletion_data);
    IM_ASSERT(items_getter);
    IM_ASSERT(items_inserter);
    IM_ASSERT(autocompletion_buffer_size>1);
    bool rv = false;
    InputComboWithAutoCompletionData* pad = pAutocompletion_data;
    pad->inited = true;if (current_item) pad->currentAutocompletionItemIndex=*current_item;
    pad->itemHovered = pad->itemActive = false;
    if (pad->buf.size()<(int)autocompletion_buffer_size) {
        pad->buf.resize(autocompletion_buffer_size);pad->buf[0]='\0';
    }
    ImGui::PushID(pAutocompletion_data);
    if (!pad->inputTextShown) {
        // ImGui::Combo(...) here
        const bool aValidItemIsSelected = items_count>0 && pad->currentAutocompletionItemIndex>=0 && pad->currentAutocompletionItemIndex<items_count;
        const bool hasDeleteButton = items_deleter && aValidItemIsSelected;
        const bool hasRenameButton = items_renamer && aValidItemIsSelected;
        const float singleButtonPadding = ImGui::GetStyle().FramePadding.x * 2.0f;
        const float addButtonWidth      = singleButtonPadding + ImGui::CalcTextSize(InputComboWithAutoCompletionData::ButtonCharcters[0]).x;
        const float renameButtonWidth   = singleButtonPadding + ImGui::CalcTextSize(InputComboWithAutoCompletionData::ButtonCharcters[1]).x;
        const float deleteButtonWidth   = singleButtonPadding + ImGui::CalcTextSize(InputComboWithAutoCompletionData::ButtonCharcters[2]).x;
        const float buttonsWidth = addButtonWidth + (hasRenameButton ? renameButtonWidth : 0.f) + (hasDeleteButton ? deleteButtonWidth : 0.f);
        const ImGuiWindow* window = ImGui::GetCurrentWindowRead();
        float comboWidth = (window->DC.ItemWidthStack.size()>0 ? window->DC.ItemWidthStack[window->DC.ItemWidthStack.size()-1] : window->ItemWidthDefault);
        bool noButtons = false;
        if (comboWidth>buttonsWidth) comboWidth-=buttonsWidth;
        else noButtons=true;

        // Combo
        ImGui::PushItemWidth(comboWidth);
        rv= ImGui::Combo("###icwac",current_item,items_getter,user_data,items_count,num_visible_items);
        ImGui::PopItemWidth();
        if (rv && current_item)  pad->currentAutocompletionItemIndex=*current_item;
        const bool comboHovered = ImGui::IsItemHovered();
        const bool comboActive = ImGui::IsItemActive();
        pad->itemHovered|=comboHovered;
        pad->itemActive|=comboActive;
        bool mustEnterEditMode = comboHovered && ImGui::IsMouseClicked(1);
        // Buttons
        if (!noButtons) {
            ImGui::SameLine(0,0);
            mustEnterEditMode|=ImGui::Button(InputComboWithAutoCompletionData::ButtonCharcters[0]);
            if (InputComboWithAutoCompletionData::ButtonTooltips[0][0]!='\0' && ImGui::IsItemHovered()) ImGui::SetTooltip("%s",InputComboWithAutoCompletionData::ButtonTooltips[0]);
            if (mustEnterEditMode) {
                ++pad->inputTextShown;
                if (pad->buf.size()<(int)autocompletion_buffer_size) {
                    pad->buf.resize(autocompletion_buffer_size);
                    pad->buf[0]='\0';
                }
            }
            if (hasRenameButton)   {
                ImGui::SameLine(0,0);
                if (ImGui::Button(InputComboWithAutoCompletionData::ButtonCharcters[1])) {
                    ++pad->inputTextShown;pad->isRenaming = true;
                    const char* textToHighlight = NULL;
                    if (items_getter(user_data,pad->currentAutocompletionItemIndex,&textToHighlight) && textToHighlight) {
                        const int len = (int) strlen(textToHighlight);
                        if (len>0 && len<(int)autocompletion_buffer_size)    {
                            strcpy(&pad->buf[0],textToHighlight);
                        }
                    }
                }
                if (InputComboWithAutoCompletionData::ButtonTooltips[1][0]!='\0' && ImGui::IsItemHovered()) ImGui::SetTooltip("%s",InputComboWithAutoCompletionData::ButtonTooltips[1]);
            }
            if (hasDeleteButton)   {
                ImGui::SameLine(0,0);
                if (ImGui::Button(InputComboWithAutoCompletionData::ButtonCharcters[2]) && items_deleter(user_data,pad->currentAutocompletionItemIndex)) {
                    rv = true;
                    if (ImGui::GetIO().KeyShift) {
                        const int num_items = items_count -1;
                        --pad->currentAutocompletionItemIndex;
                        if (pad->currentAutocompletionItemIndex<0) pad->currentAutocompletionItemIndex=num_items-1;
                    }
                    else pad->currentAutocompletionItemIndex=-1;
                    if (current_item) *current_item = pad->currentAutocompletionItemIndex;
                }
                if (InputComboWithAutoCompletionData::ButtonTooltips[2][0]!='\0' && ImGui::IsItemHovered()) ImGui::SetTooltip("%s",InputComboWithAutoCompletionData::ButtonTooltips[2]);
            }
        }
        // Label
        ImGui::SameLine();
        //ImGui::Text("%s",label);    // This doesn't cut "##". We must add all this cucumberson and error-prone code to workaround this (for correct alignment and isHovered detection):
        const ImVec2 label_size = CalcTextSize(label, NULL, true);
        if (label_size.x>0) ImGui::RenderText(ImVec2(window->DC.CursorPos.x,window->DC.CursorPos.y+window->DC.CurrLineTextBaseOffset),label);
        const ImRect label_bb(window->DC.CursorPos,ImVec2(window->DC.CursorPos.x + label_size.x, window->DC.CursorPos.y + label_size.y + ImGui::GetStyle().FramePadding.y*2.0f));
        ImGui::ItemSize(label_bb, ImGui::GetStyle().FramePadding.y);
        const ImGuiID labelID = 0;  // is this allowed ?
        if (ImGui::ItemAdd(label_bb, labelID)) pad->itemHovered|=ItemHoverable(label_bb, labelID);
    }
    else {
        // ImGui::InputText(...) here
        if (pad->inputTextShown==1) {++pad->inputTextShown;ImGui::SetKeyboardFocusHere(0);}
        const bool enter_pressed = InputTextWithAutoCompletion(label,&pad->buf[0],autocompletion_buffer_size,pad,items_getter,items_count,user_data,num_visible_items);
        const bool inputTextHovered = ImGui::IsItemHovered();
        const bool inputTextActive = ImGui::IsItemActive();
        pad->itemHovered|=inputTextHovered;
        pad->itemActive|=inputTextActive;
        bool mustAllowFurtherEditing = false;
        if (enter_pressed) {
            if (pad->buf[0]!='\0')  {
                if (pad->isRenaming) {
                    if (pad->getItemPositionOfReturnedText()>=0)    {
                        const int oldItemPosition = pad->currentAutocompletionItemIndex;
                        const int newItemPositionInOldList = pad->getItemPositionOfReturnedText();
                        int newItemPositionInNewList = newItemPositionInOldList;
                        if (oldItemPosition<newItemPositionInOldList) newItemPositionInNewList=newItemPositionInOldList-1;
                        if (items_renamer(user_data,oldItemPosition,newItemPositionInNewList,&pad->buf[0])) {
                            pad->currentAutocompletionItemIndex=newItemPositionInNewList;
                            if (current_item) *current_item = pad->currentAutocompletionItemIndex;
                            rv = true;
                            pad->buf[0]='\0';
                            pad->inputTextShown = mustAllowFurtherEditing ? 1 : 0;
                            pad->isRenaming = false;
                        }
                    }
                }
                else {
                    if (pad->getItemIndexOfReturnedText()>=0) {
                        pad->currentAutocompletionItemIndex=pad->getItemIndexOfReturnedText();
                        if (current_item) *current_item = pad->currentAutocompletionItemIndex;
                        rv = true;
                    }
                    else if (pad->getItemPositionOfReturnedText()>=0 && items_inserter(user_data,pad->getItemPositionOfReturnedText(),&pad->buf[0])) {
                        pad->currentAutocompletionItemIndex=pad->getItemPositionOfReturnedText();
                        if (current_item) *current_item = pad->currentAutocompletionItemIndex;
                        rv = true;
                    }
                }
                if (ImGui::GetIO().KeyShift && !pad->isRenaming) mustAllowFurtherEditing=true;
                pad->buf[0]='\0';
                pad->inputTextShown = mustAllowFurtherEditing ? 1 : 0;
                pad->isRenaming = false;
            }

        }
        if ((ImGui::IsItemActiveLastFrame() && !inputTextActive) || (inputTextHovered && ImGui::IsMouseClicked(1)))	{
            pad->inputTextShown = mustAllowFurtherEditing ? 1 : 0;
            pad->buf[0]='\0';
            pad->isRenaming = false;
        }
    }
    ImGui::PopID();
    return rv;
}

}   // ImGui namespace


// Tree view stuff starts here ==============================================================
#include <stdlib.h> // qsort (Maybe we could add a define to exclude sorting...)

// Enforce cdecl calling convention for functions called by the standard library, in case compilation settings changed the default to e.g. __vectorcall
#ifdef _MSC_VER
#define IMGUIVC_CDECL __cdecl
#else
#define IMGUIVC_CDECL
#endif

namespace ImGui {

struct MyTreeViewHelperStruct {
    TreeView& parentTreeView;
    bool mustDrawAllNodesAsDisabled;
    ImGuiWindow* window;
    float windowWidth;
    float arrowOffset;
    TreeViewNode::Event& event;
    bool hasCbGlyphs,hasArrowGlyphs;
    struct ContextMenuDataStruct {
        TreeViewNode* activeNode;
        TreeView* parentTreeView;
        bool activeNodeChanged;
        ContextMenuDataStruct() : activeNode(NULL),parentTreeView(NULL),activeNodeChanged(false) {}
        void reset() {*this = ContextMenuDataStruct();}
    };
    static ContextMenuDataStruct ContextMenuData;
    struct NameEditingStruct {
        TreeViewNode* editingNode;
        char textInput[256];
        bool mustFocusInputText;
        NameEditingStruct(TreeViewNode* n=NULL,const char* startingText=NULL) : editingNode(n) {
            textInput[0]='\0';
            if (startingText) {
                const int len = strlen(startingText);
                if (len<255) strcpy(&textInput[0],startingText);
                else {
                    strncpy(&textInput[0],startingText,255);
                    textInput[255]='\0';
                }
            }
            mustFocusInputText = (editingNode!=NULL);
        }
        void reset() {*this = NameEditingStruct();}
    };
    static NameEditingStruct NameEditing;
    MyTreeViewHelperStruct(TreeView& parent,TreeViewNode::Event& _event) : parentTreeView(parent),mustDrawAllNodesAsDisabled(false),event(_event) {
        window = ImGui::GetCurrentWindow();
        windowWidth = ImGui::GetWindowWidth();
        hasCbGlyphs=TreeView::FontCheckBoxGlyphs[0][0]!='\0';
        hasArrowGlyphs=TreeView::FontArrowGlyphs[0][0]!='\0';
        arrowOffset = hasArrowGlyphs ? (ImGui::CalcTextSize(&TreeView::FontArrowGlyphs[0][0]).x+ImGui::GetCurrentContext()->Style.ItemSpacing.x) : (ImGui::GetCurrentContext()->FontSize+ImGui::GetCurrentContext()->Style.ItemSpacing.x);
    }
    void fillEvent(TreeViewNode* _eventNode=NULL,TreeViewNode::State _eventFlag=TreeViewNode::STATE_NONE,bool _eventFlagRemoved=false) {
        event.node = _eventNode;
        event.state = _eventFlag;
        event.wasStateRemoved = _eventFlagRemoved;
        if (event.state!=TreeViewNode::STATE_NONE) event.type = TreeViewNode::EVENT_STATE_CHANGED;
    }
    // Sorters
    inline static int IMGUIVC_CDECL SorterByDisplayName(const void *pn1, const void *pn2)  {
        const char* s1 = (*((const TreeViewNode**)pn1))->data.displayName;
        const char* s2 = (*((const TreeViewNode**)pn2))->data.displayName;
        return strcmp(s1,s2);   // Hp) displayName can't be NULL
    }
    inline static int IMGUIVC_CDECL SorterByDisplayNameReverseOrder(const void *pn1, const void *pn2)  {
        const char* s2 = (*((const TreeViewNode**)pn1))->data.displayName;
        const char* s1 = (*((const TreeViewNode**)pn2))->data.displayName;
        return strcmp(s1,s2);   // Hp) displayName can't be NULL
    }
    inline static int IMGUIVC_CDECL SorterByTooltip(const void *pn1, const void *pn2)  {
        const char* s1 = (*((const TreeViewNode**)pn1))->data.tooltip;
        const char* s2 = (*((const TreeViewNode**)pn2))->data.tooltip;
        return (s1 && s2) ? (strcmp(s1,s2)) : (s1 ? -1 : (s2 ? 1 : -1));
    }
    inline static int IMGUIVC_CDECL SorterByTooltipReverseOrder(const void *pn1, const void *pn2)  {
        const char* s2 = (*((const TreeViewNode**)pn1))->data.tooltip;
        const char* s1 = (*((const TreeViewNode**)pn2))->data.tooltip;
        return (s1 && s2) ? (strcmp(s1,s2)) : (s1 ? -1 : (s2 ? 1 : -1));
    }
    inline static int IMGUIVC_CDECL SorterByUserText(const void *pn1, const void *pn2)  {
        const char* s1 = (*((const TreeViewNode**)pn1))->data.userText;
        const char* s2 = (*((const TreeViewNode**)pn2))->data.userText;
        return (s1 && s2) ? (strcmp(s1,s2)) : (s1 ? -1 : (s2 ? 1 : -1));
    }
    inline static int IMGUIVC_CDECL SorterByUserTextReverseOrder(const void *pn1, const void *pn2)  {
        const char* s2 = (*((const TreeViewNode**)pn1))->data.userText;
        const char* s1 = (*((const TreeViewNode**)pn2))->data.userText;
        return (s1 && s2) ? (strcmp(s1,s2)) : (s1 ? -1 : (s2 ? 1 : -1));
    }
    inline static int IMGUIVC_CDECL SorterByUserId(const void *pn1, const void *pn2)  {
        const TreeViewNode* n1 = *((const TreeViewNode**)pn1);
        const TreeViewNode* n2 = *((const TreeViewNode**)pn2);
        return n1->data.userId-n2->data.userId;
    }
    inline static int IMGUIVC_CDECL SorterByUserIdReverseOrder(const void *pn1, const void *pn2)  {
        const TreeViewNode* n2 = *((const TreeViewNode**)pn1);
        const TreeViewNode* n1 = *((const TreeViewNode**)pn2);
        return n1->data.userId-n2->data.userId;
    }
    // Serialization
#if (defined(IMGUIHELPER_H_) && !defined(NO_IMGUIHELPER_SERIALIZATION))
#ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
    static void Serialize(ImGuiHelper::Serializer& s,TreeViewNode* n) {
        if (n->parentNode)  {
            s.saveTextLines(n->data.displayName,"name");
            if (n->data.tooltip) s.saveTextLines(n->data.tooltip,"tooltip");
            if (n->data.userText) s.saveTextLines(n->data.userText,"userText");
            if (n->data.userId) s.save(&n->data.userId,"userId");
            s.save(&n->state,"state");
        }
        else {
            // It's a TreeView: we must save its own data instead
            ImGui::TreeView& tv = *(static_cast<ImGui::TreeView*>(n));
            s.save(ImGui::FT_COLOR,&tv.stateColors[0].x,"stateColors0",4);
            s.save(ImGui::FT_COLOR,&tv.stateColors[1].x,"stateColors1",4);
            s.save(ImGui::FT_COLOR,&tv.stateColors[2].x,"stateColors2",4);
            s.save(ImGui::FT_COLOR,&tv.stateColors[3].x,"stateColors3",4);
            s.save(ImGui::FT_COLOR,&tv.stateColors[4].x,"stateColors4",4);
            s.save(ImGui::FT_COLOR,&tv.stateColors[5].x,"stateColors5",4);
            int tmp = (int) tv.selectionMode;s.save(&tmp,"selectionMode");
            s.save(&tv.allowMultipleSelection,"allowMultipleSelection");
            tmp = (int) tv.checkboxMode;s.save(&tmp,"checkboxMode");
            s.save(&tv.allowAutoCheckboxBehaviour,"allowAutoCheckboxBehaviour");
            s.save(&tv.collapseToLeafNodesAtNodeDepth,"collapseToLeafNodesAtNodeDepth");
            s.save(&tv.inheritDisabledLook,"inheritDisabledLook");
        }
        const int numChildNodes = n->childNodes ? n->childNodes->size() : -1;
        s.save(&numChildNodes,"numChildNodes");

        if (n->childNodes) {
            for (int i=0,isz=n->childNodes->size();i<isz;i++) {
                TreeViewNode* node = (*(n->childNodes))[i];
                Serialize(s,node);
            }
        }
    }
#endif //NO_IMGUIHELPER_SERIALIZATION_SAVE
#ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
    struct ParseCallbackStruct {
        TreeViewNode* node;
        int numChildNodes;
    };
    static bool ParseCallback(ImGuiHelper::FieldType /*ft*/,int /*numArrayElements*/,void* pValue,const char* name,void* userPtr)    {
        ParseCallbackStruct& cbs = *((ParseCallbackStruct*)userPtr);
        TreeViewNode* n = cbs.node;
        if (strcmp(name,"name")==0)             ImGuiHelper::StringAppend(n->data.displayName,(const char*)pValue,false);
        else if (strcmp(name,"tooltip")==0)     ImGuiHelper::StringAppend(n->data.tooltip,(const char*)pValue,true);
        else if (strcmp(name,"userText")==0)    ImGuiHelper::StringAppend(n->data.userText,(const char*)pValue,true);
        else if (strcmp(name,"userId")==0)      n->getUserId() = *((int*)pValue);
        else if (strcmp(name,"state")==0)       n->state = *((int*)pValue);
        else if (strcmp(name,"numChildNodes")==0) {cbs.numChildNodes = *((int*)pValue);return true;}
        return false;
    }
    static bool ParseTreeViewDataCallback(ImGuiHelper::FieldType /*ft*/,int /*numArrayElements*/,void* pValue,const char* name,void* userPtr)    {
        TreeView& tv = *((TreeView*)userPtr);
        if      (strcmp(name,"stateColors0")==0)                tv.stateColors[0] = *((ImVec4*)pValue);
        else if (strcmp(name,"stateColors1")==0)                tv.stateColors[1] = *((ImVec4*)pValue);
        else if (strcmp(name,"stateColors2")==0)                tv.stateColors[2] = *((ImVec4*)pValue);
        else if (strcmp(name,"stateColors3")==0)                tv.stateColors[3] = *((ImVec4*)pValue);
        else if (strcmp(name,"stateColors4")==0)                tv.stateColors[4] = *((ImVec4*)pValue);
        else if (strcmp(name,"stateColors5")==0)                tv.stateColors[5] = *((ImVec4*)pValue);
        else if (strcmp(name,"selectionMode")==0)               tv.selectionMode = *((int*)pValue);
        else if (strcmp(name,"allowMultipleSelection")==0)      tv.allowMultipleSelection = *((bool*)pValue);
        else if (strcmp(name,"checkboxMode")==0)                tv.checkboxMode = *((int*)pValue);
        else if (strcmp(name,"allowAutoCheckboxBehaviour")==0)  tv.allowAutoCheckboxBehaviour = *((bool*)pValue);
        else if (strcmp(name,"collapseToLeafNodesAtNodeDepth")==0) tv.collapseToLeafNodesAtNodeDepth = *((int*)pValue);
        else if (strcmp(name,"inheritDisabledLook")==0)         {tv.inheritDisabledLook = *((bool*)pValue);return true;}
        return false;
    }
    static void Deserialize(ImGuiHelper::Deserializer& d,TreeViewNode* n,const char*& amount,TreeView* tv,TreeView::TreeViewNodeCreationDelationCallback callback=NULL)  {
        if (!n->parentNode) {
            // It's a TreeView: we must load its own data
            ImGui::TreeView& tv = *(static_cast<ImGui::TreeView*>(n));
            amount = d.parse(ParseTreeViewDataCallback,(void*)&tv);
        }
        ParseCallbackStruct cbs;cbs.node=n;cbs.numChildNodes=0;
        amount = d.parse(ParseCallback,(void*)&cbs,amount);
        if (cbs.numChildNodes>=0)   {
            if (cbs.numChildNodes==0) n->addEmptyChildNodeVector();
            else {
                for (int i=0;i<cbs.numChildNodes;i++) {
                    TreeViewNode* node = TreeViewNode::CreateNode(TreeViewNode::Data(),n);
                    Deserialize(d,node,amount,tv,callback);
                    if (callback) callback(node,*tv,false,tv->treeViewNodeCreationDelationCbUserPtr);
                }
            }
        }
    }
#endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#endif //NO_IMGUIHELPER_SERIALIZATION


    // Default event callbacks
    static void TreeViewNodePopupMenuProvider(TreeViewNode* n,TreeView& tv,void*) {

        // Actually all the nodes can be set to default or be enabled/disabled, but it's just a test...
        const bool canBeSetToDefault = n->isRootNode() && n->isStateMissing(TreeViewNode::STATE_DEFAULT);
        const bool canBeEnabledOrDisabled = true;
        const bool itsChildNodesAreSortable = n->childNodes && n->childNodes->size()>1;
        const bool canBeRenamed = !n->isInRenamingMode();
        const int nodeIndex = n->getNodeIndex();
        const int numSiblingsWithMe = n->getNumSiblings(true);
        const bool canMoveUp = numSiblingsWithMe>1 && nodeIndex>0;
        const bool canMoveDown = numSiblingsWithMe>1 && nodeIndex<(numSiblingsWithMe-1);
        const bool canAddSiblings = true;
        const bool canAddChildNodes = true;
        const bool canDeleteNode = true;
	const bool canForceCheckboxVisibility = tv.checkboxMode!=TreeViewNode::MODE_ALL;
	const bool canBeHidden = true;

        if ((canBeSetToDefault || canBeEnabledOrDisabled || itsChildNodesAreSortable
             || canBeRenamed || canMoveUp || canMoveDown
	     || canAddSiblings || canAddChildNodes || canDeleteNode
	     || canForceCheckboxVisibility || canBeHidden)
                && ImGui::BeginPopup(TreeView::GetTreeViewNodePopupMenuName()))   {
            ImGui::PushID(n);
            if (canBeSetToDefault && ImGui::MenuItem("Set as default root item"))  {
                tv.removeStateFromAllDescendants(TreeViewNode::STATE_DEFAULT);  // allow (at most) one default node
                n->addState(TreeViewNode::STATE_DEFAULT);
            }
            if (canBeEnabledOrDisabled) {
                if (n->isStatePresent(TreeViewNode::STATE_DISABLED))  {
                    if (ImGui::MenuItem("Enable item")) n->removeState(TreeViewNode::STATE_DISABLED);
                }
                else if (ImGui::MenuItem("Disable item")) n->addState(TreeViewNode::STATE_DISABLED);
            }
            if (itsChildNodesAreSortable)   {
                if (ImGui::MenuItem("Sort child nodes in acending order")) n->sortChildNodesByDisplayName(false,false);
                if (ImGui::MenuItem("Sort child nodes in descending order")) n->sortChildNodesByDisplayName(false,true);
            }
            if (canBeRenamed && ImGui::MenuItem("Rename")) {n->startRenamingMode();}
            if (canMoveUp && ImGui::MenuItem("Move up")) n->moveNodeTo(nodeIndex-1);
            if (canMoveDown && ImGui::MenuItem("Move down")) n->moveNodeTo(nodeIndex+1);
            if (canAddSiblings && ImGui::MenuItem("Create new sibling node")) {
                TreeViewNode* n2 = n->addSiblingNode(TreeViewNode::Data("New node"));
                n2->startRenamingMode();
            }
            if (canAddChildNodes && ImGui::MenuItem("Create new child node")) {
                n->addState(TreeViewNode::STATE_OPEN);
                TreeViewNode* n2 = n->addChildNode(TreeViewNode::Data("New node"));
                n2->startRenamingMode();
            }
            if (canDeleteNode && ImGui::MenuItem("Delete")) {TreeViewNode::DeleteNode(n);n=NULL;}
	    if (canForceCheckboxVisibility && ImGui::MenuItem("Toggle force checkbox")) n->toggleState(TreeViewNode::STATE_FORCE_CHECKBOX);
	    if (canBeHidden && ImGui::MenuItem("Hide")) n->addState(TreeViewNode::STATE_HIDDEN);
	    ImGui::PopID();
            ImGui::EndPopup();
        }

    }
};
MyTreeViewHelperStruct::ContextMenuDataStruct MyTreeViewHelperStruct::ContextMenuData;
MyTreeViewHelperStruct::NameEditingStruct MyTreeViewHelperStruct::NameEditing;

TreeViewNode::~TreeViewNode() {
    if (this == MyTreeViewHelperStruct::ContextMenuData.activeNode) MyTreeViewHelperStruct::ContextMenuData.reset();
    if (this == MyTreeViewHelperStruct::NameEditing.editingNode) MyTreeViewHelperStruct::NameEditing.reset();
    //dbgDisplay();
    if (childNodes) {
        // delete child nodes
        //for (int i=0,isz=childNodes->size();i<isz;i++)    // Wrong !!
        while (childNodes->size()>0)                        // Right !!
        {
	    TreeViewNode* n = (*childNodes)[0];
            if (n) {
		n->~TreeViewNode();
                ImGui::MemFree(n);
            }
        }
        childNodes->clear();
        childNodes->~ImVector<TreeViewNode*>();
        ImGui::MemFree(childNodes);
        childNodes=NULL;
    }
    if (parentNode && parentNode->childNodes) {
        // remove this node from parentNode->childNodes
        for (int i=0,isz=parentNode->childNodes->size();i<isz;i++) {
	    TreeViewNode* n = (*(parentNode->childNodes))[i];
            if (n==this) {
                for (int j=i+1;j<isz;j++) (*(parentNode->childNodes))[j-1] = (*(parentNode->childNodes))[j];
                parentNode->childNodes->pop_back();
                break;
            }
        }
    }
    if (parentNode) {
        TreeView& tv = getTreeView();
        if (tv.treeViewNodeCreationDelationCb) tv.treeViewNodeCreationDelationCb(this,tv,true,tv.treeViewNodeCreationDelationCbUserPtr);
    }
    parentNode = NULL;
}

TreeViewNode::TreeViewNode(const Data& _data, TreeViewNode *_parentNode, int nodeIndex, bool addEmptyChildNodeVector) : userPtr(NULL),data(_data) {
    parentNode = NULL;
    childNodes = NULL;
    state = 0;
    parentNode = _parentNode;
    if (parentNode) {
        //We must add this node to parentNode->childNodes
        if (!parentNode->childNodes)    {
            parentNode->childNodes = (ImVector<TreeViewNode*>*) ImGui::MemAlloc(sizeof(ImVector<TreeViewNode*>));
            IM_PLACEMENT_NEW(parentNode->childNodes) ImVector<TreeViewNode*>();
            parentNode->childNodes->push_back(this);
        }
        else if (nodeIndex<0 || nodeIndex>=parentNode->childNodes->size()) {
            // append at the end
            parentNode->childNodes->push_back(this);
        }
        else {
            // insert "this" at "nodeIndex"
            const int isz = parentNode->childNodes->size();
            parentNode->childNodes->resize(parentNode->childNodes->size()+1);
            for (int j=isz;j>nodeIndex;j--) (*(parentNode->childNodes))[j] = (*(parentNode->childNodes))[j-1];
            (*(parentNode->childNodes))[nodeIndex] = this;
        }
    }
    if (addEmptyChildNodeVector) {
        childNodes = (ImVector<TreeViewNode*>*) ImGui::MemAlloc(sizeof(ImVector<TreeViewNode*>));
        IM_PLACEMENT_NEW(childNodes) ImVector<TreeViewNode*>();
    }
    if (parentNode) {
        TreeView& tv = getTreeView();
        if (tv.treeViewNodeCreationDelationCb) tv.treeViewNodeCreationDelationCb(this,tv,false,tv.treeViewNodeCreationDelationCbUserPtr);
    }
}


TreeViewNode *TreeViewNode::CreateNode(const Data& _data, TreeViewNode *_parentNode, int nodeIndex, bool addEmptyChildNodeVector) {
    TreeViewNode* n = (TreeViewNode*) ImGui::MemAlloc(sizeof(TreeViewNode));
    IM_PLACEMENT_NEW(n) TreeViewNode(_data,_parentNode,nodeIndex,addEmptyChildNodeVector);
    return n;
}

void TreeViewNode::DeleteNode(TreeViewNode *n) {
    if (n)  {
        n->~TreeViewNode();
        ImGui::MemFree(n);
    }
}

TreeView &TreeViewNode::getTreeView() {
    TreeViewNode* n = this;
    while (n->parentNode) n=n->parentNode;
    IM_ASSERT(n);
    return *(static_cast < TreeView* > (n));
}

const TreeView &TreeViewNode::getTreeView() const {
    const TreeViewNode* n = this;
    while (n->parentNode) n=n->parentNode;
    IM_ASSERT(n);
    return *(static_cast < const TreeView* > (n));
}

TreeViewNode *TreeViewNode::getParentNode() {return (parentNode && parentNode->parentNode) ? parentNode : NULL;}

const TreeViewNode *TreeViewNode::getParentNode() const {return (parentNode && parentNode->parentNode) ? parentNode : NULL;}

int TreeViewNode::getNodeIndex() const   {
    if (!parentNode || !parentNode->childNodes) return 0;
    for (int i=0,isz=parentNode->childNodes->size();i<isz;i++)  {
        if ( (*parentNode->childNodes)[i] == this ) return i;
    }
    return 0;
}

void TreeViewNode::moveNodeTo(int nodeIndex)   {
    if (!parentNode || !parentNode->childNodes) return;
    const int isz = parentNode->childNodes->size();
    if (isz<2) return;
    if (nodeIndex<0 || nodeIndex>=isz) nodeIndex = isz-1;
    const int curNodeIndex = getNodeIndex();
    if (curNodeIndex==nodeIndex) return;
    if (curNodeIndex<nodeIndex) {
        for (int i=curNodeIndex;i<nodeIndex;i++)  (*parentNode->childNodes)[i] = (*parentNode->childNodes)[i+1];
    }
    else {
        for (int i=curNodeIndex;i>nodeIndex;i--)  (*parentNode->childNodes)[i] = (*parentNode->childNodes)[i-1];
    }
    (*parentNode->childNodes)[nodeIndex] = this;
}
void TreeViewNode::deleteAllChildNodes(bool leaveEmptyChildNodeVector)  {
    if (childNodes && childNodes->size()>0) {
        while (childNodes->size()>0)    DeleteNode((*childNodes)[0]);
        childNodes->clear();
    }
    if (!childNodes) {
        if (leaveEmptyChildNodeVector) addEmptyChildNodeVector();
    }
    else if (childNodes->size()==0 && !leaveEmptyChildNodeVector) removeEmptyChildNodeVector();
}
void TreeViewNode::addEmptyChildNodeVector()    {
    if (!childNodes) {
        childNodes = (ImVector<TreeViewNode*>*) ImGui::MemAlloc(sizeof(ImVector<TreeViewNode*>));
        IM_PLACEMENT_NEW(childNodes) ImVector<TreeViewNode*>();
    }
}
void TreeViewNode::removeEmptyChildNodeVector() {
    if (childNodes && childNodes->size()==0)    {
        childNodes->~ImVector<TreeViewNode*>();
        ImGui::MemFree(childNodes);
        childNodes=NULL;
    }
}
int TreeViewNode::getNumSiblings(bool includeMe) const	{
    if (!parentNode) return (includeMe ? 1 : 0);
    const int num = parentNode->getNumChildNodes();
    return (includeMe ? num : (num-1));
}

TreeViewNode *TreeViewNode::getSiblingNode(int nodeIndexInParentHierarchy)  {
    if (!parentNode || !parentNode->childNodes || nodeIndexInParentHierarchy<0 || nodeIndexInParentHierarchy>=parentNode->childNodes->size()) return NULL;
    return (*parentNode->childNodes)[nodeIndexInParentHierarchy];
}
const TreeViewNode *TreeViewNode::getSiblingNode(int nodeIndexInParentHierarchy) const	{
    if (!parentNode || !parentNode->childNodes || nodeIndexInParentHierarchy<0 || nodeIndexInParentHierarchy>=parentNode->childNodes->size()) return NULL;
    return (*parentNode->childNodes)[nodeIndexInParentHierarchy];
}
int TreeViewNode::getDepth() const  {
    const TreeViewNode* n = this;int depth = -1;
    while ((n = n->parentNode)) ++depth;
    return depth;
}

void TreeViewNode::Swap(TreeViewNode *&n1, TreeViewNode *&n2) {
    // To test
    if (!n1 || !n2) return;
    {TreeViewNode* tmp = n1->parentNode;n1->parentNode=n2->parentNode;n2->parentNode = tmp;}
    {ImVector<TreeViewNode*>* tmp = n1->childNodes;n1->childNodes=n2->childNodes;n2->childNodes=tmp;}
}

void TreeViewNode::sortChildNodes(bool recursive,int (*comp)(const void *, const void *)) {
    if (childNodes && childNodes->size()>1)  {
        if (recursive) {
            for (int i=0,isz=childNodes->size();i<isz;i++) (*childNodes)[i]->sortChildNodes(recursive,comp);
        }
        qsort(&(*childNodes)[0],childNodes->size(),sizeof(TreeViewNode*),comp);
    }
}
void TreeViewNode::sortChildNodesByDisplayName(bool recursive, bool reverseOrder)    {
    sortChildNodes(recursive,reverseOrder ? MyTreeViewHelperStruct::SorterByDisplayNameReverseOrder : &MyTreeViewHelperStruct::SorterByDisplayName);
}
void TreeViewNode::sortChildNodesByTooltip(bool recursive,bool reverseOrder)  {
    sortChildNodes(recursive,reverseOrder ? MyTreeViewHelperStruct::SorterByTooltipReverseOrder : MyTreeViewHelperStruct::SorterByTooltip);
}
void TreeViewNode::sortChildNodesByUserText(bool recursive, bool reverseOrder) {
    sortChildNodes(recursive,reverseOrder ? MyTreeViewHelperStruct::SorterByUserTextReverseOrder : MyTreeViewHelperStruct::SorterByUserText);
}
void TreeViewNode::sortChildNodesByUserId(bool recursive,bool reverseOrder)   {
    sortChildNodes(recursive,reverseOrder ? MyTreeViewHelperStruct::SorterByUserIdReverseOrder : MyTreeViewHelperStruct::SorterByUserId);
}

void TreeViewNode::addStateToAllChildNodes(int stateFlag,bool recursive) const {
    if (childNodes) {
        for (int i=0,isz=childNodes->size();i<isz;i++)  {
            if (recursive) (*childNodes)[i]->addStateToAllChildNodes(stateFlag,recursive);
            (*childNodes)[i]->state|=stateFlag;
        }
    }
}
void TreeViewNode::removeStateFromAllChildNodes(int stateFlag,bool recursive) const   {
    if (childNodes) {
        for (int i=0,isz=childNodes->size();i<isz;i++)  {
            if (recursive) (*childNodes)[i]->removeStateFromAllChildNodes(stateFlag,recursive);
            (*childNodes)[i]->state&=(~stateFlag);
        }
    }
}

bool TreeViewNode::isStatePresentInAllChildNodes(int stateFlag) const {
    if (childNodes) {
        for (int i=0,isz=childNodes->size();i<isz;i++)  {
            if (((*childNodes)[i]->state&stateFlag)!=stateFlag) return false;
        }
    }
    return true;
}
bool TreeViewNode::isStateMissingInAllChildNodes(int stateFlag) const {
    if (childNodes) {
        for (int i=0,isz=childNodes->size();i<isz;i++)  {
            if (((*childNodes)[i]->state&stateFlag)==stateFlag) return false;
        }
    }
    return true;
}
bool TreeViewNode::isStatePresentInAllDescendants(int stateFlag) const{
    if (childNodes) {
        for (int i=0,isz=childNodes->size();i<isz;i++)  {
            if (((*childNodes)[i]->state&stateFlag)!=stateFlag || !(*childNodes)[i]->isStatePresentInAllDescendants(stateFlag)) return false;
        }
    }
    return true;
}
bool TreeViewNode::isStateMissingInAllDescendants(int stateFlag) const{
    if (childNodes) {
        for (int i=0,isz=childNodes->size();i<isz;i++)  {
            if (((*childNodes)[i]->state&stateFlag)==stateFlag || !(*childNodes)[i]->isStateMissingInAllDescendants(stateFlag)) return false;
        }
    }
    return true;
}
TreeViewNode *TreeViewNode::getFirstParentNodeWithState(int stateFlag,bool recursive)  {
    TreeViewNode* n = parentNode;
    while (n && n->parentNode) {if ((n->state&stateFlag)==stateFlag) return n;if (!recursive) break;n=n->parentNode;}
    return NULL;
}
const TreeViewNode *TreeViewNode::getFirstParentNodeWithState(int stateFlag,bool recursive) const  {
    const TreeViewNode* n = parentNode;
    while (n && n->parentNode) {if ((n->state&stateFlag)==stateFlag) return n;if (!recursive) break;n=n->parentNode;}
    return NULL;
}
TreeViewNode *TreeViewNode::getFirstParentNodeWithoutState(int stateFlag,bool recursive)  {
    TreeViewNode* n = parentNode;
    while (n && n->parentNode) {if ((n->state&stateFlag)!=stateFlag) return n;if (!recursive) break;n=n->parentNode;}
    return NULL;
}
const TreeViewNode *TreeViewNode::getFirstParentNodeWithoutState(int stateFlag,bool recursive) const  {
    const TreeViewNode* n = parentNode;
    while (n && n->parentNode) {if ((n->state&stateFlag)!=stateFlag) return n;if (!recursive) break;n=n->parentNode;}
    return NULL;
}
void TreeViewNode::getAllChildNodesWithState(ImVector<TreeViewNode *> &result, int stateFlag, bool recursive,bool returnOnlyLeafNodes, bool clearResultBeforeUsage) const  {
    if (clearResultBeforeUsage) result.clear();
    if (childNodes) {
        for (int i=0,isz=childNodes->size();i<isz;i++)   {
            const TreeViewNode* n = (*childNodes)[i];
            if ((n->state&stateFlag)==stateFlag && (!returnOnlyLeafNodes || n->isLeafNode())) result.push_back(const_cast<TreeViewNode*>(n));
            if (recursive) n->getAllChildNodesWithState(result,stateFlag,recursive,returnOnlyLeafNodes,false);
        }
    }
}
void TreeViewNode::getAllChildNodesWithoutState(ImVector<TreeViewNode *> &result, int stateFlag, bool recursive, bool returnOnlyLeafNodes, bool clearResultBeforeUsage) const   {
    if (clearResultBeforeUsage) result.clear();
    if (childNodes) {
        for (int i=0,isz=childNodes->size();i<isz;i++)   {
            const TreeViewNode* n = (*childNodes)[i];
            if ((n->state&stateFlag)!=stateFlag && (!returnOnlyLeafNodes || n->isLeafNode())) result.push_back(const_cast<TreeViewNode*>(n));
            if (recursive) n->getAllChildNodesWithState(result,stateFlag,recursive,returnOnlyLeafNodes,false);
        }
    }
}
void TreeViewNode::getAllChildNodes(ImVector<TreeViewNode *> &result, bool recursive, bool returnOnlyLeafNodes, bool clearResultBeforeUsage) const  {
    if (clearResultBeforeUsage) result.clear();
    if (childNodes) {
        for (int i=0,isz=childNodes->size();i<isz;i++)   {
            const TreeViewNode* n = (*childNodes)[i];
            if (!returnOnlyLeafNodes || n->isLeafNode()) result.push_back(const_cast<TreeViewNode*>(n));
            if (recursive) n->getAllChildNodes(result,recursive,returnOnlyLeafNodes,false);
        }
    }
}

void TreeViewNode::startRenamingMode()  {
    MyTreeViewHelperStruct::NameEditing = MyTreeViewHelperStruct::NameEditingStruct(this,this->getDisplayName());
}
bool TreeViewNode::isInRenamingMode() const {
    return (this==MyTreeViewHelperStruct::NameEditing.editingNode);
}


TreeView::TreeView(Mode _selectionMode, bool _allowMultipleSelection, Mode _checkboxMode, bool _allowAutoCheckboxBehaviour, bool _inheritDisabledLook) : TreeViewNode(Data(),NULL,-1,true) {
    treeViewNodePopupMenuDrawerCb = &MyTreeViewHelperStruct::TreeViewNodePopupMenuProvider;
    treeViewNodePopupMenuDrawerCbUserPtr = NULL;
    treeViewNodeDrawIconCb = NULL;
    treeViewNodeDrawIconCbUserPtr = NULL;
    treeViewNodeAfterDrawCb = NULL;
    treeViewNodeAfterDrawCbUserPtr = NULL;
    treeViewNodeCreationDelationCb = NULL;
    treeViewNodeCreationDelationCbUserPtr = NULL;
    inited = false;

    selectionMode = _selectionMode;
    allowMultipleSelection = _allowMultipleSelection;

    checkboxMode = _checkboxMode;
    allowAutoCheckboxBehaviour = _allowAutoCheckboxBehaviour;

    inheritDisabledLook = _inheritDisabledLook;
    userPtr = NULL;

    collapseToLeafNodesAtNodeDepth = -1;

    stateColors[0] = ImVec4(1.f,0.f,0.f,1.f);stateColors[1] = stateColors[0];stateColors[1].w*=0.4f;
    stateColors[2] = ImVec4(0.f,1.f,0.f,1.f);stateColors[3] = stateColors[2];stateColors[3].w*=0.4f;
    stateColors[4] = ImVec4(0.f,0.f,1.f,1.f);stateColors[5] = stateColors[4];stateColors[5].w*=0.4f;

}

TreeView::~TreeView() {
    if (this == MyTreeViewHelperStruct::ContextMenuData.parentTreeView) MyTreeViewHelperStruct::ContextMenuData.reset();
}


void TreeViewNode::render(void* ptr,int numIndents)   {
    // basically it should be: numIndents == getDepth() + 1; AFAICS
    if (state&STATE_HIDDEN) return;
    MyTreeViewHelperStruct& tvhs = *((MyTreeViewHelperStruct*) ptr);
    TreeView& tv = tvhs.parentTreeView;

    bool mustShowMenu = false;
    bool isLeafNode = !childNodes;

    bool mustSkipToLeafNodes = !isLeafNode && tv.collapseToLeafNodesAtNodeDepth>=0 && numIndents-1>=tv.collapseToLeafNodesAtNodeDepth;

    if (!mustSkipToLeafNodes)   {
        bool mustTreePop = false;
        bool mustTriggerSelection = false;
        bool arrowHovered = false;
        bool itemHovered = false;

        const unsigned int mode = getMode();
        const bool allowCheckBox = (state&STATE_FORCE_CHECKBOX) || MatchMode(tv.checkboxMode,mode);
        const bool allowSelection = MatchMode(tv.selectionMode,mode);

        bool stateopen = (state&STATE_OPEN);
        bool stateselected = (state&STATE_SELECTED);
        bool mustdrawdisabled = (state&STATE_DISABLED) || tvhs.mustDrawAllNodesAsDisabled;

        int customColorState = (state&STATE_COLOR1) ? 1 : (state&STATE_COLOR2) ? 2 : (state&STATE_COLOR3) ? 3 : 0;

        ImGui::PushID(this);
        if (allowCheckBox && !tvhs.hasCbGlyphs) ImGui::AlignTextToFramePadding();

        tvhs.window->DC.CursorPos.x+= tvhs.arrowOffset*(numIndents-(isLeafNode ? 0 : 1))+(tvhs.hasArrowGlyphs?(ImGui::GetCurrentContext()->Style.FramePadding.x*2):0.f);
        if (!isLeafNode) {
            if (!tvhs.hasArrowGlyphs)  {
                ImGui::SetNextItemOpen(stateopen,ImGuiCond_Always);
                mustTreePop = ImGui::TreeNode("","%s","");
            }
            else {
                ImGui::Text("%s",stateopen?&TreeView::FontArrowGlyphs[1][0]:&TreeView::FontArrowGlyphs[0][0]);
                arrowHovered=ImGui::IsItemHovered();
            }
            arrowHovered=ImGui::IsItemHovered();itemHovered|=arrowHovered;
            ImGui::SameLine();
        }

        if (allowCheckBox)  {
            bool statechecked = (state&STATE_CHECKED);

            bool checkedChanged = false;
            if (!tvhs.hasCbGlyphs) checkedChanged = ImGui::Checkbox("###chb",&statechecked);
            else {
                ImGui::Text("%s",statechecked?&TreeView::FontCheckBoxGlyphs[1][0]:&TreeView::FontCheckBoxGlyphs[0][0]);
                checkedChanged = ImGui::GetIO().MouseClicked[0] && ImGui::IsItemHovered();
                if (checkedChanged) statechecked=!statechecked;
            }

            if (checkedChanged)   {
                toggleState(STATE_CHECKED);
                tvhs.fillEvent(this,STATE_CHECKED,!(state&STATE_CHECKED));
                if (tv.allowAutoCheckboxBehaviour)  {
                    if (childNodes && childNodes->size()>0)    {
                        if (statechecked) addStateToAllDescendants(STATE_CHECKED);
                        else if (isStatePresentInAllDescendants(STATE_CHECKED)) removeStateFromAllDescendants(STATE_CHECKED);
                    }
                    TreeViewNode* p = parentNode;
                    while (p && p->parentNode!=NULL) {
                        if (!statechecked) p->removeState(STATE_CHECKED);
                        else if (!(p->state&STATE_CHECKED)) {
                            if (p->isStatePresentInAllDescendants(STATE_CHECKED)) p->addState(STATE_CHECKED);
                        }
                        p = p->parentNode;
                    }
                }
                //if (allowSelection) mustTriggerSelection = true;  // Nope, why? ... and actually I fire a single state event at a time, so I can't do it.
            }
            ImGui::SameLine();
        }

        if (tv.treeViewNodeDrawIconCb) itemHovered|=tv.treeViewNodeDrawIconCb(this,tv,tv.treeViewNodeDrawIconCbUserPtr);

        if (this!=MyTreeViewHelperStruct::NameEditing.editingNode)  {
            if (mustdrawdisabled) {
                if (!customColorState) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetCurrentContext()->Style.Colors[ImGuiCol_TextDisabled]);
                else ImGui::PushStyleColor(ImGuiCol_Text, tv.stateColors[(customColorState-1)*2+1]);
            }
            if ((state&STATE_DEFAULT) || (allowSelection && stateselected)) {
                const ImVec4 textColor = (!customColorState || mustdrawdisabled) ? ImGui::GetCurrentContext()->Style.Colors[ImGuiCol_Text] : tv.stateColors[(customColorState-1)*2];
                ImVec2 shadowOffset(0,allowCheckBox ? (ImGui::GetCurrentContext()->Style.FramePadding.y*2) : 0);
                float textSizeY = 0.f;
                if (allowSelection && stateselected)	{
                    const ImVec2 textSize = ImGui::CalcTextSize(data.displayName);textSizeY=textSize.y;
                    const ImU32 fillColor = ImGui::ColorConvertFloat4ToU32(ImVec4(textColor.x,textColor.y,textColor.z,textColor.w*0.1f));
                    tvhs.window->DrawList->AddRectFilled(tvhs.window->DC.CursorPos+shadowOffset,tvhs.window->DC.CursorPos+textSize,fillColor);
                    const ImU32 borderColor = ImGui::ColorConvertFloat4ToU32(ImVec4(textColor.x,textColor.y,textColor.z,textColor.w*0.15f));
                    tvhs.window->DrawList->AddRect(tvhs.window->DC.CursorPos+shadowOffset,tvhs.window->DC.CursorPos+textSize,borderColor,0.0f,0x0F,textSize.y*0.1f);
                }
                if (state&STATE_DEFAULT)    {
                    shadowOffset.y*=0.5f;
                    shadowOffset.x = (textSizeY!=0.f ? textSizeY : ImGui::GetTextLineHeight())*0.05f;
                    if (shadowOffset.x<1) shadowOffset.x=1.f;
                    shadowOffset.y+=shadowOffset.x;
                    const ImU32 shadowColor = ImGui::ColorConvertFloat4ToU32(ImVec4(textColor.x,textColor.y,textColor.z,textColor.w*0.6f));
                    tvhs.window->DrawList->AddText(tvhs.window->DC.CursorPos+shadowOffset,shadowColor,data.displayName);
                }
            }

            if (!customColorState || mustdrawdisabled) ImGui::Text("%s",data.displayName);
            else ImGui::TextColored(tv.stateColors[(customColorState-1)*2],"%s",data.displayName);

            if (mustdrawdisabled) ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) {
                itemHovered=true;
                if (data.tooltip && strlen(data.tooltip)>0) ImGui::SetTooltip("%s",data.tooltip);
            }
        }
        else {
            char* ti = &MyTreeViewHelperStruct::NameEditing.textInput[0];
            bool& mustFocus = MyTreeViewHelperStruct::NameEditing.mustFocusInputText;
            if (mustFocus) {ImGui::SetKeyboardFocusHere();}
            if (ImGui::InputText("###EditingName",ti,255,ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (strlen(ti)>0) {
                    setDisplayName(ti);
                    tvhs.event.node = this;
                    tvhs.event.type = EVENT_RENAMED;
                }
                MyTreeViewHelperStruct::NameEditing.reset();
            }
            else {
                if (!mustFocus && !ImGui::IsItemActive()) MyTreeViewHelperStruct::NameEditing.reset();
                mustFocus = false;
            }
        }
        if (tv.treeViewNodeAfterDrawCb) tv.treeViewNodeAfterDrawCb(this,tv,tvhs.windowWidth,tv.treeViewNodeAfterDrawCbUserPtr);

        if (mustTreePop) ImGui::TreePop();
        ImGui::PopID();

        if (arrowHovered)   {
            if (ImGui::GetIO().MouseClicked[0] && childNodes) {
                toggleState(STATE_OPEN);
                tvhs.fillEvent(this,STATE_OPEN,!(state&STATE_OPEN));
            }
        }
        else if (itemHovered) {
            if (ImGui::GetIO().MouseDoubleClicked[0])		{
                if (allowSelection) mustTriggerSelection = true;
                tvhs.event.node = this;
                tvhs.event.type = EVENT_DOUBLE_CLICKED;
            }
            else if (ImGui::GetIO().MouseClicked[0])		{
                if (allowSelection) mustTriggerSelection = true;
                else if (childNodes) {
                    toggleState(STATE_OPEN);
                    tvhs.fillEvent(this,STATE_OPEN,!(state&STATE_OPEN));
                }
            }
            else if (ImGui::GetIO().MouseClicked[1])	{
                mustShowMenu = true;
                if (allowSelection) mustTriggerSelection = true;
            }
        }

        if (mustTriggerSelection) {
            if (!(ImGui::GetIO().KeyCtrl && tv.allowMultipleSelection)) tv.removeStateFromAllDescendants(STATE_SELECTED);
            toggleState(STATE_SELECTED);
            tvhs.fillEvent(this,STATE_SELECTED,!(state&STATE_SELECTED));
        }

    }

    if (isStatePresent(STATE_OPEN) || mustSkipToLeafNodes)  {
        //---------------------------------------------------
        for (int i=0,isz=childNodes->size();i<isz;i++)   {
            TreeViewNode* n = (*childNodes)[i];
            if (n) {
                const bool oldMustDrawAllNodesAsDisabled = tvhs.mustDrawAllNodesAsDisabled;
                if (tv.inheritDisabledLook && (state&STATE_DISABLED)) tvhs.mustDrawAllNodesAsDisabled = true;
                n->render(&tvhs,numIndents+(mustSkipToLeafNodes?0:1));
                tvhs.mustDrawAllNodesAsDisabled = oldMustDrawAllNodesAsDisabled;
            }
        }
        //---------------------------------------------------
    }


    if (mustShowMenu) {
        if (tv.treeViewNodePopupMenuDrawerCb)	{
            tvhs.ContextMenuData.activeNode = this;
            tvhs.ContextMenuData.parentTreeView = &tv;
            tvhs.ContextMenuData.activeNodeChanged = true;
        }
    }

}


bool TreeView::render() {
    inited = true;
    lastEvent.reset();
    if (!childNodes || childNodes->size()==0) return false;

    static int frameCnt = -1;
    ImGuiContext& g = *ImGui::GetCurrentContext();
    if (frameCnt!=g.FrameCount) {
        frameCnt=g.FrameCount;
        MyTreeViewHelperStruct::ContextMenuDataStruct& cmd = MyTreeViewHelperStruct::ContextMenuData;
        // Display Tab Menu ------------------------------------------
        if (cmd.activeNode && cmd.parentTreeView && cmd.parentTreeView->treeViewNodePopupMenuDrawerCb) {
            if (cmd.activeNodeChanged) {
                cmd.activeNodeChanged = false;
                ImGuiContext& g = *ImGui::GetCurrentContext(); while (g.OpenPopupStack.size() > 0) g.OpenPopupStack.pop_back();   // Close all existing context-menus
                ImGui::OpenPopup(TreeView::GetTreeViewNodePopupMenuName());
            }
            cmd.parentTreeView->treeViewNodePopupMenuDrawerCb(cmd.activeNode,*cmd.parentTreeView,cmd.parentTreeView->treeViewNodePopupMenuDrawerCbUserPtr);
        }
        // -------------------------------------------------------------
    }

    MyTreeViewHelperStruct tvhs(*this,lastEvent);

    ImGui::BeginGroup();
    ImGui::PushID(this);
    for (int i=0,isz=childNodes->size();i<isz;i++)   {
        TreeViewNode* n = (*childNodes)[i];
        tvhs.mustDrawAllNodesAsDisabled = false;
        if (n) n->render(&tvhs,1);
    }
    ImGui::PopID();
    ImGui::EndGroup();

    // TODO: Move as much as event handling stuff from TreeViewNode::render() here

    return (lastEvent.node!=NULL);
}

void TreeView::clear() {TreeViewNode::deleteAllChildNodes(true);}

ImVec4 *TreeView::getTextColorForStateColor(int aStateColorFlag) const    {
    if (aStateColorFlag&STATE_COLOR1) return &stateColors[0];
    if (aStateColorFlag&STATE_COLOR2) return &stateColors[2];
    if (aStateColorFlag&STATE_COLOR3) return &stateColors[4];
    return NULL;
}
ImVec4 *TreeView::getTextDisabledColorForStateColor(int aStateColorFlag) const    {
    if (aStateColorFlag&STATE_COLOR1) return &stateColors[1];
    if (aStateColorFlag&STATE_COLOR2) return &stateColors[3];
    if (aStateColorFlag&STATE_COLOR3) return &stateColors[5];
    return NULL;
}

void TreeView::setTextColorForStateColor(int aStateColorFlag, const ImVec4 &textColor, float disabledTextColorAlphaFactor) const    {
    if (aStateColorFlag&STATE_COLOR1) {stateColors[0] = textColor; stateColors[1] = stateColors[0]; stateColors[1].w = stateColors[0].w * disabledTextColorAlphaFactor;}
    if (aStateColorFlag&STATE_COLOR2) {stateColors[2] = textColor; stateColors[3] = stateColors[2]; stateColors[3].w = stateColors[2].w * disabledTextColorAlphaFactor;}
    if (aStateColorFlag&STATE_COLOR3) {stateColors[4] = textColor; stateColors[5] = stateColors[4]; stateColors[5].w = stateColors[4].w * disabledTextColorAlphaFactor;}
}

//-------------------------------------------------------------------------------
#       if (defined(IMGUIHELPER_H_) && !defined(NO_IMGUIHELPER_SERIALIZATION))
#       ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
        bool TreeView::save(ImGuiHelper::Serializer& s) {
            if (!s.isValid()) return false;
            MyTreeViewHelperStruct::Serialize(s,this);
            return true;
        }
        bool TreeView::save(const char* filename)   {
            ImGuiHelper::Serializer s(filename);
            return save(s);
        }
        int TreeView::Save(const char* filename,TreeView** pTreeViews,int numTreeviews)    {
            IM_ASSERT(pTreeViews && numTreeviews>0);
            ImGuiHelper::Serializer s(filename);
            int ok = 0;
            for (int i=0;i<numTreeviews;i++)   {
                IM_ASSERT(pTreeViews[i]);
                ok+=(pTreeViews[i]->save(s)?1:0);
            }
            return ok;
        }
#       endif //NO_IMGUIHELPER_SERIALIZATION_SAVE
#       ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
        bool TreeView::load(ImGuiHelper::Deserializer& d, const char **pOptionalBufferStart)   {
            if (!d.isValid()) return false;
            clear();
            // We want to call the creation callbacks after Node::Data has been created
            // Our serialization code instead creates empty Nodes before filling Data
            // Here is our workaround/hack:
            TreeView::TreeViewNodeCreationDelationCallback oldCb = treeViewNodeCreationDelationCb;
            treeViewNodeCreationDelationCb = NULL;
            const char* amount = pOptionalBufferStart ? (*pOptionalBufferStart) : 0;
            MyTreeViewHelperStruct::Deserialize(d,this,amount,this,oldCb);
            if (pOptionalBufferStart) *pOptionalBufferStart = amount;
            treeViewNodeCreationDelationCb = oldCb;
            return true;
        }
        bool TreeView::load(const char* filename)   {
            ImGuiHelper::Deserializer d(filename);
            return load(d);
        }
        int TreeView::Load(const char* filename,TreeView** pTreeViews,int numTreeviews) {
            IM_ASSERT(pTreeViews && numTreeviews>0);
            for (int i=0;i<numTreeviews;i++)   {
                IM_ASSERT(pTreeViews[i]);
                pTreeViews[i]->clear();
            }
            ImGuiHelper::Deserializer d(filename);
            const char* amount = 0; int ok = 0;
            for (int i=0;i<numTreeviews;i++)   {
                ok+=(pTreeViews[i]->load(d,&amount)?1:0);
            }
            return ok;
        }
#       endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#       endif //NO_IMGUIHELPER_SERIALIZATION
//--------------------------------------------------------------------------------

char TreeView::FontCheckBoxGlyphs[2][5]={{'\0','\0','\0','\0','\0'},{'\0','\0','\0','\0','\0'}};
char TreeView::FontArrowGlyphs[2][5]={{'\0','\0','\0','\0','\0'},{'\0','\0','\0','\0','\0'}};

void TreeView::SetFontCheckBoxGlyphs(const char *emptyState, const char *fillState) {
    if (emptyState && strlen(emptyState)>0 && strlen(emptyState)<5 && fillState && strlen(fillState)>0 && strlen(fillState)<5) {
        strcpy(&FontCheckBoxGlyphs[0][0],emptyState);
        strcpy(&FontCheckBoxGlyphs[1][0],fillState);
    }
    else FontCheckBoxGlyphs[0][0]=FontCheckBoxGlyphs[1][0]='\0';
}
void TreeView::SetFontArrowGlyphs(const char *leftArrow, const char *downArrow) {
    if (leftArrow && strlen(leftArrow)>0 && strlen(leftArrow)<5 && downArrow && strlen(downArrow)>0 && strlen(downArrow)<5) {
        strcpy(&FontArrowGlyphs[0][0],leftArrow);
        strcpy(&FontArrowGlyphs[1][0],downArrow);
    }
    else FontArrowGlyphs[0][0]=FontArrowGlyphs[1][0]='\0';
}

// Tree view stuff ends here ==============================================================
} // namespace ImGui

namespace ImGui {
// Timeline Stuff Here (from: https://github.com/nem0/LumixEngine/blob/timeline_gui/external/imgui/imgui_user.inl)=
// Improved with code by @meshula (panzoomer here: https://github.com/ocornut/imgui/issues/76)
static float s_max_timeline_value=0.f;
static int s_timeline_num_rows = 0;
static int s_timeline_display_start = 0;
static int s_timeline_display_end = 0;
static int s_timeline_display_index = 0;
static ImVec2* s_ptimeline_offset_and_scale = NULL;

bool BeginTimeline(const char* str_id, float max_value, int num_visible_rows, int opt_exact_num_rows, ImVec2 *popt_offset_and_scale)
{
    // reset global variables
    s_max_timeline_value=0.f;
    s_timeline_num_rows = s_timeline_display_start = s_timeline_display_end = 0.f;
    s_timeline_display_index = -1.f;
    s_ptimeline_offset_and_scale = popt_offset_and_scale;

    if (s_ptimeline_offset_and_scale) {
        if (s_ptimeline_offset_and_scale->y==0.f) {s_ptimeline_offset_and_scale->y=1.f;}
    }
    const float row_height = ImGui::GetTextLineHeightWithSpacing();
    const bool rv = BeginChild(str_id,ImVec2(0,num_visible_rows>=0 ? (row_height*num_visible_rows) : (ImGui::GetContentRegionAvail().y-row_height)),false);
    ImGui::PushStyleColor(ImGuiCol_Separator, ImGui::GetCurrentContext()->Style.Colors[ImGuiCol_Border]);
    ImGui::Columns(2,str_id);
    const float contentRegionWidth = ImGui::GetWindowContentRegionWidth();
    if (ImGui::GetColumnOffset(1)>=contentRegionWidth*0.48f) ImGui::SetColumnOffset(1,contentRegionWidth*0.15f);
    s_max_timeline_value = max_value>=0 ? max_value : (contentRegionWidth*0.85f);
    if (opt_exact_num_rows>0) {
        // Item culling
        s_timeline_num_rows = opt_exact_num_rows;
        ImGui::CalcListClipping(s_timeline_num_rows, row_height, &s_timeline_display_start, &s_timeline_display_end);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (s_timeline_display_start * row_height));
    }
    return rv;
}
bool TimelineEvent(const char* str_id, float* values,bool keep_range_constant)
{
    ++s_timeline_display_index;
    if (s_timeline_num_rows>0 &&
        (s_timeline_display_index<s_timeline_display_start || s_timeline_display_index>=s_timeline_display_end)) {
        if (s_timeline_display_index==s_timeline_display_start-1) {ImGui::NextColumn();ImGui::NextColumn();}    // This fixes a clipping issue at the top visible row
        return false;   // item culling
    }

    const float row_height = ImGui::GetTextLineHeightWithSpacing();
    const float TIMELINE_RADIUS = row_height*0.45f;
    const float row_height_offset = (row_height-TIMELINE_RADIUS*2.f)*0.5f;


    ImGuiWindow* win = GetCurrentWindow();
    const ImU32 inactive_color = ColorConvertFloat4ToU32(ImGui::GetCurrentContext()->Style.Colors[ImGuiCol_Button]);
    const ImU32 active_color = ColorConvertFloat4ToU32(ImGui::GetCurrentContext()->Style.Colors[ImGuiCol_ButtonHovered]);
    const ImU32 line_color = ColorConvertFloat4ToU32(ImGui::GetCurrentContext()->Style.Colors[ImGuiCol_SeparatorActive]);
    bool changed = false;
    bool hovered = false;
    bool active = false;

    ImGui::Text("%s",str_id);
    ImGui::NextColumn();


    const float s_timeline_time_offset = s_ptimeline_offset_and_scale ? s_ptimeline_offset_and_scale->x : 0.f;
    const float s_timeline_time_scale = s_ptimeline_offset_and_scale ? s_ptimeline_offset_and_scale->y : 1.f;

    const float columnOffset = ImGui::GetColumnOffset(1);
    const float columnWidth = ImGui::GetColumnWidth(1)-ImGui::GetCurrentContext()->Style.ScrollbarSize;
    const float columnWidthScaled = columnWidth * s_timeline_time_scale;
    const float columnWidthOffsetScaled = columnWidthScaled * s_timeline_time_offset;
    const ImVec2 cursor_pos(GetWindowContentRegionMin().x + win->Pos.x+columnOffset-TIMELINE_RADIUS,win->DC.CursorPos.y);
    bool mustMoveBothEnds=false;
    const bool isMouseDraggingZero = IsMouseDragging(0);
    float posx[2]={0,0};

    for (int i = 0; i < 2; ++i)
    {
        ImVec2 pos = cursor_pos;
        pos.x += columnWidthScaled * values[i] / s_max_timeline_value - columnWidthOffsetScaled + TIMELINE_RADIUS;
        pos.y += row_height_offset+TIMELINE_RADIUS;
        posx[i] = pos.x;
        if (pos.x+TIMELINE_RADIUS < cursor_pos.x ||
            pos.x-2.f*TIMELINE_RADIUS > cursor_pos.x+columnWidth) continue;   // culling

        SetCursorScreenPos(pos - ImVec2(TIMELINE_RADIUS, TIMELINE_RADIUS));
        PushID(i);
        InvisibleButton(str_id, ImVec2(2 * TIMELINE_RADIUS, 2 * TIMELINE_RADIUS));
        active = IsItemActive();
        if (active || IsItemHovered())
        {
            ImGui::SetTooltip("%f", values[i]);
            if (!keep_range_constant)	{
                // @meshula:The item hovered line needs to be compensated for vertical scrolling. Thx!
                ImVec2 a(pos.x, GetWindowContentRegionMin().y + win->Pos.y + win->Scroll.y);
                ImVec2 b(pos.x, GetWindowContentRegionMax().y + win->Pos.y + win->Scroll.y);
                // possible aternative:
                //ImVec2 a(pos.x, win->Pos.y);
                //ImVec2 b(pos.x, win->Pos.y+win->Size.y);
                win->DrawList->AddLine(a, b, line_color);
            }
            hovered = true;
        }
        if (active && isMouseDraggingZero)
        {
            if (!keep_range_constant) {
                values[i] += GetIO().MouseDelta.x / columnWidthScaled * s_max_timeline_value;
                if (values[i]<0.f) values[i]=0.f;
                else if (values[i]>s_max_timeline_value) values[i]=s_max_timeline_value;
            }
            else mustMoveBothEnds = true;
            changed = hovered = true;
        }
        PopID();
        win->DrawList->AddCircleFilled(
                    pos, TIMELINE_RADIUS, IsItemActive() || IsItemHovered() ? active_color : inactive_color,8);
    }

    ImVec2 start(posx[0]+TIMELINE_RADIUS,cursor_pos.y+row_height*0.3f);
    ImVec2 end(posx[1]-TIMELINE_RADIUS,start.y+row_height*0.4f);
    if (start.x<cursor_pos.x) start.x=cursor_pos.x;
    if (end.x>cursor_pos.x+columnWidth+TIMELINE_RADIUS) end.x=cursor_pos.x+columnWidth+TIMELINE_RADIUS;
    const bool isInvisibleButtonCulled = start.x>=cursor_pos.x+columnWidth || end.x<=cursor_pos.x;

    bool isInvisibleButtonItemActive=false;
    bool isInvisibleButtonItemHovered=false;
    if (!isInvisibleButtonCulled)   {
        PushID(-1);
        SetCursorScreenPos(start);
        InvisibleButton(str_id, end - start);
        isInvisibleButtonItemActive = IsItemActive();
        isInvisibleButtonItemHovered = isInvisibleButtonItemActive || IsItemHovered();
        PopID();
        win->DrawList->AddRectFilled(start, end, isInvisibleButtonItemActive || isInvisibleButtonItemHovered ? active_color : inactive_color);
    }
    if ((isInvisibleButtonItemActive && isMouseDraggingZero) || mustMoveBothEnds)
    {
        const float deltaX = GetIO().MouseDelta.x / columnWidthScaled * s_max_timeline_value;
        values[0] += deltaX;
        values[1] += deltaX;
        changed = hovered = true;
    }
    else if (isInvisibleButtonItemHovered) hovered = true;

    SetCursorScreenPos(cursor_pos + ImVec2(0, row_height));

    if (changed) {
        if (values[0]>values[1]) {float tmp=values[0];values[0]=values[1];values[1]=tmp;}
        if (values[1]>s_max_timeline_value) {values[0]-=values[1]-s_max_timeline_value;values[1]=s_max_timeline_value;}
        if (values[0]<0) {values[1]-=values[0];values[0]=0;}
    }

    if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    ImGui::NextColumn();
    return changed;
}
void EndTimeline(int num_vertical_grid_lines,float current_time,ImU32 timeline_running_color)    {
    const float row_height = ImGui::GetTextLineHeightWithSpacing();
    if (s_timeline_num_rows>0) ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ((s_timeline_num_rows - s_timeline_display_end) * row_height));
    ImGui::NextColumn();

    ImGuiWindow* win = GetCurrentWindow();

    const float columnOffset = ImGui::GetColumnOffset(1);
    const float columnWidth = ImGui::GetColumnWidth(1)-ImGui::GetCurrentContext()->Style.ScrollbarSize;
    const float s_timeline_time_offset = s_ptimeline_offset_and_scale ? s_ptimeline_offset_and_scale->x : 0.f;
    const float s_timeline_time_scale = s_ptimeline_offset_and_scale ? s_ptimeline_offset_and_scale->y : 1.f;
    const float columnWidthScaled = columnWidth*s_timeline_time_scale;
    const float columnWidthOffsetScaled = columnWidthScaled * s_timeline_time_offset;
    const float horizontal_interval = columnWidth / num_vertical_grid_lines;

    ImU32 color = ColorConvertFloat4ToU32(ImGui::GetCurrentContext()->Style.Colors[ImGuiCol_Button]);
    ImU32 line_color = ColorConvertFloat4ToU32(ImGui::GetCurrentContext()->Style.Colors[ImGuiCol_Border]);
    ImU32 text_color = ColorConvertFloat4ToU32(ImGui::GetCurrentContext()->Style.Colors[ImGuiCol_Text]);
    ImU32 moving_line_color = ColorConvertFloat4ToU32(ImGui::GetCurrentContext()->Style.Colors[ImGuiCol_SeparatorActive]);
    const float rounding = ImGui::GetCurrentContext()->Style.ScrollbarRounding;
    const float startY = ImGui::GetWindowHeight() + win->Pos.y;

    // Draw black vertical lines (inside scrolling area)
    for (int i = 1; i <= num_vertical_grid_lines; ++i)
    {
        ImVec2 a = GetWindowContentRegionMin() + win->Pos;
        a.x += s_timeline_time_scale * i * horizontal_interval + columnOffset - columnWidthOffsetScaled;
        win->DrawList->AddLine(a, ImVec2(a.x,startY), line_color);
    }

    // Draw moving vertical line
    if (current_time>0.f && current_time<s_max_timeline_value)	{
        ImVec2 a = GetWindowContentRegionMin() + win->Pos;
        a.x += columnWidthScaled*(current_time/s_max_timeline_value)+columnOffset-columnWidthOffsetScaled;
        win->DrawList->AddLine(a, ImVec2(a.x,startY), moving_line_color,3);
    }

    ImGui::Columns(1);
    ImGui::PopStyleColor();

    EndChild();
    const bool isChildWindowHovered = s_ptimeline_offset_and_scale ? ImGui::IsItemHovered() : false;

    // Draw bottom axis ribbon (outside scrolling region)
    win = GetCurrentWindow();
    float startx = ImGui::GetCursorScreenPos().x + columnOffset;
    float endy = ImGui::GetCursorScreenPos().y+row_height;//GetWindowContentRegionMax().y + win->Pos.y;
    ImVec2 start(startx,ImGui::GetCursorScreenPos().y);
    ImVec2 end(startx+columnWidth,endy);//start.y+row_height);
    float maxx = start.x+columnWidthScaled-columnWidthOffsetScaled;
    if (maxx<end.x) end.x = maxx;
    if (current_time<=0)			win->DrawList->AddRectFilled(start, end, color, rounding);
    else if (current_time>s_max_timeline_value) win->DrawList->AddRectFilled(start, end, timeline_running_color, rounding);
    else {
        ImVec2 median(start.x+columnWidthScaled*(current_time/s_max_timeline_value)-columnWidthOffsetScaled,end.y);
        if (median.x<startx) median.x=startx;
        else {
            if (median.x>startx+columnWidth) median.x=startx+columnWidth;
            win->DrawList->AddRectFilled(start, median, timeline_running_color, rounding,1|8);
        }
        median.y=start.y;
        if (median.x<startx+columnWidth) {
            win->DrawList->AddRectFilled(median, end, color, rounding,2|4);
            if (median.x>startx) win->DrawList->AddLine(median, ImVec2(median.x,end.y), moving_line_color,3);
        }
    }

    char tmp[256]="";
    for (int i = 0; i < num_vertical_grid_lines; ++i)
    {
        ImVec2 a = start;
        a.x = start.x + s_timeline_time_scale * i * horizontal_interval - columnWidthOffsetScaled;
        if (a.x < startx || a.x >= startx+columnWidth) continue;

        ImFormatString(tmp, sizeof(tmp), "%.2f", i * s_max_timeline_value / num_vertical_grid_lines);
        win->DrawList->AddText(a, text_color, tmp);

    }
    ImGui::SetCursorPosY(ImGui::GetCursorPosY()+row_height);


    // zoom and pan
    if (s_ptimeline_offset_and_scale)   {
        const ImGuiIO& io = ImGui::GetIO();
        if (isChildWindowHovered && io.KeyCtrl) {
            if (ImGui::IsMouseDragging(1)) {
                // pan
                s_ptimeline_offset_and_scale->x-=io.MouseDelta.x/columnWidthScaled;
                if (s_ptimeline_offset_and_scale->x>1.f) s_ptimeline_offset_and_scale->x=1.f;
                else if (s_ptimeline_offset_and_scale->x<0.f) s_ptimeline_offset_and_scale->x=0.f;
            }
            else if (io.MouseReleased[2])    {
                // reset
                s_ptimeline_offset_and_scale->x=0.f;
                s_ptimeline_offset_and_scale->y=1.f;
            }
            if (io.MouseWheel!=0) {
                // zoom
                s_ptimeline_offset_and_scale->y*=(io.MouseWheel>0) ? 1.05f : 0.95f;
                if (s_ptimeline_offset_and_scale->y<0.25f) s_ptimeline_offset_and_scale->y=0.25f;
                else if (s_ptimeline_offset_and_scale->y>4.f) s_ptimeline_offset_and_scale->y=4.f;
            }
        }
    }


}
// End Timeline =====================================================================================================


// Start PasswordDrawer (code based on ImGui::ImageButton(...))==========================================================
inline static ImU32 PasswordDrawerFadeAlpha(ImU32 color,int fragment,int numFragments) {
    int a = (IM_COL32_A_MASK&color) >> IM_COL32_A_SHIFT;
    a = (int) (a*(float)(numFragments-fragment)/(float)numFragments);
    return (color & ~IM_COL32_A_MASK) | (a << IM_COL32_A_SHIFT);
}
inline static ImU32 PasswordDrawerLighten(ImU32 color,int amount) {
    int rgba[4] = {(unsigned char) (color>>IM_COL32_R_SHIFT),
		     (unsigned char) (color>>IM_COL32_G_SHIFT),
		     (unsigned char) (color>>IM_COL32_B_SHIFT),
		     (unsigned char) (color>>IM_COL32_A_SHIFT)};
    for (int i=0;i<3;i++) rgba[0]=ImClamp(rgba[i]+amount,0,255);
    return IM_COL32(rgba[0],rgba[1],rgba[2],rgba[3]);
}
bool PasswordDrawer(char *password, int passwordSize,ImGuiPasswordDrawerFlags flags,const float size,const ImU32 colors[7])   {
    IM_ASSERT(passwordSize>4);

    const ImU32 defaultColors[7] = {
        IM_COL32(45,148,129,255),   // bg TL
        IM_COL32(29,86,103,255),    // bg TR
        IM_COL32(62,48,99,255),     // bg BL
        IM_COL32(78,62,107,255),    // bg BR
        IM_COL32(219,216,164,240),  // circles and quads
        IM_COL32(219,216,124,240),  // filled circles
        IM_COL32(32,32,0,164)       // lines
    };

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems || passwordSize<5) return false;

    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;

    // Default to using password as ID. User can still push string/integer prefixes.
    PushID((void *)password);
    const ImGuiID id = window->GetID("#password_drawer");
    PopID();

    const float itemWidth = size<=0 ? ImGui::CalcItemWidth() : size;
    const ImVec2 size1(itemWidth,itemWidth);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size1);
    const ImRect image_bb(window->DC.CursorPos, window->DC.CursorPos + size1);
    ItemSize(bb);
    if (!ItemAdd(bb, id)) return false;

    bool hovered=false, held=false, editable = !(flags&ImGuiPasswordDrawerFlags_ReadOnly);
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    const ImU32* pColors = colors ? colors : defaultColors;
    const bool border = true;
    if (editable || (!hovered && !pressed && !held))
	window->DrawList->AddRectFilledMultiColor(bb.Min, bb.Max ,pColors[0],pColors[1],pColors[3],pColors[2]);
    else {
	ImU32 bgColors[4];
	for (int i=0;i<4;i++) bgColors[i] = PasswordDrawerLighten(pColors[i],(pressed||held) ? 60 : 30);
	window->DrawList->AddRectFilledMultiColor(bb.Min, bb.Max ,bgColors[0],bgColors[1],bgColors[3],bgColors[2]);
    }
    if (border && window->WindowBorderSize)   {
        window->DrawList->AddRect(bb.Min+ImVec2(1,1), bb.Max+ImVec2(1,1), GetColorU32(ImGuiCol_BorderShadow), style.FrameRounding);
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding);
    }

    if (!editable) {
	//pressed|=held;
	held=false;
    }

    static ImGuiID draggingState = 0;
    if (draggingState==0 && held) {
        password[0]='\0';   // reset
        draggingState = id;
    }
    const bool mouseIsHoveringRect = ImGui::IsMouseHoveringRect(image_bb.Min,image_bb.Max);

    // Now we start the control logic
    static const int numRowsSquared[5] = {4,9,16,25,36};
    int passwordLen = strlen(password);
    int numRows = 0;char minChar='1';
    for (int i=4;i>=0;--i) {
        if (passwordSize>numRowsSquared[i]) {
            numRows = 2+i;
	    if (passwordLen>numRowsSquared[i]) passwordLen=numRowsSquared[i]; // Max 36
            break;
        }
    }

    const ImGuiIO& io = ImGui::GetIO();
    unsigned char selected[37]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    for (int l=0;l<passwordLen;l++) {
        const int ch = (const int) (password[l]-minChar);
        if (ch<passwordSize) {
            selected[ch]=(unsigned char) (l+1);
        }
    }


    // Here we draw the circles (and optional Quads)
    const float imageQuadWidth = image_bb.GetWidth()/(float)numRows;
    const float radius = imageQuadWidth*0.25f;
    const float radiusSquared = radius*radius;
    const float quadHalfSize = radius*0.175f;
    int num_segments = radius*(42.f/100.f);
    if (num_segments<5) num_segments=5;
    else if (num_segments>24) num_segments = 24;
    float thickness = radius*0.05f;
    ImVec2 center(0,0),tmp(0,0);int cnt=0;
    unsigned char charToAdd = 0;
    for (int row=0;row<numRows;row++)   {
        center.y = bb.Min.y+(imageQuadWidth*row)+imageQuadWidth*0.5f;
        for (int col=0;col<numRows;col++)   {
            center.x = bb.Min.x+(imageQuadWidth*col)+imageQuadWidth*0.5f;

            const unsigned char isSelected = selected[cnt];
            if (isSelected) {
                if (flags&ImGuiPasswordDrawerFlags_NoFilledCircles)  {
                    // Normal Drawing
                    window->DrawList->AddCircle(center,radius,pColors[4],num_segments,thickness);
                }
                else {
                    // Filled Drawing
                    window->DrawList->AddCircleFilled(center,radius,PasswordDrawerFadeAlpha(pColors[5],isSelected-1,passwordLen),num_segments);
                }
                window->DrawList->AddRectFilled(ImVec2(center.x-quadHalfSize,center.y-quadHalfSize),ImVec2(center.x+quadHalfSize,center.y+quadHalfSize),pColors[(flags&ImGuiPasswordDrawerFlags_NoLines) ? 4 : 6]);
            }
            else {
                window->DrawList->AddCircle(center,radius,pColors[4],num_segments,thickness);
                window->DrawList->AddRectFilled(ImVec2(center.x-quadHalfSize,center.y-quadHalfSize),ImVec2(center.x+quadHalfSize,center.y+quadHalfSize),pColors[4]);
                if (held && mouseIsHoveringRect && draggingState==id && charToAdd==0 && (passwordLen==0 || password[passwordLen-1]!=(char)(minChar+cnt)))   {
                    tmp.x = io.MousePos.x-center.x;tmp.y = io.MousePos.y-center.y;
                    //if (tmp.x>-radius && tmp.x<radius && tmp.y>-radius && tmp.y<radius) // This line can be commented out (is it faster or not?)
                    {
                        tmp.x*=tmp.x;tmp.y*=tmp.y;
                        if (tmp.x+tmp.y<radiusSquared) charToAdd = minChar+cnt+1;
                    }
                }
            }
            ++cnt;
        }
    }

    // Draw lines:
    if (!(flags&ImGuiPasswordDrawerFlags_NoLines))  {
        const float lineThickness = radius*0.35f;
        ImVec2 lastCenter(-1,-1);
        for (int l=0;l<passwordLen;l++) {
            const int ch = (const int) (password[l]-minChar);
            if (ch<passwordSize) {
                const int row = ch/numRows;
                const int col = ch%numRows;
                center.x = bb.Min.x+(imageQuadWidth*col)+imageQuadWidth*0.5f;
                center.y = bb.Min.y+(imageQuadWidth*row)+imageQuadWidth*0.5f;
                if (l>0) {
                    window->DrawList->AddLine(lastCenter,center,pColors[6],lineThickness);
                }
                lastCenter = center;
            }
        }
	if (editable && passwordLen>0 && mouseIsHoveringRect && passwordLen<numRows*numRows) {
            // Draw last live line
            window->DrawList->AddLine(lastCenter,io.MousePos,pColors[6],lineThickness);
        }
    }

    if (editable && charToAdd>0) {
        password[passwordLen] = (char)(charToAdd-1);
        password[passwordLen+1]='\0';
    }


    if (draggingState==id && !held) {
        // end
        draggingState = 0;
	if (editable) return passwordLen>0;
    }

    return editable ? 0 : pressed;
}
// End PasswordDrawer ===================================================================================================

// Start CheckboxFlags ==============================================================================================
unsigned int CheckboxFlags(const char* label,unsigned int* flags,int numFlags,int numRows,int numColumns,unsigned int flagAnnotations,int* itemHoveredOut,const unsigned int* pFlagsValues)
{
    unsigned int itemPressed = 0;
    if (itemHoveredOut) *itemHoveredOut=-1;
    if (numRows*numColumns*numFlags==0) return itemPressed;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return itemPressed;

    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    int numItemsPerRow = numFlags/numRows;if (numFlags%numRows) ++numItemsPerRow;
    int numItemsPerGroup = numItemsPerRow/numColumns;if (numItemsPerRow%numColumns) ++numItemsPerGroup;
    numItemsPerRow = numItemsPerGroup * numColumns;

    ImGui::BeginGroup();
    const ImVec2 startCurPos = window->DC.CursorPos;
    ImVec2 curPos = startCurPos, maxPos = startCurPos;
    const ImVec2 checkSize(label_size.y,label_size.y);
    const float pad = ImMax(1.0f, (float)(int)(checkSize.x / 6.0f));
    unsigned int j=0;int groupItemCnt = 0, groupCnt = 0;
    const bool buttonPressed = ImGui::IsMouseClicked(0);

    ImU32 annColor;int annSegments;
    float annThickness,annCenter,annRadius;
    if (flagAnnotations)    {
        annColor = GetColorU32(ImGuiCol_Button);
        annSegments = (int) (checkSize.x * 0.4f);if (annSegments<3) annSegments=3;
        annThickness = checkSize.x / 6.0f;
        annCenter = (float)(int)(checkSize.x * 0.5f);
        annRadius = (float)(int)(checkSize.x / 4.0f);
    }

    for (int i=0;i<numFlags;i++) {
        j = pFlagsValues ? pFlagsValues[i] : (1<<i);
        ImGui::PushID(i);
        ImGuiID itemID = window->GetID(label);
        ImGui::PopID();

        bool v = ((*flags & j) == j);
        const bool vNotif =  ((flagAnnotations & j) == j);
        ImRect bb(curPos,curPos + checkSize);
        ItemSize(bb, style.FramePadding.y);

        if (!ItemAdd(bb, itemID)) break;

        bool hovered=false, held= false, pressed = false;
        // pressed = ButtonBehavior(bb, itemID, &hovered, &held);   // Too slow!
        hovered = ItemHoverable(bb, itemID);pressed =  buttonPressed && hovered;   // Way faster
        if (pressed) {
            v = !v;
            if (!ImGui::GetIO().KeyShift) *flags=0;
            if (v)  *flags |= j;
            else    *flags &= ~j;
            itemPressed = j;
        }
        if (itemHoveredOut && hovered) *itemHoveredOut=i;

        RenderFrame(bb.Min, bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding);

        if (v)  {
            window->DrawList->AddRectFilled(bb.Min+ImVec2(pad,pad), bb.Max-ImVec2(pad,pad), GetColorU32(ImGuiCol_CheckMark), style.FrameRounding);
            if (vNotif) window->DrawList->AddCircle(bb.Min+ImVec2(annCenter,annCenter), annRadius, annColor,annSegments,annThickness);
        }
        else if (vNotif) window->DrawList->AddCircle(bb.Min+ImVec2(annCenter,annCenter), annRadius, annColor,annSegments,annThickness);

        // Nope: LogRenderedText is static inside imgui.cpp => we remove it, so that imguivariouscontrols.h/cpp can be used on their own too
        //if (g.LogEnabled) LogRenderedText(&bb.Min, v ? "[x]" : "[ ]");

        curPos.x+=checkSize.x;
        if (maxPos.x<curPos.x) maxPos.x=curPos.x;
        if (i<numFlags-1) {
            ++groupCnt;
            if (++groupItemCnt==numItemsPerGroup) {
                groupItemCnt=0;
                curPos.x+=style.FramePadding.y*2;
                if (groupCnt>=numItemsPerRow) {
                    groupCnt=0;
                    curPos.x = startCurPos.x;
                    curPos.y+=checkSize.y;
                    if (maxPos.y<curPos.y) maxPos.y=curPos.y;
                }
                else {
                    ImGui::SameLine(0,0);
                }
            }
            else ImGui::SameLine(0,0);
        }

    }
    ImGui::EndGroup();


    if (label_size.x > 0) {
        const float spacing = style.ItemInnerSpacing.x;
        SameLine(0, spacing);
        const ImVec2 textStart(maxPos.x+spacing,startCurPos.y);
        const ImRect text_bb(textStart, textStart + label_size);


        ImGuiID itemID = window->GetID(label);
        ItemSize(text_bb, style.FramePadding.y*numRows);
        if (ItemAdd(text_bb, itemID)) RenderText(text_bb.Min, label);
    }

    return itemPressed;
}
// End CheckboxFlags =================================================================================================

// Start CheckBoxStyled ==============================================================================================
bool CheckboxStyled(const char* label, bool* v,const ImU32* pOptionalEightColors,const ImVec2& checkBoxScale,float checkBoxRounding)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    IM_ASSERT(checkBoxScale.x>0 && checkBoxScale.y>0);
    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImVec2 check_size(label_size.y*2.5f*checkBoxScale.x,label_size.y);

    const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(check_size.x + style.FramePadding.x*2, check_size.y + style.FramePadding.y*2)); // We want a square shape to we use Y twice
    ItemSize(check_bb, style.FramePadding.y);

    ImRect total_bb = check_bb;
    if (label_size.x > 0) SameLine(0, style.ItemInnerSpacing.x);
    const ImRect text_bb(window->DC.CursorPos + ImVec2(0,style.FramePadding.y), window->DC.CursorPos + ImVec2(0,style.FramePadding.y) + label_size);
    if (label_size.x > 0)
    {
        ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
        total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
    }

    if (!ItemAdd(total_bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    static float timeBegin = -1.f;
    static ImGuiID timeID = 0;
    static const float timeActionLength = 0.2f;
    if (pressed) {
        *v = !(*v); // change state soon
        if (timeID==id) {
            // Fine tuning for the case when user clicks on the same checkbox twice quickly
            float elapsedTime = ImGui::GetTime()-timeBegin;
            if (elapsedTime>timeActionLength) timeBegin = ImGui::GetTime();   // restart
            else {
                // We must invert the time, tweaking timeBegin
                const float newElapsedTime = timeActionLength-elapsedTime;
                timeBegin= ImGui::GetTime()-newElapsedTime;
            }
        }
        else {
            timeID = id;
            timeBegin = ImGui::GetTime();
        }
    }

    // Widget Look Here ================================================================
    float t = 0.f;    // In [0,1] 0 = OFF 1 = ON
    bool animationActive = false;
    if (timeID==id) {
        float elapsedTime = ImGui::GetTime()-timeBegin;
        if (elapsedTime>timeActionLength) {timeBegin=-1;timeID=0;}
        else {
            t = 1.f-elapsedTime/timeActionLength;
            animationActive = t>0;
        }
    }
    if (*v) t = 1.f-t;
    if (t<0) t=0;
    else if (t>1) t=1;
    const float check_bb_height = check_bb.GetHeight();
    const float innerFrameHeight = check_bb_height*0.5f*(checkBoxScale.y<=2.f?checkBoxScale.y:2.f);
    const float heightDelta = (check_bb_height-innerFrameHeight)*0.5f;
    const float check_bb_width = check_bb.GetWidth();
    float widthFraction = check_bb_width*t;    
    float rounding = checkBoxRounding<0 ? style.WindowRounding : checkBoxRounding;//style.FrameRounding;
    rounding*=innerFrameHeight*0.065f;if (rounding>16.f) rounding = 16.f;
    ImRect innerFrame0(ImVec2(check_bb.Min.x,check_bb.Min.y+heightDelta),ImVec2(check_bb.Min.x+widthFraction,check_bb.Max.y-heightDelta));
    ImRect innerFrame1(ImVec2(check_bb.Min.x+widthFraction,check_bb.Min.y+heightDelta),ImVec2(check_bb.Max.x,check_bb.Max.y-heightDelta));
    if (t>0) {
	ImU32 fillColor0 = pOptionalEightColors ? ((held || hovered) ? pOptionalEightColors[5] : pOptionalEightColors[4]) :
	    (GetColorU32((held || hovered) ? ImGuiCol_ButtonHovered : ImGuiCol_Button));
        window->DrawList->AddRectFilled(innerFrame0.Min, innerFrame0.Max, fillColor0, rounding, t<1 ? ImDrawCornerFlags_Left : ImDrawCornerFlags_All/*9 : 15*/);
    }
    if (t<1) {
	ImU32 fillColor1 = pOptionalEightColors ? ((held || hovered) ? pOptionalEightColors[7] : pOptionalEightColors[6]) :
        (GetColorU32((held || hovered) ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg));
        window->DrawList->AddRectFilled(innerFrame1.Min, innerFrame1.Max, fillColor1, rounding, t>0 ?  ImDrawCornerFlags_Right : ImDrawCornerFlags_All/*6 : 15*/);
    }
    if (style.FrameBorderSize)   {
        ImRect innerFrame(innerFrame0.Min,innerFrame1.Max);
        window->DrawList->AddRect(innerFrame.Min+ImVec2(1,1), innerFrame.Max+ImVec2(1,1), GetColorU32(ImGuiCol_BorderShadow), rounding);
        window->DrawList->AddRect(innerFrame.Min, innerFrame.Max, GetColorU32(ImGuiCol_Border), rounding);
    }
    int numSegments = (int)(check_bb_height*0.8f);if (numSegments<3) numSegments=3;else if (numSegments>24) numSegments = 24;
    float radius = check_bb_height*0.5f;
    if (widthFraction<radius) widthFraction=radius;
    else if (widthFraction>check_bb_width-radius) widthFraction=check_bb_width-radius;
    ImVec2 center(check_bb.Min.x+widthFraction,check_bb.Min.y+check_bb_height*0.5f);
    // All the 4 circle colors will be forced to have A = 255
    const ImGuiCol defaultCircleColorOn =	    ImGuiCol_Text;
    const ImGuiCol defaultCircleColorOnHovered =    ImGuiCol_Text;
    const ImGuiCol defaultCircleColorOff =	    ImGuiCol_TextDisabled;
    const ImGuiCol defaultCircleColorOffHovered =   ImGuiCol_TextDisabled;
    if (!animationActive) {
	if (*v) {
	    ImU32 circleColorOn = pOptionalEightColors ? ((held || hovered) ? pOptionalEightColors[1] : pOptionalEightColors[0]) :
			(GetColorU32((held || hovered) ? defaultCircleColorOnHovered : defaultCircleColorOn));
	    int col = (circleColorOn & ~IM_COL32_A_MASK) | (0xFF << IM_COL32_A_SHIFT);
	    window->DrawList->AddCircleFilled(center,radius,col,numSegments);
	}
	else {
	    ImU32 circleColorOff = pOptionalEightColors ? ((held || hovered) ? pOptionalEightColors[3] : pOptionalEightColors[2]) :
			(GetColorU32((held || hovered) ? defaultCircleColorOffHovered : defaultCircleColorOff));
	    int col = (circleColorOff & ~IM_COL32_A_MASK) | (0xFF << IM_COL32_A_SHIFT);
	    window->DrawList->AddCircleFilled(center,radius,col,numSegments);
	}
    }
    else {
	int col1 = (int) (pOptionalEightColors ? ((held || hovered) ? pOptionalEightColors[1] : pOptionalEightColors[0]) :
			(GetColorU32((held || hovered) ? defaultCircleColorOnHovered : defaultCircleColorOn)));
	int col0 = (int) (pOptionalEightColors ? ((held || hovered) ? pOptionalEightColors[3] : pOptionalEightColors[2]) :
			(GetColorU32((held || hovered) ? defaultCircleColorOffHovered : defaultCircleColorOff)));
	int r = ImLerp((col0 >> IM_COL32_R_SHIFT) & 0xFF, (col1 >> IM_COL32_R_SHIFT) & 0xFF, t);
	int g = ImLerp((col0 >> IM_COL32_G_SHIFT) & 0xFF, (col1 >> IM_COL32_G_SHIFT) & 0xFF, t);
	int b = ImLerp((col0 >> IM_COL32_B_SHIFT) & 0xFF, (col1 >> IM_COL32_B_SHIFT) & 0xFF, t);
	int col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (0xFF << IM_COL32_A_SHIFT);
	window->DrawList->AddCircleFilled(center,radius,col,numSegments);
    }
    // ==================================================================================

    //if (g.LogEnabled) LogRenderedText(&text_bb.Min, *v ? "[x]" : "[ ]");
    if (label_size.x > 0.0f)    RenderText(text_bb.Min, label);

    return pressed;
}
bool CheckboxStyledFlags(const char* label, unsigned int* flags, unsigned int flags_value, const ImU32 *pOptionalEightColors, const ImVec2 &checkBoxScale,float checkBoxRounding)  {
    bool v = ((*flags & flags_value) == flags_value);
    bool pressed = CheckboxStyled(label, &v, pOptionalEightColors, checkBoxScale, checkBoxRounding);
    if (pressed)    {
        if (v)  *flags |= flags_value;
        else    *flags &= ~flags_value;
    }
    return pressed;
}
// End CheckBoxStyled ================================================================================================

// KnobFloat Starts Here =============================================================================================
bool KnobFloat(const char* label, float* p_value, float v_min, float v_max,float v_step) {
    //@ocornut https://github.com/ocornut/imgui/issues/942
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    float radius_outer = 20.0f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 center = ImVec2(pos.x + radius_outer, pos.y + radius_outer);
    float line_height = ImGui::GetTextLineHeight();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float ANGLE_MIN = 3.141592f * 0.75f;
    float ANGLE_MAX = 3.141592f * 2.25f;

    ImGui::InvisibleButton(label, ImVec2(radius_outer*2, radius_outer*2 + line_height + style.ItemInnerSpacing.y));
    bool value_changed = false;
    bool is_active = ImGui::IsItemActive();
    bool is_hovered = ImGui::IsItemHovered();
    if (is_active && io.MouseDelta.x != 0.0f)   {
        if (v_step<=0) v_step=50.f;
        float step = (v_max - v_min) / v_step;
        *p_value += io.MouseDelta.x * step;

        if (*p_value < v_min) *p_value = v_min;
        if (*p_value > v_max) *p_value = v_max;
        value_changed = true;
    }
    else if (is_hovered && (io.MouseDoubleClicked[0] || io.MouseClicked[2])) {
        *p_value = (v_max + v_min) * 0.5f;  // reset value
        value_changed = true;
    }

    float t = (*p_value - v_min) / (v_max - v_min);
    float angle = ANGLE_MIN + (ANGLE_MAX - ANGLE_MIN) * t;
    float angle_cos = cosf(angle), angle_sin = sinf(angle);
    float radius_inner = radius_outer*0.40f;
    draw_list->AddCircleFilled(center, radius_outer, ImGui::GetColorU32(ImGuiCol_FrameBg), 16);
    draw_list->AddLine(ImVec2(center.x + angle_cos*radius_inner, center.y + angle_sin*radius_inner), ImVec2(center.x + angle_cos*(radius_outer-2), center.y + angle_sin*(radius_outer-2)), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), 2.0f);
    draw_list->AddCircleFilled(center, radius_inner, ImGui::GetColorU32(is_active ? ImGuiCol_FrameBgActive : is_hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), 16);
    draw_list->AddText(ImVec2(pos.x, pos.y + radius_outer * 2 + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);

    if (is_active || is_hovered)    {
        ImGui::SetNextWindowPos(ImVec2(pos.x - style.WindowPadding.x, pos.y - line_height - style.ItemInnerSpacing.y - style.WindowPadding.y));
        ImGui::BeginTooltip();
        ImGui::Text("%.3f", *p_value);
        ImGui::EndTooltip();
    }

    return value_changed;
}
// End KnobFloat =========================================================================================================

// Posted by @alexsr here: https://github.com/ocornut/imgui/issues/1901
// Sligthly modified to provide default behaviour with default args
void LoadingIndicatorCircle(const char* label, float indicatorRadiusFactor,
                                   const ImVec4* pOptionalMainColor, const ImVec4* pOptionalBackdropColor,
                                   int circle_count,const float speed) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return;
    }

    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiID id = window->GetID(label);
    const ImGuiStyle& style = GetStyle();

    if (circle_count<=0) circle_count = 12;
    if (indicatorRadiusFactor<=0.f) indicatorRadiusFactor = 1.f;
    if (!pOptionalMainColor)        pOptionalMainColor = &style.Colors[ImGuiCol_Button];
    if (!pOptionalBackdropColor)    pOptionalBackdropColor = &style.Colors[ImGuiCol_ButtonHovered];

    const float lineHeight = GetTextLineHeight(); // or GetTextLineHeight() or GetTextLineHeightWithSpacing() ?
    float indicatorRadiusPixels = indicatorRadiusFactor*lineHeight*0.5f;

    const ImVec2 pos = window->DC.CursorPos;
    const float circle_radius = indicatorRadiusPixels / 8.f;
    indicatorRadiusPixels-= 2.0f*circle_radius;
    const ImRect bb(pos, ImVec2(pos.x + indicatorRadiusPixels*2.f+4.f*circle_radius,
                                pos.y + indicatorRadiusPixels*2.f+4.f*circle_radius));
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id)) {
        return;
    }
    const float base_num_segments = circle_radius*1.f;
    const double t = g.Time;
    const float degree_offset = 2.0f * IM_PI / circle_count;
    for (int i = 0; i < circle_count; ++i) {
        const float sinx = -ImSin(degree_offset * i);
        const float cosx = ImCos(degree_offset * i);
        const float growth = ImMax(0.0f, ImSin((float)(t*(double)(speed*3.0f)-(double)(i*degree_offset))));
        ImVec4 color;
        color.x = pOptionalMainColor->x * growth + pOptionalBackdropColor->x * (1.0f - growth);
        color.y = pOptionalMainColor->y * growth + pOptionalBackdropColor->y * (1.0f - growth);
        color.z = pOptionalMainColor->z * growth + pOptionalBackdropColor->z * (1.0f - growth);
        color.w = 1.0f;
        float grown_circle_radius = circle_radius*(1.0f + growth);
        int num_segments = (int)(base_num_segments*grown_circle_radius);
        if (num_segments<4) num_segments=4;
        window->DrawList->AddCircleFilled(ImVec2(pos.x+2.f*circle_radius + indicatorRadiusPixels*(1.0f+sinx),
                                                 pos.y+2.f*circle_radius + indicatorRadiusPixels*(1.0f+cosx)),
                                                 grown_circle_radius,
                                                 GetColorU32(color),num_segments);
    }
}

// Posted by @zfedoran here: https://github.com/ocornut/imgui/issues/1901
// Sligthly modified to provide default behaviour with default args
void LoadingIndicatorCircle2(const char* label,float indicatorRadiusFactor, float indicatorRadiusThicknessFactor, const ImVec4* pOptionalColor) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    if (indicatorRadiusFactor<=0.f) indicatorRadiusFactor = 1.f;
    if (indicatorRadiusThicknessFactor<=0.f) indicatorRadiusThicknessFactor = 1.f;
    if (!pOptionalColor)    pOptionalColor = &style.Colors[ImGuiCol_Button];
    const ImU32 color = GetColorU32(*pOptionalColor);

    const float lineHeight = GetTextLineHeight(); // or GetTextLineHeight() or GetTextLineHeightWithSpacing() ?
    float indicatorRadiusPixels = indicatorRadiusFactor*lineHeight*0.5f;
    float indicatorThicknessPixels = indicatorRadiusThicknessFactor*indicatorRadiusPixels*0.6f;
    if (indicatorThicknessPixels>indicatorThicknessPixels*0.4f) indicatorThicknessPixels=indicatorThicknessPixels*0.4f;
    indicatorRadiusPixels-=indicatorThicknessPixels;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size(indicatorRadiusPixels*2.f, (indicatorRadiusPixels + style.FramePadding.y)*2.f);

    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return;

    // Render
    window->DrawList->PathClear();



    //int num_segments = indicatorRadiusPixels/8.f;
    //if (num_segments<4) num_segments=4;

    int num_segments = 30;

    int start = abs(ImSin(g.Time*1.8f)*(num_segments-5));

    const float a_min = IM_PI*2.0f * ((float)start) / (float)num_segments;
    const float a_max = IM_PI*2.0f * ((float)num_segments-3) / (float)num_segments;

    const ImVec2 centre = ImVec2(pos.x+indicatorRadiusPixels, pos.y+indicatorRadiusPixels+style.FramePadding.y);

    for (int i = 0; i < num_segments; i++) {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a+g.Time*8) * indicatorRadiusPixels,
                                            centre.y + ImSin(a+g.Time*8) * indicatorRadiusPixels));
    }

    window->DrawList->PathStroke(color, false, indicatorThicknessPixels);
}


} // namespace ImGui
