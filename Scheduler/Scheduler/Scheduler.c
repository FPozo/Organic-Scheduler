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
int one_shot_scheduling(char *network_file, char *schedule_file) {
    
    if (parse_network_xml(network_file) < 0) {
        printf("Error reading the network file\n");
        return ERROR_PREPARING_SCHEDULER;
    }
    
    // Create variables with their period and deadlines and such
    // Make frames to not be transmitted at the same time over the same link
    // Make sure the frames follow the selected path correctly
    // Make sure the frames are transmitted before the end to end delay
    // Add the optimization function
    // Solve
    // If successful, extract the scheduler and save into the network
    // Write in a xml file the network schedule
    
    return 0;
}
