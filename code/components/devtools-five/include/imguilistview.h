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

#ifndef IMGUILISTVIEW_H_
#define IMGUILISTVIEW_H_

// USAGE:
/*
  #include <imguilistview.h>

  Then inside an ImGui window just type:
  ImGui::TestListView();    // (and see the code inside this method for further info)
*/

// WHAT'S THIS?
/*
  -> It works on ImGui library v1.31.  Please see this topics: https://github.com/ocornut/imgui/issues/124 and https://github.com/ocornut/imgui/issues/125 for further info
  -> It just display a table with some fields, NOTHING MORE THAN THAT!
     -> No editing
     -> No sorting
     -> No item text formatting
     -> Even no row selecting!
     -> And no interaction too!
*/

// THAT'S BAD! SO, WHY SHOULD I USE THIS CODE INSTEAD OF DIRECTLY USING IMGUI COLUMNS ?
/*
  Ehm... good question!
  At the moment of writing the only possible answer is "automatic clipping":
  If you have many items, only the visible ones will be fetched.

  However, to be honest, the test code here creates all the items at init time:
  that's not always possible in many cases. If your items aren't available, the best that you can do
  is to extend your class directly form ListViewBase (and that might require a lot of work!)

  UPDATE: I've just found another reason for using this code:
  if you want to extend the code and add sorting, formatting and editing support!
*/


// CHANGELOG - REVISIONS
/*
LAST REVISION:
--------------
-> Added a new first argument of ListViewBase::render(...) called "listViewHeight": it defaults to -1 (= full height), but can be set
   to the maximum height of the list view in pixel. Handy if something must be drawn below a long list view.
-> Added "columnWidth" argument to the ListView::Header::ctr(...) to define the starting column witdh in pixels (default is -1=auto).
   This is based on the code posted by an user in the ImGui Issue Section (well, the whole first revision of this gist was based on his code too :)!).
   Please note that:
    -> The column width of the last column cannot be set, because it's implicitely defined by all the other column widths.
    -> Sometimes increasing a specific column width has no effect: in these cases we must decrease the widths of the other columns first.
    -> BUG: Currently when resizing the columns with mouse, it does not work exactly as expected: it goes "too fast". I've still have to find out how to fix this issue.
       Update: the weird behaviour happens when "listViewHeight"==-1 only.
    -> BUG: when "listViewHeight" starts at -1 and dynamically changes to a positive value the column widths become equally spaced.
       It's an annoying bug that does not show up when starting with a positive value and going to -1 and then back to a positive value.
       It's not easy to fix (without breaking something else): maybe as a workaround we can start at frame 0 with a dummy positive value and at frame 1 resetting it to -1 (still looking for some idea about it).
       -> Update: as a possible workaround, we can start using ListViewBase::getMaxPossibleHeight() instead of -1 (although I guess it's not as robust as -1 in case of style-size changes, unless we call it every frame...).
-> Fixed an artifact introduced by a sudden change in the ImGui::Separator() behavior (now it seems to span over multiple columns: maybe it's an ImGui issue that will be reverted back in some future version!).
-> Added sorting direction arrows
-> Added HT_COLOR type. Precision==-1 => hex format: FF88CC, Precision>=1 => float format: 0.55,0.30,1.0
       To display the alpha component we must use the ctr that takes a complete ImGui::ListViewHeaderType struct and pass "numArrayElements" = 4 explicitely.
       By default colors are sorted based on the first float (red), but the full Sorting class can take an additional index offset that can be used to sort colors differently.
       To edit colors differently, try lv->setColorEditingMode(ImGuiColorEditMode_HSV); // (That will change the default color editing mode of the current window)
       Note that you can sort a "color column" based on the element you can specify with the ctr that takes a complete ImGui::ListViewSorting struct.
-> Added support of "array columns" (of 1 to 4 elements) for the header types: HT_INT, HT_UNSIGNED, HT_FLOAT, HT_DOUBLE.
       To use them, use the ctr that takes a complete ImGui::ListViewHeaderType and pass "numArrayElements" accordingly.
       Note that you can sort an "array column" based on the element you can specify with the ctr that takes a complete ImGui::ListViewSorting struct.
-> Added "needsRadiansToDegs" optional argument when using the complete ImGui::ListViewHeaderType ctr. It can be used with HT_FLOAT and HT_DOUBLE to show (and edit) in degrees values stored in radians.
-> Added HT_ICON (user must store a ListViewIconData struct and pass this struct as a void pointer). It works, but I cannot show it in ImGui::TestListView().
-> Added an optional tooltip to the column header (there's an additional arg after the column header name now).
-------------------------------------------------------------------------------------------------------------------
PREVIOUS REVISION 1:
--------------------
New features:
-> added (single) row selection support (ImGui::listViewBase methods: getSelectedRow(),selectRow(),updateSelectedRow(),scrollToSelectedRow())
-> now ImGui::ListView::render(...) returns true when the user changes the row selection by clicking on another row.
-> added ImGui::listViewBase::getSelectedColumn() that returns the index of the last column that has been clicked (although the visible selection encompasses all the columns of the selected row).
-> If you need to extend directly from ImGui::ListViewBase (most likely because you can't instantiate all your row-items at init time),
    now the API is much clearer (you must implement only 4 pure virtual methods).
-> added basic editing support (for all types, except HT_CUSTOM). Unlike sorting support, this is disabled by befault and can be enabled by passing an ImGui::listViewHeaderEditable struct to the ListView::Header::ctr(...).
    Please see TestListView() for further info.
   -> added (ImGui::listViewBase method isInEditingMode())
   -> changed the way HT_ENUM works to make them compliant to the ImGui::Combo callback (Please see TestListView() for further info)

Still missing some things (but most of the work has being done):
-> programmatically column width formatting (at the moment of writing ImGui does not support it).
-> Better alignment of the cells in the selected row when editing a cell (but how to do it?).
-> (optional) row-based contex menu (when ImGui will support them).
-> add other HT_TYPES (it shouldn't be too difficult to add HT_FLOAT2/3/4 HT_INT2/3/4 HT_COLOR and so on: but that would take some time...).

Note. I'll NEVER add multiple row selection support.

-------------------------------------------------------------------------------------------------------------------
PREVIOUS REVISION 2:
-------------------
Changed almost all the code syntax/classes!
New features:
-> support for programmatically (=by writing code) column reordering/hiding, through the additional optional arguments in ListViewBase::render(...). Not shown in ImGui::TestListView(), but it's commented out inside its code.
-> support for basic cell text formatting through the new optional arguments in ListView::Header::ctr(...): _precision,_prefix,_suffix. (_precision works for strings too, affecting the number of displayed characters).
-> support for basic column sorting (by clicking the correspondent column header). Sorting can be disabled through the new optional argument '_sortable' in ListView::Header::ctr(...).
   Please note that, unlike column reordering, row sorting happens IN PLACE (there is no concept of VIEW: that means that once you sort your items they will lose their original ordering forever).

Still missing a lot of things:
-> editing support (although I've starting adding some fields for future development).
-> some kind of user interaction (i.e. ListViewBase::render(...) currently does not pass back to the user any piece of information: that should be changed).
-> programmatically column width formatting (at the moment of writing ImGui does not support it).
-> (row) selecting support (how to process mouse events to do it ?).
-> (optional) row-based contex menu (when ImGui will support them).

TODO: Header columns with type HT_CUSTOM have never been tested.
*/

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API

