/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Link.h                                                                                                             *
 *  Organic Scheduler                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 18/02/18.                                                                             *
 *  Copyright © 2018 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Package that contains the information of a single link in the network.                                             *
 *  A link containts the information of the speed of the link and the type (wired or wireless).                        *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef Link_h
#define Link_h

#include <stdio.h>

#endif /* Link_h */

/* STRUCT DEFINITIONS */

/**
 Enumeration of different types of links
 */
typedef enum LinkType {
    wired,
    wireless,
    access_point
}LinkType;

/**
 Link of a network with its type and speed
 */
typedef struct Link {
    LinkType type;                      // Type of the link
    int speed;                          // Speed in MB/s of the link
}Link;

/* TYPEDEF ERRORS */

#define SPEED_NEGATIVE -1
#define NULL_LINK_POINTER -2

/* CODE DEFINITIONS */

/**
 Saves the space of the link for when a malloc is needed
 
 @param link_pt pointer to the link
 @return 0 if successful, error code otherwise
 */
int init_link(Link *link_pt);

/**
 Sets the speed and the type of the link
 
 @param link_pt pointer to the link to change
 @param speed desired speed
 @param type desired link type (wired or wireless)
 @return 0 if doing correctly, error code otherwise
 */
int set_link(Link *link_pt, int speed, LinkType type);

/**
 Gets the speed of the link
 
 @param link_pt pointer to the link to change
 @return the speed of the given link, error code otherwise
 */
int get_link_speed(Link *link_pt);

/**
 Sets the speed of the link

 @param link_pt pointer to the link
 @param speed link speed
 @return 0 if correct, error code otherwise
 */
int set_link_speed(Link *link_pt, int speed);

/**
 Gets the type of the link (wired or wireless)
 
 @param link_pt pointer to the link to change
 @return the link type, error code otherwise
 */
LinkType get_link_type(Link *link_pt);

/**
 Set the type of the link

 @param link_pt pointer to the link
 @param type link type
 @return 0 if correct, error code otherwise
 */
LinkType set_link_type(Link *link_pt, LinkType type);
