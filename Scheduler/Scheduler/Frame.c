/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Frame.c                                                                                                            *
 *  Organic Scheduler                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 18/02/18.                                                                             *
 *  Copyright © 2018 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Description in Frame.h                                                                                             *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "Frame.h"

/* FUNCTIONS */

/**
 Init all the values of the frame to avoid unwanted values when malloc
 
 @param frame_pt Pointer to the frame to init
 @return 0 if init is correct, error code otherwise
 */
int init_frame(Frame *frame_pt) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL_FRAME_POINTER;
    }
    
    frame_pt->deadline = -1;
    frame_pt->period = -1;
    frame_pt->size = -1;
    frame_pt->offset_ls = malloc(sizeof(Offset));
    frame_pt->offset_ls->next_offset_pt = NULL;
    frame_pt->offset_ls->link = -1;
    frame_pt->offset_hash = NULL;
    return 0;
}

/**
 Init the size of the array of hashes to speed up offsets allocation
 
 @param frame_pt pointer to the frame
 @param num_links number of links in the network = size of the array
 @return 0 if done correctly, error code otherwise
 */
int init_hash(Frame *frame_pt, int num_links) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL_FRAME_POINTER;
    }
    if (num_links <= 0) {
        printf("The number of links should be positive\n");
        return LINK_NOT_NATURAL;
    }
    
    frame_pt->offset_hash = malloc(sizeof(Offset *) * num_links);       // Allocate for the pointer of the array
    for (int i = 0; i < num_links; i++) {
        frame_pt->offset_hash[i] = NULL;
    }
    return 0;
}

/**
 Add the current offset to the frame hash accelerator
 
 @param frame_pt pointer to the frame
 @param offset_pt pointer to the offset
 @return 0 if done correctly, error code otherwise
 */
int add_accelerator_hash(Frame *frame_pt, Offset *offset_pt) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL_FRAME_POINTER;
    }
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL_OFFSET_POINTER;
    }
    
    frame_pt->offset_hash[offset_pt->link] = offset_pt;
    return 0;
}

/**
 Get the period of the given frame
 
 @param frame_pt pointer of the frame to get the period
 @return long long int with the period, error code otherwise
 */
long long int get_period(Frame *frame_pt) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL_FRAME_POINTER;
    }
    
    return frame_pt->period;
}

/**
 Set the period of the given frame
 
 @param frame_pt pointer of the frame to add the period
 @param period long long int with the period in ns
 @return 0 if correct, error code otherwise
 */
int set_period(Frame *frame_pt, long long int period) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL_FRAME_POINTER;
    }
    if (period <= 0) {
        printf("The period should be a positive number\n");
        return PERIOD_NOT_NATURAL;
    }
    
    frame_pt->period = period;
    return 0;
}

/**
 Get the deadline of the given frame
 
 @param frame_pt pointer to the frame
 @return long long int with the deadline, error code otherwise
 */
long long int get_deadline(Frame *frame_pt) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL_FRAME_POINTER;
    }
    
    return frame_pt->deadline;
}

/**
 Set the deadline of the given frame
 
 @param frame_pt pointer of the frame to add the deadline
 @param deadline long long int with the deadline in ns
 @return 0 if correct, error code otherwise
 */
int set_deadline(Frame *frame_pt, long long int deadline) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL_FRAME_POINTER;
    }
    if (deadline <= 0) {
        printf("The deadline should be a positive number\n");
        return DEADLINE_NOT_NATURAL;
    }
    if (deadline > frame_pt->period) {
        printf("The deadline cannot be larger than the period\n");
        return DEADLINE_LARGER_PERIOD;
    }
    
    frame_pt->deadline = deadline;
    return 0;
}

/**
 Get the size in bytes of the given frame
 
 @param frame_pt pointer of the frame
 @return integer with the size of the frame in bytes, error code otherwise
 */
int get_size(Frame *frame_pt) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL_FRAME_POINTER;
    }
    
    return frame_pt->size;
}

/**
 Set the size of the given frame
 
 @param frame_pt pointer of the frame to add the size
 @param size long long int with the size in bytes
 @return 0 is correct, error code otherwise
 */
