/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Network.h                                                                                                          *
 *  Organic Scheduler                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 20/02/18.                                                                             *
 *  Copyright © 2018 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Package that contains the information of the network.                                                              *
 *  A network has the information of all the frames.                                                                   *
 *  All frames are saved in an array.                                                                                  *
 *  Additions of new relations between frames are supposed to be added here, while the behavior is on the schedule,    *
 *  as done with the application constraints, period and deadlines.                                                    *
 *  It also contains function to read the network file and being able to write the schedule of it once it has been     *
 *  filled.                                                                                                            *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef Network_h
#define Network_h

#include <stdio.h>
#include <string.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xpath.h>
#include "Frame.h"
#include "Link.h"

#endif /* Network_h */

/* STRUCT DEFINITIONS */

typedef struct Path {
    int length;
    int *path;
}Path;

/**
 End system receiver that contains a list with all the paths to the end system sender
 */
typedef struct PathReceiver {
    Path *paths;
    int num_paths;
}PathReceiver;

/**
 End system sender that contains a list with all the possible end system receivers
 */
typedef struct PathSender {
    PathReceiver *receivers;
}PathSender;

/* ERROR CODE DEFINITIONS */

#define NUM_FRAMES_NEGATIVE -1
#define NUM_SWITCHES_NEGATIVE -2
#define NUM_END_SYSTEMS_NEGATIVE -3
#define NUM_LINKS_NEGATIVE -4
#define SWITCH_MIN_TIME_NEGATIVE -5
#define PROTOCOL_PERIOD_NEGATIVE -6
#define PROTOCOL_TIME_NEGATIVE -7
#define PROTOCOL_TIME_LARGER_PERIOD -8
#define FRAME_ID_NEGATIVE -9
#define FRAME_DEADLINE_LARGER_PERIOD -10
#define NO_MORE_FRAMES_ALLOCATED -11
#define ERROR_ADDING_FRAME -12
#define UNDEFINED_LINK_TYPE -13
#define NO_MORE_LINKS_ALLOCATED -14
#define ERROR_ADDING_LINK -15
#define READ_GENERAL_INFORMATION_ERROR -101
#define READ_FRAMES_ERROR -102
#define READ_TOPOLOGY_ERROR -103
#define PARSE_NETWORK_ERROR -104
#define NETWORK_FILE_NOT_FOUND -201
#define NO_NUM_FRAMES_FOUND -202
#define NO_NUM_SWITCHES_FOUND -203
#define NO_NUM_END_SYSTEMS_FOUND -204
#define NO_NUM_LINKS_FOUND -205
#define NO_SWITCH_MIN_TIME -206
#define NO_PROTOCOL_PERIOD -207
#define NO_PROTOCOL_TIME -208
#define NO_FRAMES_FOUND -209
#define NO_FRAME_ID_FOUND -210
#define NO_FRAME_PERIOD_FOUND -211
#define NO_FRAME_DEADLINE_FOUND -212
#define NO_FRAME_SIZE_FOUND -213
#define NO_FRAME_STARTING_TIME_FOUND -214
#define NO_FRAME_END_TO_END_FOUND -215
#define NO_LINK_FOUND -216
#define NO_LINK_ID_FOUND -217
#define NO_LINK_SPEED_FOUND -218
#define NO_LINK_TYPE_FOUND -219
#define NO_PATHS_FOUND -220
#define NO_PATH_SENDER_ID_FOUND -221
#define NO_PATH_RECEIVERS_FOUND -222
#define NO_PATH_RECEIVER_ID_FOUND -223
#define NO_PATH_FOUND -224
#define PATH_DOES_NOT_EXIST -225
#define NO_NODES_FOUND -226
#define NO_NODE_ID_FOUND -227
#define UNDEFINED_NODE_TYPE -228
#define NO_FRAME_SENDER_ID_FOUND -229
#define NO_FRAME_RECEIVER_ID_FOUND -230
#define NO_PERIODS -231


/**
 Get the number of frames in the network

 @return number of frames in the network
 */
int get_num_frames(void);

/**
 Set the number of frames in the network and reserve memory for the frames

 @param num_frames number of frames in the network
 @return 0 if successful, error code otherwise
 */
int set_num_frames(int num_frames);

/**
 Get the number of switches in the network
 
 @return number of switches in the network
 */
int get_num_switches(void);

/**
 Set the number of switches in the network
 
 @param num_switches number of switches in the network
 @return 0 if successful, error code otherwise
 */
int set_num_switches(int num_switches);

/**
 Get the number of end systems in the network
 
 @return number of end systems in the network
 */
int get_num_end_systems(void);

/**
 Set the number of end systems in the network
 
 @param num_end_systems number of end systems in the network
 @return 0 if successful, error code otherwise
 */
