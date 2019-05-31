// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.


#ifndef IMGUIVARIOUSCONTROLS_H_
#define IMGUIVARIOUSCONTROLS_H_

#define NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API

// USAGE
/*
#include "imguivariouscontrols.h"

// inside a ImGui::Window:
ImGui::TestProgressBar();

ImGui::TestPopupMenuSimple();
*/



namespace ImGui {

#ifndef IMGUIHELPER_H_
bool IsItemActiveLastFrame();
bool IsItemJustReleased();
#endif //IMGUIHELPER_H_

bool CheckButton(const char* label,bool* pvalue);
bool SmallCheckButton(const char* label,bool* pvalue);

// Please note that you can tweak the "format" argument if you want to add a prefix (or a suffix) piece of text to the text that appears at the right of the bar.
// returns the value "fraction" in 0.f-1.f.
// It does not need any ID.
float ProgressBar(const char* optionalPrefixText,float value,const float minValue=0.f,const float maxValue=1.f,const char* format="%1.0f%%",const ImVec2& sizeOfBarWithoutTextInPixels=ImVec2(-1,-1),
                 const ImVec4& colorLeft=ImVec4(0,1,0,0.8),const ImVec4& colorRight=ImVec4(0,0.4,0,0.8),const ImVec4& colorBorder=ImVec4(0.25,0.25,1.0,1));

void TestProgressBar();

// Single column popup menu without icon support. It disappears when the mouse goes away.
// Returns -1 when no entries has been selected.
// Optional scrollUpEntryText returns index of -2,scrollDownEntryText -3 (but they must be manually handled by the user)
int PopupMenuSimple(bool& open, const char** pEntries, int numEntries, const char* optionalTitle=NULL, int* pOptionalHoveredEntryOut=NULL, int startIndex=0, int endIndex=-1, bool reverseItems=false, const char* scrollUpEntryText=NULL, const char* scrollDownEntryText=NULL);

// returns -1 if nothing has been chosen, 0 if copy has been clicked, 1 if cut has been clicked and 2 if paste has been clicked
int PopupMenuSimpleCopyCutPasteOnLastItem(bool readOnly=false);

class PopupMenuSimpleParams {
public:
    bool open;
    int getSelectedEntry() const {return selectedEntry;}    // optional (use PopupMenuSimple(...) return value)
protected:
    int selectedEntry;
    int hoveredEntry;
    int endIndex;
    int startIndex;
    float scrollTimer;
    bool resetScrollingWhenRestart;
public:
    PopupMenuSimpleParams(bool _resetScrollingWhenRestart=true)
    : open(false),selectedEntry(-1),hoveredEntry(-1),endIndex(-1),startIndex(-1),scrollTimer(ImGui::GetTime()),resetScrollingWhenRestart(_resetScrollingWhenRestart)
    {}
friend int PopupMenuSimple(PopupMenuSimpleParams& params,const char** pTotalEntries,int numTotalEntries,int numAllowedEntries,bool reverseItems,const char* optionalTitle,const char* scrollUpEntryText,const char* scrollDownEntryText);
};

int PopupMenuSimple(PopupMenuSimpleParams& params,const char** pTotalEntries,int numTotalEntries,int numAllowedEntries,bool reverseItems=false,const char* optionalTitle=NULL,const char* scrollUpEntryText="   ^   ",const char* scrollDownEntryText="   v   ");

void TestPopupMenuSimple(const char* scrollUpEntryText="   ^   ",const char* scrollDownEntryText="   v   ");

// Single column popup menu with icon support. It disappears when the mouse goes away. Never tested.
// User is supposed to create a static instance of it, add entries once, and then call "render()".
class PopupMenu {
protected:
// TODO: Merge IconData into PopupMenuEntry
    struct IconData {
        ImTextureID user_texture_id;
        ImVec2 uv0;
        ImVec2 uv1;
        ImVec4 bg_col;
        ImVec4 tint_col;
        IconData(ImTextureID _user_texture_id=NULL,const ImVec2& _uv0 = ImVec2(0,0),const ImVec2& _uv1 = ImVec2(1,1),const ImVec4& _bg_col = ImVec4(0,0,0,1),const ImVec4& _tint_col = ImVec4(1,1,1,1))
            : user_texture_id(_user_texture_id),uv0(_uv0),uv1(_uv1),bg_col(_bg_col),tint_col(_tint_col)
        {}
        IconData(const IconData& o) {*this = o;}
        inline int compareTo(const IconData& o) const {
            if ((size_t) user_texture_id < (size_t) o.user_texture_id) return -1;
            if (user_texture_id==o.user_texture_id) {
                if (uv0.y < o.uv0.y) return -1;
                if (uv0.y == o.uv0.y)   {
                    if (uv0.x < o.uv0.x) return -1;
                    if (uv0.x == o.uv0.x) return 0;
                }
            }
            return 1;
        }
        const IconData& operator=(const IconData& o) {
            user_texture_id = o.user_texture_id;
            uv0 = o.uv0; uv1 = o.uv1; bg_col = o.bg_col; tint_col = o.tint_col;
            return *this;
        }
    };
public:    
    struct PopupMenuEntry : public IconData  {
    public:
        enum {
            MAX_POPUP_MENU_ENTRY_TEXT_SIZE = 512
        };
        char text[MAX_POPUP_MENU_ENTRY_TEXT_SIZE];
        bool selectable;
        PopupMenuEntry(const char* _text=NULL,bool _selectable=false,ImTextureID _user_texture_id=NULL,const ImVec2& _uv0 = ImVec2(0,0),const ImVec2& _uv1 = ImVec2(1,1),const ImVec4& _bg_col = ImVec4(0,0,0,1),const ImVec4& _tint_col = ImVec4(1,1,1,1))
            :  IconData(_user_texture_id,_uv0,_uv1,_bg_col,_tint_col),selectable(_selectable)
        {
            if (_text)   {
                IM_ASSERT(strlen(_text)<MAX_POPUP_MENU_ENTRY_TEXT_SIZE);
                strcpy(text,_text);
            }
            else text[0]='\0';
        }
        PopupMenuEntry(const PopupMenuEntry& o) : IconData(o) {*this = o;}
        inline int compareTo(const PopupMenuEntry& o) const {
            if (text[0]=='\0')  {
                if (o.text[0]!='\0') return 1;
            }
            else if (o.text[0]=='\0') return -1;
            const int c = strcmp(text,o.text);
            if (c!=0) return c;
            if ((size_t) user_texture_id < (size_t) o.user_texture_id) return -1;
            if (user_texture_id==o.user_texture_id) {
                if (uv0.y < o.uv0.y) return -1;
                if (uv0.y == o.uv0.y)   {
                    if (uv0.x < o.uv0.x) return -1;
                    if (uv0.x == o.uv0.x) return 0;
                }
            }
            return 1;
        }
        const PopupMenuEntry& operator=(const PopupMenuEntry& o) {
            IconData::operator=(o);
            selectable = o.selectable;
            if (o.text[0]!='\0') strcpy(text,o.text);
            else text[0]='\0';
            return *this;
        }
    };

mutable int selectedEntry;  // of last frame. otherwise -1
ImVector <PopupMenuEntry> entries;  // should be protected, but maybe the user wants to modify it at runtime: in case inherit from this class

void addEntryTitle(const char* text,bool addSeparator=true) {
    entries.push_back(PopupMenuEntry(text,false));
    if (addSeparator) addEntrySeparator();
}
void addEntrySeparator() {
    entries.push_back(PopupMenuEntry(NULL,false));
}
void addEntry(const char* _text,ImTextureID _user_texture_id=NULL,const ImVec2& _uv0 = ImVec2(0,0),const ImVec2& _uv1 = ImVec2(1,1),const ImVec4& _bg_col = ImVec4(0,0,0,1),const ImVec4& _tint_col = ImVec4(1,1,1,1))  {
    entries.push_back(PopupMenuEntry(_text,true,_user_texture_id,_uv0,_uv1,_bg_col,_tint_col));
}

// of last frame. otherwise -1
int getSelectedEntry() const {return selectedEntry;}

// please set "open" to "true" when starting popup.
// When the menu closes, you have open==false and as a return value "selectedEntry"
// The returned "selectedEntry" (and "getSelectedEntry()") are !=-1 only at the exact frame the menu entry is selected.
int render(bool& open) const    {
    selectedEntry = -1;
    if (!open) return selectedEntry;
    const int numEntries = (int) entries.size();
    if (numEntries==0) {
        open = false;
        return selectedEntry;
    }    

    static const ImVec4 transparentColor(1,1,1,0);   
    ImGui::PushStyleColor(ImGuiCol_Button,transparentColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered]);
    ImVec2 iconSize;iconSize.x = iconSize.y = ImGui::GetTextLineHeight();

