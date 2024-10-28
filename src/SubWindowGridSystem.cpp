#include "SubWindowGridSystem.h"

#define GREY {0.5f, 0.5f, 0.5f}
#define GREEN {0.0f, 1.0f, 0.0f}

//TODO(JENH): Solve problems with floating point precision.

Internal inline void ChangeSubWindow(State *state, SubWindow *newSubWindow) {
    SubWindow *oldSubWindow = state->currentSubWindow;

    oldSubWindow->cursor.color = GREY;
    newSubWindow->cursor.color = GREEN;

    state->currentSubWindow = newSubWindow;
    state->currentBuffer = newSubWindow->displayedBuffer;
}

Internal void MoveToSubWindow(State *state, u32 direction) {
    SubWindow *currentSubWindow = state->currentSubWindow;

    AdjacentSubWindowNode *adjacentList = currentSubWindow->adjacents[direction].list;

    u32 otherAxis = (direction + 1) % 2;

    if (adjacentList) {
	f32 glyphDimension  = state->characterInfo.glyphDimensions.E[otherAxis];
	u32 cursorPosInAxis = currentSubWindow->cursor.position.E[otherAxis];

	f32 cursorOtherAxisInWindowCoords = currentSubWindow->absTopLeftPosition.E[otherAxis] + 
		                            (cursorPosInAxis * glyphDimension);

        AdjacentSubWindowNode *traverse = adjacentList;
        AdjacentSubWindowNode *next = traverse->next;

	while (next && next->adjacent->absTopLeftPosition.E[otherAxis] <= cursorOtherAxisInWindowCoords) {
	    traverse = traverse->next;
	    next = next->next;
	}

        ChangeSubWindow(state, traverse->adjacent);
	state->savedSubWindow = state->currentSubWindow;
    }
}

Internal inline void DeleteNode(AdjacentNodes *adjacentNodePool, AdjacentSubWindowNode *nodeToDelete) {
    nodeToDelete->next = adjacentNodePool->firstEmpty;
    adjacentNodePool->firstEmpty = nodeToDelete;
}

Internal void DeleteAdjacentList(AdjacentNodes *adjacentNodePool, AdjacentList *adjacentListToDelete) {
    AdjacentSubWindowNode *traverse = adjacentListToDelete->list;
    ASSERT(traverse)

    AdjacentSubWindowNode *next;

    do {
         next = traverse->next;
         DeleteNode(adjacentNodePool, traverse);
         traverse = next;
    } while (next);

    adjacentListToDelete->list = 0;
    adjacentListToDelete->tail = 0;
}

Internal void DeleteAdjacentListOfAdjacentSubWindows(AdjacentNodes *adjacentNodePool, 
		                                     AdjacentList *listOfListToDelete, u32 oppositeDirection) {
    for (AdjacentSubWindowNode *traverse = listOfListToDelete->list; traverse; traverse = traverse->next) {
        DeleteAdjacentList(adjacentNodePool, &traverse->adjacent->adjacents[oppositeDirection]);
    }
}

Internal AdjacentSubWindowNode *GetNewNode(AdjacentNodes *adjacentNodePool) {
    AdjacentSubWindowNode *newNode;

    newNode = adjacentNodePool->firstEmpty;

    (adjacentNodePool->firstEmpty->next) ? adjacentNodePool->firstEmpty = adjacentNodePool->firstEmpty->next
					 : ++adjacentNodePool->firstEmpty;

    newNode->next = 0;

    ASSERT(adjacentNodePool->firstEmpty - adjacentNodePool->nodes < ARRAYLENGTH(adjacentNodePool->nodes));
	
    return newNode;
}

