/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Frame.h                                                                                                            *
 *  Organic Scheduler                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 18/02/18.                                                                             *
 *  Copyright © 2018 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Information in Frame.h                                                                                             *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef Frame_h
#define Frame_h

#include <stdio.h>
#include <stdlib.h>
#include <z3.h>
#include <gurobi_c.h>

#endif /* Frame_h */

/* STRUCT DEFINITIONS */

/**
 Structure with information of an appearance of an offset because the period. It has also arrays for all the information
 about its retransmissions
 */
typedef struct Offset {
    long long int **offset;             // Matrix with the transmission times in ns
    Z3_ast **z_offset;                  // Z3 matrix with the transmission times in ns
    int **g_offset;                     // Gurobi matrix with the transmission times in ns
    int num_instances;                  // Number of instances of the offset (hyperperiod / period frame)
    int num_replicas;                   // Number of replicas of the offset (retransmissions due to wireless)
    int timeslots;                      // Number of ns to transmit in the link
    int link;                           // Identifier of the link where this offset is being transmitted
    struct Offset *next_offset_pt;      // Pointer to the next offset (no order in particular)
}Offset;

/**
 Information about the frame.
 As a frame can have multiple paths and splits, we will save its linked list roots into an array.
 */
typedef struct Frame {
    int size;                           // Size of the frames in bytes
    long long int period;               // Period of the frame in ns
    long long int deadline;             // Deadline of the frame in ns
    long long int end_to_end_delay;     // Maximum end to end delay from a frame being sent to being received in ns
    long long int starting;             // Starting time of the frame in ns
    int sender_id;                      // ID of the end system sender
    int *receivers_id;                   // Array of ID of the end system receivers
    int num_receivers;                  // Number of end system receivers
    Offset *offset_ls;                  // Pointer to the roof of the offsets linked list
    Offset **offset_hash;               // Array that stores the offsets with index the link identifier (to accelerate)
}Frame;

/* TYPEDEF ERRORS */

#define NULL_FRAME_POINTER -1
#define LINK_NOT_NATURAL -2
#define PERIOD_NOT_NATURAL -3
#define DEADLINE_NOT_NATURAL -4
#define SIZE_NOT_NATURAL -5
#define STARTING_NOT_NATURAL -6
#define ENDTOEND_NOT_NATURAL -7
#define DEADLINE_LARGER_PERIOD -11
#define ENDTOEND_LARGER_DEADLINE -12
#define STARTING_LARGER_DEADLINE -13
#define SENDER_ID_NOT_NATURAL -14
#define RECEIVER_ID_NOT_NATURAL -15
#define NUM_RECEIVERS_NOT_NATURAL -16

#define NULL_OFFSET_POINTER -21
#define NUM_INSTANCES_NOT_NATURAL -22
#define NUM_REPLICAS_NEGATIVE -23
#define SIZE_TIMESLOT_NOT_NATURAL -24
#define LINK_OFFSET_NOT_NATURAL -25
#define TRANSMISSION_TIME_NOT_NATURAL -26
#define NUM_INSTANCES_OUT_RANGE -27
#define NUM_REPLICAS_OUT_RANGE -28
#define OFFSET_VALUES_NOT_FILLED -29

/* CODE DEFINITIONS */

/**
 Init all the values of the frame to avoid unwanted values when malloc
 
 @param frame_pt Pointer to the frame to init
 @return 0 if init is correct, error code otherwise
 */
int init_frame(Frame *frame_pt);

/**
 Init the size of the array of hashes to speed up offsets allocation
 
 @param frame_pt pointer to the frame
 @param num_links number of links in the network = size of the array
 @return 0 if done correctly, error code otherwise
 */
int init_hash(Frame *frame_pt, int num_links);

/**
 Add the current offset to the frame hash accelerator
 
 @param frame_pt pointer to the frame
 @param offset_pt pointer to the offset
 @return 0 if done correctly, error code otherwise
 */
int add_accelerator_hash(Frame *frame_pt, Offset *offset_pt);

/**
 Get the period of the given frame
 
 @param frame_pt pointer of the frame to get the period
 @return long long int with the period, error code otherwise
 */