    ImGui::PushID(&entries);
    //ImGui::BeginPopup(&open);
    ImGui::OpenPopup("MyOwnMenu");
    if (ImGui::BeginPopup("MyOwnMenu")) {
        bool imageClicked = false;
        for (int i = 0; i < numEntries; i++)    {
            const PopupMenuEntry& entry = entries[i];
            imageClicked = false;
            if (entry.user_texture_id) {
                imageClicked = ImGui::ImageButton((void*)entry.user_texture_id,iconSize,entry.uv0,entry.uv1,0,entry.bg_col,entry.tint_col) && entry.selectable;
                ImGui::SameLine();
            }
            if (strlen(entry.text)==0) ImGui::Separator();
            else if (entry.selectable)  {
                if (ImGui::Selectable(entry.text, false) || imageClicked)  {
                    selectedEntry = i;
                    open = false;    // Hide menu
                }
            }
            else ImGui::Text("%s",entry.text);
        }
        if (open)   // close menu when mouse goes away
        {
            ImVec2 pos = ImGui::GetWindowPos();pos.x-=5;pos.y-=5;
            ImVec2 size = ImGui::GetWindowSize();size.x+=10;size.y+=10;
            const ImVec2& mousePos = ImGui::GetIO().MousePos;
            if (mousePos.x<pos.x || mousePos.y<pos.y || mousePos.x>pos.x+size.x || mousePos.y>pos.y+size.y) open = false;
        }
    }
    ImGui::EndPopup();
    ImGui::PopID();
    ImGui::PopStyleColor(2);

    return selectedEntry;
}

bool isEmpty() const {return entries.size()==0;}

};

// Based on the code from: https://github.com/benoitjacquier/imgui
bool ColorChooser(bool* open,ImVec4* pColorOut=NULL, bool supportsAlpha=true);
// Based on the code from: https://github.com/benoitjacquier/imgui
bool ColorCombo(const char* label,ImVec4 *pColorOut=NULL,bool supportsAlpha=false,float width=0.f,bool closeWhenMouseLeavesIt=true);


// Based on the code from: https://github.com/Roflraging (see https://github.com/ocornut/imgui/issues/383)
/*
    *pOptionalCursorPosOut;      // Out
    *pOptionalSelectionStartOut; // Out (== to SelectionEnd when no selection)
    *pOptionalSelectionEndOut;   // Out
*/
bool InputTextMultilineWithHorizontalScrolling(const char* label, char* buf, size_t buf_size, float height, ImGuiInputTextFlags flags = 0, bool* pOptionalIsHoveredOut=NULL, int* pOptionalCursorPosOut=NULL, int* pOptionalSelectionStartOut=NULL, int* pOptionalSelectionEndOut=NULL, float SCROLL_WIDTH=2000.f);

// Based on the code from: https://github.com/Roflraging (see https://github.com/ocornut/imgui/issues/383)
/*
  staticBoolVar is true if the popup_menu is open
  The three integers represent the cursorPos, the selectionStart and the selectionEnd position.
  Must be static and be in an array.
*/
bool InputTextMultilineWithHorizontalScrollingAndCopyCutPasteMenu(const char* label, char* buf, int buf_size, float height,bool& staticBoolVar, int* staticArrayOfThreeIntegersHere, ImGuiInputTextFlags flags=0, bool*pOptionalHoveredOut=NULL,float SCROLL_WIDTH=2000.f,const char* copyName=NULL, const char* cutName=NULL, const char *pasteName=NULL);

// label is used as id
// <0 frame_padding uses default frame padding settings. 0 for no padding
bool ImageButtonWithText(ImTextureID texId,const char* label,const ImVec2& imageSize=ImVec2(0,0), const ImVec2& uv0 = ImVec2(0,0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));

#ifndef NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE
// One instance per image can feed multiple widgets
struct AnimatedImage {
    // TODO: load still images as fallback and load gifs from memory
    public:
    typedef void (*FreeTextureDelegate)(ImTextureID& texid);
    typedef void (*GenerateOrUpdateTextureDelegate)(ImTextureID& imtexid,int width,int height,int channels,const unsigned char* pixels,bool useMipmapsIfPossible,bool wraps,bool wrapt);
    void SetFreeTextureCallback(FreeTextureDelegate freeTextureCb) {FreeTextureCb=freeTextureCb;}
    void SetGenerateOrUpdateTextureCallback(GenerateOrUpdateTextureDelegate generateOrUpdateTextureCb) {GenerateOrUpdateTextureCb=generateOrUpdateTextureCb;}

#	ifndef STBI_NO_GIF
#   ifndef IMGUIVARIOUSCONTROLS_NO_STDIO
    AnimatedImage(char const *gif_filepath,bool useHoverModeIfSupported=false); // 'hoverMode' is supported only if all frames fit 'MaxPersistentTextureSize'
#   endif //IMGUIVARIOUSCONTROLS_NO_STDIO
    AnimatedImage(const unsigned char* gif_buffer,int gif_buffer_size,bool useHoverModeIfSupported=false); // 'hoverMode' is supported only if all frames fit 'MaxPersistentTextureSize'
#	endif //STBI_NO_GIF
    AnimatedImage(ImTextureID myTexId,int animationImageWidth,int animationImageHeight,int numFrames,int numFramesPerRowInTexture,int numFramesPerColumnInTexture,float delayBetweenFramesInCs,bool useHoverMode=false); // 'hoverMode' always available. 'myTexId' is yours.
    AnimatedImage();    // You'll need to manually call 'load' o 'create'
    ~AnimatedImage();   // calls 'clear'
    void clear();   // releases the textures that are created inside the class

    // Main methods
    void render(ImVec2 size=ImVec2(0,0), const ImVec2& uv0=ImVec2(0,0), const ImVec2& uv1=ImVec2(1,1), const ImVec4& tint_col=ImVec4(1,1,1,1), const ImVec4& border_col=ImVec4(0,0,0,0)) const;
    bool renderAsButton(const char* label,ImVec2 size=ImVec2(0,0), const ImVec2& uv0 = ImVec2(0,0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));    // <0 frame_padding uses default frame padding settings. 0 for no padding

