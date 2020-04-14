/*
 * ui_menu_internal.c
 *
 *  Created on: 24.12.2016
 *      Author: danilo
 */

#include "ui_menu.h"
#include "ui_menu_internal.h"
#include "ui_lcd_hy28.h" // for colors!
#include <stdio.h>

// we show MENUSIZE items at the same time to the user.
// right now the render code uses this global variable since
// only a single active menu is supported right now.

//Because of dynamic resolution change we have to declare the maximum amount of items displayed at once for the highest option
MenuDisplaySlot menu[MAX_MENUSIZE];

bool init_done = false;


// actions [this is an internal, not necessarily complete or accurate sketch of the used algorithms /API
// read the source to find out how it is done. Left for the purpose of explaining the basic idea
// show menu -> was previously displayed -> yes -> simply display all slots
//                                       -> no  -> get first menu group, get first entry, fill first slot, run get next entry until all slots are filled

// find next MenuEntry -> is menu group -> yes -> is unfolded -> yes -> get first item from menu group
//                                                            -> no  -> treat as normal menu entry
//                                      -> is there one more entry in my menu group ? -> yes -> takes this one
//                                                                                      -> no  -> go one level up -> get next entry in this group
//                     -> there is no such entry -> fill with dummy entry

// find prev MenuEntry -> there is prev entry in menu group -> yes -> is this unfolded menu group -> yes -> get last entry of this menu group
//                                                                                                -> no  -> fill slot with entry
//                                                          -> no  -> go one level up -> get prev entry of this level

// unfold menu group -> mark as unfold, run get next entry until all menu display slots are filled
// fold menu group   -> mark as fold,   run get next entry until all menu display slots are filled

// move to next/previous page -> (this is n times prev/next)

// ===================== BEGIN MENU LOW LEVEL MANAGEMENT =====================
const MenuGroupDescriptor* UiMenu_GetParentGroupForEntry(const MenuDescriptor* me)
{
    return me==NULL?NULL:&groups[me->menuId];
}

const MenuGroupDescriptor* UiMenu_GetGroupForGroupEntry(const MenuDescriptor* me)
{
    return me==NULL?NULL:&groups[me->number];
}

bool UiMenu_IsEnabled(const MenuDescriptor *entry)
{
    return entry==NULL?false:(entry->enabled != NULL?*entry->enabled:true);
}


inline bool UiMenu_IsGroup(const MenuDescriptor *entry)
{
    return entry==NULL?false:entry->kind == MENU_GROUP;
}
inline bool UiMenu_IsItem(const MenuDescriptor *entry)
{
    return entry==NULL?false:entry->kind == MENU_ITEM;
}
inline bool UiMenu_IsInfo(const MenuDescriptor *entry)
{
    return entry==NULL?false:entry->kind == MENU_INFO;
}


inline bool UiMenu_SlotIsEmpty(MenuDisplaySlot* slot)
{
    return slot==NULL?false:slot->entryItem == NULL;
}
inline bool UiMenu_GroupIsUnfolded(const MenuDescriptor *group)
{
    return group==NULL?false:groups[group->number].state->unfolded;
}
inline uint16_t UiMenu_MenuGroupMemberCount(const MenuGroupDescriptor* gd)
{
    uint16_t retval = 0;
    if (gd != NULL)
    {
        if (gd->state->count == 0)
        {
            const MenuDescriptor* entry;
            for (entry = gd->entries; entry->kind != MENU_STOP; entry++)
            {
                gd->state->count++;
            }
        }
        retval = gd->state->count;
    }
    return retval;
}

inline void UiMenu_GroupFold(const MenuDescriptor* entry, bool fold)
{
    if (UiMenu_IsGroup(entry))
    {
        groups[entry->number].state->unfolded = fold == false;
    }
}

inline const MenuDescriptor* UiMenu_GroupGetLast(const MenuGroupDescriptor *group)
{
    const MenuDescriptor* retval = NULL;
    uint16_t count = UiMenu_MenuGroupMemberCount(group);
    if (count>0)
    {
        retval = &(group->entries[count-1]);
    }
    return retval;
}

inline const MenuDescriptor* UiMenu_GroupGetFirst(const MenuGroupDescriptor *group)
{
    const MenuDescriptor* retval = NULL;
    uint16_t count = UiMenu_MenuGroupMemberCount(group);
    if (count>0)
    {
        retval = group->entries;
    }
    return retval;
}

