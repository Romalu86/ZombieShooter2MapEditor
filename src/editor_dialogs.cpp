#include "map_edit.h"
#include "editor_dialogs.h"
#include "region.h"
#include "registry.h"
#include "graph.h"
#include "sprite_collector.h"
#include "engine_globals.h"
#include <cstring>

namespace {
unsigned int regionSavedProperty;
int optionsShowHidden;
void AddEditorTabs(DIALOG_TABS& tabs)
{
    const char* names[]={"Dummy","Terrain","Object","Unit","Avia","Menu","Railway","Region"};
    for (int i=0;i<8;++i) { STRING text(names[i]); tabs.AddItemWithData(&text,1<<i); }
    STRING other("Other"); tabs.AddItemWithData(&other,0x600);
}
}

INT_PTR CALLBACK AppSelectVid(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{ return static_cast<MAP_EDIT*>(Map)->DialogSelectVid(hwnd,msg,wParam,lParam); }
int MAP_EDIT::DialogSelectVid(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    DIALOG_LIST_BOX vids(hwnd,1013);
    DIALOG_BUTTON sort(hwnd,2003);
    DIALOG_TABS tabs(hwnd,1101);
    DIALOG_UNSIGNED vidNumber(hwnd,1007);

    if (tabs.IsSelChange(msg,wParam,lParam)) {
        ChangeMouseVid(Mouse->Vid(),static_cast<unsigned int>(tabs.GetCurrentItemData()));
        VidToListBox(&vids,static_cast<unsigned long>(spriteType),Mouse->Vid()->nVid,options.optSortVid);
    } else if (vids.IsDoubleClicked(msg,wParam)) {
        ChangeMouseVid(Vid(vids.GetCurrentStringData()),static_cast<unsigned int>(spriteType));
        EndDialog(hwnd,1);
    } else if (sort.IsClicked(msg,wParam)) {
        options.optSortVid=!options.optSortVid;
        VidToListBox(&vids,static_cast<unsigned long>(spriteType),Mouse->Vid()->nVid,options.optSortVid);
    } else if (vidNumber.IsNotify(msg,wParam,EN_UPDATE)) {
        const int nVid=static_cast<int>(static_cast<unsigned int>(vidNumber));
        if (ValidateVid(nVid)) {
            ChangeMouseVid(Vid(nVid),Vid(nVid)->spriteType);
            VidToListBox(&vids,static_cast<unsigned long>(spriteType),Mouse->Vid()->nVid,options.optSortVid);
            tabs.SetCurrentWithData(spriteType);
        }
    }

    if (msg==272u) {
        AddEditorTabs(tabs);
        tabs.SetMinWidth(50);
        tabs.SetCurrentWithData(spriteType);
        sort.SetCheck(options.optSortVid);
        vids.SetColumnWidth(210);
        VidToListBox(&vids,static_cast<unsigned long>(spriteType),Mouse->Vid()->nVid,options.optSortVid);
        return 1;
    }
    if (msg==273u) {
        switch (wParam & 0xFFFFu) {
        case 1:
            ChangeMouseVid(Vid(vids.GetCurrentStringData()),static_cast<unsigned int>(spriteType));
            EndDialog(hwnd,1);
            break;
        case 2:
            EndDialog(hwnd,0);
            break;
        }
    }
    return 0;
}

INT_PTR CALLBACK AppRegionProperty(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{ return static_cast<MAP_EDIT*>(Map)->DialogRegionProperty(hwnd,msg,wParam,lParam); }
int MAP_EDIT::DialogRegionProperty(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    if (curRegion)
        return curRegion->DialogRegionProperty(hwnd,msg,wParam,lParam);
    EndDialog(hwnd,0);
    return 0;
}
int REGION::DialogRegionProperty(HWND hwnd,UINT msg,WPARAM wParam,LPARAM)
{
    DIALOG_COMBO_BOX texture(hwnd,1057);
    DIALOG_COMBO_BOX environment(hwnd,1058);
    DIALOG_INT x(hwnd,1007);
    DIALOG_INT y(hwnd,1009);
    DIALOG_INT z(hwnd,1011);
    DIALOG_UNSIGNED size_x(hwnd,1008);
    DIALOG_UNSIGNED size_y(hwnd,1010);
    DIALOG_INT fog_top(hwnd,1012);
    DIALOG_INT fog_bottom(hwnd,1013);
    DIALOG_UNSIGNED fog_red(hwnd,1014);
    DIALOG_UNSIGNED fog_green(hwnd,1015);
    DIALOG_UNSIGNED fog_blue(hwnd,1016);
    DIALOG_BUTTON fog_inverse(hwnd,1051);
    DIALOG_BUTTON fog_slow_born(hwnd,1055);
    DIALOG_UNSIGNED source_vid[6]={
        DIALOG_UNSIGNED(hwnd,1076),DIALOG_UNSIGNED(hwnd,1078),DIALOG_UNSIGNED(hwnd,1080),
        DIALOG_UNSIGNED(hwnd,1082),DIALOG_UNSIGNED(hwnd,1084),DIALOG_UNSIGNED(hwnd,1086)};
    DIALOG_UNSIGNED conversion_vid[6]={
        DIALOG_UNSIGNED(hwnd,1077),DIALOG_UNSIGNED(hwnd,1079),DIALOG_UNSIGNED(hwnd,1081),
        DIALOG_UNSIGNED(hwnd,1083),DIALOG_UNSIGNED(hwnd,1085),DIALOG_UNSIGNED(hwnd,1087)};

    if (msg==3u) {
        UpdateScreenForDlg(hwnd);
        return 0;
    }
    if (msg==272u) {
        regionSavedProperty=m_fogFlags;
        Map->VidToControlBox(&texture,0x80u,Vid()->nVid);
        Map->VidToControlBox(&environment,0x80u,m_regionVid->nVid);
        x=TruncateFloatToInt32(X());
        y=TruncateFloatToInt32(Y());
        z=TruncateFloatToInt32(Z());
        size_x=static_cast<unsigned int>(TruncateFloatToInt32(m_halfWidth));
        size_y=static_cast<unsigned int>(TruncateFloatToInt32(m_halfHeight));
        fog_top=m_fogEnd;
        fog_bottom=m_fogStart;
        fog_red=((m_fogColor.value>>16)&255u);
        fog_green=((m_fogColor.value>>8)&255u);
        fog_blue=(m_fogColor.value&255u);
        fog_inverse.SetCheck((m_fogFlags&2u)!=0u);
        fog_slow_born.SetCheck((m_fogFlags&1u)!=0u);
        for (int i=0;i<6;++i) {
            source_vid[i]=m_sourceVid[i] ? static_cast<unsigned int>(m_sourceVid[i]->nVid) : 0u;
            conversion_vid[i]=m_targetVid[i] ? static_cast<unsigned int>(m_targetVid[i]->nVid) : 0u;
        }
        SendDlgItemMessageA(hwnd,2001,241u,m_fogFlags&8u,0);
        SendDlgItemMessageA(hwnd,2002,241u,m_fogFlags&4u,0);
        SendDlgItemMessageA(hwnd,2003,241u,m_fogFlags&0x10u,0);
        const int enabled=(m_fogFlags&8u) ? 0 : 1;
        x.Enable(enabled); y.Enable(enabled); z.Enable(enabled); size_x.Enable(enabled); size_y.Enable(enabled);
        return 1;
    }
    if (msg!=273u)
        return 0;

    switch (wParam&0xFFFFu) {
    case 1: {
        Action(62,texture.GetCurrentStringData(),0,0);
        m_regionVid=Map->Vid(environment.GetCurrentStringData());
        const COLOR fog(static_cast<int>(static_cast<unsigned int>(fog_red)),
            static_cast<int>(static_cast<unsigned int>(fog_green)),
            static_cast<int>(static_cast<unsigned int>(fog_blue)));
        SetFogParameters(static_cast<int>(fog_bottom),static_cast<int>(fog_top),fog);
        ChangeCoor(VECTOR(static_cast<float>(static_cast<int>(x)),
                   static_cast<float>(static_cast<int>(y)),
                   static_cast<float>(static_cast<int>(z))));
        SetSize(static_cast<float>(static_cast<unsigned int>(size_x)),
                static_cast<float>(static_cast<unsigned int>(size_y)));
        for (int i=0;i<6;++i) {
            m_sourceVid[i]=Map->Vid(static_cast<int>(static_cast<unsigned int>(source_vid[i])));
            if (m_sourceVid[i]->nVid<=0) m_sourceVid[i]=0;
            m_targetVid[i]=Map->Vid(static_cast<int>(static_cast<unsigned int>(conversion_vid[i])));
            if (m_targetVid[i]->nVid<=0) m_targetVid[i]=0;
        }
        EndDialog(hwnd,1);
        break;
    }
    case 2:
        m_fogFlags=regionSavedProperty;
        EndDialog(hwnd,0);
        break;
    case 1051: m_fogFlags^=2u; break;
    case 1055: m_fogFlags^=1u; break;
    case 2001: {
        m_fogFlags^=8u;
        const int enabled=(m_fogFlags&8u) ? 0 : 1;
        x.Enable(enabled); y.Enable(enabled); z.Enable(enabled); size_x.Enable(enabled); size_y.Enable(enabled);
        break;
    }
    case 2002: m_fogFlags^=4u; break;
    case 2003: m_fogFlags^=0x10u; break;
    default: break;
    }
    return 0;
}

INT_PTR CALLBACK AppOptions(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{ return static_cast<MAP_EDIT*>(Map)->DialogOptions(hwnd,msg,wParam,lParam); }
int MAP_EDIT::DialogOptions(HWND hwnd,UINT msg,WPARAM wParam,LPARAM)
{
    DIALOG_UNSIGNED airBrushSize(hwnd,1007);
    DIALOG_UNSIGNED airBrushDensity(hwnd,1008);
    DIALOG_UNSIGNED bigStepZ(hwnd,1009);
    DIALOG_LIST_BOX vids(hwnd,1013);
    DIALOG_BUTTON showHidden(hwnd,2001), drawName(hwnd,2002), flag16(hwnd,2003);
    DIALOG_UNSIGNED autosave(hwnd,1010), gridX(hwnd,1023), gridY(hwnd,1026);
    DIALOG_UNSIGNED conversion[5]={DIALOG_UNSIGNED(hwnd,1011),DIALOG_UNSIGNED(hwnd,1014),
        DIALOG_UNSIGNED(hwnd,1016),DIALOG_UNSIGNED(hwnd,1018),DIALOG_UNSIGNED(hwnd,1020)};
    DIALOG_TEXT names[5]={DIALOG_TEXT(hwnd,1012),DIALOG_TEXT(hwnd,1015),
        DIALOG_TEXT(hwnd,1017),DIALOG_TEXT(hwnd,1019),DIALOG_TEXT(hwnd,1021)};

    if (msg==272u) {
        airBrushSize=static_cast<unsigned int>(optAirBrushSize);
        airBrushDensity=optAirBrushDensity;
        bigStepZ=optBigStepZ;
        autosave=optAutoSavePeriod/1000u;
        gridX=static_cast<unsigned int>(optGridLineX); gridY=static_cast<unsigned int>(optGridLineY);
        drawName.SetCheck(options.optDrawName); flag16.SetCheck(options.optFlag16);
        for (int i=0;i<5;++i) { conversion[i]=static_cast<unsigned int>(convertDrawingVid[i]); names[i]=&nameDrawingVid[i]; }
        showHidden.SetCheck(optionsShowHidden);
        VidToListBox(&vids,0xDFu,-2,optionsShowHidden);
        for (int i=vids.NoString()-1;i>=0;--i) {
            if (Vid(vids.GetData(i))->PropHide())
                vids.SetSelected(i);
        }
        return 1;
    }

    if (msg==273u) {
        const unsigned int id=wParam & 0xFFFFu;
        switch (id) {
        case 1:
            optAirBrushSize=static_cast<int>(static_cast<unsigned int>(airBrushSize));
            optAirBrushDensity=static_cast<unsigned int>(airBrushDensity);
            optBigStepZ=static_cast<int>(static_cast<unsigned int>(bigStepZ));
            optGridLineX=static_cast<int>(static_cast<unsigned int>(gridX));
            optGridLineY=static_cast<int>(static_cast<unsigned int>(gridY));
            optAutoSavePeriod=static_cast<unsigned int>(autosave)*1000u;
            options.optDrawName=drawName.GetCheck(); options.optFlag16=flag16.GetCheck();
            Registry->SetInt(STRING("BigStepZ"),optBigStepZ);
            for (int i=0;i<5;++i) { convertDrawingVid[i]=static_cast<int>(static_cast<unsigned int>(conversion[i])); nameDrawingVid[i]=static_cast<STRING>(names[i]); }
            for (int i=vids.NoString()-1;i>=0;--i)
                Vid(vids.GetData(i))->SetPropHide(vids.IsSelected(i));
            EndDialog(hwnd,1);
            break;
        case 2:
            EndDialog(hwnd,0);
            break;
        case 2001:
            optionsShowHidden^=1;
            VidToListBox(&vids,0xDFu,-2,optionsShowHidden);
            for (int i=vids.NoString()-1;i>=0;--i) {
                if (Vid(vids.GetData(i))->PropHide())
                    vids.SetSelected(i);
            }
            break;
        case 1073:
            airBrushSize=5u;
            airBrushDensity=20u;
            break;
        case 1074:
            airBrushSize=10u;
            airBrushDensity=20u;
            break;
        case 1075:
            airBrushSize=50u;
            airBrushDensity=5u;
            break;
        case 1022:
        case 1068:
        case 1066:
        case 1067:
        case 1024:
        case 1025: {
            unsigned int typeMask=0;
            switch (id) {
            case 1022: typeMask=0xDFu; break;
            case 1068: typeMask=1u; break;
            case 1066: typeMask=2u; break;
            case 1067: typeMask=4u; break;
            case 1024: typeMask=8u; break;
            default:    typeMask=16u; break;
            }
            DIALOG_BUTTON typeButton(hwnd,static_cast<int>(id));
            const int checked=typeButton.GetCheck() ? 1 : 0;
            for (int i=vids.NoString()-1;i>=0;--i) {
                VID* vid=Vid(vids.GetData(i));
                if ((vid->spriteType&typeMask)!=0u) {
                    if (checked) vids.SetSelected(i);
                    else vids.SetUnSelected(i);
                }
                vid->SetPropHide(vids.IsSelected(i));
            }
            UpdateScreenForDlg(hwnd);
            break;
        }
        default:
            break;
        }
        return 0;
    }

    if (msg==3u) {
        for (int i=vids.NoString()-1;i>=0;--i)
            Vid(vids.GetData(i))->SetPropHide(vids.IsSelected(i));
        UpdateScreenForDlg(hwnd);
    }
    return 0;
}

STRING& EditorSpriteSearchText() { static STRING value; return value; }
EDITOR_SPRITE_SEARCH& EditorSpriteSearchOptions()
{
    static EDITOR_SPRITE_SEARCH options={99999,9,-1,{-999999,-999999,-999999},0,0u,0,0};
    return options;
}
INT_PTR CALLBACK AppFindSprite(HWND hwnd,UINT msg,WPARAM wParam,LPARAM)
{
    EDITOR_SPRITE_SEARCH& options=EditorSpriteSearchOptions();
    DIALOG_TEXT text(hwnd,1011);
    DIALOG_INT nVid(hwnd,1007),army(hwnd,1012);
    DIALOG_COMBO_BOX command(hwnd,1057);
    DIALOG_INT arguments[3]={{hwnd,1008},{hwnd,1009},{hwnd,1010}};
    DIALOG_BUTTON inverseArguments[3]={{hwnd,1121},{hwnd,1122},{hwnd,1123}};
    DIALOG_BUTTON inverseCommand(hwnd,1124),overborder(hwnd,2005),unknownName(hwnd,1125);
    if (msg==WM_INITDIALOG) {
        const STRING off("<OFF stack search>"),empty("<empty stack>");
        const int offIndex=command.AddStringWithData(&off,-1);
        const int emptyIndex=command.AddStringWithData(&empty,-2);
        if (options.command==-1) command.SetCurrent(offIndex);
        else if (options.command==-2) command.SetCurrent(emptyIndex);
        FillEditorActionCombo(command,options.command);
        text=&EditorSpriteSearchText();
        nVid=options.nVid;
        army=options.army;
        CheckRadioButton(hwnd,1092,1095,1092+options.selection);
        overborder.SetCheck(options.mapOverborder);
        unknownName.SetCheck(options.nameNotInWeapons);
        inverseCommand.SetCheck((options.inverse&1u)!=0u);
        for (int i=0;i<3;++i) {
            arguments[i]=options.arguments[i];
            inverseArguments[i].SetCheck((options.inverse&(2u<<i))!=0u);
        }
        return TRUE;
    }
    if (msg==WM_COMMAND) switch (LOWORD(wParam)) {
    case 1116: {
        const STRING off("<OFF stack search>");
        command.SetCurrent(&off);
        inverseCommand.SetCheck(0);
        for (int i=0;i<3;++i) {
            inverseArguments[i].SetCheck(0);
            arguments[i]=-999999;
        }
        break;
    }
    case IDOK:
        EditorSpriteSearchText()=static_cast<STRING>(text);
        options.nVid=nVid.GetText().IsEmpty()?99999:static_cast<int>(nVid);
        options.army=army.GetText().IsEmpty()?9:static_cast<int>(army);
        options.selection=0;
        for (int i=0;i<4;++i)
            if (IsDlgButtonChecked(hwnd,1092+i)==BST_CHECKED) options.selection=i;
        options.command=command.GetCurrentStringData();
        options.inverse=inverseCommand.GetCheck()?1u:0u;
        for (int i=0;i<3;++i) {
            options.arguments[i]=arguments[i];
            options.inverse|=inverseArguments[i].GetCheck()?(2u<<i):0u;
        }
        options.mapOverborder=overborder.GetCheck();
        options.nameNotInWeapons=unknownName.GetCheck();
        EndDialog(hwnd,1);
        break;
    case IDCANCEL:
        EndDialog(hwnd,0);
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK AppAbout(HWND hwnd,UINT msg,WPARAM wParam,LPARAM)
{
    if (msg==WM_INITDIALOG) return TRUE;
    if (msg==WM_COMMAND && LOWORD(wParam)==IDOK) EndDialog(hwnd,1);
    return FALSE;
}

namespace {
int controlPresetVid[12];
int controlSavedVid=-1;
int controlRecursion;
}

INT_PTR CALLBACK AppControlPanel(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{ return static_cast<MAP_EDIT*>(Map)->DialogControlPanel(hwnd,msg,wParam,lParam); }

int MAP_EDIT::DialogControlPanel(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    DIALOG_TEXT coor(hwnd,1007);
    DIALOG_UNSIGNED vidNumber(hwnd,1008);
    DIALOG_TREE tree(hwnd,1111);
    DIALOG_TABS tabs(hwnd,1104);
    DIALOG_BUTTON sort(hwnd,2003);
    DIALOG_RADIO custom(hwnd,1092,1103);
    ++controlRecursion;
    auto mainWindow=[this]() { return reinterpret_cast<HWND>(static_cast<UINT_PTR>(m_dialog)); };
    auto rebuild=[&]() { VidToTree(&tree,spriteType,options.optTacticMode?-1:Mouse->Vid()->nVid,options.optSortVid); };
    auto checkPreset=[&]() {
        int i=0; for (;i<12;++i) if (controlPresetVid[i]==Mouse->Vid()->nVid) break;
        custom.Check(i==12?-1:i);
    };
    int index=custom.GetIndexClicked(msg,wParam);
    if (index>=0) {
        controlSavedVid=Mouse->Vid()->nVid;
        if (controlPresetVid[index]>=0) {
            if (options.optTacticMode) SendMessageA(mainWindow(),WM_COMMAND,40032,0);
            ChangeMouseVid(Vid(controlPresetVid[index]),Vid(controlPresetVid[index])->spriteType);
        }
    } else if ((index=custom.GetIndexDblClicked(msg,wParam))>=0) {
        if (controlSavedVid<0) controlSavedVid=Mouse->Vid()->nVid;
        controlPresetVid[index]=controlSavedVid;
        const STRING label=Vid(controlSavedVid)->GetNumberName(); custom.SetItem(index,&label);
        Registry->SetInt("CustomVid"+Int2Str(index),controlPresetVid[index]);
        ChangeMouseVid(Vid(controlSavedVid),Vid(controlSavedVid)->spriteType);
        controlSavedVid=-1;
    } else if (controlRecursion==1 && tree.IsSelChange(msg,wParam,lParam) && tree.IsLeaf()) {
        controlSavedVid=tree.GetCurrentData();
        if (options.optTacticMode) SendMessageA(mainWindow(),WM_COMMAND,40032,0);
        ChangeMouseVid(Vid(controlSavedVid),Vid(controlSavedVid)->spriteType);
    } else if (sort.IsClicked(msg,wParam)) {
        options.optSortVid=!options.optSortVid; rebuild();
    } else if (tabs.IsSelChange(msg,wParam,lParam)) {
        ChangeMouseVid(Mouse->Vid(),static_cast<unsigned int>(tabs.GetCurrentItemData()));
    }
    if (vidNumber.IsNotify(msg,wParam,EN_UPDATE) || vidNumber.IsNotify(msg,wParam,EN_SETFOCUS)) {
        const int nVid=static_cast<int>(static_cast<unsigned int>(vidNumber));
        if (ValidateVid(nVid)) ChangeMouseVid(Vid(nVid),Vid(nVid)->spriteType);
    }
    int result=FALSE;
    switch (msg) {
    case 24576:
        tabs.SetCurrentWithData(spriteType); rebuild(); checkPreset(); break;
    case 24577:
        if (Mouse->Vid()->treeItem) tree.SetCurrent(Mouse->Vid()->treeItem);
        checkPreset(); break;
    case WM_INITDIALOG: {
        for (int i=0;i<12;++i) {
            controlPresetVid[i]=Registry->GetInt("CustomVid"+Int2Str(i),-1);
            const STRING label=ValidateVid(controlPresetVid[i])?Vid(controlPresetVid[i])->GetNumberName():STRING("unsetted");
            custom.SetItem(i,&label);
        }
        rebuild(); AddEditorTabs(tabs);
        STRING all("All"); tabs.AddItemWithData(&all,0x61F);
        tabs.SetMinWidth(42);
        tabs.SetCurrentWithData(spriteType); sort.SetCheck(options.optSortVid);
        const VECTOR2& mouse=m_input.MouseXY();
        const STRING label=Printf("%-8i %-8i %6i+%-20i %i",TruncateFloatToInt32OrMin(mouse.x),TruncateFloatToInt32OrMin(mouse.y),
            TruncateFloatToInt32OrMin(insertZ),TruncateFloatToInt32OrMin(GetGroundZ(Mouse->Vid(),mouse,Mouse->Direction())),
            Graph->RealZBuffer(m_input.ClientMouseXY()));
        coor=&label; result=TRUE; break;
    }
    case WM_NCHITTEST: {
        RECT rect; GetWindowRect(mainWindow(),&rect);
        m_input.m_clientMouseXY=VECTOR2(
            BitsToSigned32(static_cast<unsigned int>(LOWORD(lParam))-static_cast<unsigned int>(rect.left)),
            BitsToSigned32(static_cast<unsigned int>(HIWORD(lParam))-static_cast<unsigned int>(rect.top)));
        break;
    }
    default: break;
    }
    --controlRecursion;
    return result;
}

namespace {
int convertSourceVid=5,convertTargetVid=5,convertKeepOld=1;
unsigned int convertFields[4];
int convertOffset[3];
}

INT_PTR CALLBACK AppConvertSprite(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{ return static_cast<MAP_EDIT*>(Map)->DialogConvertSprite(hwnd,msg,wParam,lParam); }
int MAP_EDIT::DialogConvertSprite(HWND hwnd,UINT msg,WPARAM wParam,LPARAM)
{
    DIALOG_COMBO_BOX source(hwnd,1057),target(hwnd,1058);
    DIALOG_BUTTON keepOld(hwnd,2001);
    DIALOG_UNSIGNED fields[4]={DIALOG_UNSIGNED(hwnd,1008),DIALOG_UNSIGNED(hwnd,1009),
        DIALOG_UNSIGNED(hwnd,1011),DIALOG_UNSIGNED(hwnd,1012)};
    DIALOG_INT offsets[3]={DIALOG_INT(hwnd,1007),DIALOG_INT(hwnd,1010),DIALOG_INT(hwnd,1013)};
    if (msg==WM_INITDIALOG) {
        VidToControlBox(&source,0x7F,convertSourceVid); VidToControlBox(&target,0x7F,convertTargetVid);
        keepOld.SetCheck(convertKeepOld);
        for (int i=0;i<4;++i) fields[i]=convertFields[i];
        for (int i=0;i<3;++i) offsets[i]=convertOffset[i];
        return TRUE;
    }
    if (msg!=WM_COMMAND) return FALSE;
    if (LOWORD(wParam)==IDCANCEL) { EndDialog(hwnd,0); return FALSE; }
    if (LOWORD(wParam)!=IDOK) return FALSE;
    convertSourceVid=source.GetCurrentStringData(); convertTargetVid=target.GetCurrentStringData();
    convertKeepOld=keepOld.GetCheck();
    for (int i=0;i<4;++i) convertFields[i]=fields[i];
    for (int i=0;i<3;++i) convertOffset[i]=offsets[i];
    const VECTOR delta(static_cast<float>(convertOffset[0]),static_cast<float>(convertOffset[1]),static_cast<float>(convertOffset[2]));
    int index;
    for (SPRITE* sprite=Sprites->FirstDrawingSprite(&index,Vid(convertSourceVid)->layer);sprite;
         sprite=Sprites->NextDrawingSprite(&index,Vid(convertSourceVid)->layer)) {
        if (sprite->Vid()->nVid!=convertSourceVid) continue;
        const ANGLE direction=sprite->Direction(); const VECTOR position(sprite->Coor());
        const VECTOR scale(sprite->GetScale());
        if (!convertKeepOld) delete sprite;
        SPRITE* replacement=CreateSprite(Vid(convertTargetVid),VECTOR(position.x+delta.x,position.y+delta.y,position.z+delta.z),direction,0);
        if (replacement) replacement->SetScale(scale);
    }
    EndDialog(hwnd,1); return FALSE;
}


namespace {
VID* editedVid = 0;
int editedVidHistory[1024];
int editedVidHistoryCount = 0;

struct VID_EDITOR_ITEM
{
    int control;
    const char* text;
    unsigned int data;
};
const VID_EDITOR_ITEM vidEditorItems[] = {
    {1057, "U_TERRAIN", 0x2u},
    {1057, "U_OBJECT", 0x4u},
    {1057, "U_UNIT", 0x8u},
    {1057, "U_AVIA", 0x10u},
    {1057, "U_MENU", 0x20u},
    {1057, "U_RAILWAY", 0x40u},
    {1057, "U_REGION", 0x80u},
    {1057, "U_CANNON", 0x200u},
    {1057, "U_SPRITE", 0x400u},
    {1058, "B_TERRAIN        ", 0x0u},
    {1058, "B_OBJECT         ", 0x1u},
    {1058, "B_UNIT           ", 0x2u},
    {1058, "B_BUILDING       ", 0x3u},
    {1058, "B_AVIA           ", 0x4u},
    {1058, "B_CANNON         ", 0x5u},
    {1058, "B_PRIMITIVE      ", 0x6u},
    {1058, "B_MAN            ", 0x7u},
    {1058, "B_BUILDED_TERRAIN", 0x8u},
    {1058, "B_SPRITE         ", 0x9u},
    {1058, "B_FRAME          ", 0xAu},
    {1058, "B_LINKER         ", 0xCu},
    {1058, "B_TEXT           ", 0x13u},
    {1058, "B_CIV_ROBOT      ", 0x14u},
    {1058, "B_ENGINE         ", 0x15u},
    {1058, "B_RAIL           ", 0x16u},
    {1058, "B_REGION         ", 0x17u},
    {1058, "B_DEPO           ", 0x18u},
    {1058, "B_CREATURE       ", 0x19u},
    {1058, "B_BALLOON        ", 0x1Au},
    {1058, "B_MISSILE        ", 0x1Bu},
    {1059, "P_RandBirth          ", 0x1u},
    {1059, "P_Gravity            ", 0x2u},
    {1059, "P_Gravity2           ", 0x4u},
    {1059, "P_BuildSizeToGridZ   ", 0x8u},
    {1059, "P_Track              ", 0x10u},
    {1059, "P_BuildVidZToGridZ   ", 0x20u},
    {1059, "P_Hash               ", 0x40u},
    {1059, "P_BirthAsSmoke       ", 0x80u},
    {1059, "P_Noise              ", 0x100u},
    {1059, "P_ZeroZ              ", 0x200u},
    {1059, "P_RandSpeed          ", 0x400u},
    {1059, "P_Gamma              ", 0x800u},
    {1059, "P_Wind               ", 0x1000u},
    {1059, "P_SkipMapEd          ", 0x2000u},
    {1059, "P_Crush              ", 0x4000u},
    {1059, "P_AlwaysTop          ", 0x8000u},
    {1059, "P_Wave               ", 0x10000u},
    {1059, "P_InvisibleForEnemy  ", 0x20000u},
    {1059, "P_CreateChildEnd     ", 0x40000u},
    {1059, "P_VertDir            ", 0x80000u},
    {1059, "P_MoveWithAnyDirection", 0x100000u},
    {1059, "P_Blur               ", 0x200000u},
    {1059, "P_RandZSpeed         ", 0x400000u},
    {1059, "P_DblLight           ", 0x800000u},
    {1059, "P_OnePhase           ", 0x1000000u},
    {1059, "P_NotChangeLinkerCoor", 0x2000000u},
    {1059, "P_RadialDamage       ", 0x4000000u},
    {1059, "P_SelfMoving         ", 0x8000000u},
    {1059, "P_Bounce             ", 0x10000000u},
    {1059, "P_HardwareDirect     ", 0x20000000u},
    {1059, "P_Ground             ", 0x40000000u},
    {1059, "P_NotDamageForFriend ", 0x80000000u},
};

float VidEditorDisplayRate(float value, float factor)
{
    return value == 999999.0f ? value : value * factor;
}
float VidEditorReadRate(float value, float factor)
{
    return value == 999999.0f ? value : value / factor;
}
int FindControlBoxData(DIALOG_COMBO_BOX& combo,int value)
{
    for (int index=combo.NoString()-1;index>=0;--index)
        if (combo.GetData(index)==value) return index;
    return -1;
}
void VidEditorCaption(HWND hwnd)
{
    EnableWindow(GetDlgItem(hwnd,1098),(editedVid->runtimeFlags & 0x40u) != 0u);
    const STRING caption=Printf((editedVid->runtimeFlags & 0x40u) ?
        "Vid[%i] *EDITED*" : "Vid[%i]",editedVid->nVid);
    SetWindowTextA(hwnd,caption.CharPtr());
}
}

INT_PTR CALLBACK AppVidEditor(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    return static_cast<MAP_EDIT*>(Map)->DialogVidEditor(hwnd,msg,wParam,lParam);
}

int MAP_EDIT::DialogVidEditor(HWND hwnd,UINT msg,WPARAM wParam,LPARAM)
{
    DIALOG_COMBO_BOX spriteTypeBox(hwnd,1057),spriteClassBox(hwnd,1058),linkVidBox(hwnd,1060);
    DIALOG_FLOAT sizeX(hwnd,1009),sizeY(hwnd,1010),sizeZ(hwnd,1011);
    DIALOG_FLOAT speed(hwnd,1013),zSpeed(hwnd,1014),acceleration(hwnd,1015),slow(hwnd,1016);
    DIALOG_FLOAT deathRange(hwnd,1077),linkX(hwnd,1079),linkY(hwnd,1080),linkZ(hwnd,1081);
    DIALOG_FLOAT topZ(hwnd,1082),upZ(hwnd,1083),downZ(hwnd,1084);

    bool displayVid=false;
    if (msg==WM_COMMAND)
    {
        if (LOWORD(wParam)==1104 && HIWORD(wParam)==BN_CLICKED)
        {
            editedVid=Vid(editedVidHistory[--editedVidHistoryCount]);
            displayVid=true;
        }
        else if (LOWORD(wParam)==1105 && HIWORD(wParam)==BN_CLICKED)
        {
            editedVidHistory[editedVidHistoryCount++]=editedVid->nVid;
            editedVid=editedVid->linkVid;
            displayVid=true;
        }
        else if (LOWORD(wParam)==1098 && HIWORD(wParam)==BN_CLICKED)
        {
            editedVid->runtimeFlags &= ~0x40u;
            VidEditorCaption(hwnd);
        }
    }
    if (msg==WM_INITDIALOG)
    {
        editedVidHistoryCount=0;
        editedVid=Mouse ? Mouse->Vid() : 0;
        displayVid=true;
    }
    if (displayVid)
    {
        VidEditorCaption(hwnd);
        EnableWindow(GetDlgItem(hwnd,1104),editedVidHistoryCount);
        EnableWindow(GetDlgItem(hwnd,1105),editedVid->linkVid != 0);
        SetDlgItemTextA(hwnd,1007,editedVid->name.CharPtr());
        for (int control=1057;control<=1059;++control)
        {
            DIALOG_COMBO_BOX combo(hwnd,control);
            for (const VID_EDITOR_ITEM& item:vidEditorItems)
                if (item.control==control)
                {
                    const STRING text(item.text);
                    combo.AddStringWithData(&text,static_cast<int>(item.data));
                }
            if (control==1057) combo.SetCurrent(FindControlBoxData(combo,editedVid->spriteType));
            else if (control==1058) combo.SetCurrent(FindControlBoxData(combo,editedVid->spriteClass));
        }
        SetDlgItemInt(hwnd,1008,editedVid->moveMask,TRUE);
        sizeX=editedVid->sizeXYZ.x; sizeY=editedVid->sizeXYZ.y; sizeZ=editedVid->sizeXYZ.z;
        SetDlgItemInt(hwnd,1012,editedVid->maxHp,FALSE);
        speed=VidEditorDisplayRate(editedVid->maxSpeed,1000.0f);
        zSpeed=VidEditorDisplayRate(editedVid->maxZSpeed,1000.0f);
        acceleration=VidEditorDisplayRate(editedVid->acceleration,1000000.0f);
        slow=VidEditorDisplayRate(editedVid->slow,1000000.0f);
        const float rotation=editedVid->rotationSpeed;
        const float period=rotation==999999.0f ? 0.0f : rotation==0.0f ? 999999.0f : 256.0f/rotation;
        SetDlgItemInt(hwnd,1071,static_cast<unsigned int>(TruncateFloatToInt32(period)),FALSE);
        SetDlgItemInt(hwnd,1076,editedVid->nWeapon,FALSE);
        deathRange=editedVid->deathRange;
        SetDlgItemInt(hwnd,1078,editedVid->deathDamageMin,TRUE);
        linkX=editedVid->linkXYZ.x; linkY=editedVid->linkXYZ.y; linkZ=editedVid->linkXYZ.z;
        VidToControlBox(&linkVidBox,static_cast<unsigned int>(-1),-2);
        const STRING none("None");
        linkVidBox.AddStringWithData(&none,-1);
        linkVidBox.SetCurrent(FindControlBoxData(linkVidBox,editedVid->linkVid ? editedVid->linkVid->nVid : -1));
        topZ=editedVid->topZ; upZ=editedVid->forMoveUpZ; downZ=editedVid->forMoveDownZ;
        SetDlgItemInt(hwnd,1085,editedVid->lifeTime,FALSE);
        SetDlgItemInt(hwnd,1086,editedVid->noDir,FALSE);
        return 1;
    }
    if (msg!=WM_COMMAND) return 0;
    switch (LOWORD(wParam))
    {
    case IDOK:
    {
        editedVid->runtimeFlags |= 0x40u;
        editedVid->spriteType=static_cast<unsigned int>(spriteTypeBox.GetCurrentStringData());
        editedVid->spriteClass=static_cast<unsigned int>(spriteClassBox.GetCurrentStringData());
        editedVid->property=0;
        editedVid->moveMask=GetDlgItemInt(hwnd,1008,0,TRUE);
        editedVid->sizeXYZ=VECTOR(static_cast<float>(sizeX),static_cast<float>(sizeY),static_cast<float>(sizeZ));
        editedVid->maxHp=static_cast<int>(GetDlgItemInt(hwnd,1012,0,FALSE));
        editedVid->maxSpeed=VidEditorReadRate(static_cast<float>(speed),1000.0f);
        editedVid->maxZSpeed=VidEditorReadRate(static_cast<float>(zSpeed),1000.0f);
        editedVid->acceleration=VidEditorReadRate(static_cast<float>(acceleration),1000000.0f);
        editedVid->slow=VidEditorReadRate(static_cast<float>(slow),1000000.0f);
        const unsigned int period=GetDlgItemInt(hwnd,1071,0,FALSE);
        editedVid->rotationSpeed=period==999999u ? 0.0f : period==0u ? 999999.0f : 256.0f/static_cast<float>(period);
        editedVid->deathRange=static_cast<float>(deathRange);
        editedVid->deathDamageMin=static_cast<int>(GetDlgItemInt(hwnd,1078,0,TRUE));
        editedVid->linkXYZ=VECTOR(static_cast<float>(linkX),static_cast<float>(linkY),static_cast<float>(linkZ));
        editedVid->linkVid=Vid(linkVidBox.GetCurrentStringData());
        editedVid->topZ=static_cast<float>(topZ);
        editedVid->forMoveUpZ=static_cast<float>(upZ);
        editedVid->forMoveDownZ=static_cast<float>(downZ);
        editedVid->lifeTime=static_cast<int>(GetDlgItemInt(hwnd,1085,0,FALSE));
        editedVid->noDir=static_cast<int>(GetDlgItemInt(hwnd,1086,0,FALSE));
        int index;
        for (SPRITE* sprite=::Sprites->FirstDrawingSprite(&index,editedVid->layer);
             sprite;sprite=::Sprites->NextDrawingSprite(&index,editedVid->layer))
            if (sprite->Vid()==editedVid && sprite->ExtraData())
                sprite->ExtraData()->maxSpeedOverride=editedVid->maxSpeed;
        EndDialog(hwnd,1);
        break;
    }
    case IDCANCEL: EndDialog(hwnd,0); break;
    }
    return 0;
}