    // Less useful methods
#	ifndef STBI_NO_GIF
#   ifndef IMGUIVARIOUSCONTROLS_NO_STDIO
    bool load(char const *gif_filepath,bool useHoverModeIfSupported=false); // 'hoverMode' is supported only if all frames fit 'MaxPersistentTextureSize'
#   endif //IMGUIVARIOUSCONTROLS_NO_STDIO
    bool load_from_memory(const unsigned char* gif_buffer,int gif_buffer_size,bool useHoverModeIfSupported=false);  // 'hoverMode' is supported only if all frames fit 'MaxPersistentTextureSize'
#	endif //STBI_NO_GIF
    bool create(ImTextureID myTexId,int animationImageWidth,int animationImageHeight,int numFrames,int numFramesPerRowInTexture,int numFramesPerColumnInTexture,float delayBetweenFramesInCs,bool useHoverMode=false); // 'hoverMode' always available. 'myTexId' is yours.
    int getWidth() const;
    int getHeight() const;
    int getNumFrames() const;
    bool areAllFramesInASingleTexture() const;  // when true, 'hoverMode' was available in ctr/load/create (but it can't change at runtime)

    static ImVec2 MaxPersistentTextureSize;   // 2048,2048 (Enlarge the buffer if needed for 'hoverMode': but using smaller animated images and less frames is better)

    private:
    AnimatedImage(const AnimatedImage& ) {}     // Actually maybe we could allow some of these for containers...
    void operator=(const AnimatedImage& ) {}
    static FreeTextureDelegate FreeTextureCb;
    static GenerateOrUpdateTextureDelegate GenerateOrUpdateTextureCb;
    friend struct AnimatedImageInternal;
    struct AnimatedImageInternal* ptr;
};
#endif //NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE

// zoomCenter is panning in [(0,0),(1,1)]
// returns true if some user interaction have been processed
bool ImageZoomAndPan(ImTextureID user_texture_id, const ImVec2& size,float aspectRatio,float& zoom,ImVec2& zoomCenter,int panMouseButtonDrag=1,int resetZoomAndPanMouseButton=2,const ImVec2& zoomMaxAndZoomStep=ImVec2(16.f,1.025f));

// USAGE:
/*
// Nobody will use this, it's too complicated to set up. However:

            static bool closed = false;                 // Notable exception to this rule: "use static booleans only when the button is togglable".
            bool paste = false,copy = false;
            if (!closed)    {
                static bool myTreeNodeIsOpen = false;   // 'static' here, just to reuse its address as id...
                const void* ptr_id = &myTreeNodeIsOpen;
                const float curPosX = ImGui::GetCursorPosX();   // used for clipping
                ImGui::BeginGroup();    // Not sure grouping is strictly necessary here
                myTreeNodeIsOpen = ImGui::TreeNodeEx(ptr_id,(ImGuiTreeNodeFlags_CollapsingHeader & (~ImGuiTreeNodeFlags_NoTreePushOnOpen))|ImGuiTreeNodeFlags_AllowOverlapMode,"Collapsable %d",1);
                //if (ImGui::IsItemHovered()) // optional condition if we want buttons to appear only when the collapsable header is hovered
                {
                    ImGui::AppendTreeNodeHeaderButtons(ptr_id,curPosX,
                        3,                                // Num Buttons
                        &closed,"delete",NULL,0,      // Button 0 (far-right) quartet:        &pressed | tooltip | single glyph as const char* (if NULL it's a close button) | isToggleButton?1:0
                        &paste,"paste","v",0,         // Button 1 (second far-right) quartet: &pressed | tooltip | single glyph as const char* (if NULL it's a close button) | isToggleButton?1:0
                        &copy,"copy","^",0            // Button 2 (third far-right) quartet:  &pressed | tooltip | single glyph as const char* (if NULL it's a close button) | isToggleButton?1:0
                    );  // return value non-negative if one button is hovered or clicked, so that you can prevent the 'tree node header' tooltip to show up if you use it.
                    // return value can be: -1 => No button is hovered or clicked | [0,numButtons-1] => buttons[rv] has been clicked | [numButtons,2*numButtons-1] => buttons[rv-numButtons] is hovered
                }
                if (myTreeNodeIsOpen) {
                    // (optional) Fill the header with data within tree node indent
                }
                if (myTreeNodeIsOpen) ImGui::TreePop();   // Mandatory when we want to close the indent (before or after filling the header with data)
                if (myTreeNodeIsOpen) {
                    // (optional) Fill the header with data without tree node indent
                    static ImVec4 color(1,1,1,1);ImGui::ColorEdit4("MyColor##AppendTreeNodeHeaderButtonsMyColor",&color.x);
                }
                ImGui::EndGroup();    // Not sure grouping is strictly necessary here
            }
            else if (ImGui::Button("Reset collapsable header##AppendTreeNodeHeaderButtonsReset")) closed = false;
*/
// Return value rv can be: -1 => No button is hovered or clicked | [0,numButtons-1] => buttons[rv] has been clicked | [numButtons,2*numButtons-1] => buttons[rv-numButtons] is hovered
int AppendTreeNodeHeaderButtons(const void* ptr_id, float startWindowCursorXForClipping, int numButtons, ...);

// Returns the hovered value index WITH 'values_offset' ( (hovered_index+values_offset)%values_offset or -1). The index of the hovered histogram can be retrieved through 'pOptionalHoveredHistogramIndexOut'.
int PlotHistogram(const char* label, const float** values,int num_histograms,int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0,0), int stride = sizeof(float),float histogramGroupSpacingInPixels=0.f,int* pOptionalHoveredHistogramIndexOut=NULL,float fillColorGradientDeltaIn0_05=0.05f,const ImU32* pColorsOverride=NULL,int numColorsOverride=0);
int PlotHistogram(const char* label, float (*values_getter)(void* data, int idx,int histogramIdx), void* data,int num_histograms, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0,0),float histogramGroupSpacingInPixels=0.f,int* pOptionalHoveredHistogramIndexOut=NULL,float fillColorGradientDeltaIn0_05=0.05f,const ImU32* pColorsOverride=NULL,int numColorsOverride=0);
// Shortcut for a single histogram to ease user code a bit (same signature as one of the 2 default Dear ImGui PlotHistogram(...) methods):
int PlotHistogram2(const char* label, const float* values,int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0,0), int stride = sizeof(float),float fillColorGradientDeltaIn0_05=0.05f,const ImU32* pColorsOverride=NULL,int numColorsOverride=0);

// This one plots a generic function (or multiple functions together) of a float single variable.
// Returns the index of the hovered curve (or -1).
// Passing rangeX.y = FLT_MAX should ensure that the aspect ratio between axis is correct.
// By doubling 'precisionInPixels', we halve the times 'values_getter' gets called.
// 'numGridLinesHint' is currently something we must still fix. Set it to zero to hide lines.
int PlotCurve(const char* label, float (*values_getter)(void* data, float x,int numCurve), void* data,int num_curves,const char* overlay_text,const ImVec2 rangeY,const ImVec2 rangeX=ImVec2(-.1f,FLT_MAX), ImVec2 graph_size=ImVec2(0,0),ImVec2* pOptionalHoveredValueOut=NULL,float precisionInPixels=1.f,float numGridLinesHint=4.f,const ImU32* pColorsOverride=NULL,int numColorsOverride=0);