#include <stdio.h>  // snprintf(...) TODO: see if it's worth moving stuff to imguilistview.cpp (now it's empty)

// TODO: move to the .cpp files together with all its references
#if (defined(_MSC_VER) && !defined(snprintf))
#   define snprintf _snprintf
#endif //(defined(_MSC_VER) && !defined(snprintf))

namespace ImGui {


// Base class that should be used (=extended) only by people that can't use the ListView class.
// Otherwise just skip it
class ListViewBase {
public:
    // enum that defines the variable types that each ListView column can have
    enum HeaderType {
        HT_INT=0,
        HT_UNSIGNED,
        HT_FLOAT,
        HT_DOUBLE,
        //--------------- End types that support 1 to 4 array components ----------
        HT_STRING,
        HT_ENUM,        // like HT_INT, but the text is retrieved through HeaderData::textFromEnumFunctionPointer function ptr
        HT_BOOL,
        HT_COLOR,
        HT_ICON,
        HT_CUSTOM       // By default not editable
    };
    // struct that is used in the "void getHeaderData(size_t column,HeaderData& headerDataOut)" virtual method
    struct HeaderData {
        const char* name;       // make it point to the name of your column header. Do not allocate it.


        // type
        struct Type {
            HeaderType headerType;    // The type of the variable this column contains.
            int numEnumElements;
            int numArrayElements;
            bool needsRadiansToDegs;
            typedef bool (*TextFromEnumDelegate)(void*, int, const char**);
            TextFromEnumDelegate  textFromEnumFunctionPointer;  // used only when type==HT_ENUM, otherwise set it to NULL. The method is used to convert an int to a char*.
            void* textFromEnumFunctionPointerUserData;          // used only when type==HT_ENUM, if you want to share the same TextFromEnumDelegate for multiple enums. Otherwise set it to NULL.
            Type(HeaderType _headerType,int _numEnumElements, TextFromEnumDelegate  _textFromEnumFunctionPointer, void* _textFromEnumFunctionPointerUserData=NULL,int _numArrayElements=1)
                : headerType(_headerType),numEnumElements(_numEnumElements),numArrayElements(_numArrayElements<1?1:_numArrayElements),needsRadiansToDegs(false),textFromEnumFunctionPointer(_textFromEnumFunctionPointer),textFromEnumFunctionPointerUserData(_textFromEnumFunctionPointerUserData) {
                if (numArrayElements>4) numArrayElements=4;
                if (headerType == HT_COLOR) {
                    if (numArrayElements<3) numArrayElements=3;
                    //else if (numArrayElements>4) numArrayElements=4;
                }
                else if (headerType>HT_DOUBLE) numArrayElements=0;  // To forbid possible misusage
            }
            Type(HeaderType _headerType,int _numArrayElements=1,bool _needsRadiansToDegs=false)
                : headerType(_headerType),numEnumElements(0),numArrayElements(_numArrayElements<1?1:_numArrayElements),needsRadiansToDegs(_needsRadiansToDegs),textFromEnumFunctionPointer(NULL),textFromEnumFunctionPointerUserData(NULL){
                 if (numArrayElements>4) numArrayElements=4;
                 if (headerType == HT_COLOR) {
                    if (numArrayElements<3) numArrayElements=3;
                    //else if (numArrayElements>4) numArrayElements=4;
                 }
                 else if (headerType>HT_DOUBLE) numArrayElements=0;  // To forbid possible misusage
            }
        };
        Type type;

