#include "imgui_internal.h"

struct ImGuiLayoutItem
{
    ImGuiLayoutItemType     Type;               // Type of an item
    ImRect                  MeasuredBounds;
    //-----------------------------------------------------------------------------
    // Docking, Tabs
    float                   SpringWeight;       // Weight of a spring
    float                   SpringSpacing;      // Spring spacing
    float                   SpringSize;         // Calculated spring size
// Tabs
    float                   CurrentAlign;
    float                   CurrentAlignOffset;
    //-----------------------------------------------------------------------------
    unsigned int            VertexIndexBegin;
    unsigned int            VertexIndexEnd;
    ImGuiLayoutItem(ImGuiLayoutItemType type)
    {
        Type = type;
        MeasuredBounds = ImRect(0, 0, 0, 0);    // FIXME: @thedmd are you sure the default ImRect value FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX aren't enough here?
        SpringWeight = 1.0f;
        SpringSpacing = -1.0f;
        SpringSize = 0.0f;
        CurrentAlign = 0.0f;
        CurrentAlignOffset = 0.0f;
        VertexIndexBegin = VertexIndexEnd = (ImDrawIdx)0;
    }
};

struct ImGuiLayout
{
    ImGuiID                     Id;
    ImGuiLayoutType             Type;
    bool                        Live;
    ImVec2                      Size;               // Size passed to BeginLayout
    ImVec2                      CurrentSize;        // Bounds of layout known at the beginning the frame.
    ImVec2                      MinimumSize;        // Minimum possible size when springs are collapsed.
    ImVec2                      MeasuredSize;       // Measured size with springs expanded.

    ImVector<ImGuiLayoutItem>   Items;
    int                         CurrentItemIndex;
    int                         ParentItemIndex;
    ImGuiLayout*                Parent;
    ImGuiLayout*                FirstChild;
    ImGuiLayout*                NextSibling;
    float                       Align;              // Current item alignment.
    float                       Indent;             // Indent used to align items in vertical layout.
    ImVec2                      StartPos;           // Initial cursor position when BeginLayout is called.
    ImVec2                      StartCursorMaxPos;  // Maximum cursor position when BeginLayout is called.

    ImGuiLayout(ImGuiID id, ImGuiLayoutType type)
    {
        Id = id;
        Type = type;
        Live = false;
        Size = CurrentSize = MinimumSize = MeasuredSize = ImVec2(0, 0);
        CurrentItemIndex = 0;
        ParentItemIndex = 0;
        Parent = FirstChild = NextSibling = NULL;
        Align = -1.0f;
        Indent = 0.0f;
        StartPos = ImVec2(0, 0);
        StartCursorMaxPos = ImVec2(0, 0);
    }
};
