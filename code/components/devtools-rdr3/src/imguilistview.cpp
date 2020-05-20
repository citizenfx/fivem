// Cfx addition
#include "StdInc.h"

//- Common Code For All Addons needed just to ease inclusion as separate files in user code ----------------------
#include <imgui.h>
#undef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h> // Just for IM_PLACEMENT_NEW
//-----------------------------------------------------------------------------------------------------------------


#include "imguilistview.h"

#include <stdlib.h> // qsort

namespace ImGui {

bool ListViewBase::render(float listViewHeight, const ImVector<int> *pOptionalColumnReorderVector, int maxNumColumnToDisplay, float contentRegionWidthForHorizontalScrolling) const {
    //if (listViewHeight<0) listViewHeight = getMaxPossibleHeight();  // Hack to fix an issue, but I leave it to the user, and comment it out
    ImGui::PushID(this);

    const int numColumns = (int) getNumColumns();
    const size_t numRows = getNumRows();

    if (maxNumColumnToDisplay<0) maxNumColumnToDisplay = numColumns;
    if (pOptionalColumnReorderVector && (int)pOptionalColumnReorderVector->size()<maxNumColumnToDisplay) maxNumColumnToDisplay = (int)pOptionalColumnReorderVector->size();
    int col = 0;

    //ImVector<HeaderData> m_headerData;        // We can remove this ImVector, if we call getHeaderData(...) 2X times (not sure if it's faster). UPDATE: used member variable (but this way we can't update it at runtime!)
    const bool mustFetchHeaderData = (int)m_headerData.size()<numColumns;
    if (mustFetchHeaderData) updateHeaderData();

    int columnSortingIndex = -1;

    static ImVec4 transparentColor(1,1,1,0);
    const bool useFullHeight = listViewHeight <0;
    const ImGuiStyle& style = ImGui::GetStyle();
    const float columnHeaderDeltaOffsetX = style.WindowPadding.x;

    // Column headers
    float columnWidthSum = 0;
    ImGui::PushStyleColor(ImGuiCol_Border,style.Colors[ImGuiCol_Separator]);
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Columns(maxNumColumnToDisplay);
    ImGui::PushStyleColor(ImGuiCol_Button,transparentColor);
    ImGui::PushStyleColor(ImGuiCol_Border,transparentColor);
    ImGui::PushStyleColor(ImGuiCol_BorderShadow,transparentColor);
    bool mustDisplayTooltip=false;
    for (int colID=0;colID<maxNumColumnToDisplay;colID++)   {
        col = pOptionalColumnReorderVector ? (*pOptionalColumnReorderVector)[colID] : colID;
        HeaderData& hd = m_headerData[col];
        if (mustFetchHeaderData)    {
            m_columnOffsets[col] = (colID>0 && !useFullHeight) ? (columnWidthSum-columnHeaderDeltaOffsetX) : columnWidthSum;
            if (hd.formatting.columnWidth>0) {
                ImGui::SetColumnOffset(colID,m_columnOffsets[col]);
                columnWidthSum+=hd.formatting.columnWidth;
            }
            else columnWidthSum+=ImGui::GetColumnWidth(colID);
        }
        else if (!useFullHeight) ImGui::SetColumnOffset(colID,m_columnOffsets[col]+columnHeaderDeltaOffsetX);//useFullHeight ? 0 : columnHeaderDeltaOffsetX);
        mustDisplayTooltip = hd.formatting.headerTooltip && strlen(hd.formatting.headerTooltip)>0;
        if (!hd.sorting.sortable) {
            ImGui::Text("%s",hd.name);
            if (mustDisplayTooltip && ImGui::IsItemHovered()) ImGui::SetTooltip("%s",hd.formatting.headerTooltip);
        }
        else {
            if (ImGui::SmallButton(hd.name)) {
                lastSortedColumn = columnSortingIndex = col;
            }
            if (mustDisplayTooltip && ImGui::IsItemHovered()) ImGui::SetTooltip("%s",hd.formatting.headerTooltip);
            if (lastSortedColumn == col)    {
                if (!mustFetchHeaderData) {
                    hd.reset();
                    getHeaderData(col,hd);  // Needed because we must update "hd.sorting.sortingAscending"
                }
                ImGui::SameLine(0,ImGui::GetColumnWidth(colID)-ImGui::CalcTextSize(hd.name).x-ImGui::CalcTextSize(hd.sorting.sortingAscending ? " v" : " ^").x-style.FramePadding.x*2.0-style.ItemSpacing.x);
                ImGui::Text(hd.sorting.sortingAscending ? "v" : "^");
            }
        }
        if (colID!=maxNumColumnToDisplay-1) ImGui::NextColumn();
    }
    ImGui::PopStyleColor(3);
    ImGui::Columns(1);
    ImGui::PushStyleColor(ImGuiCol_Border,style.Colors[ImGuiCol_Separator]);
    ImGui::Separator();
    ImGui::PopStyleColor();

    // Rows
    bool rowSelectionChanged = false;bool colSelectionChanged = false;  // The latter is not exposed but might turn useful
    bool skipDisplaying = false;
    if (!useFullHeight) {
        //ImGui::SetNextWindowContentWidth(ImGui::GetWindowContentRegionWidth() + 50);    // Last number is hard-coded! Bad!
        if (contentRegionWidthForHorizontalScrolling>0) ImGui::SetNextWindowContentSize(ImVec2(contentRegionWidthForHorizontalScrolling,0.f));
        skipDisplaying = !ImGui::BeginChild("##ListViewRows",ImVec2(0,listViewHeight),false,contentRegionWidthForHorizontalScrolling>0 ? ImGuiWindowFlags_HorizontalScrollbar : 0);
    }
    if (!skipDisplaying) {
        const float textLineHeight = ImGui::GetTextLineHeight();
        float itemHeight = ImGui::GetTextLineHeightWithSpacing();        
        int displayStart = 0, displayEnd = (int) numRows;

        ImGui::CalcListClipping(numRows, itemHeight, &displayStart, &displayEnd);

        if (scrollToRow>=0) {
            if (displayStart>scrollToRow)  displayStart = scrollToRow;
            else if (displayEnd<=scrollToRow)   displayEnd = scrollToRow+1;
            else scrollToRow = -1;   // we reset it now
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (displayStart * itemHeight));

        bool isThisRowSelected = false;const char* txt=NULL;bool mustDisplayEditor = false;
        editingModePresent = false;

        const ImVec4 ImGuiColHeader = ImGui::GetStyle().Colors[ImGuiCol_Header];
        HeaderData* hd;CellData cd;columnWidthSum=0;
        ImGui::Columns(maxNumColumnToDisplay);
        for (int colID=0;colID<maxNumColumnToDisplay;colID++)   {
            col = pOptionalColumnReorderVector ? (*pOptionalColumnReorderVector)[colID] : colID;
            hd = &m_headerData[col];

            if (firstTimeDrawingRows)    {
                m_columnOffsets[col] = columnWidthSum;
                if (hd->formatting.columnWidth>0) columnWidthSum+=hd->formatting.columnWidth;
                else columnWidthSum+=ImGui::GetColumnWidth(colID);
                ImGui::SetColumnOffset(colID,m_columnOffsets[col]);
            }
            else m_columnOffsets[col] = ImGui::GetColumnOffset(colID);

            const HeaderData::Type& hdType              = hd->type;
            //const HeaderData::Formatting& hdFormatting  = hd->formatting;
            //const HeaderData::Editing& hdEditing        = hd->editing;
            const bool hdEditable = hd->editing.editable;
            bool partOfTheCellClicked = false;  // currently used only when the quad that displays the HT_COLOR is clicked

            if (hdType.headerType==HT_COLOR || hdType.headerType==HT_ICON)    {
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,transparentColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,transparentColor);
				ImGui::PushStyleColor(ImGuiCol_Button, transparentColor);
            }
            if (!hdEditable) {
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered,transparentColor);
                ImGui::PushStyleColor(ImGuiCol_HeaderActive,transparentColor);
            }
            for (int row = displayStart; row < displayEnd; ++row) {
                isThisRowSelected = (selectedRow == row);
                mustDisplayEditor = isThisRowSelected && hdEditable && selectedColumn==col && hd->type.headerType!=HT_CUSTOM  && hd->type.headerType!=HT_ICON && editorAllowed;

                if (colID==0 && row==scrollToRow) ImGui::SetScrollHere();

                cd.reset();
                getCellData((size_t)row,col,cd);
                ImGui::PushID(cd.fieldPtr);


                if (mustDisplayEditor)  {
                    editingModePresent = true;
                    const HeaderData::Editing& hdEditing = hd->editing;

                    // Draw editor here--------------------------------------------
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
                    const int hdPrecision = hdEditing.precisionOrStringBufferSize;
                    static const int precisionStrSize = 16;static char precisionStr[precisionStrSize];int precisionLastCharIndex;
                    if (hdPrecision>0) {
                        strcpy(precisionStr,"%.");
                        snprintf(&precisionStr[2], precisionStrSize-2,"%ds",hdPrecision);
                        precisionLastCharIndex = strlen(precisionStr)-1;
                    }
                    else {
                        strcpy(precisionStr,"%s");
                        precisionLastCharIndex = 1;
                    }

                    switch (hdType.headerType) {
                    case HT_DOUBLE: {
                        precisionStr[precisionLastCharIndex]='f';
                        const float minValue = (float) hdEditing.minValue;
                        const float maxValue = (float) hdEditing.maxValue;
                        const double rtd = hdType.needsRadiansToDegs ? GetRadiansToDegs<double>() : 1.f;
                        const double dtr = hdType.needsRadiansToDegs ? GetDegsToRadians<double>() : 1.f;
                        double* pField = (double*)cd.fieldPtr;
                        float value[4] = {0,0,0,0};
                        for (int vl=0;vl<hdType.numArrayElements;vl++) {
                            value[vl] = (float) ((*(pField+vl))*rtd);
                        }
                        bool changed = false;
                        switch (hdType.numArrayElements)    {
                        case 2: changed = ImGui::SliderFloat2("##SliderDouble2Editor",value,minValue,maxValue,precisionStr);break;
                        case 3: changed = ImGui::SliderFloat3("##SliderDouble3Editor",value,minValue,maxValue,precisionStr);break;
                        case 4: changed = ImGui::SliderFloat4("##SliderDouble4Editor",value,minValue,maxValue,precisionStr);break;
                        default: changed = ImGui::SliderFloat("##SliderDoubleEditor",value,minValue,maxValue,precisionStr);break;
                        }
                        if (changed)    {
                            for (int vl=0;vl<hdType.numArrayElements;vl++) {
                                *(pField+vl) = (double) value[vl] * dtr;
                            }
                        }
                    }
                        break;
                    case HT_FLOAT: {
                        precisionStr[precisionLastCharIndex]='f';
                        const float minValue = (float) hdEditing.minValue;
                        const float maxValue = (float) hdEditing.maxValue;
                        const float rtd = hdType.needsRadiansToDegs ? GetRadiansToDegs<float>() : 1.f;
                        const float dtr = hdType.needsRadiansToDegs ? GetDegsToRadians<float>() : 1.f;
                        float* pField = (float*)cd.fieldPtr;
                        float value[4] = {0,0,0,0};
                        for (int vl=0;vl<hdType.numArrayElements;vl++) {
                            value[vl] = (float) ((*(pField+vl))*rtd);
                        }
                        bool changed = false;
                        switch (hdType.numArrayElements)    {
                        case 2: changed = ImGui::SliderFloat2("##SliderFloat2Editor",value,minValue,maxValue,precisionStr);break;
                        case 3: changed = ImGui::SliderFloat3("##SliderFloat3Editor",value,minValue,maxValue,precisionStr);break;
                        case 4: changed = ImGui::SliderFloat4("##SliderFloat4Editor",value,minValue,maxValue,precisionStr);break;
                        default: changed = ImGui::SliderFloat("##SliderFloatEditor",value,minValue,maxValue,precisionStr);break;
                        }
                        if (changed)    {
                            for (int vl=0;vl<hdType.numArrayElements;vl++) {
                                *(pField+vl) = (float) value[vl]*dtr;
                            }
                        }
                    }
                        break;
                    case HT_UNSIGNED: {
                        //precisionStr[precisionLastCharIndex]='d';
                        const int minValue = (int) hdEditing.minValue;
                        const int maxValue = (int) hdEditing.maxValue;
                        unsigned* pField = (unsigned*) cd.fieldPtr;
                        int value[4] = {0,0,0,0};
                        for (int vl=0;vl<hdType.numArrayElements;vl++) {
                            value[vl] = (int) *(pField+vl);
                        }
                        bool changed = false;
                        switch (hdType.numArrayElements)    {
                        case 2: changed = ImGui::SliderInt2("##SliderUnsigned2Editor",value,minValue,maxValue,precisionStr);break;
                        case 3: changed = ImGui::SliderInt3("##SliderUnsigned3Editor",value,minValue,maxValue,precisionStr);break;
                        case 4: changed = ImGui::SliderInt4("##SliderUnsigned4Editor",value,minValue,maxValue,precisionStr);break;
                        default: changed = ImGui::SliderInt("##SliderUnsignedEditor",value,minValue,maxValue,precisionStr);break;
                        }
                        if (changed)    {
                            for (int vl=0;vl<hdType.numArrayElements;vl++) {
                                *(pField+vl) = (unsigned) value[vl];
                            }
                        }
                    }
                        break;
                    case HT_INT: {
                        //precisionStr[precisionLastCharIndex]='d';
                        const int minValue = (int) hdEditing.minValue;
                        const int maxValue = (int) hdEditing.maxValue;
                        int* pField = (int*) cd.fieldPtr;
                        int value[4] = {0,0,0,0};
                        for (int vl=0;vl<hdType.numArrayElements;vl++) {
                            value[vl] = (int) *(pField+vl);
                        }
                        bool changed = false;
                        switch (hdType.numArrayElements)    {
                        case 2: changed = ImGui::SliderInt2("##SliderInt2Editor",value,minValue,maxValue,precisionStr);break;
                        case 3: changed = ImGui::SliderInt3("##SliderInt3Editor",value,minValue,maxValue,precisionStr);break;
                        case 4: changed = ImGui::SliderInt4("##SliderInt4Editor",value,minValue,maxValue,precisionStr);break;
                        default: changed = ImGui::SliderInt("##SliderIntEditor",value,minValue,maxValue,precisionStr);break;
                        }
                        if (changed)    {
                            for (int vl=0;vl<hdType.numArrayElements;vl++) {
                                *(pField+vl) = (int) value[vl];
                            }
                        }
                    }
                        break;
                    case HT_BOOL:   {
                        bool * boolPtr = (bool*) cd.fieldPtr;
                        if (*boolPtr) ImGui::Checkbox("true##CheckboxBoolEditor",boolPtr);    // returns true when pressed
                        else ImGui::Checkbox("false##CheckboxBoolEditor",boolPtr);            // returns true when pressed
                    }
                        break;
                    case HT_ENUM: {
                        ImGui::Combo("##ComboEnumEditor",(int*) cd.fieldPtr,hdType.textFromEnumFunctionPointer,hdType.textFromEnumFunctionPointerUserData,hdType.numEnumElements);
                    }
                        break;
                    case HT_STRING: {
                        char* txtField = (char*)  cd.fieldPtr;
                        ImGui::InputText("##InputTextEditor",txtField,hdPrecision,ImGuiInputTextFlags_EnterReturnsTrue);
                    }
                        break;
                    case HT_COLOR:  {
                        float* pColor = (float*) cd.fieldPtr;
                        //ImGui::ColorEditMode(colorEditingMode);
                        if (hdType.numArrayElements==3) ImGui::ColorEdit3("##ColorEdit3Editor",pColor,ImGuiColorEditFlags_NoAlpha);
                        else ImGui::ColorEdit4("##ColorEdit4Editor",pColor,ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview);
                    }
                    default: break;
                    }
                    ImGui::PopStyleVar();
                    // End Draw Editor here----------------------------------------

                }
                else {
                    partOfTheCellClicked = false;
                    txt = NULL;
                    if (hdType.headerType==HT_CUSTOM) {
                        txt = cd.customText;
                        if (txt && txt[0]=='\0') txt=NULL;  // Optional
                    }
                    else {
                        txt = GetTextFromCellFieldDataPtr(*hd,cd.fieldPtr);
                        if (txt && txt[0]=='\0') txt=NULL;  // Optional
                        if (hdType.headerType==HT_COLOR)   {
                            const float *pFloat = (const float*) cd.fieldPtr;
                            const ImVec4 color = ImVec4(*pFloat,*(pFloat+1),*(pFloat+2),hdType.numArrayElements==3?1.f:(*(pFloat+3)));
                            ImGui::PushStyleColor(ImGuiCol_Button,color);
                            //partOfTheCellClicked = ImGui::ColorButton("",color,true);
                            partOfTheCellClicked = ImGui::ColorButton("",color,ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | (hdType.numArrayElements==3 ? ImGuiColorEditFlags_NoAlpha : 0),ImVec2(0,textLineHeight));
                            if (txt) ImGui::SameLine();
                        }
                        else if (hdType.headerType==HT_ICON)    {
                            const CellData::IconData* pIconData = (const CellData::IconData*) cd.fieldPtr;
                            if (pIconData->user_texture_id) {
                                ImVec2 iconSize;
                                iconSize.x = iconSize.y = textLineHeight;
                                partOfTheCellClicked = ImGui::ImageButton(pIconData->user_texture_id,iconSize,pIconData->uv0,pIconData->uv1,0,pIconData->bg_col,pIconData->tint_col);
                                if (txt) ImGui::SameLine();
                            }
                        }
                    }

                    if (txt) {
                        if (isThisRowSelected && !hdEditable) {
                            ImGui::PushStyleColor(ImGuiCol_HeaderHovered,ImGuiColHeader);
                            ImGui::PushStyleColor(ImGuiCol_HeaderActive,ImGuiColHeader);
                        }
                        if (ImGui::Selectable(txt,cd.selectedRowPtr) || partOfTheCellClicked)  {
                            if (!*cd.selectedRowPtr) {
                                *cd.selectedRowPtr = true;
                                if (!partOfTheCellClicked) editorAllowed = (selectedColumn==col);
                                else editorAllowed = false;
                            }
                            else editorAllowed = false;
                            if (selectedRow!=row)   {
                                rowSelectionChanged = true;
                                popupMenuOpenAtSelectedColumn = -1;
                                if (selectedRow>=0 && selectedRow<(int)numRows)  {
                                    // remove old selection
                                    CellData cdOld;getCellData((size_t)selectedRow,0,cdOld);  // Note that we use column 0 (since we retrieve a row data it makes no difference)
                                    if (cdOld.selectedRowPtr) *cdOld.selectedRowPtr = false;
                                }
                            }
                            selectedRow = row;
                            if (selectedColumn!=col) colSelectionChanged = true;
                            selectedColumn = col;
                        }
                        if (isThisRowSelected && !hdEditable) {
                            //-------------------------------------------------------------------------
                            if (selectedRowPopupMenu)    {
                                if (ImGui::GetIO().MouseDown[1] && ImGui::IsItemHovered()) popupMenuOpenAtSelectedColumn = col;
                                if (popupMenuOpenAtSelectedColumn==col) {
                                    //static bool open=true;  // Should it be member variable ?
                                    //ImGui::BeginPopup(&open);
                                    ImGui::OpenPopup("MyOwnListViewMenu");
                                    if (ImGui::BeginPopup("MyOwnListViewMenu")) {
                                        if (selectedRowPopupMenu(row,col,selectedRowPopupMenuUserData)) popupMenuOpenAtSelectedColumn = -1;
                                        else {
                                            ImVec2 pos = ImGui::GetWindowPos();pos.x-=5;pos.y-=5;
                                            ImVec2 size = ImGui::GetWindowSize();size.x+=10;size.y+=10;
                                            const ImVec2& mousePos = ImGui::GetIO().MousePos;
                                            if (mousePos.x<pos.x || mousePos.y<pos.y || mousePos.x>pos.x+size.x || mousePos.y>pos.y+size.y) popupMenuOpenAtSelectedColumn = -1;
                                        }
                                    }
                                    ImGui::EndPopup();
                                }
                            }
                            //----------------------------------------------------------------------------

                            // must be the same colors as (*)
                            ImGui::PopStyleColor();
                            ImGui::PopStyleColor();
                        }
                    }

                    if (hdType.headerType==HT_COLOR) {
                        ImGui::PopStyleColor();
                    }
                }
                ImGui::PopID();
            }
            if (!hdEditable) {
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
            }
            if (hdType.headerType==HT_COLOR || hdType.headerType==HT_ICON)    {
				ImGui::PopStyleColor();
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
            }
            if (colID!=maxNumColumnToDisplay-1) ImGui::NextColumn();
        }
        ImGui::Columns(1);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ((numRows - displayEnd) * itemHeight));
        firstTimeDrawingRows = false;
    }   // skipDisplaying
    if (!useFullHeight) ImGui::EndChild();

    ImGui::PushStyleColor(ImGuiCol_Border,style.Colors[ImGuiCol_Separator]);
    ImGui::Separator();
    ImGui::PopStyleColor();


    ImGui::PopID();

    scrollToRow = -1;   // we must reset it

    // Sorting:
    if (columnSortingIndex>=0) const_cast<ListViewBase*>(this)->sort((size_t) columnSortingIndex);

    return rowSelectionChanged; // Optional data we might want to expose: local variable: 'colSelectionChanged' and class variable: 'isInEditingMode'.
}