long long int get_period(Frame *frame_pt);

/**
 Set the period of the given frame
 
 @param frame_pt pointer of the frame to add the period
 @param period long long int with the period in ns
 @return 0 if correct, error code otherwise
 */
int set_period(Frame *frame_pt, long long int period);

/**
 Get the deadline of the given frame
 
 @param frame_pt pointer to the frame
 @return long long int with the deadline, error code otherwise
 */
long long int get_deadline(Frame *frame_pt);

/**
 Set the deadline of the given frame
 
 @param frame_pt pointer of the frame to add the deadline
 @param deadline long long int with the deadline in ns
 @return 0 if correct, error code otherwise
 */
int set_deadline(Frame *frame_pt, long long int deadline);

/**
 Get the size in bytes of the given frame
 
 @param frame_pt pointer of the frame
 @return integer with the size of the frame in bytes, error code otherwise
 */
int get_size(Frame *frame_pt);

/**
 Set the size of the given frame
 
 @param frame_pt pointer of the frame to add the size
 @param size long long int with the size in bytes
 @return 0 is correct, error code otherwise
 */
int set_size(Frame *frame_pt, int size);

/**
 Get the end to end delay of the given frame
 
 @param frame_pt pointer to the frame
 @return long long int with the end to end delay of the frame, error code otherwise
 */
long long int get_end_to_end_delay(Frame *frame_pt);

/**
 Set the end to end delay of the given frame
 
 @param frame_pt pointer to the frame
 @param delay long long int with the desired end to end delay
 @return 0 if correct, error code otherwise
 */
int set_end_to_end_delay(Frame *frame_pt, long long int delay);

/**
 Get the starting time of the given frame
 
 @param frame_pt pointer to the frame
 @return long long int with the starting time, error code otherwise
 */
long long int get_starting(Frame *frame_pt);

/**
 Set the starting time of the given frame
 
 @param frame_pt pointer to the frame
 @param starting long long in with the desired starting time
 @return 0 if correct, error code otherwise
 */
int set_starting(Frame *frame_pt, long long int starting);

int get_num_receivers(Frame *frame_pt);

/**
 Get the sender identifier

 @param frame_pt pointer to the frame
 @return sender identifier, error code otherwise
 */
int get_sender_id(Frame *frame_pt);

/**
 Set the sender identifier

 @param frame_pt pointer to the frame
 @param sender_id sender identifier
 @return 0 if done correctly, error code otherwise
 */
int set_sender_id(Frame *frame_pt, int sender_id);

/**
 Get receiver identifier

 @param frame_pt pointer to the frame
 @return receiver id
 */
int get_receiver_id(Frame *frame_pt, int receiver_id);

/**
 Set the array of receivers identifiers

 @param frame_pt pointer to the frame
 @param receivers_id_array pointer where the array of receivers is stored
 @param num_receivers number of receivers in the array
 @return 0 if done correctly, error code otherwise
 */
int set_receivers_id(Frame *frame_pt, int *receivers_id_array, int num_receivers);

/**
 Get the number of instances of the offset
 
 @param offset_pt pointer to the offset
 @return number of instances, error code otherwise
 */
int get_num_instances(Offset *offset_pt);

/**
 Set the number of instances of the given frame
 
 @param offset_pt pointer to the offset
 @param num_instances number of instances to set
 @return 0 if correct, error code otherwise
 */
int set_num_instances(Offset *offset_pt, int num_instances);

/**
 Get the number of replicas of the offset
 
 @param offset_pt offset pointer
 @return number of replicas, error code otherwise
 */
int get_num_replicas(Offset *offset_pt);

/**
 Set the number of replicas of the given frame
 
 @param offset_pt pointer to the offset
 @param num_replicas number of replicas to set
 @return 0 if correct, error code otherwise
 */
int set_replicas(Offset *offset_pt, int num_replicas);

/**
 Gets the number of timeslots that the offset needs to be transmitted
 
 @param offset_pt pointer to the offset
 @return size in timeslots (ns), error code otherwise
 */
long long int get_timeslot_size(Offset *offset_pt);