Internal void OneAdjacentCaseDeleteNodeBySubWindow(SubWindow *subWindowToDelete, AdjacentList *adjacentList, 
						   AdjacentNodes *adjacentNodePool) {
    ASSERT(adjacentList->list);

    if (adjacentList->first->adjacent == subWindowToDelete) {
	AdjacentSubWindowNode *nodeToDelete = adjacentList->first;

	adjacentList->list = adjacentList->first->next;
	if (!adjacentList->first) { adjacentList->tail = 0; }

	DeleteNode(adjacentNodePool, nodeToDelete);
	return;
    }

    AdjacentSubWindowNode *traverse = adjacentList->list;

    while (traverse->next->adjacent != subWindowToDelete) {
	traverse = traverse->next;
    }

    if (traverse->next == adjacentList->tail) { adjacentList->tail = traverse; }

    AdjacentSubWindowNode *nodeToDelete = traverse->next;

    traverse->next = traverse->next->next;

    DeleteNode(adjacentNodePool, nodeToDelete);
}

Internal b32 UpdateAdjacentSubWindowOfAdjacentList(AdjacentList *listToUpdate, SubWindow *newSubWindow, 
		                                   u32 directionToNewSubWindow) {
    b32 isOneOppCase;

    AdjacentSubWindowNode *traverse = listToUpdate->list;
    AdjacentSubWindowNode *tail = listToUpdate->tail;

    if (traverse != tail) {
	traverse = traverse->next;

        u32 direction = directionToNewSubWindow;

        listToUpdate->first->adjacent->adjacents[direction].tail->adjacent = newSubWindow;

        for (;traverse != tail; traverse = traverse->next) {
	    traverse->adjacent->adjacents[direction].first->adjacent = newSubWindow;
	}

        listToUpdate->tail->adjacent->adjacents[direction].first->adjacent = newSubWindow;

        isOneOppCase = false;
    } else {
        isOneOppCase = true;
    }

    return isOneOppCase;
}

Internal void OneAdjacentCaseReplaceSubWindow(SubWindow *newSubWindow, SubWindow *oldSubWindow,
		                              u32 directionAdjacentSW, u32 oppositeDirectionAdjacentSW) {
    u32 direction = directionAdjacentSW;
    u32 opposite  = oppositeDirectionAdjacentSW;

    AdjacentSubWindowNode *traverse = newSubWindow->adjacents[direction].first->adjacent->adjacents[opposite].list;

    while (traverse->adjacent != oldSubWindow) {
	traverse = traverse->next;
    }

    traverse->adjacent = newSubWindow;
}

Internal b32 CanBeMerged(AdjacentList *adjacentList, SubWindow *subWindow, u32 axis) {
    b32 result;

    f32 beginVerticeSubWindow = subWindow->absTopLeftPosition.E[axis];
    f32 endVerticeSubWindow   = beginVerticeSubWindow + subWindow->absDimensions.E[axis];

    f32 beginVerticeList = adjacentList->first->adjacent->absTopLeftPosition.E[axis];
    f32 endVerticeSubWindowList = adjacentList->tail->adjacent->absTopLeftPosition.E[axis] + 
	                          adjacentList->tail->adjacent->absDimensions.E[axis];

    result = (beginVerticeSubWindow == beginVerticeList && endVerticeSubWindow == endVerticeSubWindowList);

    return result;
}

Internal b32 AreAlingedAtTheEnd(SubWindow *subWindow1, SubWindow *subWindow2, u32 axis) {
    b32 result;

    result = subWindow1->absTopLeftPosition.E[axis] + subWindow1->absDimensions.E[axis] == 
             subWindow2->absTopLeftPosition.E[axis] + subWindow2->absDimensions.E[axis];

    return result;
}

Internal b32 AreAlinged(SubWindow *subWindow1, SubWindow *subWindow2, u32 axis) {
    b32 result;

    result = subWindow1->absTopLeftPosition.E[axis] == subWindow2->absTopLeftPosition.E[axis];

    return result;
}

