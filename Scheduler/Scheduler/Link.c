/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Link.c                                                                                                             *
 *  Organic Scheduler                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 18/02/18.                                                                             *
 *  Copyright © 2018 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Description in Link.h                                                                                              *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "Link.h"

/**
 Saves the space of the link for when a malloc is needed
 
 @param link_pt pointer to the link
 @return 0 if successful, error code otherwise
 */
int init_link(Link *link_pt) {
    
    if (link_pt == NULL) {
        printf("The given link pointer is null");
        return NULL_LINK_POINTER;
    }
    
    link_pt->speed = -1;
    link_pt->type = wired;
    return 0;
}

/**
 Sets the speed and the type of the link
 
 @param link_pt pointer to the link to change
 @param speed desired speed
 @param type desired link type (wired or wireless)
 @return 0 if doing correctly, error code otherwise
 */
int set_link(Link *link_pt, int speed, LinkType type) {
    
    if (link_pt == NULL) {
        printf("The given link pointer is null");
        return NULL_LINK_POINTER;
    }
    if (speed <= 0) {
        printf("The given link speed has to be positive");
        return SPEED_NEGATIVE;
    }
    
    link_pt->speed = speed;
    link_pt->type = type;
    return 0;
}

/**
 Gets the speed of the link
 
 @param link_pt pointer to the link to change
 @return the speed of the given link, error code otherwise
 */
int get_link_speed(Link *link_pt) {
    
    if (link_pt == NULL) {
        printf("The given link pointer is null");
        return NULL_LINK_POINTER;
    }
    
    return link_pt->speed;
}

/**
 Sets the speed of the link
 
 @param link_pt pointer to the link
 @param speed link speed
 @return 0 if correct, error code otherwise
 */
int set_link_speed(Link *link_pt, int speed) {
    
    if (link_pt == NULL) {
        printf("The given link pointer is null");
        return NULL_LINK_POINTER;
    }
    if (speed <= 0) {
        printf("The given link speed has to be positive");
        return SPEED_NEGATIVE;
    }
    
    link_pt->speed = speed;
    return 0;
}

/**
 Gets the type of the link (wired or wireless)
 
 @param link_pt pointer to the link to change
 @return the link type, error code otherwise
 */
LinkType get_link_type(Link *link_pt) {
    
    if (link_pt == NULL) {
        printf("The given link pointer is null");
        return NULL_LINK_POINTER;
    }
    
    return link_pt->type;
}

/**
 Set the type of the link
 
 @param link_pt pointer to the link
 @param type link type
 @return 0 if correct, error code otherwise
 */
LinkType set_link_type(Link *link_pt, LinkType type) {
    
    if (link_pt == NULL) {
        printf("The given link pointer is null");
        return NULL_LINK_POINTER;
    }
    
    link_pt->type = type;
    return 0;
}