        // display formatting
        struct Formatting   {
            int precision;      // in case of HT_STRING max number of displayed characters, in case of HT_FLOAT or HT_DOUBLE the number of decimals to be displayed (experiment for other types and see)
            const char* prefix; // make it point to a string that must be displayed BEFORE the text in each column cell, or just set it to NULL or to "".Do not allocate it.
            const char* suffix; // make it point to a string that must be displayed AFTER the text in each column cell, or just set it to NULL or to "".Do not allocate it.
            float columnWidth;  // -1
            const char* headerTooltip;// optional. Make it point to the name of your column header. Do not allocate it.
            Formatting(int _precision=-1,const char* _prefix=NULL,const char* _suffix=NULL,float _columnWidth=-1,const char* _headerTooltip=NULL) : precision(_precision),prefix(_prefix),suffix(_suffix),columnWidth(_columnWidth),headerTooltip(_headerTooltip) {}
        };
        Formatting formatting;

        // sortable properties
        struct Sorting {
            bool sortable;      // true by default. It enables row sorting by clicking on this column header
            mutable bool sortingAscending;  // used internally (AFAIR). Do not touch
            int sortableElementsOfPossibleArray[4];  // internal usage for now: MUST BE 0!
            Sorting(bool _sortable=true,int _sortableElementOfPossibleArray=0,int _sortableElementOfPossibleArray2=-1,int _sortableElementOfPossibleArray3=-1,int _sortableElementOfPossibleArray4=-1) : sortable(_sortable),sortingAscending(false) {
                sortableElementsOfPossibleArray[0] = _sortableElementOfPossibleArray<0 ? 0 : _sortableElementOfPossibleArray;
                sortableElementsOfPossibleArray[1] = _sortableElementOfPossibleArray2;
                sortableElementsOfPossibleArray[2] = _sortableElementOfPossibleArray3;
                sortableElementsOfPossibleArray[3] = _sortableElementOfPossibleArray4;
            }
        };
        Sorting sorting;

        // editing properties
        struct Editing {
            bool editable;
            int precisionOrStringBufferSize;    // for HT_STRING this must be the size of the string buffer in bytes (it can't be left to -1), for HT_FLOAT or HT_DOUBLE the number of decimals
            double minValue;
            double maxValue;
            Editing(bool _editable=false,int _precisionOrStringBufferSize=-1,double _minValue=0,double _maxValue=100) :editable(_editable),precisionOrStringBufferSize(_precisionOrStringBufferSize),minValue(_minValue),maxValue(_maxValue) {}
        };
        Editing editing;

        HeaderData() : name(NULL),type(HT_STRING),formatting(),sorting(),editing() {}
        void reset() {*this=HeaderData();}
    };
    // struct that is used in the "void getCellData(size_t row,size_t column,CellData& cellDataOut)" virtual method
    struct CellData {
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
        const void* fieldPtr;   // make it point to the variable of the cell of type "HeaderType". >>>> CANNOT BE NULL!!!! <<<< (for HT_ICON it must point to a IconData struct)
        const char* customText; // (only for HT_CUSTOM only; otherwise set it to NULL) make it point to the string you want the cell to display. Do not allocate the string!
        bool* selectedRowPtr;   // a pointer to a mutable bool variable that states whether the cell ROW is selected or not (note that the bool variable it refers is a ROW data, not strictly a CELL data(= row,col) )
        CellData() : fieldPtr(NULL),customText(NULL),selectedRowPtr(NULL) {}
        void reset() {*this=CellData();}
    };

    typedef bool (*PopupMenuDelegate)(int,int,void*);   // row, column, selectedRowPopupMenuUserData

    // virtual methods that can/must be implemented by derived classes:--------------
    virtual size_t getNumColumns() const=0;
    virtual size_t getNumRows() const=0;

protected:
    virtual void getHeaderData(size_t column,HeaderData& headerDataOut) const=0;        // Just fill as many fields as you can in your implementation: string fields are not intended to be allocated! Just make them point your copies!
    virtual void getCellData(size_t row,size_t column,CellData& cellDataOut) const=0;   // Just fill cellDataOut. string fields are not intended to be allocated! Just make them point your copies!

public:
    virtual bool sort(size_t /*column*/) {return false;}    // This must be implemented to perform sorting ('selectedRow' is going to change after sorting: that's why it's a good practice to call updateSelectedRow(...) at the end of its implementation)
    // end virtual methods that can/must be implemented by derived classes:-----------