//TODO(JENH): Is still not updating correctly?.
Internal void UpdateOppositeMergeAxisSubWindowInDelete(SubWindow *subWindowDelete, 
		                                       SubWindow *subWindowReplace, 
						       AdjacentNodes *adjacentNodePool, 
						       u32 mergeDirection, u32 mergeAxis,
						       u32 mergeOppositeAxis, u32 oppMergeOppAxis) {
    if (subWindowDelete->adjacents[mergeOppositeAxis].list) {
        AdjacentList *newSubWindowList;
        AdjacentList *oldSubWindowList;

        b32 areAlinged;

        if (mergeDirection <= SW_UP) {
            newSubWindowList = &subWindowReplace->adjacents[mergeOppositeAxis];
            oldSubWindowList = &subWindowDelete->adjacents[mergeOppositeAxis];
            areAlinged = AreAlinged(subWindowDelete, 
			            subWindowDelete->adjacents[mergeOppositeAxis].first->adjacent, mergeAxis);
        } else {
            newSubWindowList = &subWindowDelete->adjacents[mergeOppositeAxis];
            oldSubWindowList = &subWindowReplace->adjacents[mergeOppositeAxis];
            areAlinged = AreAlinged(subWindowReplace, 
			            subWindowReplace->adjacents[mergeOppositeAxis].first->adjacent, mergeAxis);
	}

	//NOTE*(JENH): Here may be the problem?.
	if (areAlinged) {
	    AdjacentList *ToDeleteAdjacentList = &subWindowDelete->adjacents[mergeOppositeAxis];
		
            if (mergeDirection <= SW_UP) {
	        ToDeleteAdjacentList->first->adjacent->adjacents[oppMergeOppAxis].first->adjacent = 
			                                                                        subWindowReplace;
            } else {
	        ToDeleteAdjacentList->first->adjacent->adjacents[oppMergeOppAxis].tail->adjacent = 
			                                                                        subWindowReplace;
	    }
	} else {
            OneAdjacentCaseDeleteNodeBySubWindow(subWindowDelete,
                     &oldSubWindowList->first->adjacent->adjacents[oppMergeOppAxis],
	             adjacentNodePool);
	}

        b32 isOneCase = UpdateAdjacentSubWindowOfAdjacentList(&subWindowDelete->adjacents[mergeOppositeAxis], 
			                                      subWindowReplace, oppMergeOppAxis);

        AdjacentSubWindowNode *listToAppend = oldSubWindowList->list;

	if (!areAlinged) {
	    listToAppend = listToAppend->next; 
	    DeleteNode(adjacentNodePool, oldSubWindowList->first);
        }

        newSubWindowList->tail->next = listToAppend;
	if (newSubWindowList->tail->next) {
            newSubWindowList->tail = oldSubWindowList->tail;
	}

        subWindowReplace->adjacents[mergeOppositeAxis] = *newSubWindowList;
    }
}

Internal AdjacentSubWindowNode *
ConectOppMergeWithMergeSubWindow(AdjacentSubWindowNode *traverseMerge, SubWindow *oppMergeSubWindow,
		 		 AdjacentSubWindowNode *startAppendOppMerge, u32 mergeOppositeAxis,
				 u32 mergeDirection, u32 oppositeMergeDirection, 
				 b32 lastTraverseMergeSubWindowWasAligned, AdjacentNodes *adjacentNodePool) {
    ASSERT(traverseMerge);

    AdjacentSubWindowNode *lastMergeNodeAdded = traverseMerge;
	
    AdjacentSubWindowNode *lastAppendOppMerge = startAppendOppMerge;

    b32 firstTraverseMergeNodeWasUsed = !lastTraverseMergeSubWindowWasAligned; 

    AdjacentList *traverseMergeList = &traverseMerge->adjacent->adjacents[oppositeMergeDirection];
    AdjacentList *traverseOppMergeList = &oppMergeSubWindow->adjacents[mergeDirection];

    startAppendOppMerge->adjacent = traverseMerge->adjacent;

    if (!firstTraverseMergeNodeWasUsed) {
        traverseMergeList->first->adjacent = oppMergeSubWindow;
        firstTraverseMergeNodeWasUsed = false;
    } else {
        AdjacentSubWindowNode *newNode = GetNewNode(adjacentNodePool);
        newNode->adjacent = oppMergeSubWindow;
        traverseMergeList->tail->next = newNode;
        traverseMergeList->tail = traverseMergeList->tail->next;
    }

    traverseMerge = traverseMerge->next;

    while (traverseMerge && oppMergeSubWindow->absTopLeftPosition.E[mergeOppositeAxis] + 
           oppMergeSubWindow->absDimensions.E[mergeOppositeAxis] > 
           traverseMerge->adjacent->absTopLeftPosition.E[mergeOppositeAxis]) {

        traverseMergeList = &traverseMerge->adjacent->adjacents[oppositeMergeDirection];

        AdjacentSubWindowNode *newNode;

	newNode = GetNewNode(adjacentNodePool);
	newNode->adjacent = traverseMerge->adjacent;
	newNode->next = lastAppendOppMerge->next;
	lastAppendOppMerge->next = newNode;
        lastAppendOppMerge = lastAppendOppMerge->next; 

	if (!newNode->next) { traverseOppMergeList->tail = newNode; }

	traverseMergeList->first->adjacent = oppMergeSubWindow;

        lastMergeNodeAdded = traverseMerge;
        traverseMerge = traverseMerge->next;
    }

    return lastMergeNodeAdded;
}