// These 2 have a completely different implementation:
// Posted by @JaapSuter and @maxint (please see: https://github.com/ocornut/imgui/issues/632)
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
    ImVec2 graph_size);

// Posted by @JaapSuter and @maxint (please see: https://github.com/ocornut/imgui/issues/632)
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
    ImVec2 graph_size);


class InputTextWithAutoCompletionData  {
    protected:
    // auto completion:
    int deltaTTItems;                           // modified by UP/DOWN keys
    bool tabPressed;                            // triggers autocompletion
    ImVector<char> newTextToSet;                // needed because ImGui does not allow changing an InputText(...) buffer directly, while it's active
    int itemPositionOfReturnedText;
    int itemIndexOfReturnedText;
    int additionalFlags,bufTextLen,lastSelectedTTItemIndex;
    bool inited;                                // turns true the first time a method that use this class is called

    public:
    int currentAutocompletionItemIndex;         // completely user-side (if!=-1, that item is displayed in a different way in the autocompletion menu)
    InputTextWithAutoCompletionData(ImGuiInputTextFlags _additionalFlags=0,int _currentAutocompletionItemIndex=-1) : deltaTTItems(0),tabPressed(false),itemPositionOfReturnedText(-1),itemIndexOfReturnedText(-1),
    additionalFlags(_additionalFlags&(ImGuiInputTextFlags_CharsDecimal|ImGuiInputTextFlags_CharsHexadecimal|ImGuiInputTextFlags_CharsNoBlank|ImGuiInputTextFlags_CharsUppercase)),bufTextLen(-1),lastSelectedTTItemIndex(-1),
    inited(false),currentAutocompletionItemIndex(_currentAutocompletionItemIndex) {}

    bool isInited() const {return inited;}      // added just for my laziness (to init elements inside DrawGL() of similiar)
    int getItemPositionOfReturnedText() const {return itemPositionOfReturnedText;}  // usable only after "return" is pressed: it returns the item position at which the newly entered text can be inserted, or -1
    int getItemIndexOfReturnedText() const {return itemIndexOfReturnedText;}        // usable only after "return" is pressed: it returns the item index that exactly matches the newly entered text, or -1

    static float Opacity;   // 0.6f;

    friend bool InputTextWithAutoCompletion(const char* label, char* buf, size_t buf_size,InputTextWithAutoCompletionData* pAutocompletion_data, bool (*autocompletion_items_getter)(void*, int, const char**), int autocompletion_items_size, void* autocompletion_user_data, int num_visible_autocompletion_items);
    friend int DefaultInputTextAutoCompletionCallback(ImGuiInputTextCallbackData *data);

    // Some useful helper methods
    static int HelperGetItemInsertionPosition(const char* txt,bool (*items_getter)(void*, int, const char**), int items_count, void* user_data=NULL,bool* item_is_already_present_out=NULL);
    static int HelperInsertItem(const char* txt,bool (*items_getter)(void*, int, const char**),bool (*items_inserter)(void*, int,const char*), int items_count, void* user_data=NULL,bool* item_is_already_present_out=NULL);
};
bool InputTextWithAutoCompletion(const char* label, char* buf, size_t buf_size, InputTextWithAutoCompletionData* pAutocompletion_data, bool (*autocompletion_items_getter)(void*, int, const char**), int autocompletion_items_size, void* autocompletion_user_data=NULL, int num_visible_autocompletion_items=-1);


class InputComboWithAutoCompletionData : protected InputTextWithAutoCompletionData {
    protected:
    int inputTextShown;
    ImVector<char> buf;
    bool isRenaming;
    bool itemHovered,itemActive;
    public:
    InputComboWithAutoCompletionData(ImGuiInputTextFlags additionalInputTextFlags=0) : InputTextWithAutoCompletionData(additionalInputTextFlags),inputTextShown(0),isRenaming(false),itemHovered(false),itemActive(false) {}

    bool isInited() const {return inited;}              // added just for my laziness (to init elements inside DrawGL() of similiar)
    bool isItemHovered() const {return itemHovered;}    // well, this widget is made of 2 widgets with buttons, so ImGui::IsItemHovered() does not always work
    bool isItemActive() const {return itemActive;}      // well, this widget is made of 2 widgets with buttons, so ImGui::IsItemActive() does not always work
    bool isInputTextVisibleNextCall() const {return inputTextShown!=0;}
    const char* getInputTextBuffer() const {return buf.size()>0 ? &buf[0] : NULL;}

    static char ButtonCharcters[3][5];  // = {"+","r","x"};
    static char ButtonTooltips[3][128]; // = {"add","rename","delete"};

    friend bool InputComboWithAutoCompletion(const char* label, int *current_item, size_t autocompletion_buffer_size, InputComboWithAutoCompletionData* pAutocompletion_data,
                                  bool (*items_getter)(void*, int, const char**),       // gets item at position ... (cannot be NULL)
                                  bool (*items_inserter)(void*, int,const char*),       // inserts item at position ... (cannot be NULL)
                                  bool (*items_deleter)(void*, int),                    // deletes item at position ... (can be NULL)
                                  bool (*items_renamer)(void *, int, int, const char *),// deletes item at position, and inserts renamed item at new position  ... (can be NULL)
                                  int items_count, void* user_data, int num_visible_items);
};
bool InputComboWithAutoCompletion(const char* label,int* current_item,size_t autocompletion_buffer_size,InputComboWithAutoCompletionData* pAutocompletion_data,
    bool (*items_getter)(void*, int, const char**),  // gets item at position ... (cannot be NULL)
    bool (*items_inserter)(void*, int,const char*),  // inserts item at position ... (cannot be NULL)
    bool (*items_deleter)(void*, int),               // deletes item at position ... (can be NULL)
    bool (*items_renamer)(void*,int,int,const char*),// deletes item at position, and inserts renamed item at new position  ... (can be NULL)
    int items_count, void* user_data=NULL, int num_visible_items=-1);


// Basic tree view implementation
// TODO: See if we can switch context-menu with a single-shot RMB click (when a menu is already open)
class TreeViewNode {
protected:
    friend class TreeView;
    friend struct MyTreeViewHelperStruct;
public:
    enum State {
        STATE_NONE      = 0,
        STATE_OPEN      = 1,
        STATE_SELECTED  = 1<<2,
        STATE_CHECKED   = 1<<3,
        STATE_DEFAULT   = 1<<4,     // user state (but its look is hard-coded)
        STATE_DISABLED  = 1<<5,     // user state (but its look is hard-coded)
        STATE_FORCE_CHECKBOX  = 1<<6, // checkbox always visible (even if disabled in its TreeView)
        STATE_HIDDEN    = 1<<7,       // node (and its child nodes) not visible at all

        // user states (without any specific look)
        STATE_USER5     = 1<<8,

        // user states (but its look can be set in TreeView::getTextColorForStateColor(...))
        STATE_COLOR1    = 1<<9,
        STATE_COLOR2    = 1<<10,    // STATE_COLOR1 wins on it (but only as far as the look is concerned)
        STATE_COLOR3    = 1<<11,    // STATE_COLOR1 and STATE_COLOR2 win on it (but only as far as the look is concerned)

