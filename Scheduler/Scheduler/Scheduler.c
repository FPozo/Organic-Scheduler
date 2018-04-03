/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Scheduler.c                                                                                                        *
 *  Organic Scheduler                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 13/03/18.                                                                             *
 *  Copyright © 2018 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Description in Scheduler.h                                                                                         *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "Scheduler.h"

int select_path;
int optimization;
int tune;
int timelimit;
int tunetimelimit;
double distance_frame_weigth;
double distance_link_weigth;
Solver solver;

/**
 Given a schedule configuration file, load the needed variables to start the scheduling

 @param filename name of the scheduling file
 @return 0 if done correctly, error code otherwise
 */
int read_schedule_configuration(char *filename) {
    
    // Init xml variables needed to search information in the file
    xmlChar *value;
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;
    xmlDocPtr file_configuration;
    
    file_configuration = xmlReadFile(filename, NULL, 0);
    if (file_configuration == NULL) {
        printf("The xml schedule configuration file does not exist\n");
        return CONFIGURATION_NOT_FOUND;
    }
    
    context = xmlXPathNewContext(file_configuration);
    
    // Search the time limit and save it
    result = xmlXPathEvalExpression((xmlChar*) "/ScheduleConfiguration/TimeLimit", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Schedule Configuration file is wrongly constructed, no TimeLimit found\n");
        return TIMELIMIT_NOT_FOUND;
    }
    value = xmlNodeListGetString(file_configuration, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    timelimit = atoi((const char*) value);
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    
    // Search the optimization and save it
    result = xmlXPathEvalExpression((xmlChar*) "/ScheduleConfiguration/Optimization", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Schedule Configuration file is wrongly constructed, no Optimization found\n");
        return OPTIMIZATION_NOT_FOUND;
    }
    value = xmlNodeListGetString(file_configuration, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    optimization = atoi((const char*) value);
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    
    // Search the path selector and save it
    result = xmlXPathEvalExpression((xmlChar*) "/ScheduleConfiguration/PathSelector", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Schedule Configuration file is wrongly constructed, no PathSelector found\n");
        return PATH_SELECTOR_NOT_FOUND;
    }
    value = xmlNodeListGetString(file_configuration, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    select_path = atoi((const char*) value);
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    
    // Search frame distance weigth and save it
    result = xmlXPathEvalExpression((xmlChar*) "/ScheduleConfiguration/FrameDistanceWeigth", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Schedule Configuration file is wrongly constructed, no FrameDistanceWeigth found\n");
        return FRAME_DISTANCE_WEIGTH_NOT_FOUND;
    }
    value = xmlNodeListGetString(file_configuration, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    distance_frame_weigth = atof((const char*) value);
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    
    // Search link distance weigth and save it
    result = xmlXPathEvalExpression((xmlChar*) "/ScheduleConfiguration/LinkDistanceWeigth", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Schedule Configuration file is wrongly constructed, no LinkDistanceWeigth found\n");
        return LINK_DISTANCE_WEIGTH_NOT_FOUND;
    }
    value = xmlNodeListGetString(file_configuration, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    distance_link_weigth = atof((const char*) value);
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    
    // Search tune and save it
    result = xmlXPathEvalExpression((xmlChar*) "/ScheduleConfiguration/Tune", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Schedule Configuration file is wrongly constructed, no Tune found\n");
        return TUNE_NOT_FOUND;
    }
    value = xmlNodeListGetString(file_configuration, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    tune = atoi((const char*) value);
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    
    // Search tune time limit and save it
    result = xmlXPathEvalExpression((xmlChar*) "/ScheduleConfiguration/TuneTimeLimit", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Schedule Configuration file is wrongly constructed, no TuneTimeLimit found\n");
        return TUNE_LIMIT_TIME_NOT_FOUND;
    }
    value = xmlNodeListGetString(file_configuration, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    tunetimelimit = atoi((const char*) value);
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    
    // Search solver and save it
    result = xmlXPathEvalExpression((xmlChar*) "/ScheduleConfiguration/Solver", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Schedule Configuration file is wrongly constructed, no Solver found\n");
        return SOLVER_NOT_FOUND;
    }
    value = xmlNodeListGetString(file_configuration, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    if (strcmp((const char*) value, "gurobi") == 0) {
        solver = gurobi;
    } else if (strcmp((const char*) value, "z3") == 0) {
        solver = z3;
    } else {
        printf("Solver not recognized or implemented\n");
        return SOLVER_NOT_FOUND;
    }
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    xmlFreeDoc(file_configuration);
    
    return 0;
}

/**
 Produces the schedule solving all constraints in one call to the Solver for a given network.
 It inits the solver and the network.
 It starts creating all the constraints (one variable for each transmission offset), then adds constraints relating
 different offsets. At the end solves the logical context and the model obtained is the solver.
 It creates an xml file with the output schedule.
 It also creates different constraint files for every switch in the network containing specific constraints for each
 switch
 
 @param network_file name of the file with the description of the network
 @param schedule_file name of the file with the scheduled network
 @return 0 if the schedule was found, error code otherwise
 */
int one_shot_scheduling(char *network_file, char *schedule_file, char *configuration_file) {
    
    if (parse_network_xml(network_file) < 0) {
        printf("Error reading the network file\n");
        return ERROR_SCHEDULING_ONE_SHOT;
    }
    if (read_schedule_configuration(configuration_file) < 0) {
        printf("Error reading the configuration file\n");
        return ERROR_SCHEDULING_ONE_SHOT;
    }
    initialize_solver(solver);
    initialize_network();
    if (select_path == 1) {
        init_path_selector(solver);
    }
    if (create_offset_variables(solver) < 0) {
        printf("Error creating offset variables\n");
        return ERROR_SCHEDULING_ONE_SHOT;
    }
    if (solver == gurobi) {
        initialize_distances(optimization, distance_frame_weigth, distance_link_weigth);
    }
    if (select_path == 1) {
        choose_path(solver);
    }
    if (contention_free(solver) < 0) {
        printf("Error creating contention free constraints\n");
        return ERROR_SCHEDULING_ONE_SHOT;
    }
    if (frame_path_dependent(solver) < 0) {
        printf("Error creating path dependent constraints\n");
        return ERROR_SCHEDULING_ONE_SHOT;
    }
    if (frame_end_to_end_delay(solver) < 0) {
        printf("Error creating end to end delay constraints\n");
        return ERROR_SCHEDULING_ONE_SHOT;
    }
    if (check_solver(solver, timelimit, tune, tunetimelimit) < 0) {
        printf("Error finding the schedule\n");
        return ERROR_SCHEDULING_ONE_SHOT;
    }
    // If successful, extract the scheduler and save into the network
    // Write in a xml file the network schedule
    
    return 0;
}