int set_size(Frame *frame_pt, int size) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL_FRAME_POINTER;
    }
    if (size <= 0) {
        printf("The size should be a positive number\n");
        return SIZE_NOT_NATURAL;
    }
    
    frame_pt->size = size;
    return 0;
}

/**
 Get the end to end delay of the given frame
 
 @param frame_pt pointer to the frame
 @return long long int with the end to end delay of the frame, error code otherwise
 */
long long int get_end_to_end_delay(Frame *frame_pt) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL_FRAME_POINTER;
    }
    
    return frame_pt->end_to_end_delay;
}

/**
 Set the end to end delay of the given frame
 
 @param frame_pt pointer to the frame
 @param delay long long int with the desired end to end delay
 @return 0 if correct, error code otherwise
 */
int set_end_to_end_delay(Frame *frame_pt, long long int delay) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL_FRAME_POINTER;
    }
    if (delay <= 0) {
        printf("The end to end delay should be a positive number\n");
        return ENDTOEND_NOT_NATURAL;
    }
    if (delay > frame_pt->deadline) {
        printf("The end to end delay should not be larger than the deadline\n");
        return ENDTOEND_LARGER_DEADLINE;
    }
    
    frame_pt->end_to_end_delay = delay;
    return 0;
}

/**
 Get the starting time of the given frame
 
 @param frame_pt pointer to the frame
 @return long long int with the starting time, error code otherwise
 */
long long int get_starting(Frame *frame_pt) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL_FRAME_POINTER;
    }
    
    return frame_pt->starting;
}

/**
 Set the starting time of the given frame
 
 @param frame_pt pointer to the frame
 @param starting long long in with the desired starting time
 @return 0 if correct, error code otherwise
 */
int set_starting(Frame *frame_pt, long long int starting) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL_FRAME_POINTER;
    }
    if (starting < 0) {
        printf("The starting time should be a positive number\n");
        return STARTING_NOT_NATURAL;
    }
    if (starting >= frame_pt->deadline) {
        printf("The starting time should not be larger than the deadline\n");
        return STARTING_LARGER_DEADLINE;
    }
    
    frame_pt->starting = starting;
    return 0;
}

/**
 Get the sender identifier
 
 @param frame_pt pointer to the frame
 @return sender identifier, error code otherwise
 */
int get_sender_id(Frame *frame_pt) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL_FRAME_POINTER;
    }
    
    return frame_pt->sender_id;
}

/**
 Set the sender identifier
 
 @param frame_pt pointer to the frame
 @param sender_id sender identifier
 @return 0 if done correctly, error code otherwise
 */
int set_sender_id(Frame *frame_pt, int sender_id) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL_FRAME_POINTER;
    }
    if (sender_id < 0) {
        printf("The sender identifier should be a positive number\n");
        return SENDER_ID_NOT_NATURAL;
    }
    
    frame_pt->sender_id = sender_id;
    return 0;
}

/**
 Get  receiver identifier
 
 @param frame_pt pointer to the frame
 @return array with receivers, error code otherwise
 */
int get_receiver_id(Frame *frame_pt, int receiver_id) {
    
    return frame_pt->receivers_id[receiver_id];

}

int get_num_receivers(Frame *frame_pt) {
    return frame_pt->num_receivers;
}

/**
 Set the array of receivers identifiers
 
 @param frame_pt pointer to the frame
 @param receivers_id_array pointer where the array of receivers is stored
 @param num_receivers number of receivers in the array
 @return 0 if done correctly, error code otherwise
 */
int set_receivers_id(Frame *frame_pt, int *receivers_id_array, int num_receivers) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL_FRAME_POINTER;
    }
    if (num_receivers <= 0) {
        printf("The number of receivers should be a posisite number\n");
        return NUM_RECEIVERS_NOT_NATURAL;
    }
    
    frame_pt->receivers_id = malloc(sizeof(int) * num_receivers);   // Allocate the memory
    frame_pt->num_receivers = num_receivers;
    for (int receiver_id = 0; receiver_id < num_receivers; receiver_id++) {
        if (receivers_id_array[receiver_id] < 0) {
            printf("The receiver id should be a positive number\n");
            return RECEIVER_ID_NOT_NATURAL;
        }
        frame_pt->receivers_id[receiver_id] = receivers_id_array[receiver_id];
    }
    return 0;
}