bool ListView::sort(size_t column) {
    if ((int)column>=headers.size()) return false;
    Header& h = headers[column];
    HeaderData::Sorting& hds = h.hd.sorting;
    if (!hds.sortable) return false;

    // void qsort( void *ptr, size_t count, size_t size,int (*comp)(const void *, const void *) );
    bool& sortingOrder = hds.sortingAscending;
    ItemBase::SortingHelper sorter((int)column,sortingOrder,&hds.sortableElementsOfPossibleArray[0]);   // This IS actually used!
    typedef int (*CompareDelegate)(const void *, const void *);
    CompareDelegate compareFunction = NULL;

    switch (h.hd.type.headerType)  {
    case HT_BOOL:
        compareFunction = ItemBase::SortingHelper::Compare_HT_BOOL;
        break;
    case HT_CUSTOM:
        compareFunction = ItemBase::SortingHelper::Compare_HT_CUSTOM;
        break;
    case HT_INT:
    case HT_ENUM:
        compareFunction = ItemBase::SortingHelper::Compare<int>;
        break;
    case HT_UNSIGNED:
        compareFunction = ItemBase::SortingHelper::Compare<unsigned>;
        break;
    case HT_FLOAT:
    case HT_COLOR:
        compareFunction = ItemBase::SortingHelper::Compare<float>;
        break;
    case HT_DOUBLE:
        compareFunction = ItemBase::SortingHelper::Compare<double>;
        break;
    case HT_STRING:
        compareFunction = ItemBase::SortingHelper::Compare<char*>;
        break;
    case HT_ICON:
        compareFunction = ItemBase::SortingHelper::Compare_HT_ICON;
        break;
    default:
        return false;
    }
    if (!compareFunction) return false;

    qsort((void *) &items[0],items.size(),sizeof(ItemBase*),compareFunction);
    sortingOrder = !sortingOrder;   // next time it sorts backwards

    updateSelectedRow(); // rows get shuffled after sorting: the visible selection is still correct (the boolean flag ItemBase::selected is stored in our row-item),
    // but the 'selectedRow' field is not updated and must be adjusted
    return true;
}