    // ctr dctr
    ListViewBase() : selectedRow(-1),selectedColumn(-1),editingModePresent(false),editorAllowed(false),scrollToRow(-1),lastSortedColumn(-1),firstTimeDrawingRows(true),/*colorEditingMode(ImGuiColorEditMode_RGB),*/popupMenuOpenAtSelectedColumn(-1),selectedRowPopupMenu(NULL),selectedRowPopupMenuUserData(NULL) {}
    virtual ~ListViewBase() {}

    // (single) selection API
    inline int getSelectedRow() const {return selectedRow;}
    inline int getSelectedColumn() const {return selectedColumn;}
    inline void selectRow(int row) {
        if (selectedRow!=row && getNumColumns()>0)   {
            const size_t numRows = getNumRows();
            CellData cd;
            if (selectedRow>=0 && selectedRow<(int)numRows)  {
                // remove old selection
                getCellData((size_t)selectedRow,0,cd);
                if (cd.selectedRowPtr) *cd.selectedRowPtr = false;
                cd.reset();
            }
            selectedRow = row;
            if (selectedRow>=0 && selectedRow<(int)numRows)  {
                // add new selection
                getCellData((size_t)selectedRow,0,cd);
                if (cd.selectedRowPtr) *cd.selectedRowPtr = true;
                //cd.reset();
            }
            else selectedRow = -1;
            popupMenuOpenAtSelectedColumn = -1;
        }         
        //return selectedRow;
    }
//protected:
    // This methods can be called to retrieve the selected row index after sorting, or after some item is inserted before the selected row.
    // Otherwise the selection will still look correct (it points to a row item field), but the 'selectedRow' field will retain the old value.
    int updateSelectedRow() {
        const size_t numRows = getNumRows();
        if (selectedRow>=0 && getNumColumns()>0)  {
            selectedRow = -1;CellData cd;
            for (size_t row = 0; row < numRows; ++row) {
                cd.reset();
                getCellData((size_t)row,0,cd);
                if (cd.selectedRowPtr && *cd.selectedRowPtr) {
                    selectedRow = (int) row;
                    break;
                }
            }
        }
        return selectedRow;
    }

    void removeSelectedRow() {
        const size_t numRows = getNumRows();
        if (numRows>0 && getNumColumns()>0)  {
            selectedRow = -1;CellData cd;
            for (size_t row = 0; row < numRows; ++row) {
                cd.reset();getCellData((size_t)row,0,cd);
                if (cd.selectedRowPtr && *cd.selectedRowPtr) {
                    *cd.selectedRowPtr = false;
                }
            }
        }
    }

    inline bool isInEditingMode() const {return editingModePresent;}    // true if cell(getSelectedRow(),getSelectedColumn()) is being edited

    void scrollToSelectedRow() const {
        const int numRows = (int) getNumRows();
        if (numRows==0) return;
        scrollToRow = selectedRow;
        if (scrollToRow<0) scrollToRow=0;
        else if (scrollToRow>=numRows) scrollToRow = numRows-1;
        // Next time render() is called we'll try to scroll to it
    }

    void updateHeaderData() const {
        const int numColumns = (int) getNumColumns();
        m_headerData.resize(numColumns);
        for (int col=0;col<numColumns;col++)   {
            HeaderData& hd = m_headerData[col];
            hd.reset();
            getHeaderData(col,hd);
        }
        m_columnOffsets.resize(numColumns);
    }

    float getMaxPossibleHeight() const {
        return ImGui::GetTextLineHeightWithSpacing()*((float)getNumRows()+0.25f);
    }

    //ImGuiColorEditMode getColorEditingMode() const {return colorEditingMode;}
    //void setColorEditingMode(ImGuiColorEditMode mode) {colorEditingMode = mode;}