const MenuDescriptor* UiMenu_GetNextEntryInGroup(const MenuDescriptor* me)
{
    const MenuDescriptor* retval = NULL;

    if (me != NULL)
    {
        const MenuGroupDescriptor* group_ptr = UiMenu_GetParentGroupForEntry(me);
        for (const MenuDescriptor* nxt = me + 1; UiMenu_GroupGetLast(group_ptr)>= nxt; nxt++)
        {
            if (UiMenu_IsEnabled(nxt))
            {
                retval = nxt;
                break;
            }
        }
    }
    return retval;
}



const MenuDescriptor* UiMenu_GetPrevEntryInGroup(const MenuDescriptor* me)
{
    const MenuGroupDescriptor* group_ptr = UiMenu_GetParentGroupForEntry(me);
    const MenuDescriptor* retval = NULL;
    if (me != NULL && me != &group_ptr->entries[0])
    {
        for (const MenuDescriptor* prv = me - 1; prv >= &group_ptr->entries[0]; prv--)
        {
            if (UiMenu_IsEnabled(prv))
            {
                retval = prv;
                break;
            }
        }
    }
    return retval;
}



const MenuDescriptor* UiMenu_GetParentForEntry(const MenuDescriptor* me)
{
    const MenuDescriptor* retval = NULL;
    if (me != NULL)
    {
        const MenuGroupDescriptor* gd = UiMenu_GetParentGroupForEntry(me);
        if (gd->parent != NULL)
        {
            if (gd->state->me == NULL )
            {
                const MenuGroupDescriptor* gdp = &groups[gd->parent->menuId];
                uint16_t count = UiMenu_MenuGroupMemberCount(gdp);
                uint16_t idx;
                for(idx = 0; idx < count; idx++)
                {
                    if ((gdp->entries[idx].kind == MENU_GROUP) && (gdp->entries[idx].number == me->menuId))
                    {
                        gd->state->me = &gdp->entries[idx];
                        break;
                    }
                }
            }
            retval = gd->state->me;
        }
    }
    return retval;
}

/*
 * @returns true if this is the last ACTIVE entry in the menu group
 */
inline bool UiMenu_IsLastActiveItemInMenuGroup(const MenuDescriptor* here)
{
    const MenuGroupDescriptor* gd = UiMenu_GetParentGroupForEntry(here);
    const MenuDescriptor* lastActive = UiMenu_GroupGetLast(gd);

    for (; lastActive != NULL && UiMenu_IsEnabled(lastActive) == false && lastActive != here; lastActive = UiMenu_GetPrevEntryInGroup(lastActive)  )
    {
    	// we need to do nothing here
    }
    return lastActive == here;
}
inline bool UiMenu_IsFirstInMenuGroup(const MenuDescriptor* here)
{
    const MenuGroupDescriptor* gd = UiMenu_GetParentGroupForEntry(here);
    return UiMenu_GroupGetFirst(gd) == here;
}


// ===================== END MENU LOW LEVEL MANAGEMENT =====================


// ===================== BEGIN MENU ITERATION STRATEGY =====================
// this code implements a specific strategy to walk through a menu structure

// Helper Functions
const MenuDescriptor* UiMenu_FindNextEntryInUpperLevel(const MenuDescriptor* here)
{
    const MenuDescriptor* next = NULL, *focus = here;
    if (here != NULL)
    {
        while(focus != NULL && next == NULL)
        {
            // we have a parent group, we are member of a sub menu group,
            // we need next entry in containing menu group, no matter if our menu group is folded or not
            next = UiMenu_GetNextEntryInGroup(UiMenu_GetParentForEntry(focus));
            if (next == NULL)
            {
                focus = UiMenu_GetParentForEntry(focus);
            }
        }
    }
    return next;
}

const MenuDescriptor* UiMenu_FindLastEntryInLowerLevel(const MenuDescriptor* here)
{
    const MenuDescriptor *last = here;
    while (UiMenu_IsGroup(here) && UiMenu_GroupIsUnfolded(here) && here == last)
    {
        const MenuDescriptor* last = UiMenu_GroupGetLast(UiMenu_GetGroupForGroupEntry(here));
        if (last)
        {
            here = last;
        }
    }
    return here;
}


// Main Strategy  Functions
/*
 * Strategy: Provide a 'virtual' flat list of menu entries, list members are dynamically inserted/removed if menu groups are (un)folded.
 * External code navigates through only with next/prev operations.
 *
 */
