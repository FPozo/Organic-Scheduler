/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Scheduler.h                                                                                                        *
 *  Organic Scheduler                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 13/03/18.                                                                             *
 *  Copyright © 2018 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Package that contains the information and algorithgms to schedule the network.                                     *
 *  The scheduler add all the constraints and the optimization objectives to find a schedule.                          *
 *  The scheduler is also responsible to find the the paths that the frames have to follow to arrive to its            *
 *  destination.                                                                                                       *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef Scheduler_h
#define Scheduler_h

#include <stdio.h>
#include "Network.h"

#endif /* Scheduler_h */

/* ERROR CODE DEFINITIONS */

#define ERROR_PREPARING_SCHEDULER -101

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
int one_shot_scheduling(char *network_file, char *schedule_file);