    void setupPopupMenu(PopupMenuDelegate menu,void* menuUserData=NULL) {selectedRowPopupMenu=menu;selectedRowPopupMenuUserData=menuUserData;}
    void hidePopupMenu() {popupMenuOpenAtSelectedColumn=-1;}

private:
    mutable int selectedRow;mutable int selectedColumn;mutable bool editingModePresent;mutable bool editorAllowed;mutable int scrollToRow;
    mutable ImVector<HeaderData> m_headerData;    // Optimization (just for speedup)
    mutable ImVector<float> m_columnOffsets;
    mutable int lastSortedColumn;
    mutable bool firstTimeDrawingRows;
    //mutable ImGuiColorEditMode colorEditingMode;
    mutable int popupMenuOpenAtSelectedColumn;
    PopupMenuDelegate selectedRowPopupMenu;
    void* selectedRowPopupMenuUserData;
    template<typename T> inline static T GetRadiansToDegs() {
        static T factor = T(180)/(3.1415926535897932384626433832795029);
        return factor;
    }
    template<typename T> inline static T GetDegsToRadians() {
        static T factor = T(3.1415926535897932384626433832795029)/T(180);
        return factor;
    }
    inline static void FormatPrecisionString(char* precisionStr,int precisionStrLen,bool isArray,const char formatChar) {
        if (!isArray) precisionStr[precisionStrLen-1] = formatChar;
        else for (int l=0;l<precisionStrLen;l++) if (precisionStr[l]=='s') precisionStr[l]=formatChar;
    }
    static const char* GetTextFromCellFieldDataPtr(HeaderData& hd,const void*& cellFieldDataPtr)    {
        if (hd.type.headerType==HT_CUSTOM || !cellFieldDataPtr) return "";
        static const int bufferSize = 1024;static char buf[bufferSize];buf[0]='\0';
        static const int precisionStrSize = 24;static char precisionStr[precisionStrSize];precisionStr[0]='\0';
        const int precision = hd.formatting.precision;

        // Here we format "precisionStr", so that later we can replace 's' with a type-based char.
        const bool typeIsArray = hd.type.numArrayElements>1;
        if (!typeIsArray)    {
            if (precision>0) {
                strcpy(precisionStr,"%.");
                snprintf(&precisionStr[2], precisionStrSize-2,"%ds",precision);
            }
            else strcpy(precisionStr,"%s");
        }
        else if ((hd.type.headerType!=HT_COLOR && hd.type.headerType!=HT_ICON) || precision>0){
            if (precision>0) {
                int len=0;int tmp=0;
                while (tmp<hd.type.numArrayElements)    {
                    if (precisionStrSize<=len+6) break;
                    if (tmp>0) {
                        strcat(precisionStr,",%.");len+=3;
                    }
                    else {strcat(precisionStr,"%.");len+=2;}
                    snprintf(&precisionStr[len], precisionStrSize-len,"%ds",precision);
                    len = (int)strlen(precisionStr);
                    ++tmp;

                }
            }
            else {
                int len=0;int tmp=0;
                while (tmp<hd.type.numArrayElements)    {
                    if (precisionStrSize<=len+3) break;
                    if (tmp>0) {
                        strcat(precisionStr,",%s");len+=3;
                    }
                    else {strcat(precisionStr,"%s");len+=2;}
                    ++tmp;
                }
            }
        }

        size_t bufid = 0;int pbufsz = bufferSize;

        const char* prefix = hd.formatting.prefix;
        const char* suffix = hd.formatting.suffix;
        const bool hasPrefix = prefix && strlen(prefix)>0;
        const bool hasSuffix = suffix && strlen(suffix)>0;

        // prefix:
        if (hasPrefix) {
            snprintf(&buf[bufid], pbufsz,"%s",prefix);
            bufid = strlen(buf);
            pbufsz-=bufid;
        }

        // value:
        const bool allowDirectStringForwarding = !hasPrefix && !hasSuffix && precision<=0;
        const int precisionStrLen = (int) strlen(precisionStr);
        switch (hd.type.headerType)   {
        case HT_STRING:     if (allowDirectStringForwarding) return (const char*) cellFieldDataPtr;
            else snprintf(&buf[bufid], pbufsz,precisionStr,(const char*) cellFieldDataPtr);
            break;
        case HT_ENUM:       if (allowDirectStringForwarding) {
                const char * txt = NULL;
                hd.type.textFromEnumFunctionPointer(hd.type.textFromEnumFunctionPointerUserData,*((const int*)cellFieldDataPtr),&txt);
                return txt;
            }
            else {
                const char * txt = NULL;
                if (hd.type.textFromEnumFunctionPointer(hd.type.textFromEnumFunctionPointerUserData,*((const int*)cellFieldDataPtr),&txt) &&
                txt) snprintf(&buf[bufid], pbufsz,precisionStr,txt);
            }
            break;
        case HT_BOOL:       if (allowDirectStringForwarding) return (*((const bool*)cellFieldDataPtr)) ? "true" : "false";
            else {
                if (*((const bool*)cellFieldDataPtr)) snprintf(&buf[bufid], pbufsz,precisionStr,"true");
                else                                  snprintf(&buf[bufid], pbufsz,precisionStr,"false");
            }
            break;
        case HT_INT:        {
            FormatPrecisionString(precisionStr,precisionStrLen,typeIsArray,'d');
            const int* pValue = (const int*)cellFieldDataPtr;
            switch (hd.type.numArrayElements)   {
            case 2: snprintf(&buf[bufid], pbufsz,precisionStr,*(pValue),*(pValue+1));break;
            case 3: snprintf(&buf[bufid], pbufsz,precisionStr,*(pValue),*(pValue+1),*(pValue+2));break;
            case 4: snprintf(&buf[bufid], pbufsz,precisionStr,*(pValue),*(pValue+1),*(pValue+2),*(pValue+3));break;
            default: snprintf(&buf[bufid], pbufsz,precisionStr,*(pValue));break;
            }
        }
            break;
        case HT_UNSIGNED:    {
            FormatPrecisionString(precisionStr,precisionStrLen,typeIsArray,'u');
            const unsigned int* pValue = (const unsigned int*)cellFieldDataPtr;
            switch (hd.type.numArrayElements)   {
            case 2: snprintf(&buf[bufid], pbufsz,precisionStr,*(pValue),*(pValue+1));break;
            case 3: snprintf(&buf[bufid], pbufsz,precisionStr,*(pValue),*(pValue+1),*(pValue+2));break;
            case 4: snprintf(&buf[bufid], pbufsz,precisionStr,*(pValue),*(pValue+1),*(pValue+2),*(pValue+3));break;
            default: snprintf(&buf[bufid], pbufsz,precisionStr,*(pValue));break;
            }
        }
            break;
        case HT_FLOAT:  {
            FormatPrecisionString(precisionStr,precisionStrLen,typeIsArray,'f');
            const float rtd = hd.type.needsRadiansToDegs ? GetRadiansToDegs<float>() : 1.f;
            const float* pValue = (const float*)cellFieldDataPtr;
            switch (hd.type.numArrayElements)   {
            case 2: snprintf(&buf[bufid], pbufsz,precisionStr,(*(pValue))*rtd,(*(pValue+1))*rtd);break;
            case 3: snprintf(&buf[bufid], pbufsz,precisionStr,(*(pValue))*rtd,(*(pValue+1))*rtd,(*(pValue+2))*rtd);break;
            case 4: snprintf(&buf[bufid], pbufsz,precisionStr,(*(pValue))*rtd,(*(pValue+1))*rtd,(*(pValue+2))*rtd,(*(pValue+3))*rtd);break;
            default: snprintf(&buf[bufid], pbufsz,precisionStr,(*(pValue))*rtd);break;
            }
        }
            break;
        case HT_DOUBLE:     {
            FormatPrecisionString(precisionStr,precisionStrLen,typeIsArray,'f');
            const float rtd = hd.type.needsRadiansToDegs ? GetRadiansToDegs<float>() : 1.f;
            const double* pValue = (const double*)cellFieldDataPtr;
            switch (hd.type.numArrayElements)   {
            case 2: snprintf(&buf[bufid], pbufsz,precisionStr,(*(pValue))*rtd,(*(pValue+1))*rtd);break;
            case 3: snprintf(&buf[bufid], pbufsz,precisionStr,(*(pValue))*rtd,(*(pValue+1))*rtd,(*(pValue+2))*rtd);break;
            case 4: snprintf(&buf[bufid], pbufsz,precisionStr,(*(pValue))*rtd,(*(pValue+1))*rtd,(*(pValue+2))*rtd,(*(pValue+3))*rtd);break;
            default: snprintf(&buf[bufid], pbufsz,precisionStr,(*(pValue))*rtd);break;
            }
        }
        break;
        case HT_COLOR:  {
            const float* pFloat = (const float*)cellFieldDataPtr;
            if (precision>0)    {
                // Float version
                FormatPrecisionString(precisionStr,precisionStrLen,typeIsArray,'f');
                if (hd.type.numArrayElements==3) snprintf(&buf[bufid], pbufsz,precisionStr,*pFloat,*(pFloat+1),*(pFloat+2));
                else snprintf(&buf[bufid], pbufsz,precisionStr,*pFloat,*(pFloat+1),*(pFloat+2),*(pFloat+3));
            }
            else {
                // Hex version
                if (hd.type.numArrayElements==3) snprintf(&buf[bufid], pbufsz,"%.2X%.2X%.2X",(unsigned int) (*pFloat*255.0+0.5),(unsigned int) (*(pFloat+1)*255.0+0.5),(unsigned int) (*(pFloat+2)*255.0+0.5));
                else snprintf(&buf[bufid], pbufsz,"%.2X%.2X%.2X%.2X",(unsigned int) (*pFloat*255.0+0.5),(unsigned int) (*(pFloat+1)*255.0+0.5),(unsigned int) (*(pFloat+2)*255.0+0.5),(unsigned int) (*(pFloat+3)*255.0+0.5));
            }
        }
            break;        
        default: break;//return "";
        }

        // suffix:
        if (hasSuffix)  {
            bufid = strlen(buf);
            pbufsz-=bufid;
            snprintf(&buf[bufid], pbufsz,"%s",suffix);
        }

       return buf;
    }

public:
    // main method.
    // listViewHeight: if >= 0 the maximum height of the list view is clipped to this value in pixels. Handy if you want to display something below it.
    // pOptionalColumnReorderVector: can be used to reorder columns in the view (but 'real' column indices won't be changed)
    // maxNumColumnToDisplay: can be used to reduce the number of columns that are displayed.
    // contentRegionWidthForHorizontalScrolling: if > 0, the ImGuiWindowFlags_HorizontalScrollbar flag will be added. Note that the value passed (in pixels) must be
    //                                           bigger than the width of the ImGui window for the scrollbar to show up. Also, in some cases its amount should be
    //                                           proportional to the font size, if we want the size of the columns to grow when the font scales.
    IMGUI_API virtual bool render(float listViewHeight=-1, const ImVector<int> *pOptionalColumnReorderVector=NULL, int maxNumColumnToDisplay=-1, float contentRegionWidthForHorizontalScrolling=-1) const;
};



class ListView : public ListViewBase {
public:

    static const int MaxHeaderSizeInBytes = 256;
    class Header {
    public:
        char name[MaxHeaderSizeInBytes];
        char prefix[MaxHeaderSizeInBytes];
        char suffix[MaxHeaderSizeInBytes];
        char tooltip[1024];
        HeaderData hd;              // Not necesssary. It's here just to cut code length
        void* userPtr;              // user responsibility

        Header(const char* _name,const char* _tooltip,const HeaderData::Type& _type,const int _precision=-1,const float _startColumnWidth=-1,const char* _prefix="",const char* _suffix="",const HeaderData::Sorting& _sorting = HeaderData::Sorting(),const HeaderData::Editing& _editing =  HeaderData::Editing())  {
            init(_name,_tooltip,_type,_precision,_startColumnWidth,_prefix,_suffix,_sorting,_editing);
        }
        Header(const char* _name,const char* _tooltip,const HeaderType _type,const int _precision,const float _startColumnWidth,const char* _prefix="",const char* _suffix="",const HeaderData::Sorting& _sorting = HeaderData::Sorting(),const HeaderData::Editing& _editing =  HeaderData::Editing())  {
            init(_name,_tooltip,HeaderData::Type(_type),_precision,_startColumnWidth,_prefix,_suffix,_sorting,_editing);
        }
        Header(const char* _name,const char* _tooltip,const HeaderData::Type& _type,const int _precision,const float _startColumnWidth,const char* _prefix,const char* _suffix,const bool _sorting,const HeaderData::Editing& _editing =  HeaderData::Editing())  {
            init(_name,_tooltip,_type,_precision,_startColumnWidth,_prefix,_suffix,HeaderData::Sorting(_sorting),_editing);
        }
        Header(const char* _name,const char* _tooltip,const HeaderType _type,const int _precision,const float _startColumnWidth,const char* _prefix,const char* _suffix,const bool _sorting,const HeaderData::Editing& _editing =  HeaderData::Editing())  {
            init(_name,_tooltip,HeaderData::Type(_type),_precision,_startColumnWidth,_prefix,_suffix,HeaderData::Sorting(_sorting),_editing);
        }