/*
 * @brief Get next menu entry. If a menu group is unfolded, next entry after menu group item is first item from menu group
 *
 */
const MenuDescriptor* UiMenu_NextMenuEntry(const MenuDescriptor* here)
{
    const MenuDescriptor* next = NULL;

    if (here != NULL)
    {
        if (UiMenu_IsGroup(here))
        {
            // is group entry

            if (UiMenu_GroupIsUnfolded(here))
            {
                const MenuGroupDescriptor* group = &groups[here->number];
                next = UiMenu_GroupGetFirst(group);
                if (next == NULL)
                {
                    // this is an empty menu group, should not happen, does make  sense
                    // but we handle this anyway
                    next = UiMenu_FindNextEntryInUpperLevel(here);
                }
            }
            else
            {
                // folded group, so we behave  like a normal entry
                next = UiMenu_GetNextEntryInGroup(here);
            }
        }
        if (next == NULL)
        {
            // we are currently at a normal entry or a folded group or empty group (in this case these are treated as simple entries)
            // only 3 cases possible:
            //   - final entry of menu, fill next slot with blank entry, return false
            //   - next entry is normal entry (group or entry, no difference), just use this one
            //   - last entry in menu group, go up, and search for next entry in this parent menu (recursively).

            if (UiMenu_IsLastActiveItemInMenuGroup(here))
            {
                // we need the parent menu in order to ask for the  entry after our
                // menu group entry
                // if we cannot find the parent group, we  are top level and the last menu entry
                // so there is no further entry
                next = UiMenu_FindNextEntryInUpperLevel(here);
            }
            else
            {
                next =  UiMenu_GetNextEntryInGroup(here);
            }
        }
    }
    return next;
}


/*
 * @brief Get previous menu entry. If on first item of a menu group, show the last entry of the previous menu group/menu item
 *
 */
const MenuDescriptor* UiMenu_PrevMenuEntry(const MenuDescriptor* here)
{
    const MenuDescriptor* prev = NULL;


    if (here != NULL)
    {
        if (UiMenu_IsFirstInMenuGroup(here))
        {
            // we go up, get previous entry
            // if first entry,  go one further level up, ...
            //  if not first entry -> get prev entry
            //     if normal entry or folded menu -> we are done
            //     if unfolded menu_entry -> go to last entry
            //           -> if normal entry or folded menu -> we are done
            //           -> if unfolded menu entry -> go to last entry
            prev = UiMenu_GetParentForEntry(here);
        }
        else
        {
            prev = UiMenu_GetPrevEntryInGroup(here);
            if (UiMenu_IsGroup(prev) && UiMenu_GroupIsUnfolded(prev))
            {
                prev = UiMenu_FindLastEntryInLowerLevel(prev);
            }
        }
    }
    return prev;
}



bool UiMenu_FillSlotWithEntry(MenuDisplaySlot* here, const MenuDescriptor* entry)
{
    bool retval = false;
    if (entry != NULL)
    {
        here->entryItem = entry;
        retval = true;
    }
    else
    {
        here->entryItem = NULL;
    }
    return retval;
}

// DISPLAY SPECIFIC CODE BEGIN
void UiMenu_DisplayValue(const char* value,uint32_t clr,uint16_t pos)
{
    UiLcdHy28_PrintTextRight(ts.Layout->MENU_CURSOR_X - 4, ts.Layout->MENU_IND.y + (pos * 12), value, clr, Black, 0);       // yes, normal position
}
static void UiMenu_DisplayLabel(const char* label,uint32_t clr,uint16_t pos)
{
    UiLcdHy28_PrintText(ts.Layout->MENU_IND.x, ts.Layout->MENU_IND.y + (12*(pos)),label,clr,Black,0);
}
static void UiMenu_DisplayCursor(const char* label,uint32_t clr,uint16_t pos)
{
    UiLcdHy28_PrintText(ts.Layout->MENU_CURSOR_X, ts.Layout->MENU_IND.y + (12*(pos)),label,clr,Black,0);
}
// DISPLAY SPECIFIC CODE END


void UiMenu_MoveCursor(uint32_t newpos)
{
    static uint32_t oldpos = 999;  // y position of option cursor, previous
    if(oldpos != 999)         // was the position of a previous cursor stored?
    {
        UiMenu_DisplayCursor(" ", Green, oldpos);
    }
    oldpos = newpos;   // save position of new "old" cursor position
    if (newpos != 999)
    {
        UiMenu_DisplayCursor("<", Green, newpos);
    }
}