/**
 Get the number of instances of the offset
 
 @param offset_pt pointer to the offset
 @return number of instances, error code otherwise
 */
int get_num_instances(Offset *offset_pt) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL_OFFSET_POINTER;
    }
    
    return offset_pt->num_instances;
}

/**
 Set the number of instances of the given frame
 
 @param offset_pt pointer to the offset
 @param num_instances number of instances to set
 @return 0 if correct, error code otherwise
 */
int set_num_instances(Offset *offset_pt, int num_instances) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL_OFFSET_POINTER;
    }
    if (num_instances <= 0) {
        printf("The number of instances should be a positive number\n");
        return NUM_INSTANCES_NOT_NATURAL;
    }
    
    offset_pt->num_instances = num_instances;
    return 0;
}

/**
 Get the number of replicas of the offset
 
 @param offset_pt offset pointer
 @return number of replicas, error code otherwise
 */
int get_num_replicas(Offset *offset_pt) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL_OFFSET_POINTER;
    }
    
    return offset_pt->num_replicas;
}

/**
 Set the number of replicas of the given frame
 
 @param offset_pt pointer to the offset
 @param num_replicas number of replicas to set
 @return 0 if correct, error code otherwise
 */
int set_replicas(Offset *offset_pt, int num_replicas) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL_OFFSET_POINTER;
    }
    if (num_replicas < 0) {
        printf("The number of replicas should not be negative\n");
        return NUM_REPLICAS_NEGATIVE;
    }
    
    offset_pt->num_replicas = num_replicas;
    return 0;
}

/**
 Gets the number of timeslots that the offset needs to be transmitted
 
 @param offset_pt pointer to the offset
 @return size in timeslots (ns), error code otherwise
 */
long long int get_timeslot_size(Offset *offset_pt) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL_OFFSET_POINTER;
    }
    
    return offset_pt->timeslots;
}

/**
 Set the number of timeslots that the offset needs to be transmitted
 
 @param offset_pt pointer of the offset
 @param size in timeslots (ns)
 @return 0 if correct, error code otherwise
 */
int set_timeslot_size(Offset *offset_pt, int size) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL_OFFSET_POINTER;
    }
    if (size <= 0) {
        printf("The timeslot size should be positive\n");
        return SIZE_TIMESLOT_NOT_NATURAL;
    }
    
    offset_pt->timeslots = size;
    return 0;
}

/**
 Get the offset root of the given frame
 
 @param frame_pt pointer to the frame
 @return offset pointer to the root of the offsets linked list, NULL otherwise
 */
Offset * get_offset_root(Frame *frame_pt) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL;
    }
    
    return frame_pt->offset_ls;
}

/**
 Get the next offset pointer if exists, NULL if it is the last one
 
 @param offset_pt pointer to the offset
 @return next offset pointer, or NULL if is the last offset
 */
Offset * get_next_offset(Offset *offset_pt) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL;
    }
    
    return offset_pt->next_offset_pt;
}

/**
 Checks if it is the last offset of the offsets linked list
 
 @param offset_pt pointer to the offset
 @return 1 if is the last offset, 0 if not, error code if a problem was found
 */
int is_last_offset(Offset *offset_pt) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL_OFFSET_POINTER;
    }
    
    if (offset_pt->next_offset_pt == NULL) {        // If it is the last offset, next is always NULL
        return 1;
    }
    return 0;
}

/**
 For the given Offsets Linked List, adds the offsets if is new.
 If it not new and already present, returns the pointer to the found one
 
 @param offset_pt offset linked list root
 @param link link to find or add
 @return the offset pointer to the offset created or found, NULL if already exists
 */