Internal void DeleteSubWindow(State *state, SubWindowGridSystem *subWindowGridSystem, SubWindow *subWindow) {
    AdjacentList *traverseList = subWindow->adjacents; 

    u32 mergeDirection = SW_LEFT;

    for (;!traverseList->list || !CanBeMerged(traverseList, subWindow, (mergeDirection + 1) % 2); 
	 ++traverseList, ++mergeDirection);

    AdjacentList *mergeList = traverseList; 

    ChangeSubWindow(state, mergeList->first->adjacent);

    state->savedSubWindow = mergeList->first->adjacent;

    u32 mergeAxis = mergeDirection % 2;

    u32 mergeOppositeAxis = (mergeAxis == AXIS_HOR) ? SW_UP : SW_LEFT;

    SubWindow *subWindowReplace = mergeList->first->adjacent;

    UpdateOppositeMergeAxisSubWindowInDelete(subWindow, subWindowReplace, &subWindowGridSystem->adjacentNodePool,
		     			     mergeDirection, mergeAxis, mergeOppositeAxis, mergeOppositeAxis + 2);
    
    subWindowReplace = mergeList->tail->adjacent;

    UpdateOppositeMergeAxisSubWindowInDelete(subWindow, subWindowReplace, &subWindowGridSystem->adjacentNodePool,
		     			     mergeDirection, mergeAxis, mergeOppositeAxis + 2, mergeOppositeAxis);

    u32 oppositeMergeDirection = mergeDirection + 2 - (4 * (mergeDirection / 2));

    AdjacentList *oppositeMergeList = &subWindow->adjacents[oppositeMergeDirection];

    if (!oppositeMergeList->first) {
        DeleteAdjacentListOfAdjacentSubWindows(&subWindowGridSystem->adjacentNodePool, mergeList, 
		      		   	       oppositeMergeDirection);
    } else if (!oppositeMergeList->first->next) {
	SubWindow *oppMergeSubWindow = oppositeMergeList->first->adjacent;
	AdjacentList *oppositeMergeToDelSWList = &oppMergeSubWindow->adjacents[mergeDirection];

	AdjacentSubWindowNode *traverseOppMerge = oppositeMergeToDelSWList->list;

	while (traverseOppMerge->adjacent != subWindow) { traverseOppMerge = traverseOppMerge->next; }

        traverseOppMerge->adjacent = mergeList->first->adjacent;
	mergeList->first->adjacent->adjacents[oppositeMergeDirection].first->adjacent = oppMergeSubWindow;

        AdjacentSubWindowNode *traverseMerge = mergeList->first->next;

	b32 isTail = traverseOppMerge == oppositeMergeToDelSWList->tail;

	while (traverseMerge) {
	    traverseMerge->adjacent->adjacents[oppositeMergeDirection].first->adjacent = oppMergeSubWindow; 

            AdjacentSubWindowNode *newNode = GetNewNode(&subWindowGridSystem->adjacentNodePool);
            newNode->adjacent = traverseMerge->adjacent;
            newNode->next = traverseOppMerge->next;
	    traverseOppMerge->next = newNode;
	    traverseOppMerge = traverseOppMerge->next;

            if (isTail) { oppositeMergeToDelSWList->tail = newNode; }

            traverseMerge = traverseMerge->next;
	}

    } else {
	AdjacentSubWindowNode *traverseMerge = mergeList->list;
	AdjacentSubWindowNode *traverseOppMerge = oppositeMergeList->list;

	AdjacentSubWindowNode *startAppendOppMerge = 
		                               oppositeMergeList->first->adjacent->adjacents[mergeDirection].tail;

	b32 lastTraverseMergeSubWindowWasAligned = true;

	for (;;) {
            traverseMerge = ConectOppMergeWithMergeSubWindow(traverseMerge, traverseOppMerge->adjacent, 
			                                     startAppendOppMerge, mergeOppositeAxis, 
							     mergeDirection, oppositeMergeDirection, 
				                             lastTraverseMergeSubWindowWasAligned, 
					                     &subWindowGridSystem->adjacentNodePool);

	    if (AreAlingedAtTheEnd(traverseMerge->adjacent, traverseOppMerge->adjacent, mergeOppositeAxis)) {
                traverseMerge = traverseMerge->next;
                lastTraverseMergeSubWindowWasAligned = true;
	    } else {
                lastTraverseMergeSubWindowWasAligned = false;
	    }

            traverseOppMerge = traverseOppMerge->next;

	    if (!traverseOppMerge) { break; }

            startAppendOppMerge = traverseOppMerge->adjacent->adjacents[mergeDirection].first;
	}
    }

    v2_f32 glyphDimensions = state->characterInfo.glyphDimensions;

    if (mergeDirection <= SW_UP) {
        for (AdjacentSubWindowNode *traverse = mergeList->list; traverse; traverse = traverse->next) {
	    SubWindow *adjacent = traverse->adjacent;

	    adjacent->absDimensions.E[mergeAxis] += subWindow->absDimensions.E[mergeAxis];
	    
            adjacent->dimensionsInGlyphs = Tou32(HadamardDiv(adjacent->absDimensions, glyphDimensions));
        }
    } else {
        for (AdjacentSubWindowNode *traverse = mergeList->list; traverse; traverse = traverse->next) {
	    SubWindow *adjacent = traverse->adjacent;

	    adjacent->absTopLeftPosition.E[mergeAxis] -= subWindow->absDimensions.E[mergeAxis];
	    adjacent->absDimensions.E[mergeAxis] += subWindow->absDimensions.E[mergeAxis];

            adjacent->dimensionsInGlyphs = Tou32(HadamardDiv(adjacent->absDimensions, glyphDimensions));
        }
    }

    DeleteAdjacentList(&subWindowGridSystem->adjacentNodePool, mergeList);

    if (oppositeMergeList->list) {
        DeleteAdjacentList(&subWindowGridSystem->adjacentNodePool, oppositeMergeList);
    }

    subWindow->isActive = false;

    return;
}