static void UiMenu_UpdateHWInfoLines(uchar index, MenuProcessingMode_t mode, int pos)
{
    uint32_t m_clr;
    const char* outs = UiMenu_GetSystemInfo(&m_clr, index);
    UiMenu_DisplayValue(outs,m_clr,pos);
}

/**
 * @brief Display and and change line items
 * @param select item to display/change
 * @param mode 0=display/update 1=change item 3=set default
 * @param pos (0-5) use this line as position
 */
static void UiMenu_UpdateLines(uint16_t select, MenuProcessingMode_t mode, int pos)
{
    char options[32];
    const char* txt_ptr = NULL; // if filled, we use this string for display, otherwise options
    uint32_t clr = White;       // color used it display of adjusted options

    int var; // holder for the menu item value change


    if(mode == MENU_RENDER_ONLY)        // are we in update/display mode?
    {
        var = 0;        // prevent any change of variable
    }
    else                // this is "change" mode
    {
        var = ts.menu_var;      // change from encoder
        ts.menu_var = 0;        // clear encoder change detect
    }

    strcpy(options, "ERROR");   // pre-load to catch error condition

    UiMenu_UpdateItem(select, mode, pos, var, options,&txt_ptr , &clr);

    if (txt_ptr == NULL)
    {
        txt_ptr = options;
    }
    UiMenu_DisplayValue(txt_ptr,clr,pos);
    if(mode == MENU_PROCESS_VALUE_CHANGE)       // Shifted over
    {
        UiMenu_MoveCursor(pos);
    }
    return;
}

/*
 * Render a menu entry on a given menu position
 */
void UiMenu_UpdateMenuEntry(const MenuDescriptor* entry, MenuProcessingMode_t mode, uint8_t pos)
{
    uint32_t  m_clr;
    m_clr = Yellow;
    char out[40];

    char blank[40] = "                                     ";
    blank[ts.Layout->MENU_TEXT_SIZE_MAX-1]=0;
//#else
//    const char blank[34] = "                               ";
//#endif

    if (entry != NULL && (entry->kind == MENU_ITEM || entry->kind == MENU_GROUP || entry->kind == MENU_INFO || entry->kind == MENU_TEXT) )
    {
        if (mode == MENU_RENDER_ONLY)
        {
            uint16_t level = 0;
            const MenuDescriptor* parent = entry;
            do
            {
                parent = UiMenu_GetParentForEntry(parent);
                level++;
            }
            while (parent != NULL);
            level--;

            // level = 3;
            // uint16_t labellen = strlen(entry->id)+strlen(entry->label) + 1;
            uint16_t labellen = level+strlen(entry->label);
            // snprintf(out,34,"%s-%s%s",entry->id,entry->label,(&blank[labellen>33?33:labellen]));
//#ifdef USE_DISP_480_320
            //snprintf(out,ts.40,"%s%s%s",(&blank[level>5?37-5:37-level]),entry->label,(&blank[labellen>39?39:labellen]));
            snprintf(out,ts.Layout->MENU_TEXT_SIZE_MAX,"%s%s%s",
            		(&blank[level>5?ts.Layout->MENU_TEXT_SIZE_MAX-8:ts.Layout->MENU_TEXT_SIZE_MAX-3-level]),
					entry->label,(&blank[labellen>ts.Layout->MENU_TEXT_SIZE_MAX-1?ts.Layout->MENU_TEXT_SIZE_MAX-1:labellen]));

  //         snprintf(out,34,"%s%s%s",(&blank[level>5?31-5:31-level]),entry->label,(&blank[labellen>33?33:labellen]));

            UiMenu_DisplayLabel(out,m_clr,pos);
        }
        switch(entry->kind)
        {
        case MENU_ITEM:
            // TODO: Better Handler Selection with need for change in this location to add new handlers
            UiMenu_UpdateLines(entry->number,mode,pos);
            break;
        case MENU_INFO:
            UiMenu_UpdateHWInfoLines(entry->number,mode,pos);
            break;
        case MENU_TEXT:
            break;
        case MENU_GROUP:
            if (mode == MENU_PROCESS_VALUE_CHANGE)
            {
                bool old_state = UiMenu_GroupIsUnfolded(entry);
                if (ts.menu_var < 0 )
                {
                	UiMenu_GroupFold(entry, !(ts.flags2 & FLAGS2_UI_INVERSE_SCROLLING));
                }
                if (ts.menu_var > 0 )
                {
                	UiMenu_GroupFold(entry, ts.flags2 & FLAGS2_UI_INVERSE_SCROLLING);
                }
                if (old_state != UiMenu_GroupIsUnfolded(entry))
                {
                    int idx;
                    for (idx = pos+1; idx < ts.Layout->MENUSIZE; idx++)
                    {
                        UiMenu_FillSlotWithEntry(&menu[idx],UiMenu_NextMenuEntry(menu[idx-1].entryItem));
                        UiMenu_UpdateMenuEntry(menu[idx].entryItem, MENU_RENDER_ONLY, idx);
                    }
                }
                ts.menu_var = 0;
            }

            UiMenu_DisplayValue(
                    UiMenu_GroupIsUnfolded(entry)?"HIDE":"SHOW",
                    m_clr,pos);
            break;
        }
    }
    else
    {
        UiMenu_DisplayLabel(blank,m_clr,pos);
    }
    if (mode == MENU_PROCESS_VALUE_CHANGE)
    {
        UiMenu_MoveCursor(pos);
    }
}