Offset * add_new_offset(Offset *offset_pt, int link) {
    
    // While there are offsets to search and the link is no the same, we keep searching until the last offset
    while (offset_pt->next_offset_pt != NULL && offset_pt->link != link) {
        offset_pt = offset_pt->next_offset_pt;
    }
    
    // If the link is not in the linked list, we create a new offset and add it, if not we just return the found offset
    if (offset_pt->link != link) {
        offset_pt->g_offset = NULL;
        offset_pt->link = link;
        offset_pt->next_offset_pt = malloc(sizeof(Offset));     // We create the next offset as empty
        offset_pt->next_offset_pt->next_offset_pt = NULL;
        offset_pt->next_offset_pt->link = -1;                   // Just in case to control the link value
        offset_pt->num_instances = 0;
        offset_pt->num_replicas = 0;
        offset_pt->offset = NULL;
        offset_pt->timeslots = 0;
        offset_pt->z_offset = NULL;
    } else {
        return NULL;
    }
    
    return offset_pt;
}

/**
 Get the link of the given offset
 
 @param offset_pt pointer of the offset
 @return the identifier of the link, error code otherwise
 */
int get_offset_link(Offset *offset_pt) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL_OFFSET_POINTER;
    }
    
    return offset_pt->link;
}

/**
 Set the link of the given offset
 
 @param offset_pt pointer of the offset
 @param link link identifier
 @return 0 if correct, error code otherwise
 */
int set_offset_link(Offset *offset_pt, int link) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL_OFFSET_POINTER;
    }
    if (link <= 0) {
        printf("Link identifiers should be positive\n");
        return LINK_OFFSET_NOT_NATURAL;
    }
    
    offset_pt->link = link;
    return 0;
}

/**
 Get a transmission time to the offset of the given Offset
 
 @param offset_pt offset pointer
 @param num_instance number of instance in the offset
 @param num_replica number of replica in the offset
 @return long long integer of the transmission time, error code otherwise
 */
long long int get_offset(Offset *offset_pt, int num_instance, int num_replica) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL_OFFSET_POINTER;
    }
    if (num_instance < 0) {
        printf("The number of instances should not be negative\n");
        return NUM_INSTANCES_NOT_NATURAL;
    }
    if (num_replica < 0) {
        printf("The number of replicas should not be negative\n");
        return NUM_REPLICAS_NEGATIVE;
    }
    
    return offset_pt->offset[num_instance][num_replica];
}

/**
 Set a transmission time to the offset of the given Offset
 
 @param offset_pt offset pointer
 @param num_instance number of instance in the offset
 @param num_replica number of replica in the offset
 @param value transmission time in ns
 @return 0 if correct, error code otherwise
 */
int set_offset(Offset *offset_pt, int num_instance, int num_replica, long long int value) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL_OFFSET_POINTER;
    }
    if (num_instance < 0) {
        printf("The number of instances should not be negative\n");
        return NUM_INSTANCES_NOT_NATURAL;
    }
    if (num_replica < 0) {
        printf("The number of replicas should not be negative\n");
        return NUM_REPLICAS_NEGATIVE;
    }
    if (num_instance >= offset_pt->num_instances) {
        printf("The instance index is out of range\n");
        return NUM_INSTANCES_OUT_RANGE;
    }
    if (num_replica >= offset_pt->num_replicas) {
        printf("The replica index is out of range\n");
        return NUM_REPLICAS_OUT_RANGE;
    }
    if (value < 0) {
        printf("The transmission time cannot be negative\n");
        return TRANSMISSION_TIME_NOT_NATURAL;
    }
    
    offset_pt->offset[num_instance][num_replica] = value;
    return 0;
}

/**
 Get Z3 offset of the given instance and replica
 
 @param offset_pt pointer to the offset
 @param num_instance number
 @param num_replica number
 @return Z3_ast offset constraint of z3, NULL if error ocurred
 */
Z3_ast get_z3_offset(Offset *offset_pt, int num_instance, int num_replica) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL;
    }
    if (num_instance < 0) {
        printf("The number of instances should not be negative\n");
        return NULL;
    }
    if (num_replica < 0) {
        printf("The number of replicas should not be negative\n");
        return NULL;
    }
    
    return offset_pt->z_offset[num_instance][num_replica];
}