Internal b32 UpdateOtherSplitAxisAdjacentSubWindows(SubWindow *newSubWindow, AdjacentList *adjacentsNewSubWindow,
		                                    SubWindow *oldSubWindow, AdjacentList *adjacentsOldSubWindow,
						    u32 axis, AdjacentNodes *adjacentNodePool) {
    *adjacentsNewSubWindow = *adjacentsOldSubWindow;

    AdjacentSubWindowNode *traverse = adjacentsOldSubWindow->list;
    if (!traverse) { return false; }

    AdjacentSubWindowNode *next = traverse->next;

    while (next) {
	if (oldSubWindow->absTopLeftPosition.E[axis] <= next->adjacent->absTopLeftPosition.E[axis]) {
            b32 adjacentSubWindowsAreAling = next->adjacent->absTopLeftPosition.E[axis] ==
		                             oldSubWindow->absTopLeftPosition.E[axis];
 
            if (adjacentSubWindowsAreAling) {
                adjacentsOldSubWindow->list = next;
            } else {
                AdjacentSubWindowNode *copyNode = GetNewNode(adjacentNodePool);
                *copyNode = *traverse;
                adjacentsOldSubWindow->list = copyNode;
            }

            traverse->next = 0;
            adjacentsNewSubWindow->tail = traverse;

	    return adjacentSubWindowsAreAling;
	}

        traverse = traverse->next;
	next = next->next;
    }

    AdjacentSubWindowNode *copyNode = GetNewNode(adjacentNodePool);
    *copyNode = *traverse;
    adjacentsOldSubWindow->list = copyNode;
    adjacentsOldSubWindow->tail = copyNode;

    return false;
}