void UiMenu_DisplayInitSlots(const MenuDescriptor* entry)
{
    int idx;
    for (idx=0; idx < ts.Layout->MENUSIZE; idx++)
    {
        UiMenu_FillSlotWithEntry(&menu[idx],entry);
        entry = UiMenu_NextMenuEntry(entry);
    }
}
void UiMenu_DisplayInitSlotsBackwards(const MenuDescriptor* entry)
{
    int idx;
    for (idx=ts.Layout->MENUSIZE; idx > 0; idx--)
    {
        UiMenu_FillSlotWithEntry(&menu[idx-1],entry);
        entry = UiMenu_PrevMenuEntry(entry);
    }
}

/*
 * @returns true if at least one slot was moved, false if no change done
 */
bool UiMenu_DisplayMoveSlotsBackwards(int16_t change)
{
    int idx;
    int dist = (change % ts.Layout->MENUSIZE);
    int screens = change / ts.Layout->MENUSIZE;
    bool retval = false; // n
    for (idx = 0; idx < screens; idx++)
    {
        const MenuDescriptor *prev = UiMenu_PrevMenuEntry(menu[0].entryItem);
        if (prev != NULL)
        {
            retval = true;
            UiMenu_DisplayInitSlotsBackwards(prev);
        }
        else
        {
            // we stop here, since no more previous elements.
            // TODO: Decide if roll over, in this case we would have to get very last element and
            // then continue from there.
            dist = 0;
            break;
        }
    }

    if (dist != 0)
    {
        retval = true;
        for (idx = ts.Layout->MENUSIZE-dist; idx > 0; idx--)
        {
            UiMenu_FillSlotWithEntry(&menu[ts.Layout->MENUSIZE-idx],menu[ts.Layout->MENUSIZE-(dist+idx)].entryItem);
        }

        for (idx = ts.Layout->MENUSIZE-dist; idx >0; idx--)
        {
            UiMenu_FillSlotWithEntry(&menu[idx-1],UiMenu_PrevMenuEntry(menu[idx].entryItem));
        }
    }
    return retval;
}
/*
 * @returns true if at least one slot was moved, false if no change done
 */
bool UiMenu_DisplayMoveSlotsForward(int16_t change)
{
    int idx;
    int dist = (change % ts.Layout->MENUSIZE);
    int screens = change / ts.Layout->MENUSIZE;
    bool retval = false;
    // first jump screens. we have to iterate through the menu structure one by one
    // in order to respect fold/unfold state etc.
    for (idx = 0; idx < screens; idx++)
    {
        const MenuDescriptor *next = UiMenu_NextMenuEntry(menu[ts.Layout->MENUSIZE-1].entryItem);
        if (next != NULL)
        {
            UiMenu_DisplayInitSlots(next);
            retval = true;
        }
        else
        {
            // stop here
            // TODO: Rollover?
            dist = 0;
            break;
        }
    }
    if (dist != 0)
    {
        retval = true;
        for (idx = 0; idx < ts.Layout->MENUSIZE-dist; idx++)
        {
            UiMenu_FillSlotWithEntry(&menu[idx],menu[dist+idx].entryItem);
        }
        for (idx = ts.Layout->MENUSIZE-dist; idx < ts.Layout->MENUSIZE; idx++)
        {
            UiMenu_FillSlotWithEntry(&menu[idx],UiMenu_NextMenuEntry(menu[idx-1].entryItem));
        }
    }
    return retval;
}