    protected:
        void init(const char* _name,const char* _tooltip,const HeaderData::Type& _type,const int _precision=-1,const float _startColumnWidth=-1,const char* _prefix="",const char* _suffix="",const HeaderData::Sorting& _sorting = HeaderData::Sorting(),const HeaderData::Editing& _editing =  HeaderData::Editing())
        {
            IM_ASSERT(_name && strlen(_name)<MaxHeaderSizeInBytes);
            IM_ASSERT(_type.headerType!=HT_ENUM || _type.textFromEnumFunctionPointer);
            IM_ASSERT(_prefix && strlen(_prefix)<MaxHeaderSizeInBytes);
            IM_ASSERT(_suffix && strlen(_suffix)<MaxHeaderSizeInBytes);
            IM_ASSERT(!(_type.headerType==HT_STRING && _editing.editable && _editing.precisionOrStringBufferSize<=0));   // _editing.precisionOrStringBufferSize must be >=0 (the size of the string buffer in bytes)
            if (_tooltip)   {
                IM_ASSERT(strlen(_tooltip)<1024);
                strcpy(tooltip,_tooltip);
            }
            else tooltip[0] = '\0';
            strcpy(name,_name);
            strcpy(prefix,_prefix);
            strcpy(suffix,_suffix);
            hd.type = _type;
            hd.formatting.precision = _precision;
            hd.formatting.columnWidth=_startColumnWidth;
            hd.sorting = _sorting;
            hd.editing = _editing;

            userPtr = NULL;
        }
    };
    class ItemBase {
    public:
        virtual const char* getCustomText(size_t /*column*/) const {return "";}     // Must be implemented only for columns with type HT_CUSTOM
        virtual const void* getDataPtr(size_t column) const=0;                  // Must be implemented for all fields

        ItemBase() : selected(false) {}
        virtual ~ItemBase() {}