int set_num_end_systems(int num_end_systems);

/**
 Get the number of links in the network
 
 @return number of links in the network
 */
int get_num_links(void);

/**
 Set the number of links in the network and reserve memory for the links
 
 @param num_links number of links in the network
 @return 0 if successful, error code otherwise
 */
int set_num_links(int num_links);

/**
 Get the switch minimum time in ns

 @return switch minimum time in ns
 */
long long int get_switch_minimum_time(void);

/**
 Set the switch minimum time in ns

 @param min_time switch minimum time in ns
 @return 0 if successful, error code otherwise
 */
long long int set_switch_minimum_time(long long int min_time);

/**
 Get the self-healing protocol period in ns
 
 @return the self-healing protocol period in ns
 */
long long int get_protocol_period(void);

/**
 Set the self-healing protocol period in ns
 
 @param period self-healing protocol period in ns
 @return 0 if successful, error code otherwise
 */
long long int set_protocol_period(long long int period);

/**
 Get the self-healing protocol time in ns
 
 @return the self-healing protocol time in ns
 */
long long int get_protocol_time(void);

/**
 Set the self-healing protocol time in ns
 
 @param time self-healing time period in ns
 @return 0 if successful, error code otherwise
 */
long long int set_protocol_time(long long int time);

/**
 Get the frame pointer given the frame id
 
 @param frame_id integer with the frame identifier
 @return pointer of the frame
 */
Frame * get_frame(int frame_id);

/**
 Adds to the given index frame the general information of the period, deadline and size in the frame array
 
 @param frame_id index of the frame to add information from the frame array
 @param period long long int of the period in ns
 @param deadline long long int of the deadline in ns
 @param size int of the size in bytes
 @param starting_time long long int of the starting time of the frame in ns
 @param end_to_end long long int of the end to end delay in ns
 @param sender_id sender identifier
 @param receivers_id pointer to the array of receivers id
 @param num_receivers number of receivers in the array
 @return 0 if done correctly, error code if index out of array of frames
 */
int add_frame_information(int frame_id, long long int period, long long int deadline, int size,
                          long long int starting_time, long long int end_to_end, int sender_id, int *receivers_id,
                          int num_receivers);

/**
 Get the frame pointer given the link id
 
 @param link_id integer with the link identifier
 @return pointer of the link
 */
Link * get_link(int link_id);

/**
 Add a link information to the link array
 
 @param link_id index of the link
 @param speed integer with the speed of the link in MB/s
 @param link_type type of the link (wired or wireless)
 @return 0 if added correctly, -1 if out of index
 */
int add_link(int link_id, int speed, LinkType link_type);

/**
 Allocate all the needed memory to store the paths for the number of end systems

 @return 0 if done correctly, error code otherwise
 */
int init_path_structure(void);

/**
 Get the number of possible paths that connect the given sender and receiver

 @param sender_id sender end system id
 @param receiver_id receiver end system id
 @return the number of possible paths, error code otherwise
 */
int get_num_paths(int sender_id, int receiver_id);

/**
 Get the path given the path id for the one end system to another

 @param sender_id end system sender id
 @param receiver_id end system receiver id
 @param path_id path id
 @return the path pointer, null if error occur
 */
Path * get_path(int sender_id, int receiver_id, int path_id);

/**
 Add a new path from the sender end system to the receiver end system

 @param sender_id sender end system id
 @param receiver_id receiver end system id
 @param path pointer to the array that contains the path
 @param len_path number of links in the path
 @return 0 if done correctly, error code otherwise
 */
int add_path(int sender_id, int receiver_id, int* path, int len_path);

/**
 Get the hyper_period of the network

 @return hyper_period of the network
 */
long long int get_hyper_period(void);

/**
 Get the utilization of the link with the maximum utilization

 @return maximum utilization in any link
 */
float get_max_link_utilization(void);

/**
 Init all the needed variables in the network to start the scheduling, such as frame appearances, instances and similar
 */
void initialize_network(void);

/* INPUT OUTPUT FUNCTIONS */

/**
 Reads the given network xml file and parse everything into the network variables.
 It starts reading the general information of the network.
 Then continues with the important information from the network components description (links and its speeds).
 It also reads all the possible paths from different nodes.
 It ends with the information of each frame.
 
 @param filename name of the xml input file
 @return 0 if correctly read, error code otherwise
 */
int parse_network_xml(char *filename);

/**
 Write the obtained schedule in a XML file with all the paths and transmission times of all the frames
 
 @param namefile path and name of the xml file to create with the written schedule
 @return 0 if correctly written, -1 otherwise
 */
int write_schedule_xml(char* namefile);