#ifndef IMGUILISTVIEW_NOTESTDEMO
class MyListViewTestItem : public ImGui::ListView::ItemBase {
public:
    // Support static method for enum1 (the signature is the same used by ImGui::Combo(...))
    static bool GetTextFromEnum1(void* ,int value,const char** pTxt) {
        if (!pTxt) return false;
        static const char* values[] = {"APPLE","LEMON","ORANGE"};
        static int numValues = (int)(sizeof(values)/sizeof(values[0]));
        if (value>=0 && value<numValues) *pTxt = values[value];
        else *pTxt = "UNKNOWN";
        return true;
    }

    // Fields and their pointers (MANDATORY!)
    int index;
    char path[1024];            // Note that if this column is editable, we must specify: ImGui::ListViewHeaderEditing(true,1024); in the ImGui::ListViewHeader::ctr().
    int offset;
    unsigned bytes;
    bool valid;
    float length[3];
    ImVec4 color;
    int enum1;      // Note that it's an enum!
    const void* getDataPtr(size_t column) const {
        switch (column) {
        case 0: return (const void*) &index;
        case 1: return (const void*) path;
        case 2: return (const void*) &offset;
        case 3: return (const void*) &bytes;
        case 4: return (const void*) &valid;
        case 5: return (const void*) &length[0];
        case 6: return (const void*) &color;
        case 7: return (const void*) &enum1;
        }
        return NULL;
        // Please note that we can easily try to speed up this method by adding a new field like:
        // const void* fieldPointers[number of fields];    // and assigning them in our ctr
        // Then here we can just use:
        // IM_ASSERT(column<number of fields);
        // return fieldPointers[column];
    }