        // user states (without any specific look)
        STATE_USER1     = 1<<12,
        STATE_USER2     = 1<<13,
        STATE_USER3     = 1<<14,
        STATE_USER4     = 1<<15
    };
    mutable int state;

    enum Mode {
        MODE_NONE = 0,

        MODE_ROOT = 1,
        MODE_INTERMEDIATE = 2,
        MODE_LEAF = 4,

        MODE_ROOT_INTERMEDIATE = 3,
        MODE_ROOT_LEAF = 5,
        MODE_INTERMEDIATE_LEAF = 6,
        MODE_ALL = 7,
    };

    enum EventType {
        EVENT_NONE = 0,
        EVENT_STATE_CHANGED,
        EVENT_DOUBLE_CLICKED,
        EVENT_RENAMED
    };
    struct Event {
        TreeViewNode* node;
        EventType type;
        State state;bool wasStateRemoved;
        Event() {set();}
        inline void reset() {set();}
        inline void set(TreeViewNode* _node=NULL,EventType _type=EVENT_NONE,State _state=STATE_NONE,bool _wasStateRemoved=false)    {node=_node;type=_type;state=_state;wasStateRemoved=_wasStateRemoved;}
    };

    struct Data {
        char* displayName;      // can't be NULL (it's set to "" when NULL)
        char* tooltip;          // optional (can be NULL)
        char* userText;         // user stuff, optional (can be NULL)
        int   userId;           // user stuff
        Data(const char* _displayedName=NULL,const char* _tooltip=NULL,const char* _userText=NULL,const int _id=0) : displayName(NULL),tooltip(NULL),userText(NULL),userId(0) {
            set(_displayedName,_tooltip,_userText,_id);
        }
        Data(const Data& o) : displayName(NULL),tooltip(NULL),userText(NULL),userId(0) {*this=o;}
        ~Data() {
            if (displayName){ImGui::MemFree(displayName);displayName=NULL;}
            if (tooltip)    {ImGui::MemFree(tooltip);tooltip=NULL;}
            if (userText)   {ImGui::MemFree(userText);userText=NULL;}
            userId=0;
        }
        void set(const char* _displayName=NULL,const char* _tooltip=NULL,const char* _userText=NULL,const int _id=0) {
            SetString(displayName,_displayName,false);
            SetString(tooltip,_tooltip,true);
            SetString(userText,_userText,true);
            userId = _id;
        }        
        inline static void SetString(char*& destText,const char* text,bool allowNullDestText=true) {
            if (destText) {ImGui::MemFree(destText);destText=NULL;}
            const char e = '\0';
            if (!text && !allowNullDestText) text=&e;
            if (text)  {
                const int sz = strlen(text);
                destText = (char*) ImGui::MemAlloc(sz+1);strcpy(destText,text);
            }
        }       
        const Data& operator=(const Data& o) {
            set(o.displayName,o.tooltip,o.userText,o.userId);
            return *this;
        }
    };


protected:

    static TreeViewNode* CreateNode(const Data& _data,TreeViewNode* _parentNode=NULL,int nodeIndex=-1,bool addEmptyChildNodeVector=false);

    void render(void *ptr, int numIndents=1);

public:

    inline TreeViewNode* addChildNode(const Data& _data,int nodeIndex=-1,bool addEmptyChildNodeVector=false)    {
        return CreateNode(_data,this,nodeIndex,addEmptyChildNodeVector);
    }
    inline TreeViewNode* addSiblingNode(const Data& _data,int parentNodeIndex=-1)    {
        IM_ASSERT(parentNode);
        return CreateNode(_data,parentNode,parentNodeIndex);
    }
    static void DeleteNode(TreeViewNode* n);

    class TreeView& getTreeView();              // slightly slow
    const class TreeView& getTreeView() const;  // slightly slow
    TreeViewNode* getParentNode();
    const TreeViewNode* getParentNode() const;
    int getNodeIndex() const;
    void moveNodeTo(int nodeIndex);
    inline bool isLeafNode() const {return childNodes==NULL;}       // Please note that non-leaf nodes can have childNodes->size()==0
    inline bool isRootNode() const {return !parentNode || !parentNode->parentNode;}
    inline int getNumChildNodes() const {return childNodes ? childNodes->size() : 0;}
    inline TreeViewNode* getChildNode(int index=0) {return (childNodes && childNodes->size()>index) ? (*childNodes)[index] : NULL;}
    inline const TreeViewNode* getChildNode(int index=0) const {return (childNodes && childNodes->size()>index) ? (*childNodes)[index] : NULL;}
    void deleteAllChildNodes(bool leaveEmptyChildNodeVector=false);
    void addEmptyChildNodeVector();         // Only works if "childNodes==NULL" (and allocates it)
    void removeEmptyChildNodeVector();      // Only works if (childNodes->size()==0" (and deallocates it)
    int getNumSiblings(bool includeMe=true) const;
    TreeViewNode* getSiblingNode(int nodeIndexInParentHierarchy=-1);
    const TreeViewNode* getSiblingNode(int nodeIndexInParentHierarchy=-1) const;
    int getDepth() const;   // root nodes have depth = 0

    static void Swap(TreeViewNode*& n1,TreeViewNode*& n2); // untested
    void startRenamingMode();       // starts renaming the node
    bool isInRenamingMode() const;

    void sortChildNodes(bool recursive,int (*comp)(const void *, const void *));
    void sortChildNodesByDisplayName(bool recursive=false,bool reverseOrder=false);
    void sortChildNodesByTooltip(bool recursive=false,bool reverseOrder=false);
    void sortChildNodesByUserText(bool recursive=false,bool reverseOrder=false);
    void sortChildNodesByUserId(bool recursive=false,bool reverseOrder=false);

    inline void addState(int stateFlag) const {state|=stateFlag;}
    inline void removeState(int stateFlag) const {state&=~stateFlag;}
    inline void toggleState(int stateFlag) const {state^=stateFlag;}
    inline bool isStatePresent(int stateFlag) const {return ((state&stateFlag)==stateFlag);}
    inline bool isStateMissing(int stateFlag) const {return ((state&stateFlag)!=stateFlag);}

    void addStateToAllChildNodes(int stateFlag, bool recursive = false) const;
    void removeStateFromAllChildNodes(int stateFlag, bool recursive = false) const;
    bool isStatePresentInAllChildNodes(int stateFlag) const;
    bool isStateMissingInAllChildNodes(int stateFlag) const;

    void addStateToAllDescendants(int stateFlag) const {addStateToAllChildNodes(stateFlag,true);}
    void removeStateFromAllDescendants(int stateFlag) const {removeStateFromAllChildNodes(stateFlag,true);}
    bool isStatePresentInAllDescendants(int stateFlag) const;
    bool isStateMissingInAllDescendants(int stateFlag) const;

    // These return the first matching parentNode (if "recursive==false" they return node->parentNode or NULL).
    TreeViewNode* getFirstParentNodeWithState(int stateFlag,bool recursive=true);
    const TreeViewNode* getFirstParentNodeWithState(int stateFlag,bool recursive=true) const;
    TreeViewNode* getFirstParentNodeWithoutState(int stateFlag,bool recursive=true);
    const TreeViewNode* getFirstParentNodeWithoutState(int stateFlag,bool recursive=true) const;