Internal void OneAdjacentSubWindowCaseOppSplitAxis(SubWindow *newSubWindow, SubWindow *oldSubWindow,
		                                   u32 directionAdjacentSW, u32 oppositeDirectionAdjacentSW,
				                   AdjacentSubWindowNode *newNode) {
    u32 direction = directionAdjacentSW;
    u32 opposite  = oppositeDirectionAdjacentSW;

    newNode->adjacent = newSubWindow;

    AdjacentSubWindowNode *traverse = newSubWindow->adjacents[direction].tail->adjacent->adjacents[opposite].list;

    if (traverse->adjacent == oldSubWindow) {
        newSubWindow->adjacents[direction].tail->adjacent->adjacents[opposite].list = newNode;
	newNode->next = traverse;
    } else {
        AdjacentSubWindowNode *next = traverse->next;

        while (next->adjacent != oldSubWindow) {
	    traverse = traverse->next;
	    next = next->next;
        }

        traverse->next = newNode;
        newNode->next = next;
    }

    if (!newNode->next) {
	newSubWindow->adjacents[direction].first->adjacent->adjacents[opposite].tail = newNode;
    }
}

Internal SubWindow *SplitSubWindow(SubWindowGridSystem *subWindowGridSystem, SubWindow *oldSubWindow,
		                   v2_f32 glyphDimensions, u32 axis) {
    SubWindow *newSubWindow = subWindowGridSystem->firstSubWindowEmpty++;

    newSubWindow->isActive = true;

    newSubWindow->absDimensions = oldSubWindow->absDimensions;
    newSubWindow->absDimensions.E[axis] /= 2.0f;
    oldSubWindow->absDimensions.E[axis] -= newSubWindow->absDimensions.E[axis];

    v2_f32 windowDimensions = subWindowGridSystem->windowDimensions;

    oldSubWindow->dimensionsInGlyphs.E[axis] = (i32)(oldSubWindow->absDimensions.E[axis] / 
		                                     glyphDimensions.E[axis]);

    u32 otherAxis = (axis == AXIS_HOR) ? SW_UP : SW_LEFT;

    newSubWindow->dimensionsInGlyphs.E[otherAxis] = oldSubWindow->dimensionsInGlyphs.E[otherAxis];
    newSubWindow->dimensionsInGlyphs.E[axis] = (i32)(newSubWindow->absDimensions.E[axis] / 
		                                     glyphDimensions.E[axis]);

    newSubWindow->absTopLeftPosition = oldSubWindow->absTopLeftPosition;
    oldSubWindow->absTopLeftPosition.E[axis] += (f32)oldSubWindow->absDimensions.E[axis];

    AdjacentList *firstOtherSplitAxisSide  = &oldSubWindow->adjacents[otherAxis];
    AdjacentList *secondOtherSplitAxisSide = firstOtherSplitAxisSide + 2;

    AdjacentNodes *adjacentNodePool = &subWindowGridSystem->adjacentNodePool;

    b32 areAlinged;

    areAlinged = UpdateOtherSplitAxisAdjacentSubWindows(newSubWindow, &newSubWindow->adjacents[otherAxis], 
		                                        oldSubWindow, &oldSubWindow->adjacents[otherAxis], axis, adjacentNodePool);

    if (newSubWindow->adjacents[otherAxis].first) {
        if (areAlinged) {
            newSubWindow->adjacents[otherAxis].first->adjacent->adjacents[otherAxis + 2].tail->adjacent = 
		                                                                                    newSubWindow;
	} else {
            OneAdjacentSubWindowCaseOppSplitAxis(newSubWindow, oldSubWindow, otherAxis, otherAxis + 2, 
				                 GetNewNode(adjacentNodePool));
	}

	b32 result = UpdateAdjacentSubWindowOfAdjacentList(&newSubWindow->adjacents[otherAxis], 
			                                   newSubWindow, otherAxis + 2);
    }

    areAlinged = UpdateOtherSplitAxisAdjacentSubWindows(newSubWindow, &newSubWindow->adjacents[otherAxis + 2],
		                                      oldSubWindow, &oldSubWindow->adjacents[otherAxis + 2], axis, adjacentNodePool);

    if (newSubWindow->adjacents[otherAxis + 2].first) {
        if (areAlinged) {
            newSubWindow->adjacents[otherAxis + 2].first->adjacent->adjacents[otherAxis].tail->adjacent = 
		                                                                                    newSubWindow;
	} else {
            OneAdjacentSubWindowCaseOppSplitAxis(newSubWindow, oldSubWindow, otherAxis + 2, otherAxis, 
				                 GetNewNode(adjacentNodePool));
	}

	b32 result = UpdateAdjacentSubWindowOfAdjacentList(&newSubWindow->adjacents[otherAxis + 2], 
			                                   newSubWindow, otherAxis);
    }

    newSubWindow->adjacents[axis] = oldSubWindow->adjacents[axis];
    oldSubWindow->adjacents[axis] = {0};

    if (newSubWindow->adjacents[axis].first) {
        if (UpdateAdjacentSubWindowOfAdjacentList(&newSubWindow->adjacents[axis], newSubWindow, axis + 2)) {
            OneAdjacentCaseReplaceSubWindow(newSubWindow, oldSubWindow, axis, axis + 2);
	}
    }

    AdjacentSubWindowNode *newNode;

    newNode = GetNewNode(adjacentNodePool);
    newNode->adjacent = oldSubWindow;
    newSubWindow->adjacents[axis + 2] = {newNode, newNode};

    newNode = GetNewNode(adjacentNodePool);
    newNode->adjacent = newSubWindow;
    oldSubWindow->adjacents[axis] = {newNode, newNode};

    return newSubWindow;
}

