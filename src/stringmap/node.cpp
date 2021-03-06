//
// $Id: node.cpp 5027 2010-02-18 19:41:48Z rares $
//
// node.cpp
//
//
//  Copyright (C) 2003 - 2007 by The Regents of the University of
//  California
//
// Redistribution of this file is permitted under the terms of the 
// BSD license
//
// Date: March 2002
//
// Authors: Michael Ortega-Binderberger <miki (at) ics.uci.edu>
//          Liang Jin <liangj (at) ics.uci.edu>
//          Chen Li <chenli (at) ics.uci.edu>
//


#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include "assert.h"
#include "index.h"


// Initialize one branch cell in a node.
//
static void RTreeInitBranch(struct Branch *b)
{
	RTreeInitRect(&(b->rect));
	b->child = NULL;
}



// Initialize a Node structure.
//
void RTreeInitNode(struct Node *N)
{
	register struct Node *n = N;
	register int i;
	n->count = 0;
	n->level = -1;
	for (i = 0; i < NODECARD; i++)
		RTreeInitBranch(&(n->branch[i]));
}



// Make a new node and initialize to have all branch cells empty.
//
struct Node * RTreeNewNode()
{
	register struct Node *n;

	//n = new Node;
	n = (struct Node*)malloc(sizeof(struct Node));
	assert(n);
	RTreeInitNode(n);
	return n;
}


void RTreeFreeNode(struct Node *p)
{
	assert(p);
	//delete p;
	free(p);
}



static void RTreePrintBranch(struct Branch *b, int depth)
{
	RTreePrintRect(&(b->rect), depth);
	RTreePrintNode(b->child, depth);
}


extern void RTreeTabIn(int depth)
{
	int i;
	for(i=0; i<depth; i++)
		putchar('\t');
}


// Print out the data in a node.
//
void RTreePrintNode(struct Node *n, int depth) {
	int i;
	assert(n);

	RTreeTabIn(depth);
	printf("node");
	if (n->level == 0)
		printf(" LEAF");
	else if (n->level > 0)
		printf(" NONLEAF");
	else
		printf(" TYPE=?");
	printf("  level=%d  count=%d  address=%o\n", 
               n->level,
               n->count,
               static_cast<int>(reinterpret_cast<uintptr_t>(n)));

	for (i=0; i<n->count; i++) {
		if(n->level == 0) {
			// RTreeTabIn(depth);
			// printf("\t%d: data = %d\n", i, n->branch[i].child);
		} else {
			RTreeTabIn(depth);
			printf("branch %d\n", i);
			RTreePrintBranch(&n->branch[i], depth+1);
		}
	}
}



// Find the smallest rectangle that includes all rectangles in
// branches of a node.
//
struct Rect RTreeNodeCover(struct Node *N) {
	register struct Node *n = N;
	register int i, first_time=1;
	struct Rect r;
	assert(n);

	RTreeInitRect(&r);
	for (i = 0; i < NODECARD; i++)
		if (n->branch[i].child) {
			if (first_time) {
				r = n->branch[i].rect;
				first_time = 0;
			} else
				r = RTreeCombineRect(&r, &(n->branch[i].rect));
		}
	return r;
}


// compute the radius of the data node given the centroid

float ComputeRadius(struct Point *centroid, struct Node *N) {
  register struct Node *n = N;
  register int i /*,ii*/;
  float distance, radius;
  struct Point data_point;
//  float dist_array[NUMDIMS];
  assert(n);
  assert(n->level == 0); // data node

  radius = 0.0;

  for (i = 0; i < NODECARD; i++)
    if (n->branch[i].child) {
	data_point=Center(&n->branch[i].rect);
        distance=Distance(centroid, &data_point,  DIST_METRIC);
        // printf("point %d = %f \n", i+1, distance);
        if (radius < distance) radius=distance;
      }
  // getchar();
  return radius;
}




// Pick a branch.  Pick the one that will need the smallest increase
// in area to accomodate the new rectangle.  This will result in the
// least total area for the covering rectangles in the current node.
// In case of a tie, pick the one which was smaller before, to get
// the best resolution when searching.
//
int RTreePickBranch(struct Rect *R, struct Node *N) {
	register struct Rect *r = R;
	register struct Node *n = N;
	register struct Rect *rr;
	register int i, first_time=1;
	RectReal increase, bestIncr=(RectReal)-1, area, bestArea;
	int best;
	struct Rect tmp_rect;
	assert(r && n);

	for (i=0; i<NODECARD; i++) {
		if (n->branch[i].child) {
			rr = &n->branch[i].rect;
			area = RTreeRectSphericalVolume(rr);
			tmp_rect = RTreeCombineRect(r, rr);
			increase = RTreeRectSphericalVolume(&tmp_rect) - area;
			if (increase < bestIncr || first_time) {
				best = i;
				bestArea = area;
				bestIncr = increase;
				first_time = 0;
			} else if (increase == bestIncr && area < bestArea) {
				best = i;
				bestArea = area;
				bestIncr = increase;
			}
		}
	}
	return best;
}



// Add a branch to a node.  Split the node if necessary.
// Returns 0 if node not split.  Old node updated.
// Returns 1 if node split, sets *new_node to address of new node.
// Old node updated, becomes one of two.
//
int RTreeAddBranch(struct Branch *B, struct Node *N, struct Node **New_node) {
	register struct Branch *b = B;
	register struct Node *n = N;
	register struct Node **new_node = New_node;
	register int i;

	assert(b);
	assert(n);

	if (n->count < NODECARD)  /* split won't be necessary */ {
		for (i = 0; i < NODECARD; i++)  /* find empty branch */ {
			if (n->branch[i].child == NULL) {
				n->branch[i] = *b;
				n->count++;
				break;
			}
		}
		assert(i<NODECARD);
		return 0;
	} else {
		assert(new_node);
		RTreeSplitNode(n, b, new_node);
		return 1;
	}
}



// Disconnect a dependent node.
//
void RTreeDisconnectBranch(struct Node *n, int i) {
	assert(n && i>=0 && i<NODECARD);
	assert(n->branch[i].child);

	RTreeInitBranch(&(n->branch[i]));
	n->count--;
}