    // if "recursive==true" deleting the "result nodes" in order shouldn't work, but probably it works in the reverse order (TO TEST)
    void getAllChildNodesWithState(ImVector<TreeViewNode*>& result,int stateFlag,bool recursive = false, bool returnOnlyLeafNodes=false, bool clearResultBeforeUsage=true) const;
    void getAllChildNodesWithoutState(ImVector<TreeViewNode*>& result,int stateFlag,bool recursive = false, bool returnOnlyLeafNodes=false, bool clearResultBeforeUsage=true) const;
    void getAllChildNodes(ImVector<TreeViewNode*>& result,bool recursive = false,bool returnOnlyLeafNodes=false,bool clearResultBeforeUsage=true) const;

    // Deleting the "result nodes" in order shouldn't work, but probably it works in the reverse order (TO TEST)
    void getAllDescendants(ImVector<TreeViewNode*>& result,bool clearResultBeforeUsage=true) const {return getAllChildNodes(result,true,false,clearResultBeforeUsage);}
    void getAllDescendantsWithState(ImVector<TreeViewNode*>& result,int stateFlag,bool clearResultBeforeUsage=true) const {return getAllChildNodesWithState(result,stateFlag,true,false,clearResultBeforeUsage);}
    void getAllDescendantsWithoutState(ImVector<TreeViewNode*>& result,int stateFlag,bool clearResultBeforeUsage=true) const {return getAllChildNodesWithoutState(result,stateFlag,true,false,clearResultBeforeUsage);}

    // Deleting the "result nodes" in order shouldn't work, but probably it works in the reverse order (TO TEST)
    void getAllLeafNodes(ImVector<TreeViewNode*>& result,bool clearResultBeforeUsage=true) const {return getAllChildNodes(result,true,true,clearResultBeforeUsage);}
    void getAllLeafNodesWithState(ImVector<TreeViewNode*>& result,int stateFlag,bool clearResultBeforeUsage=true) const {return getAllChildNodesWithState(result,stateFlag,true,true,clearResultBeforeUsage);}
    void getAllLeafNodesWithoutState(ImVector<TreeViewNode*>& result,int stateFlag,bool clearResultBeforeUsage=true) const {return getAllChildNodesWithoutState(result,stateFlag,true,true,clearResultBeforeUsage);}

    // To remove
/*
    void dbgDisplay(const int indent = 0) const {
        for (int i=0;i<indent;i++) printf(" ");
        printf("%s (%s) (%d) [parent=%s]\n",data.displayName?data.displayName:"NULL",data.text?data.text:"NULL",data.id,parentNode?(parentNode->data.displayName):"NULL");
        if (childNodes && childNodes->size()>0) {
            for (int i=0;i<childNodes->size();i++) {
                TreeViewNode* n = (*childNodes)[i];
                if (n) {n->dbgDisplay(indent+3);}
            }
        }
    }
*/

    // "data" accessors
    inline const char* getDisplayName() const {return data.displayName;}
    inline void setDisplayName(const char* _displayName) {Data::SetString(data.displayName,_displayName,false);}
    inline const char* getTooltip() const {return data.tooltip;}
    inline void setTooltip(const char* _tooltip) {Data::SetString(data.tooltip,_tooltip,true);}
    inline const char* getUserText() const {return data.userText;}
    inline void setUserText(const char* _userText) {Data::SetString(data.userText,_userText,true);}
    inline int& getUserId() {return data.userId;}
    inline const int& getUserId() const {return data.userId;}
    inline void setUserId(int uid) {data.userId=uid;}




    void *userPtr;  // Not mine

protected:

    TreeViewNode(const TreeViewNode::Data& _data=TreeViewNode::Data(), TreeViewNode* _parentNode=NULL, int nodeIndex=-1, bool addEmptyChildNodeVector=false);
    virtual ~TreeViewNode();

    Data data;

    TreeViewNode* parentNode;
    ImVector<TreeViewNode*>* childNodes;

    inline unsigned int getMode() const {
        int m = MODE_NONE;if (childNodes==NULL) m|=MODE_LEAF;
        if (!parentNode || !parentNode->parentNode) m|=MODE_ROOT;
        if (m==MODE_NONE) m = MODE_INTERMEDIATE;
        return m;
    }
    inline static bool MatchMode(unsigned int m,unsigned int nodeM) {
        // Hp) nodeM can't be MODE_NONE
        return (m==MODE_ALL || (m!=MODE_NONE && (m&nodeM)));
    }

protected:
    TreeViewNode(const TreeViewNode&) {}
    void operator=(const TreeViewNode&) {}
};

class TreeView : protected TreeViewNode {
protected:
friend class TreeViewNode;
friend struct MyTreeViewHelperStruct;
public:

    TreeView(Mode _selectionMode=MODE_ALL,bool _allowMultipleSelection=false,Mode _checkboxMode=MODE_NONE,bool _allowAutoCheckboxBehaviour=true,bool _inheritDisabledLook=true);
    virtual ~TreeView();
    bool isInited() {return inited;}
    bool render();  // Main method (makes inited = true). Returns "lastEvent", containing the node that's changed in some way (e.g. double-clicked, end-edited or basic state changed)
    inline Event& getLastEvent() const {return lastEvent;}

    inline int getNumRootNodes() const {return childNodes->size();}
    inline TreeViewNode* getRootNode(int index=0) {return ((childNodes->size()>index) ? (*childNodes)[index] : NULL);}
    inline const TreeViewNode* getRootNode(int index=0) const {return ((childNodes->size()>index) ? (*childNodes)[index] : NULL);}

    inline TreeViewNode* addRootNode(const TreeViewNode::Data& _data,int nodeIndex=-1,bool addEmptyChildNodeVector=false)    {
        return TreeViewNode::CreateNode(_data,this,nodeIndex,addEmptyChildNodeVector);
    }
    inline static void DeleteNode(TreeViewNode* n) {TreeViewNode::DeleteNode(n);}    

    void clear();

    // sorting
    void sortRootNodes(bool recursive,int (*comp)(const void *, const void *))  {TreeViewNode::sortChildNodes(recursive,comp);}
    void sortRootNodesByDisplayName(bool recursive=false,bool reverseOrder=false) {TreeViewNode::sortChildNodesByDisplayName(recursive,reverseOrder);}
    void sortRootNodesByTooltip(bool recursive=false,bool reverseOrder=false) {TreeViewNode::sortChildNodesByTooltip(recursive,reverseOrder);}
    void sortRootNodesByUserText(bool recursive=false,bool reverseOrder=false) {TreeViewNode::sortChildNodesByUserText(recursive,reverseOrder);}
    void sortRootNodesByUserId(bool recursive=false,bool reverseOrder=false) {TreeViewNode::sortChildNodesByUserId(recursive,reverseOrder);}

    // state
    void addStateToAllRootNodes(int stateFlag, bool recursive = false) const {TreeViewNode::addStateToAllChildNodes(stateFlag,recursive);}
    void removeStateFromAllRootNodes(int stateFlag, bool recursive = false) const {TreeViewNode::removeStateFromAllChildNodes(stateFlag,recursive);}
    bool isStatePresentInAllRootNodes(int stateFlag) const {return TreeView::isStatePresentInAllChildNodes(stateFlag);}
    bool isStateMissingInAllRootNodes(int stateFlag) const {return TreeView::isStateMissingInAllChildNodes(stateFlag);}