MACRO_SetSubWindowsForResizing(SetSubWindowsForResizing) {
    v2_f32 minTopLeftPadding = subWindowGridSystem->minTopLeftPadding;
    v2_f32 windowDimensions = subWindowGridSystem->windowDimensions;
    windowDimensions.height -= miniBufferSubWindow->absDimensions.height;

    SubWindow *subWindows = subWindowGridSystem->subWindows;
    SubWindow *firstSubWindowEmpty = subWindowGridSystem->firstSubWindowEmpty;

    for (SubWindow *subWindow = subWindows; subWindow < firstSubWindowEmpty; subWindow++) {
        subWindow->relTopLeftPosition = HadamardDiv(subWindow->absTopLeftPosition, windowDimensions);

        subWindow->relDimensions = HadamardDiv(subWindow->absDimensions, windowDimensions);

        v2_u32 cursorPosition   = subWindow->cursor.position;
        v2_u32 screenCharOffset = subWindow->screenCharOffsetRelToText;

        v2_u32 cursorRelPosition = cursorPosition - screenCharOffset;
        v2_f32 relPositionForResizing = HadamardDiv(Tof32(cursorRelPosition), Tof32(subWindow->dimensionsInGlyphs));

        subWindow->cursor.relPositionForResizing = relPositionForResizing;
    }
}

