/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Optimizator.h                                                                                                      *
 *  Organic Scheduler                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 14/03/18.                                                                             *
 *  Copyright © 2018 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Package that contains the optimizations solvers.                                                                   *
 *  We plan to implement many different solvers, such as Z3, ILP, Gurobi, etc.                                         *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef Optimizator_h
#define Optimizator_h

#include <stdio.h>
#include <z3.h>
#include <gurobi_c.h>
#include "Network.h"

#endif /* Optimizator_h */

/* ERROR CODE DEFINITIONS */

#define ERROR_EXTRACTING_Z3_OFFSET -1
#define OPTIMIZATOR_NOT_IMPLEMENTED -101
#define ERROR_INIT_CONSTRAINTS -201
#define ERROR_CONTENTION_FREE_CONSTRAINTS -202
#define ERROR_END_TO_END_DELAY_CONSTRAINTS -203
#define ERROR_PATH_DEPENDENT_CONSTRAINS -204
#define ERROR_MAXIMIZING_SAME_FRAMES_DISTANCES -205
#define ERROR_SETTING_GUROBI_VAR -301
#define ERROR_SETTING_GUROBI_CONSTRAINT -302

/* STRUCT DEFINITIONS */

/**
 Avaliable solvers
 */
typedef enum Solver{
    z3,
    gurobi
}Solver;

/**
 Initialize the given solver to start the scheduling process
 
 @param s solver willed to be used and initialized
 @return 0 if done correctly, error code otherwise
 */
int initialize_solver(Solver s);

/**
 Initialize the variables to maximize with the distances between the same frame and the distances with frames of the
 same link. If optimization is 0, we then do not care about such distances and set them always to 0.
 You can give different weights to the distances between frames and links, as the distance in links is usually much
 larger and will have more importance.

 @param optimization 1 if we take care of maximizing distances, 0 oterhwise
 @param weight_frame weight ratio of the frame distance
 @param weight_link weight ratio of the link distance
 @return 0 if done correctly, error code otherwise
 */
int initialize_distances(int optimization, double weight_frame, double weight_link);

/**
 Init the variables needed to allow the solver the selection of which path to choose

 @param csolver indicates which solver are we using
 @return 0 if everything was ok, error code otherwise
 */
int init_path_selector(Solver csolver);

/**
 Creates the offset variables for all frames in the network, then adds them into the logical context
 
 @param csolver indicates which solver are we using
 @return 0 if everything was ok, error code otherwise
 */
int create_offset_variables(Solver csolver);

/**
 Add constraints for the solver to choose the paths of frames. For now, we allow only one possible path, chosen by the
 solver

 @param csolver indicates which solver are we using
 @return 0 if everything was ok, error code otherwise
 */
int choose_path(Solver csolver);

/**
 Assures that no frames are allowed to be transmitted at the same time in the same link
 
 @param csolver indicates which solver are we using
 @return 0 if everything was ok, error code otherwise
 */
int contention_free(Solver csolver);

/**
 Assures that all frames follow their path in the correct order
 
 @param csolver indicates which solver are we using
 @return 0 if everything was ok, error code otherwise
 */
int frame_path_dependent(Solver csolver);

/**
 Assures that all frames follow their end to end delay in all paths from the first transmission to the last
 
 @param csolver indicates which solver are we using
 @return 0 if everything was ok, error code otherwise
 */
int frame_end_to_end_delay(Solver csolver);

/**
 Optimize distances between transmission of the same frame during its path and frames transmitted at the same link

 @param csolver indicates which solver are we using
 @return 0 if everything was ok, error code otherwise
 */
int optimize_distances(Solver csolver);

/**
 Check the constraint solver and returns the status of it, if everything went well, it creates the schedule model
 
 @param csolver indicates which solver are we using
 @param time limit time in seconds to solve the schedule
 @param tune if tune is active, instead of solving the schedule, it will tune and find good parameters
 @param tunetimelimit limit in seconds to tune
 @return 1 if the schedule was found, error code otherwise
 */
int check_solver(Solver csolver, int time, int tune, int tunetimelimit);