    // These methods are related to the whole node hierarchy
    void addStateToAllDescendants(int stateFlag) const {TreeViewNode::addStateToAllDescendants(stateFlag);}
    void removeStateFromAllDescendants(int stateFlag) const {TreeViewNode::removeStateFromAllDescendants(stateFlag);}
    bool isStatePresentInAllDescendants(int stateFlag) const {return TreeViewNode::isStatePresentInAllDescendants(stateFlag);}
    bool isStateMissingInAllDescendants(int stateFlag) const {return TreeViewNode::isStateMissingInAllDescendants(stateFlag);}

    // if "recursive==true" deleting the "result nodes" in order shouldn't work, but probably it works in the reverse order (TO TEST)
    void getAllRootNodesWithState(ImVector<TreeViewNode*>& result,int stateFlag,bool recursive = false, bool returnOnlyLeafNodes=false,bool clearResultBeforeUsage=true) const {TreeViewNode::getAllChildNodesWithState(result,stateFlag,recursive,returnOnlyLeafNodes,clearResultBeforeUsage);}
    void getAllRootNodesWithoutState(ImVector<TreeViewNode*>& result,int stateFlag,bool recursive = false, bool returnOnlyLeafNodes=false,bool clearResultBeforeUsage=true) const {TreeViewNode::getAllChildNodesWithoutState(result,stateFlag,recursive,returnOnlyLeafNodes,clearResultBeforeUsage);}
    void getAllRootNodes(ImVector<TreeViewNode*>& result,bool recursive = false,bool returnOnlyLeafNodes=false,bool clearResultBeforeUsage=true) const {TreeViewNode::getAllChildNodes(result,recursive,returnOnlyLeafNodes,clearResultBeforeUsage);}

    // deleting the "result nodes" in order shouldn't work, but probably it works in the reverse order (TO TEST)
    void getAllNodes(ImVector<TreeViewNode*>& result,bool clearResultBeforeUsage=true) const {return getAllRootNodes(result,true,false,clearResultBeforeUsage);}
    void getAllNodesWithState(ImVector<TreeViewNode*>& result,int stateFlag,bool clearResultBeforeUsage=true) const {return getAllRootNodesWithState(result,stateFlag,true,false,clearResultBeforeUsage);}
    void getAllLeafNodesWithoutState(ImVector<TreeViewNode*>& result,int stateFlag,bool clearResultBeforeUsage=true) const {return getAllRootNodesWithoutState(result,stateFlag,true,true,clearResultBeforeUsage);}

    // deleting the "result nodes" in order shouldn't work, but probably it works in the reverse order (TO TEST)
    void getAllLeafNodes(ImVector<TreeViewNode*>& result,bool clearResultBeforeUsage=true) const {return getAllRootNodes(result,true,true,clearResultBeforeUsage);}
    void getAllLeafNodesWithState(ImVector<TreeViewNode*>& result,int stateFlag,bool clearResultBeforeUsage=true) const {return getAllRootNodesWithState(result,stateFlag,true,true,clearResultBeforeUsage);}
    void getAllNodesWithoutState(ImVector<TreeViewNode*>& result,int stateFlag,bool clearResultBeforeUsage=true) const {return getAllRootNodesWithoutState(result,stateFlag,true,false,clearResultBeforeUsage);}

    // -1 = disabled. When >= 0 if node->getDepth()==collapseToLeafNodesAtNodeDepth the hierarchy is flattened to leaf nodes
    void setCollapseNodesToLeafNodesAtDepth(int nodeDepth) const {collapseToLeafNodesAtNodeDepth=nodeDepth;}
    int getCollapseNodesToLeafNodesAtDepth() const {return collapseToLeafNodesAtNodeDepth;}

    // Callbacks:
    typedef void (*TreeViewNodeCallback)(TreeViewNode* node,TreeView& parent,void* userPtr);
    void setTreeViewNodePopupMenuDrawerCb(TreeViewNodeCallback cb,void* userPtr=NULL) {treeViewNodePopupMenuDrawerCb = cb;treeViewNodePopupMenuDrawerCbUserPtr = userPtr;}
    inline TreeViewNodeCallback getTreeViewNodePopupMenuDrawerCb() const {return treeViewNodePopupMenuDrawerCb;}
    inline static const char* GetTreeViewNodePopupMenuName() {return "TreeViewNodePopupMenu";}  // you can use this name inside the callback: e.g. ImGui::BeginPopup(ImGui::TreeView::GetTreeViewNodePopupMenuName());
    // must return true if icon is hovered. If set, use ImGui::SameLine() before returning
    typedef bool (*TreeViewNodeDrawIconCallback)(TreeViewNode* node,TreeView& parent,void* userPtr);
    void setTreeViewNodeDrawIconCb(TreeViewNodeDrawIconCallback cb,void* userPtr=NULL) {treeViewNodeDrawIconCb = cb;treeViewNodeDrawIconCbUserPtr = userPtr;}
    inline TreeViewNodeDrawIconCallback getTreeViewNodeDrawIconCb() const {return treeViewNodeDrawIconCb;}
    // just called after all rendering in this node (can be used to append stuff at the right of the line)
    typedef void (*TreeViewNodeAfterDrawCallback)(TreeViewNode* node,TreeView& parent,float windowWidth,void* userPtr);
    void setTreeViewNodeAfterDrawCb(TreeViewNodeAfterDrawCallback cb,void* userPtr=NULL) {treeViewNodeAfterDrawCb = cb;treeViewNodeAfterDrawCbUserPtr = userPtr;}
    inline TreeViewNodeAfterDrawCallback getTreeViewNodeAfterDrawCb() const {return treeViewNodeAfterDrawCb;}
    // called after a node is created and before it's deleted (usable for TreeViewNode::userPtrs)
    typedef void (*TreeViewNodeCreationDelationCallback)(TreeViewNode* node,TreeView& parent,bool delation,void* userPtr);
    void setTreeViewNodeCreationDelationCb(TreeViewNodeCreationDelationCallback cb,void* userPtr=NULL) {treeViewNodeCreationDelationCb = cb;treeViewNodeCreationDelationCbUserPtr = userPtr;}
    inline TreeViewNodeCreationDelationCallback getTreeViewNodeCreationDelationCb() const {return treeViewNodeCreationDelationCb;}


    void *userPtr;                  // user stuff, not mine

    ImVec4* getTextColorForStateColor(int aStateColorFlag) const;
    ImVec4* getTextDisabledColorForStateColor(int aStateColorFlag) const;

    void setTextColorForStateColor(int aStateColorFlag,const ImVec4& textColor,float disabledTextColorAlphaFactor=0.5f) const;

//-------------------------------------------------------------------------------
#       if (defined(IMGUIHELPER_H_) && !defined(NO_IMGUIHELPER_SERIALIZATION))
#       ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
public:
        bool save(ImGuiHelper::Serializer& s);
        bool save(const char* filename);
        static int Save(const char* filename, TreeView **pTreeViews, int numTreeviews);   // returns the number of saved TreeViews
#       endif //NO_IMGUIHELPER_SERIALIZATION_SAVE
#       ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
public:
        bool load(ImGuiHelper::Deserializer& d,const char** pOptionalBufferStart=NULL);
        bool load(const char* filename);
        static int Load(const char* filename,TreeView** pTreeViews,int numTreeviews);   // returns the number of loaded TreeViews
#       endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#       endif //NO_IMGUIHELPER_SERIALIZATION
//--------------------------------------------------------------------------------

