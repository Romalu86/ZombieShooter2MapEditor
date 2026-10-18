#pragma once
#include "dialog_controls.h"

INT_PTR CALLBACK AppControlPanel(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AppSelectVid(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AppMapProperty(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AppConvertSprite(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AppUnitProperty(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AppTextProperty(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AppRegionProperty(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AppOptions(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AppVidEditor(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AppMapInfo(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AppUnusedVid(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AppAbout(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AppConvertStackLine(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AppEditStackLine(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AppFindSprite(HWND, UINT, WPARAM, LPARAM);

void UpdateScreenForDlg(HWND window);
struct ACT;
ACT* EditorStackSearchAction();

STRING& EditorSpriteSearchText();
struct EDITOR_SPRITE_SEARCH {
    int nVid;
    int army;
    int command;
    int arguments[3];
    int selection;
    unsigned int inverse;
    int mapOverborder;
    int nameNotInWeapons;
};
EDITOR_SPRITE_SEARCH& EditorSpriteSearchOptions();
void FillEditorActionCombo(DIALOG_COMBO_BOX& combo,int command);

INT_PTR CALLBACK AppDialogText(HWND,UINT,WPARAM,LPARAM);
INT_PTR CALLBACK AppEditNumberLine(HWND,UINT,WPARAM,LPARAM);
STRING ActionToText(int command);
STRING ActionComment(int command);