/**
 Set a Z3 offset with the given contraint

 @param offset_pt pointer to the offset
 @param num_instance number
 @param num_replica number
 @param z3_constraint z3 constraint to add
 @return 0 if done correctly, error otherwise
 */
int set_z3_offset(Offset *offset_pt, int num_instance, int num_replica, Z3_ast z3_constraint) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL_OFFSET_POINTER;
    }
    if (num_instance < 0) {
        printf("The number of instances should not be negative\n");
        return NUM_INSTANCES_NOT_NATURAL;
    }
    if (num_replica < 0) {
        printf("The number of replicas should not be negative\n");
        return NUM_REPLICAS_OUT_RANGE;
    }
    
    offset_pt->z_offset[num_instance][num_replica] = z3_constraint;
    return 0;
}

/**
 Get gurobi offset of the given instance and replica
 
 @param offset_pt pointer to the offset
 @param num_instance number
 @param num_replica number
 @return long long int offset constraint of gurobi, error code otherwise
 */
int get_gurobi_offset(Offset *offset_pt, int num_instance, int num_replica) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL_OFFSET_POINTER;
    }
    if (num_instance < 0) {
        printf("The number of instances should not be negative\n");
        return NUM_INSTANCES_NOT_NATURAL;
    }
    if (num_replica < 0) {
        printf("The number of replicas should not be negative\n");
        return NUM_REPLICAS_NEGATIVE;
    }
    
    return offset_pt->g_offset[num_instance][num_replica];
}

/**
 Set a Gurobi offset with the given contraint
 
 @param offset_pt pointer to the offset
 @param num_instance number
 @param num_replica number
 @param gurobi_constraint z3 constraint to add
 @return 0 if done correctly, error otherwise
 */
int set_gurobi_offset(Offset *offset_pt, int num_instance, int num_replica, int gurobi_constraint) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL_OFFSET_POINTER;
    }
    if (num_instance < 0) {
        printf("The number of instances should not be negative\n");
        return NUM_INSTANCES_NOT_NATURAL;
    }
    if (num_replica < 0) {
        printf("The number of replicas should not be negative\n");
        return NUM_REPLICAS_OUT_RANGE;
    }
    
    offset_pt->g_offset[num_instance][num_replica] = gurobi_constraint;
    return 0;
}

/**
 Allocates the memory needed and prepare all variables for the used to be ready to be used
 
 @param offset_pt pointer of the offset
 @return 0 if correct, error code otherwise
 */
int prepare_offset(Offset *offset_pt) {
    
    if (offset_pt == NULL) {
        printf("The offset pointer is null\n");
        return NULL_OFFSET_POINTER;
    }
    if (offset_pt->num_instances <= 0 || offset_pt->num_replicas < 0) {
        printf("The offset has not be initialized properly\n");
        return OFFSET_VALUES_NOT_FILLED;
    }
    
    // Dynamically allocate an array for the offsets of size [num_instance][num_replica + 1]
    offset_pt->offset = malloc(sizeof(long long int *) * offset_pt->num_instances);
    offset_pt->z_offset = malloc(sizeof(Z3_ast *) * offset_pt->num_instances);
    offset_pt->g_offset = malloc(sizeof(int *) * offset_pt->num_instances);
    for (int i = 0; i < offset_pt->num_instances; i++) {
        offset_pt->offset[i] = malloc(sizeof(long long int) * (offset_pt->num_replicas));
        offset_pt->z_offset[i] = malloc(sizeof(Z3_ast) * (offset_pt->num_replicas));
        offset_pt->g_offset[i] = malloc(sizeof(int) * (offset_pt->num_replicas));
    }
    return 0;
}

/**
 Get the Offset pointer of a frame with the given link.
 This function is O(1) using a hash table and tries to avoid to find the offset iterating the whole offset linked list
 
 @param frame_pt pointer to the frame
 @param link identifier of the link being search
 @return pointer to the offset of the frame that has the link given in the parameters, NULL if not in the linked list or
 error
 */
Offset * get_frame_offset_by_link(Frame *frame_pt, int link) {
    
    if (frame_pt == NULL) {
        printf("The frame pointer is null\n");
        return NULL;
    }
    
    return frame_pt->offset_hash[link];
}