    static void SetFontCheckBoxGlyphs(const char* emptyState,const char* fillState);
    static inline bool HasCustomCheckBoxGlyphs() {return FontCheckBoxGlyphs[0][0]!='\0';}
    static void SetFontArrowGlyphs(const char* leftArrow,const char* downArrow);
    static inline bool HasCustomArrowGlyphs() {return FontArrowGlyphs[0][0]!='\0';}


    // TODO: Fix stuff in this area ------------------------------------------------
    // we leave these public...     // to make protected
    unsigned int selectionMode;     // it SHOULD be a TreeViewNode::Mode, but we use a uint to fit ImGui::ComboFlag(...) directly (to change)
    bool allowMultipleSelection;

    unsigned int checkboxMode;
    bool allowAutoCheckboxBehaviour;

    bool inheritDisabledLook;   // Not the STATE_DISABLED flag! Just the look. (Can we replace it with a unsigned int for generic state look-inheritance?)
    //------------------------------------------------------------------------------

protected:
    TreeViewNodeCallback treeViewNodePopupMenuDrawerCb;
    void* treeViewNodePopupMenuDrawerCbUserPtr;
    TreeViewNodeDrawIconCallback treeViewNodeDrawIconCb;
    void* treeViewNodeDrawIconCbUserPtr;
    TreeViewNodeAfterDrawCallback treeViewNodeAfterDrawCb;
    void* treeViewNodeAfterDrawCbUserPtr;
    TreeViewNodeCreationDelationCallback treeViewNodeCreationDelationCb;
    void* treeViewNodeCreationDelationCbUserPtr;

    bool inited;
    mutable ImVec4 stateColors[6];  // 3 pairs of textColor-textDisabledColor

    mutable Event lastEvent;

    static char FontCheckBoxGlyphs[2][5];
    static char FontArrowGlyphs[2][5];

    mutable int collapseToLeafNodesAtNodeDepth; // -1 = disabled. When >= 0 if node->getDepth()>=collapseToLeafNodesAtNodeDepth the hierarchy is flattened to leaf nodes


protected:
    TreeView(const TreeView& tv) : TreeViewNode(tv) {}
    void operator=(const TreeView&) {}
    TreeView(const TreeViewNode&) {}
    void operator=(const TreeViewNode&) {}



};
typedef TreeViewNode::Data TreeViewNodeData;
typedef TreeViewNode::State TreeViewNodeState;
typedef TreeViewNode::Mode TreeViewNodeMode;
typedef TreeViewNode::Event TreeViewEvent;

// Timeline (from: https://github.com/nem0/LumixEngine/blob/timeline_gui/external/imgui/imgui_user.h)=
/* Possible enhancements:
 * Add some kind of "snap to grid" epsilon
 * Add different types of TimelineEvent (e.g. multiple ranges in a single line, dot-like markers, etc.)
*/
// opt_exact_num_rows: optional, when !=0, enables item culling
// popt_offset_and_scale: optional reference to a static ImVec2 (CTRL+RMB drag: pan in [0.f,1.f]; CTRL+MW: zoom in [0.25f,4.f]). Thanks @meshula!
bool BeginTimeline(const char* str_id, float max_value=0.f, int num_visible_rows=0, int opt_exact_num_rows=0,ImVec2* popt_offset_and_scale=NULL);
bool TimelineEvent(const char* str_id, float* values, bool keep_range_constant=false);
void EndTimeline(int num_vertical_grid_lines=5.f,float current_time=0.f,ImU32 timeline_running_color=IM_COL32(0,128,0,200));
// End Timeline ======================================================================================


// mobile lock control: a very unsafe way of using password
// passwordSize: must be: gridSize*gridSize+1, where gridSize is in [2,6]
// size: is the width and height of the widget in pixels
// colors: please copy defaultColors in the definition (.cpp file), and modify it as needed.
typedef int ImGuiPasswordDrawerFlags;
bool PasswordDrawer(char* password, int passwordSize, ImGuiPasswordDrawerFlags flags=0, const float size=0, const ImU32 colors[7]=NULL);



} // namespace ImGui

enum ImGuiPasswordDrawerFlags_  {
    ImGuiPasswordDrawerFlags_ReadOnly         = 1 << 1,   // password is not touched, it's simply shown. Returns true when the (whole) widget pressed. [Note that passwordSize must still be gridSize*gridSize+1, even if the (untouched) password buffer is shorter]
    ImGuiPasswordDrawerFlags_NoFilledCircles  = 1 << 2,   // Filled circles are hidden
    ImGuiPasswordDrawerFlags_NoLines          = 1 << 4,   // Draw lines are hidden
    ImGuiPasswordDrawerFlags_Hidden           =           // Everything is hidden [to prevent someone from spotting your password]
    ImGuiPasswordDrawerFlags_NoFilledCircles|ImGuiPasswordDrawerFlags_NoLines
};


namespace ImGui {

// Experimental: CheckboxFlags(...) overload to handle multiple flags with a single call
// returns the value of the pressed flag (not the index of the check box), or zero
// flagAnnotations, when!=0, just displays a circle in the required checkboxes
// itemHoveredOut, when used, returns the index of the hovered check box (not its flag), or -1.
// pFlagsValues, when used, must be numFlags long, and must contain the flag values (not the flag indices) that the control must use.
// KNOWN BUG: When ImGui::SameLine() is used after it, the alignment is broken
unsigned int CheckboxFlags(const char* label,unsigned int* flags,int numFlags,int numRows,int numColumns,unsigned int flagAnnotations=0,int* itemHoveredOut=NULL,const unsigned int* pFlagsValues=NULL);

// These just differ from the default ones for their look:
// checkBoxScale.y max is clamped to 2.f
// pOptionalEightColors are: {circle_on, circle_on_hovered, circle_off, circle_off_hovered, bg_on, bg_on_hovered, bg_off, bg_off_hovered} [The 4 circle colors will have A = 255, even if users choose otherwise]
// checkBoxRounding if negative defaults to style.WindowRounding
bool CheckboxStyled(const char* label, bool* v, const ImU32 *pOptionalEightColors=NULL, const ImVec2 &checkBoxScale=ImVec2(1.f,1.f), float checkBoxRounding=-1.f);
bool CheckboxStyledFlags(const char* label, unsigned int* flags, unsigned int flags_value,const ImU32 *pOptionalEightColors=NULL,const ImVec2 &checkBoxScale=ImVec2(1.f,1.f), float checkBoxRounding=-1.f);

// Minimal implementation from: https://github.com/ocornut/imgui/issues/942
bool KnobFloat(const char* label, float* p_value, float v_min, float v_max, float v_step=50.f);


// Posted by @alexsr here: https://github.com/ocornut/imgui/issues/1901
// Sligthly modified to provide default behaviour with default args
void LoadingIndicatorCircle(const char* label, float indicatorRadiusFactor=1.f,
                                   const ImVec4* pOptionalMainColor=NULL, const ImVec4* pOptionalBackdropColor=NULL,
                                   int circle_count=8, const float speed=1.f);

// Posted by @zfedoran here: https://github.com/ocornut/imgui/issues/1901
// Sligthly modified to provide default behaviour with default args
void LoadingIndicatorCircle2(const char* label, float indicatorRadiusFactor=1.f, float indicatorRadiusThicknessFactor=1.f, const ImVec4* pOptionalColor=NULL);

} // namespace ImGui

#endif