MACRO_UpdateSubWindowsSize(UpdateSubWindowsSize) {
    State *state = (State *)memory->permanent;

    v2_f32 glyphDimensions = state->characterInfo.glyphDimensions;

    v2_f32 minTopLeftPadding = state->subWindowGridSystem.minTopLeftPadding;
    v2_f32 minBottomRightPadding = state->subWindowGridSystem.minBottomRightPadding;

    SubWindow *miniBufferSubWindow = &state->miniBufferSW;

    v2_f32 windowDimensionsWithPadding = newWindowDimensions - minTopLeftPadding;

    miniBufferSubWindow->absTopLeftPosition.height = windowDimensionsWithPadding.height - 
	                                             miniBufferSubWindow->absDimensions.height;
    miniBufferSubWindow->absDimensions.width = windowDimensionsWithPadding.width;

    v2_f32 fixedScreenDimensions = windowDimensionsWithPadding;
    fixedScreenDimensions.height -= miniBufferSubWindow->absDimensions.height;

    SubWindow *subWindows = state->subWindowGridSystem.subWindows;
    SubWindow *firstSubWindowEmpty = state->subWindowGridSystem.firstSubWindowEmpty;

    for (SubWindow *subWindow = subWindows; subWindow < firstSubWindowEmpty; ++subWindow) {
        subWindow->absTopLeftPosition = HadamardProd(fixedScreenDimensions, subWindow->relTopLeftPosition);

        v2_u32 startScreenInChars = subWindow->screenCharOffsetRelToText;
        v2_u32 oldDimensionsInGlyphs = subWindow->dimensionsInGlyphs;

        subWindow->absDimensions = HadamardProd(fixedScreenDimensions, subWindow->relDimensions);

        v2_f32 absDimensionsWithPadding = HadamardProd(fixedScreenDimensions - minBottomRightPadding, 
                                               subWindow->relDimensions);

        subWindow->dimensionsInGlyphs = Tou32(HadamardDiv(absDimensionsWithPadding, glyphDimensions));

        v2_u32 newDimensionsInGlyphs = subWindow->dimensionsInGlyphs;

	v2_f32 cursorRelPosition = subWindow->cursor.relPositionForResizing;

        v2_i32 oldDimensionsInGlyphsFromCursorToTopLeft = Toi32(HadamardProd(cursorRelPosition, 
			                                                     Tof32(oldDimensionsInGlyphs)));

        v2_i32 oldDimensionsInGlyphsFromCursorToBottomRight = Toi32(oldDimensionsInGlyphs) - 
		                                              oldDimensionsInGlyphsFromCursorToTopLeft;

        v2_i32 newDimensionsInGlyphsFromCursorToTopLeft = Toi32(HadamardProd(cursorRelPosition, 
			                                                     Tof32(newDimensionsInGlyphs)));

        v2_i32 newDimensionsInGlyphsFromCursorToBottomRight = Toi32(newDimensionsInGlyphs) - 
		                                              newDimensionsInGlyphsFromCursorToTopLeft;

        v2_i32 screenOffsetTopLeft = oldDimensionsInGlyphsFromCursorToTopLeft - 
		                     newDimensionsInGlyphsFromCursorToTopLeft;

        v2_i32 newScreenOffsetBottomRight = oldDimensionsInGlyphsFromCursorToBottomRight - 
		                            newDimensionsInGlyphsFromCursorToBottomRight;

	if (startScreenInChars.x == 0 && screenOffsetTopLeft.x < 0) {
	    v2_u32 cursorRelPosition = subWindow->cursor.position - startScreenInChars;
	    v2_f32 relPositionForResizing = HadamardDiv(Tof32(cursorRelPosition), Tof32(newDimensionsInGlyphs));

	    subWindow->cursor.relPositionForResizing = relPositionForResizing;
            screenOffsetTopLeft.x = 0;
	}

	if (startScreenInChars.y == 0 && screenOffsetTopLeft.y < 0) {
	    v2_u32 cursorRelPosition = subWindow->cursor.position - startScreenInChars;
	    v2_f32 relPositionForResizing = HadamardDiv(Tof32(cursorRelPosition), Tof32(newDimensionsInGlyphs));

	    subWindow->cursor.relPositionForResizing = relPositionForResizing;
            screenOffsetTopLeft.y = 0;
	}

        MoveScreen(subWindow, -screenOffsetTopLeft, screenOffsetTopLeft);
    }

    state->subWindowGridSystem.windowDimensions = windowDimensionsWithPadding;
}