/**
 Set the number of timeslots that the offset needs to be transmitted
 
 @param offset_pt pointer of the offset
 @param size in timeslots (ns)
 @return 0 if correct, error code otherwise
 */
int set_timeslot_size(Offset *offset_pt, int size);

/**
 Get the offset root of the given frame
 
 @param frame_pt pointer to the frame
 @return offset pointer to the root of the offsets linked list, NULL otherwise
 */
Offset * get_offset_root(Frame *frame_pt);

/**
 Get the next offset pointer if exists, NULL if it is the last one
 
 @param offset_pt pointer to the offset
 @return next offset pointer, or NULL if is the last offset
 */
Offset * get_next_offset(Offset *offset_pt);

/**
 Checks if it is the last offset of the offsets linked list
 
 @param offset_pt pointer to the offset
 @return 1 if is the last offset, 0 if not, error code if a problem was found
 */
int is_last_offset(Offset *offset_pt);

/**
 For the given Offsets Linked List, adds the offsets if is new.
 If it not new and already present, returns the pointer to the found one
 
 @param offset_pt offset linked list root
 @param link link to find or add
 @return the offset pointer to the offset created or found
 */
Offset * add_new_offset(Offset *offset_pt, int link);

/**
 Get the link of the given offset
 
 @param offset_pt pointer of the offset
 @return the identifier of the link, error code otherwise
 */
int get_offset_link(Offset *offset_pt);

/**
 Set the link of the given offset

 @param offset_pt pointer of the offset
 @param link link identifier
 @return 0 if correct, error code otherwise
 */
int set_offset_link(Offset *offset_pt, int link);

/**
 Get a transmission time to the offset of the given Offset
 
 @param offset_pt offset pointer
 @param num_instance number of instance in the offset
 @param num_replica number of replica in the offset
 @return long long integer of the transmission time, error code otherwise
 */
long long int get_offset(Offset *offset_pt, int num_instance, int num_replica);

/**
 Set a transmission time to the offset of the given Offset
 
 @param offset_pt offset pointer
 @param num_instance number of instance in the offset
 @param num_replica number of replica in the offset
 @param value transmission time in ns
 @return 0 if correct, error code otherwise
 */
int set_offset(Offset *offset_pt, int num_instance, int num_replica, long long int value);

/**
 Get Z3 offset of the given instance and replica
 
 @param offset_pt pointer to the offset
 @param num_instance number
 @param num_replica number
 @return Z3_ast offset constraint of z3, error code otherwise
 */
Z3_ast get_z3_offset(Offset *offset_pt, int num_instance, int num_replica);

/**
 Set a Z3 offset with the given contraint
 
 @param offset_pt pointer to the offset
 @param num_instance number
 @param num_replica number
 @param z3_constraint z3 constraint to add
 @return 0 if done correctly, error otherwise
 */
int set_z3_offset(Offset *offset_pt, int num_instance, int num_replica, Z3_ast z3_constraint);

/**
 Get gurobi offset of the given instance and replica
 
 @param offset_pt pointer to the offset
 @param num_instance number
 @param num_replica number
 @return long long int offset constraint of gurobi, error code otherwise
 */
int get_gurobi_offset(Offset *offset_pt, int num_instance, int num_replica);

/**
 Set a Gurobi offset with the given contraint
 
 @param offset_pt pointer to the offset
 @param num_instance number
 @param num_replica number
 @param gurobi_constraint z3 constraint to add
 @return 0 if done correctly, error otherwise
 */
int set_gurobi_offset(Offset *offset_pt, int num_instance, int num_replica, int gurobi_constraint);

/**
 Allocates the memory needed and prepare all variables for the used to be ready to be used
 
 @param offset_pt pointer of the offset
 @return 0 if correct, error code otherwise
 */
int prepare_offset(Offset *offset_pt);

/**
 Get the Offset pointer of a frame with the given link.
 This function is O(1) using a hash table and tries to avoid to find the offset iterating the whole offset linked list
 
 @param frame_pt pointer to the frame
 @param link identifier of the link being search
 @return pointer to the offset of the frame that has the link given in the parameters, NULL if not in the linked list or
 error
 */
Offset * get_frame_offset_by_link(Frame *frame_pt, int link);