    public:
        class SortingHelper {
            inline static int& getColumn() {static int column=0;return column;}
            inline static const int*& getArrayIndices() {
                static const int* arrayIndices=NULL;
                return arrayIndices;
            }
            inline static bool& getAscendingOrder() {static bool ascendingOrder=true;return ascendingOrder;}
        public:
            SortingHelper(int _column=0,bool _ascendingOrder=true,const int* _arrayIndices=NULL) {
                getColumn()=_column;
                getArrayIndices()=_arrayIndices;
                getAscendingOrder()=_ascendingOrder;
            }
            template <typename T> inline static int Compare(const void* item0,const void* item1) {
                const ItemBase* it0 = *((const ItemBase**) item0);
                const ItemBase* it1 = *((const ItemBase**) item1);
                const int* pais = getArrayIndices();
                IM_ASSERT(pais);static const int paisSize = 4;int ai = 0;
                while (ai<paisSize) {
                    const int& arrayIndex = *(pais+ai++);if (arrayIndex<0) return 0;
                    const T& v0 = *((const T*)(it0->getDataPtr(getColumn()))+arrayIndex);
                    const T& v1 = *((const T*)(it1->getDataPtr(getColumn()))+arrayIndex);
                    if (getAscendingOrder()) {
                        if (v0<v1)      return -1;
                        else if (v0>v1) return  1;
                        else            continue;
                    }
                    else {
                        if (v0>v1)      return -1;
                        else if (v0<v1) return  1;
                        else            continue;
                    }
                }
                return 0;
            }
            inline static int Compare_HT_BOOL(const void* item0,const void* item1) {
                const ItemBase* it0 = *((const ItemBase**) item0);
                const ItemBase* it1 = *((const ItemBase**) item1);
                const int* pais = getArrayIndices();
                IM_ASSERT(pais);static const int paisSize = 4;int ai = 0;
                while (ai<paisSize) {
                    const int& arrayIndex = *(pais+ai++);if (arrayIndex<0) return 0;
                    const bool& v0 = *((const bool*)(it0->getDataPtr(getColumn()))+arrayIndex);
                    const bool& v1 = *((const bool*)(it1->getDataPtr(getColumn()))+arrayIndex);
                    if (v0==v1) continue;
                    return (getAscendingOrder() ? (v0?-1:1) : (v0?1:-1));
                }
                return 0;
            }
            inline static int Compare_HT_CUSTOM(const void* item0,const void* item1) {
                const ItemBase* it0 = *((const ItemBase**) item0);
                const ItemBase* it1 = *((const ItemBase**) item1);
                const char* v0 = it0->getCustomText(getColumn());
                const char* v1 = it1->getCustomText(getColumn());
                return getAscendingOrder() ? ((v0<v1)?-1:(v0>v1)?1:0) : ((v0>v1)?-1:(v0<v1)?1:0);
            }
            inline static int Compare_HT_ICON(const void* item0,const void* item1) {
                const ItemBase* it0 = *((const ItemBase**) item0);
                const ItemBase* it1 = *((const ItemBase**) item1);
                const CellData::IconData& v0 = *((const CellData::IconData*)(it0->getDataPtr(getColumn()))+0);
                const CellData::IconData& v1 = *((const CellData::IconData*)(it1->getDataPtr(getColumn()))+0);
                return getAscendingOrder() ? v0.compareTo(v1) : (-v0.compareTo(v1));
            }

        };
    private:
        mutable bool selected;              // true selects the item row, false deselects it.
        friend class ListView;
    };
    ImVector<Header>    headers;    // one per column
    ImVector<ItemBase*> items;      // one per row
public:
    virtual ~ListView() {
        for (size_t i=0,isz=items.size();i<isz;i++) {
            ItemBase*& item = items[i];
            item->~ItemBase();              // ImVector does not call it
            ImGui::MemFree(item);           // items MUST be allocated by the user using ImGui::MemAlloc(...)
            item=NULL;
        }
        items.clear();
    }

    // overridden methods:
    void getHeaderData(size_t column,HeaderData& headerDataOut) const {
        // Here we just have to fill as many headerDataOut fields as we can. IMPORTANT: headerDataOut strings are only references (i.e. don't use strcpy(...)!)
        if ((int)column>=headers.size()) return;
        const Header& h = headers[column];
        headerDataOut = h.hd;  // To speed up this code I've added hd inside h, but this is not necessary.
        // Mandatory: headerDataOut just stores the string references:
        headerDataOut.name = h.name;
        headerDataOut.formatting.prefix = h.prefix;
        headerDataOut.formatting.suffix = h.suffix;
        headerDataOut.formatting.headerTooltip = h.tooltip;
    }
    void getCellData(size_t row,size_t column,CellData& cellDataOut) const  {
        if ((int)row>=items.size() || (int)column>=headers.size()) return;
        const ItemBase& it = *(items[row]);
        cellDataOut.fieldPtr = it.getDataPtr(column);
        cellDataOut.selectedRowPtr = &it.selected;
        if (headers[column].hd.type.headerType==HT_CUSTOM) cellDataOut.customText = it.getCustomText(column);
        else cellDataOut.customText = NULL;
    }

    IMGUI_API bool sort(size_t column);

    size_t getNumColumns() const {return headers.size();}
    size_t getNumRows() const {return items.size();}

protected:

};
typedef ListView::Header ListViewHeader;
typedef ListViewBase::HeaderData::Type ListViewHeaderType;
typedef ListViewBase::HeaderData::Formatting ListViewHeaderFormatting;
typedef ListViewBase::HeaderData::Sorting ListViewHeaderSorting;
typedef ListViewBase::HeaderData::Editing ListViewHeaderEditing;
typedef ImGui::ListView::CellData::IconData ListViewIconData;


#ifndef IMGUILISTVIEW_NOTESTDEMO
// A handy method just to test the classes above. Can be removed otherwise.
IMGUI_API void TestListView();
#endif //IMGUILISTVIEW_NOTESTDEMO

} // namespace ImGui


#endif //IMGUILISTVIEW_H_