    // (Optional) ctr for setting values faster later
    MyListViewTestItem(int _index,const char* _path,int _offset,unsigned _bytes,bool _valid,const ImVec4& _length,const ImVec4& _color,int _enum1)
        : index(_index),offset(_offset),bytes(_bytes),valid(_valid),color(_color),enum1(_enum1) {
        IM_ASSERT(_path && strlen(_path)<1024);
        strcpy(path,_path);
        length[0] = _length.x;length[1] = _length.y;length[2] = _length.z;  // Note that we have used "ImVec4" for _length, just because ImVec3 does not exist...
    }
    virtual ~MyListViewTestItem() {}

};

void TestListView() {
    ImGui::Spacing();
    static ImGui::ListView lv;
    if (lv.headers.size()==0) {
        lv.headers.push_back(ImGui::ListViewHeader("Index",NULL,ImGui::ListView::HT_INT,-1,50));
        lv.headers.push_back(ImGui::ListViewHeader("Path",NULL,ImGui::ListView::HT_STRING,-1,125,"","",true,ImGui::ListViewHeaderEditing(true,1024)));
        lv.headers.push_back(ImGui::ListViewHeader("Offset",NULL,ImGui::ListView::HT_INT,-1,52,"","",true));
        lv.headers.push_back(ImGui::ListViewHeader("Bytes","The number of bytes",ImGui::ListView::HT_UNSIGNED,-1,52));
        lv.headers.push_back(ImGui::ListViewHeader("Valid","A boolean flag",ImGui::ListView::HT_BOOL,-1,85,"Flag: ","!",true,ImGui::ListViewHeaderEditing(true)));
        lv.headers.push_back(ImGui::ListViewHeader("Length","A float[3] array",ImGui::ListViewHeaderType(ImGui::ListView::HT_FLOAT,3),2,90,""," mt",ImGui::ListViewHeaderSorting(true,1,2,0),ImGui::ListViewHeaderEditing(true,3,-180.0,180.0))); // Note that here we use 2 decimals (precision), but 3 when editing; we use an explicit call to "ListViewHeaderType",specifying that the HT_FLOAT is composed by three elements; we have used an explicit call to "ListViewHeaderSorting" specifying that the items must be sorted based on the second float.
        lv.headers.push_back(ImGui::ListViewHeader("Color",NULL,ImGui::ListView::HT_COLOR,-1,65,"","",true,ImGui::ListViewHeaderEditing(true))); // precision = -1 -> Hex notation; precision > 1 -> float notation; other = undefined behaviour. To display alpha we must use "ListViewHeaderType" explicitely like in the line above, specifying 4.


        // for enums we must use the ctr that takes an ImGui::ListViewHeaderType, so we can pass the additional params to bind the enum:
        lv.headers.push_back(ImGui::ListViewHeader("Enum1","An editable enumeration",ImGui::ListViewHeaderType(ImGui::ListView::HT_ENUM,3,&MyListViewTestItem::GetTextFromEnum1),-1,-1,"","",true,ImGui::ListViewHeaderEditing(true)));

        // Just a test: 10000 items
        lv.items.resize(10000);
        MyListViewTestItem* item;
        for (int i=0,isz=(int)lv.items.size();i<isz;i++) {
            item = (MyListViewTestItem*) ImGui::MemAlloc(sizeof(MyListViewTestItem));                       // MANDATORY (ImGuiListView::~ImGuiListView() will delete these with ImGui::MemFree(...))
            IM_PLACEMENT_NEW(item) MyListViewTestItem(
                        i,
                        "My '  ' Dummy Path",
                        i*3,
                        (unsigned)i*4,(i%3==0)?true:false,
                        ImVec4((float)(i*30)/2.7345672,(float)(i%30)/2.7345672,(float)(i*5)/1.34,1.f),  // ImVec3 does not exist... so we use an ImVec4 to initialize a float[3]
                        ImVec4((float)i/(float)(isz-1),0.8f,1.0f-(float)i/(float)(isz-1),1.0f),         // HT_COLOR
                        i%3
            );    // MANDATORY even with blank ctrs. Reason: ImVector does not call ctrs/dctrs on items.
            item->path[4]=(char) (33+(i%64));   //just to test sorting on strings
            item->path[5]=(char) (33+(i/127));  //just to test sorting on strings
            lv.items[i] = item;
        }

        //lv.setColorEditingMode(ImGuiColorEditMode_HSV);   // Optional, but it's window-specific: it affects everything in this window AFAIK
    }

    // 2 lines just to have some feedback
    if (ImGui::Button("Scroll to selected row")) lv.scrollToSelectedRow();ImGui::SameLine();
    ImGui::Text("selectedRow:%d selectedColumn:%d isInEditingMode:%s",lv.getSelectedRow(),lv.getSelectedColumn(),lv.isInEditingMode() ? "true" : "false");

    /*
    static ImVector<int> optionalColumnReorder;
    if (optionalColumnReorder.size()==0) {
        const int numColumns = lv.headers.size();
        optionalColumnReorder.resize(numColumns);
        for (int i=0;i<numColumns;i++) optionalColumnReorder[i] = numColumns-i-1;
    }
    */

    static int maxListViewHeight=200;                             // optional: by default is -1 = as height as needed
    ImGui::SliderInt("ListView Height",&maxListViewHeight,-1,500);// Just Testing "maxListViewHeight" here:

    lv.render((float)maxListViewHeight);//(float)maxListViewHeight,&optionalColumnReorder,-1);   // This method returns true when the selectedRow is changed by the user (however when selectedRow gets changed because of sorting it still returns false, because the pointed row-item does not change)


}
#endif //IMGUILISTVIEW_NOTESTDEMO


} //namespace ImGui