/*
 * @brief Display and change menu items
 * @param mode   0=show all, 1=update current item, 3=restore default setting for selected item
 *
 */
void UiMenu_RenderMenu(MenuProcessingMode_t mode)
{
    if (init_done == false )
    {
        UiMenu_DisplayInitSlots(groups[MENU_START_IDX].entries);
        init_done = true;
    }
    // UiMenu_DisplayMoveSlotsForward(6);
    // UiMenu_DisplayMoveSlotsForward(3);
    // UiMenu_DisplayMoveSlotsBackwards(10);
    switch (mode)
    {
    case MENU_RENDER_ONLY:  // (re)draw all labels and values
    {
        int idx;
        for (idx = 0; idx < ts.Layout->MENUSIZE; idx++)
        {
            UiMenu_UpdateMenuEntry(menu[idx].entryItem,mode, idx);
        }
        UiMenu_MoveCursor(ts.menu_item%ts.Layout->MENUSIZE);		//redraw of cursor
    }
    break;

    case MENU_PROCESS_VALUE_SETDEFAULT:
    case MENU_PROCESS_VALUE_CHANGE:
    {
        // wrapping to next screen (and from end to start and vice versa)
        if (ts.menu_item >= ts.Layout->MENUSIZE)
        {
            if (UiMenu_RenderNextScreen() == false)
            {
                UiMenu_RenderFirstScreen();
            }
        }
        else if (ts.menu_item < 0)
        {
            if (UiMenu_RenderPrevScreen() == false)
            {
                UiMenu_RenderLastScreen();
            }

        }

        ts.menu_item%=ts.Layout->MENUSIZE;
        if (ts.menu_item < 0) ts.menu_item+=ts.Layout->MENUSIZE;

        uint16_t current_item = ts.menu_item%ts.Layout->MENUSIZE;
        UiMenu_UpdateMenuEntry(menu[current_item].entryItem,mode, current_item);
    }
    break;
    default:
        break;
    }
}

void UiMenu_RenderChangeItemValue(int16_t pot_diff)
{
    if(pot_diff < 0)
    {
        ts.menu_var--;      // increment selected item
    }
    else
    {
        ts.menu_var++;      // decrement selected item
    }
    UiMenu_RenderMenu(MENU_PROCESS_VALUE_CHANGE);        // perform update of selected item
}

void UiMenu_RenderChangeItem(int16_t pot_diff)
{
	if(ts.flags2 & FLAGS2_UI_INVERSE_SCROLLING)
	{
		pot_diff = -pot_diff;
	}
    if(pot_diff < 0)
    {
    	ts.menu_item--;
    }
    else  if(pot_diff > 0)
    {
    	ts.menu_item++;
    }
    ts.menu_var = 0;            // clear variable that is used to change a menu item
    UiMenu_RenderMenu(MENU_PROCESS_VALUE_CHANGE);      // Update that menu item
}

void UiMenu_RenderLastScreen()
{
    while (menu[ts.Layout->MENUSIZE-1].entryItem != NULL && UiMenu_NextMenuEntry(menu[ts.Layout->MENUSIZE-1].entryItem) != NULL )
    {
        UiMenu_DisplayMoveSlotsForward(ts.Layout->MENUSIZE);
    }
    UiMenu_RenderMenu(MENU_RENDER_ONLY);
}

void UiMenu_RenderFirstScreen()
{
    init_done = false;
    UiMenu_RenderMenu(MENU_RENDER_ONLY);
}

bool UiMenu_RenderNextScreen()
{
    bool retval = UiMenu_DisplayMoveSlotsForward(ts.Layout->MENUSIZE);
    if (retval)
    {
        UiMenu_RenderMenu(MENU_RENDER_ONLY);
    }
    return retval;
}

bool UiMenu_RenderPrevScreen()
{
    bool retval = UiMenu_DisplayMoveSlotsBackwards(ts.Layout->MENUSIZE);
    if (retval)
    {
        UiMenu_RenderMenu(MENU_RENDER_ONLY);
    }
    return retval;
}
