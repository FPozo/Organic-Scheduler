/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Network.c                                                                                                          *
 *  Organic Scheduler                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 20/02/18.                                                                             *
 *  Copyright © 2018 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Description in Network.h                                                                                           *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "Network.h"

/* VARIABLES */

long long int protocol_time = 0;            // Self-Healing Protocol time in ns
long long int protocol_period = 0;          // Self-Healing Protocol period in ns
long long int switch_minimum_time = 0;      // Minimum time in ns that a frame has to stay in the switch when received
int number_frames = 0;                      // Number of frames in the network
int number_switches = 0;                    // Number of switches in the network
int number_end_systems = 0;                 // Number of end systems in the network
int number_links = 0;                       // Number of links in the network
Frame *frames;                              // Array with all the frames in the network
Link *links;                                // Array with all the links in the network
PathSender *paths;                          // Array of end systems Sender structs to all other possible end systems
int *end_systems_hash;                      // Array that given the node id, matches the end system in the "paths" array

/* PRIVATE FUNCTIONS */

/**
 Read the switches information of the network from the given xml tree pointer

 @param file_network pointr to the top of the network xml tree
 @return 0 if correctly read, error code otherwise
 */
int read_switch_information_xml(xmlDocPtr file_network) {
    
    // Init xml variables needed to search information in the file
    xmlChar *value;
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;
    
    context = xmlXPathNewContext(file_network);
    
    // Search the minimum switch time in the network and save it
    result = xmlXPathEvalExpression((xmlChar*) "/Network/General_Information/Switch_Information/Minimum_Time", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Network xml file is wrongly constructed, no switch Minimum_Time found\n");
        return NO_SWITCH_MIN_TIME;
    }
    value = xmlNodeListGetString(file_network, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    set_switch_minimum_time(atoll((const char*) value));
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    
    xmlXPathFreeContext(context);
    return 0;
}

/**
 Read the self-healing protocol information of the network from the given xml tree pointer

 @param file_network pointr to the top of the network xml tree
 @return 0 if correctly read, error code otherwise
 */
int read_self_healing_protocol_information_xml(xmlDocPtr file_network) {
    
    // Init xml variables needed to search information in the file
    xmlChar *value;
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;
    
    context = xmlXPathNewContext(file_network);
    
    // Search the self-healing protocol period in the network and save it
    result = xmlXPathEvalExpression((xmlChar*) "/Network/General_Information/Self-Healing_Protocol/Period", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Network xml file is wrongly constructed, no self-healing protocol Period found\n");
        return NO_PROTOCOL_PERIOD;
    }
    value = xmlNodeListGetString(file_network, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    set_protocol_period(atoll((const char*) value));
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    
    // Search the self-healing protocol time in the network and save it
    result = xmlXPathEvalExpression((xmlChar*) "/Network/General_Information/Self-Healing_Protocol/Time", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Network xml file is wrongly constructed, no self-healing protocol Time found\n");
        return NO_PROTOCOL_TIME;
    }
    value = xmlNodeListGetString(file_network, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    set_protocol_time(atoll((const char*) value));
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    
    xmlXPathFreeContext(context);
    return 0;
}

/**
 Read the general information of the network from the given xml tree pointer
 
 @param file_network pointer to the top of the network xml tree
 @return 0 if correctly read, error code otherwise
 */
int read_general_information_xml(xmlDocPtr file_network) {
    
    // Init xml variables needed to search information in the file
    xmlChar *value;
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;
    
    context = xmlXPathNewContext(file_network);
    
    // Search the number of frames in the network and save it
    result = xmlXPathEvalExpression((xmlChar*) "/Network/General_Information/Number_Frames", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Network xml file is wrongly constructed, no Number_Frames found\n");
        return NO_NUM_FRAMES_FOUND;
    }
    value = xmlNodeListGetString(file_network, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    set_num_frames(atoi((const char*) value));
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    
    // Search the number of switches in the network and save it
    result = xmlXPathEvalExpression((xmlChar*) "/Network/General_Information/Number_Switches", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Network xml file is wrongly constructed, no Number_Switches found\n");
        return NO_NUM_SWITCHES_FOUND;
    }
    value = xmlNodeListGetString(file_network, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    set_num_switches(atoi((const char*) value));
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    
    // Search the number of end systems in the network and save it
    result = xmlXPathEvalExpression((xmlChar*) "/Network/General_Information/Number_End_Systems", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Network xml file is wrongly constructed, no Number_End_Systems found\n");
        return NO_NUM_END_SYSTEMS_FOUND;
    }
    value = xmlNodeListGetString(file_network, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    set_num_end_systems(atoi((const char*) value));
    init_path_structure();      // Now that we now the number of end systems, init the path structure
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    
    // Once we have the number of switches and end systems, we can initalize the end systems hash accelerator
    end_systems_hash = malloc(sizeof(int) * (number_switches + number_end_systems));
    
    // Search the number of links in the network and save it
    result = xmlXPathEvalExpression((xmlChar*) "/Network/General_Information/Number_Links", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Network xml file is wrongly constructed, no Number_Links found\n");
        return NO_NUM_LINKS_FOUND;
    }
    value = xmlNodeListGetString(file_network, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    set_num_links(atoi((const char*) value));
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    
    if (read_switch_information_xml(file_network) < 0) {
        printf("Error reading the switch information\n");
        return READ_GENERAL_INFORMATION_ERROR;
    }
    if (read_self_healing_protocol_information_xml(file_network) < 0) {
        printf("Error reading the self-healing protocol information\n");
        return READ_GENERAL_INFORMATION_ERROR;
    }
    
    xmlXPathFreeContext(context);
    return 0;
}

/**
 Read and store the paths information of the network from the given xml tree pointer

 @param file_network pointer to the top of the network xml tree
 @return 0 if correctly read, error code otherwise
 */
int read_paths_information_xml(xmlDocPtr file_network) {
    
    // Init xml variables needed to search information in the file
    xmlChar *value;
    xmlXPathContextPtr context, context_sender, context_receiver = NULL;
    xmlXPathObjectPtr result, result_sender, result_receiver = NULL;
    
    // Init variables that to save the information of the paths
    int sender_id, receiver_id, link_char_it;
    char *link_char;
    int* path_array = NULL;
    
    // Seach on the xml tree where the paths are stored
    context = xmlXPathNewContext(file_network);
    result = xmlXPathEvalExpression((xmlChar*) "/Network/Topology/Paths/Sender", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Network xml file is wrongly constructed, no Paths found\n");
        return NO_PATHS_FOUND;
    }
    
    for (int sender_it = 0; sender_it < result->nodesetval->nodeNr; sender_it++) {
        context_sender = xmlXPathNewContext(file_network);
        xmlXPathSetContextNode(result->nodesetval->nodeTab[sender_it], context_sender);
        
        // Search for the sender end system id
        result_sender = xmlXPathEvalExpression((xmlChar*) "SenderID", context_sender);
        if (result_sender->nodesetval->nodeTab == NULL) {
            printf("The Network xml file is wrongly cobtructed, no SenderID found\n");
            return NO_PATH_SENDER_ID_FOUND;
        }
        value = xmlNodeListGetString(file_network, result_sender->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        sender_id = atoi((const char*) value);
        // Free xml structures
        xmlFree(value);
        xmlXPathFreeObject(result_sender);
        
        // Search for all the possible receivers for the current end system
        result_sender = xmlXPathEvalExpression((xmlChar*) "Receivers/Receiver", context_sender);
        if (result_sender->nodesetval->nodeTab == NULL) {
            printf("The Network xml file is wrongly constructed, no paths Receiver found\n");
            return NO_PATH_RECEIVERS_FOUND;
        }
        
        for (int receiver_it = 0; receiver_it < result_sender->nodesetval->nodeNr; receiver_it++) {
            context_receiver = xmlXPathNewContext(file_network);
            xmlXPathSetContextNode(result_sender->nodesetval->nodeTab[receiver_it], context_receiver);
            
            // Search for the receiver end system id
            result_receiver = xmlXPathEvalExpression((xmlChar*) "ReceiverID", context_receiver);
            if (result_receiver->nodesetval->nodeTab == NULL) {
                printf("The Network xml file is wrongly cobtructed, no ReceiverID found\n");
                return NO_PATH_RECEIVER_ID_FOUND;
            }
            value = xmlNodeListGetString(file_network, result_receiver->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
            receiver_id = atoi((const char*) value);
            // Free xml structures
            xmlFree(value);
            xmlXPathFreeObject(result_receiver);
            
            // Search for all the possible paths to connect the sender and receiver
            result_receiver = xmlXPathEvalExpression((xmlChar*) "Paths/Path", context_receiver);
            if (result_receiver->nodesetval->nodeTab == NULL) {
                printf("The Network xml file is wrongly cobtructed, no Path found\n");
                return NO_PATH_FOUND;
            }
            
            for (int path_it = 0; path_it < result_receiver->nodesetval->nodeNr; path_it++) {
                
                value = xmlNodeListGetString(file_network,
                                             result_receiver->nodesetval->nodeTab[path_it]->xmlChildrenNode, 1);
                // Parse the string into an array and save it into the frame
                link_char = strtok((char*) value, ";");
                link_char_it = 0;
                path_array = NULL;
                while (link_char != NULL) {
                    path_array = realloc(path_array, sizeof(int) * (link_char_it + 1));
                    path_array[link_char_it] = atoi(link_char);
                    link_char = strtok(NULL, ";");
                    link_char_it++;
                }
                add_path(sender_id, receiver_id, path_array, link_char_it);
                
                free(path_array);
                xmlFree(value);
            }
            
            xmlXPathFreeObject(result_receiver);
            xmlXPathFreeContext(context_receiver);
        }
        
        xmlXPathFreeObject(result_sender);
        xmlXPathFreeContext(context_sender);
    }
    
    xmlXPathFreeContext(context);
    xmlXPathFreeObject(result);
    return 0;
}

/**
 Read and store the link information of the network from the given xml tree pointer

 @param file_network pointer to the top of the network xml tree
 @return 0 if correctly read, error code otherwise
 */
int read_links_information_xml(xmlDocPtr file_network) {
    
    // Init xml variables needed to search information in the file
    xmlChar *value;
    xmlXPathContextPtr context, context_link = NULL;
    xmlXPathObjectPtr result, result_link = NULL;
    
    // Init variables to save the value of the link found
    int speed, link_id;
    LinkType link_type;
    
    // Seach on the xml tree where the links are stored
    context = xmlXPathNewContext(file_network);
    result = xmlXPathEvalExpression((xmlChar*) "/Network/Topology/Links/Link", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Network xml file is wrongly constructed, no Links found\n");
        return NO_LINK_FOUND;
    }
    
    for (int link_it = 0; link_it < result->nodesetval->nodeNr; link_it++) {
        context_link = xmlXPathNewContext(file_network);
        xmlXPathSetContextNode(result->nodesetval->nodeTab[link_it], context_link);
        
        // Search the link id of the current link
        result_link = xmlXPathEvalExpression((xmlChar*) "LinkID", context_link);
        if (result_link->nodesetval->nodeTab == NULL) {
            printf("The Network xml file is wrongly constructed, no link LinkID found\n");
            return NO_LINK_ID_FOUND;
        }
        value = xmlNodeListGetString(file_network, result_link->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        link_id = atoi((const char*) value);
        // Free xml structures
        xmlFree(value);
        xmlXPathFreeObject(result_link);
        
        // Search the category of the current link
        value = xmlGetProp(result->nodesetval->nodeTab[link_it], (xmlChar*) "category");
        if (xmlStrcmp(value, (xmlChar*) "LinkType.wired") == 0) {
            link_type = wired;
        } else if (xmlStrcmp(value, (xmlChar*) "LinkType.wireless") == 0) {
            link_type = wireless;
        } else {
            printf("The link has a unknown category\n");
            return UNDEFINED_LINK_TYPE;
        }
        xmlFree(value);
        
        // Search the link id of the current link
        result_link = xmlXPathEvalExpression((xmlChar*) "Speed", context_link);
        if (result_link->nodesetval->nodeTab == NULL) {
            printf("The Network xml file is wrongly cobtructed, no link Speed found\n");
            return NO_LINK_SPEED_FOUND;
        }
        value = xmlNodeListGetString(file_network, result_link->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        speed = atoi((const char*) value);
        // Free xml structures
        xmlFree(value);
        xmlXPathFreeObject(result_link);
        
        add_link(link_id, speed, link_type);
        
        xmlXPathFreeContext(context_link);
    }
    
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    return 0;
}

/**
 Read and store the nodes information of the network from the given xml tree pointer.
 From now we only see which nodes are end systems to initialize the end system accelerator hash

 @param file_network pointer to the top of the network xml tree
 @return 0 if correctly read, error code otherwise
 */
int read_nodes_information_xml(xmlDocPtr file_network) {
    
    // Init xml variables needed to search information in the file
    xmlChar *value;
    xmlXPathContextPtr context, context_node = NULL;
    xmlXPathObjectPtr result, result_node = NULL;
    
    // Init variables for the nodes
    int node_id, end_system_it = 0;
    
    // Seach on the xml tree where the nodes are stored
    context = xmlXPathNewContext(file_network);
    result = xmlXPathEvalExpression((xmlChar*) "/Network/Topology/Nodes/Node", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Network xml file is wrongly constructed, no Nodes found\n");
        return NO_NODES_FOUND;
    }
    
    for (int node_it = 0; node_it < result->nodesetval->nodeNr; node_it++) {
        context_node = xmlXPathNewContext(file_network);
        xmlXPathSetContextNode(result->nodesetval->nodeTab[node_it], context_node);
        
        // Search the link id of the current link
        result_node = xmlXPathEvalExpression((xmlChar*) "NodeID", context_node);
        if (result_node->nodesetval->nodeTab == NULL) {
            printf("The Network xml file is wrongly constructed, no node NodeID found\n");
            return NO_NODE_ID_FOUND;
        }
        value = xmlNodeListGetString(file_network, result_node->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        node_id = atoi((const char*) value);
        // Free xml structures
        xmlFree(value);
        xmlXPathFreeObject(result_node);
        
        // Search the category of the current node
        value = xmlGetProp(result->nodesetval->nodeTab[node_it], (xmlChar*) "category");
        if (xmlStrcmp(value, (xmlChar*) "end_system") == 0) {   // If it is an end system, add it to the hash
            end_systems_hash[node_id] = end_system_it;
            end_system_it++;
        } else if (xmlStrcmp(value, (xmlChar*) "switch") == 0) {
            NULL;
        } else {
            printf("The node has a unknown category\n");
            return UNDEFINED_NODE_TYPE;
        }
        xmlFree(value);
        
        xmlXPathFreeContext(context_node);
        
    }
    
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    return 0;
}

/**
 Read and store the topology information of the network from the given xml tree pointer
 For now includes the link information and all the possible paths, it might include the nodes information and
 connections in a future if they are needed (probably for the protocol)

 @param file_network pointer to the top of the network xml tree
 @return 0 if correctly read, error code otherwise
 */
int read_topology_information_xml(xmlDocPtr file_network) {
    
    if (read_nodes_information_xml(file_network) < 0) {
        printf("Error reading the nodes information\n");
        return READ_TOPOLOGY_ERROR;
    }
    if (read_links_information_xml(file_network) < 0) {
        printf("Error reading the links information\n");
        return READ_TOPOLOGY_ERROR;
    }
    if (read_paths_information_xml(file_network) < 0) {
        printf("Error reading the paths information\n");
        return READ_TOPOLOGY_ERROR;
    }
    
    return 0;
}

/**
 Read and store all the frames information of the network from the given xml tree pointer
 
 @param file_network pointer to the top of the network xml tree
 @return 0 if correctly read, error code otherwise
 */
int read_frames_information_xml(xmlDocPtr file_network) {
    
    // Init xml variables needed to search information in the file
    xmlChar *value;
    xmlXPathContextPtr context, context_frame;
    xmlXPathObjectPtr result, result_frame;
    
    // Init variables for the general information of a frame
    long long int period, deadline, end_to_end, starting_time;
    int frame_id, size;
    
    context = xmlXPathNewContext(file_network);
    result = xmlXPathEvalExpression((xmlChar*) "/Network/Frames/Frame", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Network xml file is wrongly cobtructed, no FRAMES found\n");
        return NO_FRAMES_FOUND;
    }
    
    // Iterate over all frames on the network file
    for (int frame_it = 0; frame_it < result->nodesetval->nodeNr; frame_it++) {
        
        // Set the context to the actual frame
        context_frame = xmlXPathNewContext(file_network);
        xmlXPathSetContextNode(result->nodesetval->nodeTab[frame_it], context_frame);
        
        // Search the period of the current frame
        result_frame = xmlXPathEvalExpression((xmlChar*) "FrameID", context_frame);
        if (result_frame->nodesetval->nodeTab == NULL) {
            printf("The Network xml file is wrongly cobtructed, no frame FrameID found\n");
            return NO_FRAME_ID_FOUND;
        }
        value = xmlNodeListGetString(file_network, result_frame->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        frame_id = atoi((const char*) value);
        // Free xml structures
        xmlFree(value);
        xmlXPathFreeObject(result_frame);
        
        // Search the period of the current frame
        result_frame = xmlXPathEvalExpression((xmlChar*) "Period", context_frame);
        if (result_frame->nodesetval->nodeTab == NULL) {
            printf("The Network xml file is wrongly cobtructed, no frame PERIOD found\n");
            return NO_FRAME_PERIOD_FOUND;
        }
        value = xmlNodeListGetString(file_network, result_frame->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        period = atoll((const char*) value);
        // Free xml structures
        xmlFree(value);
        xmlXPathFreeObject(result_frame);
        
        // Search the deadline of the current frame
        result_frame = xmlXPathEvalExpression((xmlChar*) "Deadline", context_frame);
        if (result_frame->nodesetval->nodeTab == NULL) {
            printf("The Network xml file is wrongly cobtructed, no frame DEADLINE found\n");
            return NO_FRAME_DEADLINE_FOUND;
        }
        value = xmlNodeListGetString(file_network, result_frame->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        deadline = atoll((const char*) value);
        // Free xml structures
        xmlFree(value);
        xmlXPathFreeObject(result_frame);
        
        // Search the size of the current frame
        result_frame = xmlXPathEvalExpression((xmlChar*) "Size", context_frame);
        if (result_frame->nodesetval->nodeTab == NULL) {
            printf("The Network xml file is wrongly cobtructed, no frame Size found\n");
            return NO_FRAME_SIZE_FOUND;
        }
        value = xmlNodeListGetString(file_network, result_frame->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        size = atoi((const char*) value);
        // Free xml structures
        xmlFree(value);
        xmlXPathFreeObject(result_frame);
        
        // Search the starting time of the current frame
        result_frame = xmlXPathEvalExpression((xmlChar*) "StartingTime", context_frame);
        if (result_frame->nodesetval->nodeTab == NULL) {
            printf("The Network xml file is wrongly cobtructed, no frame StartingTime found\n");
            return NO_FRAME_STARTING_TIME_FOUND;
        }
        value = xmlNodeListGetString(file_network, result_frame->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        starting_time = atoi((const char*) value);
        // Free xml structures
        xmlFree(value);
        xmlXPathFreeObject(result_frame);
        
        // Search the end to end delay of the current frame
        result_frame = xmlXPathEvalExpression((xmlChar*) "EndToEnd", context_frame);
        if (result_frame->nodesetval->nodeTab == NULL) {
            printf("The Network xml file is wrongly cobtructed, no frame EndToEnd found\n");
            return NO_FRAME_END_TO_END_FOUND;
        }
        value = xmlNodeListGetString(file_network, result_frame->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        end_to_end = atoi((const char*) value);
        // Free xml structures
        xmlFree(value);
        xmlXPathFreeObject(result_frame);
        
        add_frame_information(frame_id, period, deadline, size, starting_time, end_to_end);
        
        xmlXPathFreeContext(context_frame);
    }
    
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    return 0;
}

/* PUBLIC FUNCTIONS */

/**
 Get the number of frames in the network
 
 @return number of frames in the network
 */
int get_num_frames(void) {
    
    return number_frames;
}

/**
 Set the number of frames in the network and reserve memory for the frames
 
 @param num_frames number of frames in the network
 @return 0 if successful, error code otherwise
 */
int set_num_frames(int num_frames) {
    
    if (num_frames <= 0) {
        printf("The number of frames cannot be negative\n");
        return NUM_FRAMES_NEGATIVE;
    }
    number_frames = num_frames;
    frames = malloc(sizeof(Frame) * num_frames);        // Init the array of frames now that we now the number
    return 0;
}

/**
 Get the number of switches in the network
 
 @return number of switches in the network
 */
int get_num_switches(void) {
    
    return number_switches;
}

/**
 Set the number of switches in the network
 
 @param num_switches number of switches in the network
 @return 0 if successful, error code otherwise
 */
int set_num_switches(int num_switches) {
    
    if (num_switches <= 0) {
        printf("The number of switches cannot be negative\n");
        return NUM_SWITCHES_NEGATIVE;
    }
    number_switches = num_switches;
    return 0;
}

/**
 Get the number of end systems in the network
 
 @return number of end systems in the network
 */
int get_num_end_systems(void) {
    
    return number_end_systems;
}

/**
 Set the number of end systems in the network
 
 @param num_end_systems number of end systems in the network
 @return 0 if successful, error code otherwise
 */
int set_num_end_systems(int num_end_systems) {
    
    if (num_end_systems <= 0) {
        printf("The number of end systems cannot be negative\n");
        return NUM_END_SYSTEMS_NEGATIVE;
    }
    number_end_systems = num_end_systems;
    return 0;
}

/**
 Get the number of links in the network
 
 @return number of links in the network
 */
int get_num_links(void) {
    
    return number_links;
}

/**
 Set the number of links in the network and reserve memory for the links
 
 @param num_links number of links in the network
 @return 0 if successful, error code otherwise
 */
int set_num_links(int num_links) {
    
    if (num_links <= 0) {
        printf("The number of links cannot be negative\n");
        return NUM_LINKS_NEGATIVE;
    }
    number_links = num_links;
    links = malloc(sizeof(Link) * num_links);               // Init the array of links now that we now the number
    return 0;
}

/**
 Get the switch minimum time in ns
 
 @return switch minimum time in ns
 */
long long int get_switch_minimum_time(void) {
    
    return switch_minimum_time;
}

/**
 Set the switch minimum time in ns
 
 @param min_time switch minimum time in ns
 @return 0 if successful, error code otherwise
 */
long long int set_switch_minimum_time(long long int min_time) {
    
    if (min_time < 0) {
        printf("The switch minimum time cannot be negative\n");
        return SWITCH_MIN_TIME_NEGATIVE;
    }
    switch_minimum_time = min_time;
    return 0;
}

/**
 Get the self-healing protocol period in ns
 
 @return the self-healing protocol period in ns
 */
long long int get_protocol_period(void) {
    
    return protocol_period;
}

/**
 Set the self-healing protocol period in ns
 
 @param period self-healing protocol period in ns
 @return 0 if successful, error code otherwise
 */
long long int set_protocol_period(long long int period) {
    
    if (period <= 0) {
        printf("The self-healing protocol period cannot be negative\n");
        return PROTOCOL_PERIOD_NEGATIVE;
    }
    protocol_period = period;
    return 0;
}

/**
 Get the self-healing protocol time in ns
 
 @return the self-healing protocol time in ns
 */
long long int get_protocol_time(void) {
    
    return protocol_time;
}

/**
 Set the self-healing protocol time in ns
 
 @param time self-healing time period in ns
 @return 0 if successful, error code otherwise
 */
long long int set_protocol_time(long long int time) {
    
    if (time <= 0) {
        printf("The self-healing protocol time cannot be negative\n");
        return PROTOCOL_TIME_NEGATIVE;
    }
    if (time >= protocol_period) {
        printf("The self-healing protocol time cannot be larger then the period\n");
        return PROTOCOL_TIME_LARGER_PERIOD;
    }
    protocol_time = time;
    return 0;
}

/**
 Get the frame pointer given the frame id
 
 @param frame_id integer with the frame identifier
 @return pointer of the frame
 */
Frame * get_frame(int frame_id) {
    
    return &frames[frame_id];
}

/**
 Adds to the given index frame the general information of the period, deadline and size in the frame array
 
 @param frame_id index of the frame to add information from the frame array
 @param period long long int of the period in ns
 @param deadline long long int of the deadline in ns
 @param size int of the size in bytes
 @param starting_time long long int of the starting time of the frame in ns
 @param end_to_end long long int of the end to end delay in ns
 @return 0 if done correctly, -1 if index out of array of frames
 */
int add_frame_information(int frame_id, long long int period, long long int deadline, int size,
                          long long int starting_time, long long int end_to_end) {
    
    // Check if we allocated memory for the new frame
    if (frame_id >= number_frames) {
        printf("There are more frames that the stated in the network\n");
        return NO_MORE_FRAMES_ALLOCATED;
    }
    
    // Save all the information
    if (set_period(&frames[frame_id], period) < 0) {
        printf("Error adding the frame period\n");
        return ERROR_ADDING_FRAME;
    }
    if (set_deadline(&frames[frame_id], deadline) < 0) {
        printf("Error adding the frame deadline\n");
        return ERROR_ADDING_FRAME;
    }
    if (set_size(&frames[frame_id], size) < 0) {
        printf("Error adding the frame size\n");
        return ERROR_ADDING_FRAME;
    }
    if (set_starting(&frames[frame_id], starting_time)) {
        printf("Error adding the frame starting time\n");
        return ERROR_ADDING_FRAME;
    }
    if (set_end_to_end_delay(&frames[frame_id], end_to_end)) {
        printf("Error adding the frame end to end delay\n");
        return ERROR_ADDING_FRAME;
    }
    
    // Init also the hash array to accelerate the search of link offsets
    init_hash(&frames[frame_id], number_links);
    return 0;
}

/**
 Get the frame pointer given the link id
 
 @param link_id integer with the link identifier
 @return pointer of the link
 */
Link * get_link(int link_id) {
    
    return &links[link_id];
}

/**
 Add a link information to the link array
 
 @param link_id index of the link
 @param speed integer with the speed of the link in MB/s
 @param link_type type of the link (wired or wireless)
 @return 0 if added correctly, -1 if out of index
 */
int add_link(int link_id, int speed, LinkType link_type) {
    
    // Check if we allocated mmory for the new link
    if (link_id >= number_links) {
        printf("There are more links that the stated in the network\n");
        return NO_MORE_LINKS_ALLOCATED;
    }
    
    // Save all the information
    if (set_link_type(&links[link_id], link_type) < 0) {
        printf("Error adding the link type\n");
        return ERROR_ADDING_LINK;
    }
    if (set_link_speed(&links[link_id], speed) < 0) {
        printf("Error adding the link speed\n");
        return ERROR_ADDING_LINK;
    }
    
    return 0;
}

/**
 Allocate all the needed memory to store the paths for the given number of end systems
 
 @return 0 if done correctly, error code otherwise
 */
int init_path_structure(void) {
    
    // Check if there exist end systems in the network
    if (number_end_systems <= 0) {
        printf("The path structure cannot be created with 0 or less end systems\n");
        return NUM_END_SYSTEMS_NEGATIVE;
    }
    
    // Allocate memory for every sender, and in every sender for every receiver
    paths = malloc(sizeof(PathSender) * number_end_systems);
    for (int sender_it = 0; sender_it < number_end_systems; sender_it++) {
        paths[sender_it].receivers = malloc(sizeof(PathReceiver) * number_end_systems);
        for (int receiver_it = 0; receiver_it < number_end_systems; receiver_it++) {
            paths[sender_it].receivers[receiver_it].num_paths = 0;
            paths[sender_it].receivers[receiver_it].paths = NULL;
        }
    }
    
    return 0;
}

/**
 Get the path given the path id for the one end system to another
 
 @param sender_id end system sender id
 @param receiver_id end system receiver id
 @param path_id path id
 @return the path pointer, null if error occur
 */
Path * get_path(int sender_id, int receiver_id, int path_id) {
    
    int sender_pos, receiver_pos;
    
    // Check if the ids are in range
    if (sender_id < 0 ||sender_id >= number_end_systems || receiver_id < 0 || receiver_id >= number_end_systems) {
        printf("The path does not exist\n");
        return NULL;
    }
    
    // Convert the given ids to the positions in the path structure
    sender_pos = end_systems_hash[sender_id];
    receiver_pos = end_systems_hash[receiver_id];
    if (paths[sender_pos].receivers[receiver_pos].num_paths >= path_id) {
        printf("The path does not exist, there are not that many paths between both end systems\n");
        return NULL;
    }
    return &paths[sender_pos].receivers[sender_pos].paths[path_id];
}

/**
 Add a new path from the sender end system to the receiver end system
 
 @param sender_id sender end system id
 @param receiver_id receiver end system id
 @param path pointer to the array that contains the path
 @param len_path number of links in the path
 @return 0 if done correctly, error code otherwise
 */
int add_path(int sender_id, int receiver_id, int* path, int len_path) {
    
    int sender_pos, receiver_pos, num_paths;
    
    // Check if the ids are in range
    if (sender_id < 0 ||sender_id >= (number_end_systems + number_switches) || receiver_id < 0 ||
        receiver_id >= (number_end_systems + number_switches) || len_path <= 0) {
        printf("The path does not exist\n");
        return PATH_DOES_NOT_EXIST;
    }
    // Convert the given ids to the positions in the path structure
    sender_pos = end_systems_hash[sender_id];
    receiver_pos = end_systems_hash[receiver_id];
    
    // Add the path, first allocate memory for the next path, then allocate memory for the array of link ids, and save
    // every link one by one
    paths[sender_pos].receivers[receiver_pos].num_paths++;
    num_paths = paths[sender_pos].receivers[receiver_pos].num_paths;
    paths[sender_pos].receivers[receiver_pos].paths = realloc(paths[sender_pos].receivers[receiver_pos].paths,
                                                              sizeof(Path) * num_paths);
    paths[sender_pos].receivers[receiver_pos].paths[num_paths - 1].length = len_path;
    paths[sender_pos].receivers[receiver_pos].paths[num_paths - 1].path = malloc(sizeof(int) * len_path);
    for (int i = 0; i < len_path; i++) {
        paths[sender_pos].receivers[receiver_pos].paths[num_paths - 1].path[i] = path[i];
    }
    return 0;
}

/**
 Reads the given network xml file and parse everything into the network variables.
 It starts reading the general information of the network.
 Then continues with the important information from the network components description (links and its speeds).
 It also reads all the possible paths from different nodes.
 It ends with the information of each frame.
 
 @param filename name of the xml input file
 @return 0 if correctly read, error code otherwise
 */
int parse_network_xml(char *filename) {
    
    xmlDocPtr file_network;       // Pointer where all the xml network file will be saved
    
    file_network = xmlReadFile(filename, NULL, 0);
    if (file_network == NULL) {
        printf("The xml network file does not exist\n");
        return NETWORK_FILE_NOT_FOUND;
    }
    
    // Parse everything and save it into internal memory
    if (read_general_information_xml(file_network) < 0) {
        printf("Error parsing the general information in the network file\n");
        return PARSE_NETWORK_ERROR;
    }
    if (read_topology_information_xml(file_network) < 0) {
        printf("Error parsing the topology information in the network file\n");
        return PARSE_NETWORK_ERROR;
    }
    if (read_frames_information_xml(file_network) < 0) {
        printf("Error parsing the frames in the network file\n");
        return PARSE_NETWORK_ERROR;
    }
    
    return 0;
}

/**
 Write the obtained schedule in a XML file with all the paths and transmission times of all the frames
 
 @param namefile path and name of the xml file to create with the written schedule
 @return 0 if correctly written, -1 otherwise
 */
int write_schedule_xml(char* namefile) {
    
    return 0;
}
