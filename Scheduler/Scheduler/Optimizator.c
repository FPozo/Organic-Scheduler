/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Optimizator.c                                                                                                      *
 *  Organic Scheduler                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 14/03/18.                                                                             *
 *  Copyright © 2018 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Description in Optimizator.h                                                                                       *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "Optimizator.h"

Z3_config z3_configuration;         // Configuration of z3
Z3_params z3_parameters;            // Parameters of z3
Z3_context z3_context;              // Context where the constraint are added in z3
Z3_optimize z3_optimize;            // Optimization solver in z3
Z3_model z3_model;                  // Model of z3 that contains the full schedule
Z3_ast ***path_selector = NULL;     // 3d-Matrix to save the path selector contraints in z3 (frame, receiver, path)

GRBenv *gurobi_env = NULL;          // Gurobi environment
GRBmodel *gurobi_model = NULL;      // Gurobi model
int gurobi_var_counter = 0;
int ***gurobi_path_selector = NULL; // 3d-Matrix to save the path selector constraints in gurobi (same as z3)
int *gurobi_frame_distance = NULL;  // Array with distances of frames to maximize
int *gurobi_link_distance = NULL;   // Array with distances of links to maximize

/* PRIVATE FUNCTIONS */

/**
 Init the constraint variable into the offset
 
 @param offset_pt pointer of the offset
 @param instance of the offset
 @param replica of the offset
 @param name given to the variable
 @param csolver constraint solver used
 @return 0 if done correctly, error code otherwise
 */
int init_variable(Offset *offset_pt, int instance, int replica, char *name, Solver csolver) {
    
    Z3_sort z3_integer;
    Z3_symbol z3_name;
    
    switch (csolver) {
        case z3:
            z3_integer = Z3_mk_int_sort(z3_context);
            z3_name = Z3_mk_string_symbol(z3_context, name);
            set_z3_offset(offset_pt, instance, replica, Z3_mk_const(z3_context, z3_name, z3_integer));
            return 0;
        default:
            return OPTIMIZATOR_NOT_IMPLEMENTED;
    }
}

/**
 Adds into the solver a constraint to set the distance between two offsets
 offset1[instance][replica] = offset2[instance][replica] + distance
 
 @param offset1_pt pointer to the offset 1
 @param instance1 of the offset 1
 @param replica1 of the offset 1
 @param offset2_pt pointer to the offset 2
 @param instance2 of the offset 2
 @param replica2 of the offset 2
 @param distance long long int with the distance between the two offsets in ns
 @param csolver constraint solver used
 @return 0 if everything went ok, error code otherwise
 */
int set_fixed_distance(Offset *offset1_pt, int instance1, int replica1, Offset *offset2_pt, int instance2,
                       int replica2, long long int distance, Solver csolver) {
    
    // Auxiliar variables to store constraints
    Z3_sort z3_integer;
    Z3_ast z3_add_args[2], z3_add, z3_formula, z3_offset2, z3_int0, z3_offset1_eq0, z3_offset2_eq0;
    
    // Gurobi needed variables
    int variables[2];
    double values[2] = {1.0, -1.0};
    
    switch (csolver) {
        case z3:
            // Set the distance between both offsets
            z3_integer = Z3_mk_int_sort(z3_context);
            z3_add_args[0] = get_z3_offset(offset1_pt, instance1, replica1);
            if (z3_add_args[0] == NULL) {
                printf("Error extracting the constraint of offset1\n");
                return ERROR_EXTRACTING_Z3_OFFSET;
            }
            z3_add_args[1] = Z3_mk_int64(z3_context, distance, z3_integer);
            // z3_add <= offset1 + distance
            z3_add = Z3_mk_add(z3_context, 2, z3_add_args);
            // z3_formula <= offset2 = z3_add
            z3_offset2 = get_z3_offset(offset2_pt, instance2, replica2);
            if (z3_offset2 == NULL) {
                printf("Error extracting the constraint of offset2\n");
                return ERROR_EXTRACTING_Z3_OFFSET;
            }
            z3_formula = Z3_mk_eq(z3_context, z3_offset2, z3_add);
            if (path_selector != NULL) {        // If we need to define a path
                // z3_formula <= if offset1 = 0 then offset2 = 0 else z3_formula
                z3_int0 = Z3_mk_int64(z3_context, 0, z3_integer);
                z3_offset2_eq0 = Z3_mk_eq(z3_context, z3_offset2, z3_int0);
                z3_offset1_eq0 = Z3_mk_eq(z3_context, get_z3_offset(offset1_pt, instance1, replica1), z3_int0);
                z3_formula = Z3_mk_ite(z3_context, z3_offset1_eq0, z3_offset2_eq0, z3_formula);
            }
            Z3_optimize_assert(z3_context, z3_optimize, z3_formula);
            break;
        case gurobi:
            variables[0] = get_gurobi_offset(offset2_pt, instance2, replica2);
            variables[1] = get_gurobi_offset(offset1_pt, instance1, replica1);
            if (gurobi_path_selector == NULL) {
                if (GRBaddconstr(gurobi_model, 2, variables, values, GRB_EQUAL, distance, NULL) != 0) {
                    printf("Error adding setting fix distance constraint in gurobi\n");
                    return ERROR_SETTING_GUROBI_CONSTRAINT;
                }
            } else {
                // We create a binary variable that will be 1 if offset1 = 0, which implies offset2 has to be 0 too
                // If not, we activate the previus constraint
                GRBaddvar(gurobi_model, 0, NULL, NULL, 0, 0, 1, GRB_BINARY, NULL);
                gurobi_var_counter++;
                GRBaddgenconstrIndicator(gurobi_model, NULL, gurobi_var_counter - 1, 1, 2, variables, values,
                                         GRB_EQUAL, distance);
                values[1] = 1.0;
                GRBaddgenconstrIndicator(gurobi_model, NULL, gurobi_var_counter - 1, 0, 2, variables, values,
                                         GRB_EQUAL, 0);
            }
            break;
        default:
            break;
    }
    
    return 0;
}

/**
 Adds into the solver the constraints to limit transmission time range
 offset[instance][replica] = (min, max]
 
 @param offset_pt pointer to the offset
 @param instance of the offset
 @param replica of the offset
 @param min minimum transmission time in ns
 @param max maximum transmission time in ns
 @param name of the variable (needed only for gurobi)
 @param csolver constraint solver used
 @return 0 if everything went ok, error code otherwise
 */
int set_offset_range(Offset *offset_pt, int instance, int replica, long long int min, long long int max, char *name,
                     Solver csolver) {
    
    // Auxiliar variables to store constraints
    Z3_sort z3_integer;
    Z3_ast  z3_minimum, z3_maximum, z3_formula, z3_offset, z3_or_args[2], z3_int0, z3_eq;
    int minimum_posible;
    
    switch (csolver) {
        case z3:
            // Set the minimum transmission time
            z3_integer = Z3_mk_int_sort(z3_context);
            z3_minimum = Z3_mk_int64(z3_context, min, z3_integer);
            z3_offset = get_z3_offset(offset_pt, instance, replica);
            if (z3_offset == NULL) {
                printf("Error extracting the offset\n");
                return ERROR_EXTRACTING_Z3_OFFSET;
            }
            // z3_formula <= offset >= min
            z3_formula = Z3_mk_gt(z3_context, z3_offset, z3_minimum);
            if (path_selector != NULL) {        // If we have to select paths
                // z3_eq <= offset1 = 0
                z3_int0 = Z3_mk_int64(z3_context, 0, z3_integer);
                z3_eq = Z3_mk_eq(z3_context, z3_offset, z3_int0);
                z3_or_args[0] = z3_formula;
                z3_or_args[1] = z3_eq;
                // z3_formula <= z3_formula OR offset2 = 0
                z3_formula = Z3_mk_or(z3_context, 2, z3_or_args);
            }
            Z3_optimize_assert(z3_context, z3_optimize, z3_formula);
            
            // Set the maximum transmission time
            z3_maximum = Z3_mk_int64(z3_context, max, z3_integer);
            // z3_formula <= offset < max
            z3_formula = Z3_mk_le(z3_context, z3_offset, z3_maximum);
            Z3_optimize_assert(z3_context, z3_optimize, z3_formula);
            
            break;
        case gurobi:
            // Min is always 0 as offset == 0 implies that we do not use the offset, unless no path selector
            if (path_selector == NULL) {
                minimum_posible = 1;
            } else {
                minimum_posible = 0;
            }
            if (GRBaddvar(gurobi_model, 0, NULL, NULL, 0, minimum_posible, max, GRB_INTEGER, name) != 0) {
                printf("Error add gurobi variable for a frame instance\n");
                return ERROR_SETTING_GUROBI_VAR;
            }
            // In gurobi, to access a variable you need to add the position when it was added, so we save that and add
            // the counter for the next one
            set_gurobi_offset(offset_pt, instance, replica, gurobi_var_counter);
            gurobi_var_counter++;
            break;
        default:
            break;
    }
    
    return 0;
}

/**
 Returns 1 if it is possible for both offsets to collide in a transmission dependening on theirs allowed transmission
 time
 
 @param frame1_pt pointer of the frame 1
 @param offset1_pt pointer of the offset 1
 @param instance1 of the offset 1
 @param replica1 of the offset 1
 @param frame2_pt pointer of the frame 2
 @param offset2_pt pointer of the offset 2
 @param instance2 of the offset 2
 @param replica2 of the offset 2
 @return 1 if it is possible for the times to collide, 0 otherwise
 */
int offsets_share_interval(Frame *frame1_pt, Offset *offset1_pt, int instance1, int replica1, Frame *frame2_pt,
                           Offset *offset2_pt, int instance2, int replica2) {
    
    long long int period1, period2;         // Periods of the given offsets
    long long int deadline1, deadline2;     // Deadlines of the given offsets
    long long int starting1, starting2;     // Starting of the given offsets
    long long int min1, max1, min2, max2;   // Time intervals of both offsets
    
    period1 = get_period(frame1_pt);
    period2 = get_period(frame2_pt);
    deadline1 = get_deadline(frame1_pt);
    deadline2 = get_deadline(frame2_pt);
    starting1 = get_starting(frame1_pt);
    starting2 = get_starting(frame2_pt);
    min1 = (period1 * instance1) + starting1 + 1;       // Minimum is the period * the number of instance, +1 to avoid 0
    max1 = (period1 * instance1) + deadline1 + 1;       // Maximum is the period * (next instance) - 1, +1 to avoid 0
    min2 = (period2 * instance2) + starting2 + 1;
    max2 = (period2 * instance2) + deadline2 + 1;
    
    // if the first interval starts before and the second interval starts before the first ends
    // or if the second interval starts before and the first interval starts before the second ends
    if ((min1 <= min2 && min2 < max1) || (min2 <= min1 && min1 < max2)) {
        return 1;
    }
    
    return 0;
}

/**
 Avoids that the two given offsets share any transmission time
 offset1[instance][replica] + distance1 <= offset2[instance][replica]
 OR
 offset2[instance][replica] + distance2 <= offset1[instance][replica]
 
 @param offset1_pt pointer to the offset 1
 @param instance1 of the offset 1
 @param replica1 of the offset 1
 @param offset2_pt pointer to the offset 2
 @param instance2 of the offset 2
 @param replica2 of the offset 2
 @param distance1 long long int with the distance the first offset can go
 @param distance2 long long int with the distance the second offset can go
 @param csolver constraint solver used
 @return 0 if everything went ok, error code otherwise
 */
int avoid_intersection(Offset *offset1_pt, int instance1, int replica1, Offset *offset2_pt, int instance2,
                       int replica2, long long int distance1, long long int distance2, Solver csolver) {

    // Auxiliar variables to hold constraints
    Z3_sort z3_integer;
    Z3_ast z3_add_args[2], z3_or_args[2], z3_add, z3_less, z3_greater, z3_offset1, z3_offset2, z3_formula, z3_int0;
    Z3_ast z3_eq;
    
    // Gurobi variables
    int variables[3];
    double values[3] = {1.0, -1.0, 1.0};
    int or_variables[2];
    
    switch (csolver) {
        case z3:
            z3_integer = Z3_mk_int_sort(z3_context);
            z3_offset1 = get_z3_offset(offset1_pt, instance1, replica1);
            z3_offset2 = get_z3_offset(offset2_pt, instance2, replica2);
            if (z3_offset1 == NULL || z3_offset2 == NULL) {
                printf("Error getting the offsets to avoid the intersection\n");
                return ERROR_EXTRACTING_Z3_OFFSET;
            }
            
            z3_add_args[0] = get_z3_offset(offset1_pt, instance1, replica1);
            z3_add_args[1] = Z3_mk_int64(z3_context, distance1, z3_integer);
            // z3_add <= offset1 + distance1
            z3_add = Z3_mk_add(z3_context, 2, z3_add_args);
            // z3_less <= z3_add < offset2
            z3_less = Z3_mk_le(z3_context, z3_add, z3_offset2);
            z3_add_args[0] = z3_offset2;
            z3_add_args[1] = Z3_mk_int64(z3_context, distance2, z3_integer);
            // z3_add <= offset2 + distance
            z3_add = Z3_mk_add(z3_context, 2, z3_add_args);
            // z3_greater <= z3_add >= offset1
            z3_greater = Z3_mk_le(z3_context, z3_add, z3_offset1);
            z3_or_args[0] = z3_less;
            z3_or_args[1] = z3_greater;
            z3_formula = Z3_mk_or(z3_context, 2, z3_or_args);
            if (path_selector != NULL) {        // If we have to select paths
                // z3_eq <= offset1 = 0
                z3_int0 = Z3_mk_int64(z3_context, 0, z3_integer);
                z3_eq = Z3_mk_eq(z3_context, z3_offset1, z3_int0);
                z3_or_args[0] = z3_formula;
                z3_or_args[1] = z3_eq;
                // z3_formula <= z3_formula OR offset2 = 0
                z3_formula = Z3_mk_or(z3_context, 2, z3_or_args);
            }
            Z3_optimize_assert(z3_context, z3_optimize, z3_formula);
            break;
        case gurobi:
            if (gurobi_path_selector == NULL) {
                // Add OR of two variables, link them with a Indicator function
                GRBaddvar(gurobi_model, 0, NULL, NULL, 0, 0, 1, GRB_BINARY, NULL);
                gurobi_var_counter++;
                GRBaddvar(gurobi_model, 0, NULL, NULL, 0, 0, 1, GRB_BINARY, NULL);
                gurobi_var_counter++;
                GRBaddvar(gurobi_model, 0, NULL, NULL, 0, 1, 1, GRB_BINARY, NULL);
                gurobi_var_counter++;
                or_variables[0] = gurobi_var_counter - 3;
                or_variables[1] = gurobi_var_counter - 2;
                GRBaddgenconstrOr(gurobi_model, NULL, gurobi_var_counter - 1, 2, or_variables);
                variables[0] = get_gurobi_offset(offset1_pt, instance1, replica1);
                variables[1] = get_gurobi_offset(offset2_pt, instance2, replica2);
                variables[2] = gurobi_link_distance[get_offset_link(offset1_pt)];
                GRBaddgenconstrIndicator(gurobi_model, NULL, gurobi_var_counter - 3, 1, 3, variables, values,
                                             GRB_LESS_EQUAL, -distance1);
                variables[0] = get_gurobi_offset(offset2_pt, instance2, replica2);
                variables[1] = get_gurobi_offset(offset1_pt, instance1, replica1);
                GRBaddgenconstrIndicator(gurobi_model, NULL, gurobi_var_counter - 2, 1, 3, variables, values,
                                             GRB_LESS_EQUAL, -distance2);
            } else {
                // If we have to select path, we add one OR more that states both offsets are = 0, so it is activated
                // iif offsets are not used
                GRBaddvar(gurobi_model, 0, NULL, NULL, 0, 0, 1, GRB_BINARY, NULL);
                gurobi_var_counter++;
                GRBaddvar(gurobi_model, 0, NULL, NULL, 0, 0, 1, GRB_BINARY, NULL);
                gurobi_var_counter++;
                GRBaddvar(gurobi_model, 0, NULL, NULL, 0, 0, 1, GRB_BINARY, NULL);
                gurobi_var_counter++;
                GRBaddvar(gurobi_model, 0, NULL, NULL, 0, 1, 1, GRB_BINARY, NULL);
                gurobi_var_counter++;

                variables[0] = gurobi_var_counter - 4;
                variables[1] = gurobi_var_counter - 3;
                variables[2] = gurobi_var_counter - 2;
                GRBaddgenconstrOr(gurobi_model, NULL, gurobi_var_counter - 1, 3, variables);
                variables[0] = get_gurobi_offset(offset1_pt, instance1, replica1);
                variables[1] = get_gurobi_offset(offset2_pt, instance2, replica2);
                variables[2] = gurobi_link_distance[get_offset_link(offset1_pt)];
                GRBaddgenconstrIndicator(gurobi_model, NULL, gurobi_var_counter - 4, 1, 3, variables, values,
                                         GRB_LESS_EQUAL, -distance1);
                variables[0] = get_gurobi_offset(offset2_pt, instance2, replica2);
                variables[1] = get_gurobi_offset(offset1_pt, instance1, replica1);
                GRBaddgenconstrIndicator(gurobi_model, NULL, gurobi_var_counter - 3, 1, 3, variables, values,
                                         GRB_LESS_EQUAL, -distance2);
                values[1] = 1.0;
                GRBaddgenconstrIndicator(gurobi_model, NULL, gurobi_var_counter - 2, 1, 2, variables, values,
                                         GRB_EQUAL, 0);
            }
            break;
        default:
            break;
    }
    
    return 0;
}

/**
 Adds into the solver a constraint to set the maximum distance between two offsets
 offset1[instance][replica] + distance >= offset2[instance][replica]
 
 @param offset1_pt pointer to the offset 1
 @param instance1 of the offset 1
 @param replica1 of the offset 1
 @param offset2_pt pointer to the offset 2
 @param instance2 of the offset 2
 @param replica2 of the offset 2
 @param distance int with the distance between the two offsets in ns
 @param frame_it frame id
 @param receiver_it receiver id
 @param path_it path id
 @param csolver constraint solver used
 @return 0 if everything went ok, error code otherwise
 */
int set_maximum_distance(Offset *offset1_pt, int instance1, int replica1, Offset *offset2_pt, int instance2,
                         int replica2, long long int distance, int frame_it, int receiver_it, int path_it,
                         Solver csolver) {
    
    // Auxiliar variables to hold constraints
    Z3_sort z3_integer;
    Z3_ast z3_add_args[2], z3_add, z3_offset2, z3_formula;
    
    // Gurobi variables
    int variables[3];
    double values[3] = {1.0, -1.0, 1.0};
    
    switch (csolver) {
        case z3:
            z3_integer = Z3_mk_int_sort(z3_context);
            z3_add_args[0] = get_z3_offset(offset1_pt, instance1, replica1);
            if (z3_add_args[0] == NULL) {
                printf("Error getting an offset");
                return ERROR_EXTRACTING_Z3_OFFSET;
            }
            z3_add_args[1] = Z3_mk_int64(z3_context, distance, z3_integer);
            // z3_add <= offset1 + distance
            z3_add = Z3_mk_add(z3_context, 2, z3_add_args);
            z3_offset2 = get_z3_offset(offset2_pt, instance2, replica2);
            if (z3_offset2 == NULL) {
                printf("Error getting an offset");
                return ERROR_EXTRACTING_Z3_OFFSET;
            }
            // z3_formula <= z3_add >= offset2
            z3_formula = Z3_mk_ge(z3_context, z3_add, z3_offset2);
            Z3_optimize_assert(z3_context, z3_optimize, z3_formula);
            break;
        case gurobi:
            // Set the frame distances here, trying to limit the distances to the starting time and from the end to end
            // Only add the constraint if the path is active
            //Last offset distance
            if (gurobi_path_selector != NULL) {
                variables[0] = get_gurobi_offset(offset2_pt, instance2, replica2);
                variables[1] = get_gurobi_offset(offset1_pt, instance1, replica1);
                if (GRBaddgenconstrIndicator(gurobi_model, NULL, gurobi_path_selector[frame_it][receiver_it][path_it],
                                             1, 2, variables, values, GRB_LESS_EQUAL, distance) < 0) {
                    printf("Error adding maximum distance constraint in gurobi\n");
                    return ERROR_SETTING_GUROBI_CONSTRAINT;
                }
                // First offset distance
                variables[0] = get_gurobi_offset(offset1_pt, instance1, replica1);
                variables[1] = gurobi_frame_distance[frame_it];
                if (GRBaddgenconstrIndicator(gurobi_model, NULL, gurobi_path_selector[frame_it][receiver_it][path_it],
                                             1, 2, variables, values, GRB_GREATER_EQUAL, get_starting(get_frame(frame_it))) < 0) {
                    printf("Error adding maximum distance constraint in gurobi\n");
                    return ERROR_SETTING_GUROBI_CONSTRAINT;
                }
                // Last offset distance
                variables[0] = get_gurobi_offset(offset2_pt, instance2, replica2);
                variables[1] = gurobi_frame_distance[frame_it];
                if (GRBaddgenconstrIndicator(gurobi_model, NULL, gurobi_path_selector[frame_it][receiver_it][path_it],
                                             1, 2, variables, values, GRB_LESS_EQUAL,
                                             get_deadline(get_frame(frame_it))) < 0) {
                    printf("Error adding maximum distance constraint in gurobi\n");
                    return ERROR_SETTING_GUROBI_CONSTRAINT;
                }
            } else {
                variables[0] = get_gurobi_offset(offset2_pt, instance2, replica2);
                variables[1] = get_gurobi_offset(offset1_pt, instance1, replica1);
                if (GRBaddconstr(gurobi_model, 2, variables, values, GRB_LESS_EQUAL, distance, NULL) < 0) {
                    printf("Error adding maximum distance constraint in gurobi\n");
                    return ERROR_SETTING_GUROBI_CONSTRAINT;
                }
                // First offset distance
                variables[0] = get_gurobi_offset(offset1_pt, instance1, replica1);
                variables[1] = gurobi_frame_distance[frame_it];
                
                if (GRBaddconstr(gurobi_model, 2, variables, values, GRB_GREATER_EQUAL,
                                 get_starting(get_frame(frame_it)), NULL) < 0) {
                    printf("Error adding maximum distance constraint in gurobi\n");
                    return ERROR_SETTING_GUROBI_CONSTRAINT;
                }
                // Last offset distance
                variables[0] = get_gurobi_offset(offset2_pt, instance2, replica2);
                variables[1] = gurobi_frame_distance[frame_it];
                values[1] = 1.0;
                if (GRBaddconstr(gurobi_model, 2, variables, values, GRB_LESS_EQUAL, get_deadline(get_frame(frame_it)),
                                 NULL) < 0) {
                    printf("Error adding maximum distance constraint in gurobi\n");
                    return ERROR_SETTING_GUROBI_CONSTRAINT;
                }
            }
            
            break;
        default:
            break;
    }
    return 0;
}

/**
 Adds into the solver a constraint to set the minimum distance between two offsets
 offset1[instance][replica] + distance < offset2[instance][replica]
 
 @param offset1_pt pointer to the offset 1
 @param instance1 of the offset 1
 @param replica1 of the offset 1
 @param offset2_pt pointer to the offset 2
 @param instance2 of the offset 2
 @param replica2 of the offset 2
 @param distance int with the distance between the two offsets in ns
 @param frame_it id of the frame
 @param receiver_it id of the receiver
 @param path_it id of the path
 @param csolver constraint solver used
 @return 0 if everything went ok, -1 if something failed
 */
int set_minimum_distance(Offset *offset1_pt, int instance1, int replica1, Offset *offset2_pt, int instance2,
                         int replica2, long long int distance, int frame_it, int receiver_it, int path_it,
                         Solver csolver) {
 
    // Auxiliar variables to hold constraints
    Z3_sort z3_integer;
    Z3_ast z3_add_args[2], z3_add, z3_offset2, z3_formula, z3_or_args[2], z3_int0;
    
    // Gurobi variables
    int variables[3];
    double values[3] = {1.0, -1.0, -1.0};
    
    switch (csolver) {
        case z3:
            z3_integer = Z3_mk_int_sort(z3_context);
            z3_add_args[0] = get_z3_offset(offset1_pt, instance1, replica1);
            if (z3_add_args[0] == NULL) {
                printf("Error getting the z3 constraints\n");
                return ERROR_EXTRACTING_Z3_OFFSET;
            }
            z3_add_args[1] = Z3_mk_int64(z3_context, distance, z3_integer);
            // z3_add <= offset1 + distance
            z3_add = Z3_mk_add(z3_context, 2, z3_add_args);
            z3_offset2 = get_z3_offset(offset2_pt, instance2, replica2);
            if (z3_offset2 == NULL) {
                printf("Error getting the z3 constraints\n");
                return ERROR_EXTRACTING_Z3_OFFSET;
            }
            // z3_formula = z3_add < offset2
            z3_formula = Z3_mk_le(z3_context, z3_add, z3_offset2);
            if (path_selector != NULL) {     // If we have to choose paths
                // Original constraint OR path = 0
                z3_or_args[0] = z3_formula;
                z3_int0 = Z3_mk_int64(z3_context, 0, z3_integer);
                z3_or_args[1] = Z3_mk_eq(z3_context, path_selector[frame_it][receiver_it][path_it], z3_int0);
                z3_formula = Z3_mk_or(z3_context, 2, z3_or_args);
            }
            Z3_optimize_assert(z3_context, z3_optimize, z3_formula);
            break;
        case gurobi:
            variables[0] = get_gurobi_offset(offset2_pt, instance2, replica2);
            variables[1] = get_gurobi_offset(offset1_pt, instance1, replica1);
            variables[2] = gurobi_frame_distance[frame_it];
            if (gurobi_path_selector == NULL) { // If we have the path given, just add the constraint
                if (GRBaddconstr(gurobi_model, 3, variables, values, GRB_GREATER_EQUAL, distance + 1, NULL) < 0) {
                    printf("Error setting minimum distance constraint in gurobi\n");
                    return ERROR_SETTING_GUROBI_CONSTRAINT;
                }
            } else {    // If we have to choose paths, add the constraint if the path selector is 1
                if (GRBaddgenconstrIndicator(gurobi_model, NULL, gurobi_path_selector[frame_it][receiver_it][path_it],
                                             1, 3, variables, values, GRB_GREATER_EQUAL, distance + 1) < 0) {
                    printf("Error setting minimum distance constraint with path selector in gurobi\n");
                    return ERROR_SETTING_GUROBI_CONSTRAINT;
                }
            }
            break;
        default:
            break;
    }
    
    return 0;
}

/* PUBLIC FUNCTIONS */

/**
 Initialize the given solver to start the scheduling process
 
 @param s solver willed to be used and initialize
 @return 0 if done correctly, error code otherwise
 */
int initialize_solver(Solver s) {
    
    // Z3_symbol z3_priority, z3_pareto;
    
    switch (s) {
        case z3:
            z3_configuration = Z3_mk_config();
            z3_context = Z3_mk_context(z3_configuration);
            Z3_del_config(z3_configuration);
            z3_optimize = Z3_mk_optimize(z3_context);
            Z3_global_param_set("model", "true");
            Z3_global_param_set("auto_config", "true");
            return 0;
        case gurobi:
            GRBloadenv(&gurobi_env, "schedule.log");
            GRBnewmodel(gurobi_env, &gurobi_model, "schedule", 0, NULL, NULL, NULL, NULL, NULL);
            GRBsetintattr(gurobi_model, GRB_INT_ATTR_MODELSENSE, GRB_MAXIMIZE);
        default:
            return OPTIMIZATOR_NOT_IMPLEMENTED;
    }
}

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
int initialize_distances(int optimization, double weight_frame, double weight_link) {
    
    char name[100];
    int variables[1];
    double values[1] = {1.0};
    gurobi_frame_distance = malloc(sizeof(int) * get_num_frames()); // Allocate memory for every frame distance
    gurobi_link_distance = malloc(sizeof(int) * get_num_links());   // Allocate memory for every link distance
    
    if (optimization == 0) {
        weight_frame = 0.0;
        weight_link = 0.0;
    }
    
    for (int frame_it = 0; frame_it < get_num_frames(); frame_it++) {   // For every frame
        
        sprintf(name, "FrameDistance_%d", frame_it);                // Create the distance variable in the solver
        GRBaddvar(gurobi_model, 0, NULL, NULL, weight_frame, 0, get_end_to_end_delay(get_frame(frame_it)), GRB_INTEGER,
                  name);
        gurobi_frame_distance[frame_it] = gurobi_var_counter;
        gurobi_var_counter++;
        if (optimization == 0) {        // If no optimization is needed, we equal the distance to 0
            variables[0] = gurobi_frame_distance[frame_it];
            GRBaddconstr(gurobi_model, 1, variables, values, GRB_EQUAL, 0, NULL);
            gurobi_var_counter++;
        }
    }
    
    for (int link_it = 0; link_it < get_num_links(); link_it++) {     // For every link
        sprintf(name, "LinkDistance_%d", link_it);                // Create the distance variable in the solverch
        GRBaddvar(gurobi_model, 0, NULL, NULL, weight_link, 0, get_hyper_period(), GRB_INTEGER, name);
        gurobi_link_distance[link_it] = gurobi_var_counter;
        gurobi_var_counter++;
        if (optimization == 0) {
            variables[0] = gurobi_link_distance[link_it];
            GRBaddconstr(gurobi_model, 1, variables, values, GRB_EQUAL, 0, NULL);
            gurobi_var_counter++;
        }
    }
    
    return 0;
}

/**
 Init the variables needed to allow the solver the selection of which path to choose
 
 @param csolver indicates which solver are we using
 @return 0 if everything was ok, error code otherwise
 */
int init_path_selector(Solver csolver) {
    
    Frame *frame_pt;
    int num_paths;                              // Number of paths to arrive
    int num_receivers;                          // Number of receivers of the frame
    int receiver;                             //  reciever of the frame
    int sender;                                 // Sender of the frame
    Z3_sort z3_integer = NULL;
    Z3_symbol z3_name;
    Z3_ast z3_add, z3_formula, z3_1, z3_0;
    double *values = NULL;
    char name[100];
    
    // For all given frames, check how many paths to we have, for each path allocate memory in the array and create
    // the constraints
    switch (csolver) {
        case z3:
            path_selector = malloc(sizeof(Z3_ast **) * get_num_frames());    // Create an array, one per each frame
            z3_integer = Z3_mk_int_sort(z3_context);
            break;
        case gurobi:
            gurobi_path_selector = malloc(sizeof(int **) * get_num_frames());
            break;
        default:
            break;
    }
    for (int frame_it = 0; frame_it < get_num_frames(); frame_it++) {
        frame_pt = get_frame(frame_it);
        sender = get_sender_id(frame_pt);
        num_receivers = get_num_receivers(frame_pt);
        switch (csolver) {
            case z3:
                path_selector[frame_it] = malloc(sizeof(Z3_ast *) * num_receivers);
                break;
            case gurobi:
                gurobi_path_selector[frame_it] = malloc(sizeof(int *) * num_receivers);
                break;
            default:
                break;
        }
        for (int receiver_it = 0; receiver_it < num_receivers; receiver_it++) {         // For all the receivers
            receiver = get_receiver_id(frame_pt, receiver_it);
            num_paths = get_num_paths(sender, receiver);
            switch (csolver) {
                case z3:
                    path_selector[frame_it][receiver_it] = malloc(sizeof(Z3_ast) * num_paths);
                    break;
                case gurobi:
                    gurobi_path_selector[frame_it][receiver_it] = malloc(sizeof(int) * num_paths);
                    values = malloc(sizeof(double) * num_paths);
                    break;
                default:
                    break;
            }
            for (int path_it = 0; path_it < num_paths; path_it++) {                     // Init the constraint
                sprintf(name, "X_%d_%d_%d", frame_it, receiver_it, path_it);
                switch (csolver) {
                    case z3:
                        z3_name = Z3_mk_string_symbol(z3_context, name);
                        path_selector[frame_it][receiver_it][path_it] = Z3_mk_const(z3_context, z3_name, z3_integer);
                        // Limit the path selector to 0 or 1
                        z3_0 = Z3_mk_int(z3_context, 0, z3_integer);
                        z3_1 = Z3_mk_int(z3_context, 1, z3_integer);
                        z3_formula = Z3_mk_ge(z3_context, path_selector[frame_it][receiver_it][path_it], z3_0);
                        Z3_optimize_assert(z3_context, z3_optimize, z3_formula);
                        z3_formula = Z3_mk_le(z3_context, path_selector[frame_it][receiver_it][path_it], z3_1);
                        Z3_optimize_assert(z3_context, z3_optimize, z3_formula);
                        break;
                    case gurobi:
                        GRBaddvar(gurobi_model, 0, NULL, NULL, 0, 0, 1, GRB_BINARY, name);
                        gurobi_path_selector[frame_it][receiver_it][path_it] = gurobi_var_counter;
                        gurobi_var_counter++;
                        values[path_it] = 1.0;
                        break;
                    default:
                        break;
                }
                
            }
            // Add that the constraint can only be 1
            switch (csolver) {
                case z3:
                    z3_add = Z3_mk_add(z3_context, num_paths, path_selector[frame_it][receiver_it]);
                    z3_1 = Z3_mk_int(z3_context, 1, z3_integer);
                    z3_formula = Z3_mk_eq(z3_context, z3_add, z3_1);
                    Z3_optimize_assert(z3_context, z3_optimize, z3_formula);
                    break;
                case gurobi:
                    GRBaddconstr(gurobi_model, num_paths, gurobi_path_selector[frame_it][receiver_it], values,
                                 GRB_EQUAL, 1, NULL);
                    free(values);
                default:
                    break;
            }
        }
    }
    return 0;
}

/**
 Creates the offset variables for all frames in the network, then adds them into the logical context
 
 @param csolver indicates which solver are we using
 @return 0 if everything was ok, error code otherwise
 */
int create_offset_variables(Solver csolver) {
    
    Frame *frame_pt;                    // Pointer to a frame of the network
    Offset *offset_pt;                  // Pointer to the frame offset
    int num_frames;                     // Number of frames in the network
    char name[100];                     // Name to identify variables
    long long int distance;
    long long int maximum_time;         // Maximum time allowed to start the transmission of an offset
    long long int minimum_time;         // Minimum time allowed to start the transmission of an offset
    
    num_frames = get_num_frames();
    // For all the frames, loof for all its offsets
    for (int frame_it = 0; frame_it < num_frames; frame_it++) {
        frame_pt = get_frame(frame_it);
        offset_pt = get_offset_root(frame_pt);
        while (!is_last_offset(offset_pt)) {        // For all the frame offsets
            // For all replicas and instances
            for (int instance = 0; instance < get_num_instances(offset_pt); instance++) {
                for (int replica = 0; replica < get_num_replicas(offset_pt); replica++) {
                    sprintf(name, "O_%d_%d_%d_%d", frame_it, instance, replica, get_offset_link(offset_pt));
                    init_variable(offset_pt, instance, replica, name, csolver);
                    
                    // Set the minimum and maximum transmission time for the offset, note that we only do it for the
                    // instance 0, replica 0, as the time between different instances and replicas are related to 0, 0
                    // We need to extract the transmission time of the offset to the deadline to allow it to finish
                    maximum_time = get_deadline(frame_pt) - get_timeslot_size(offset_pt);
                    maximum_time = maximum_time + (get_period(frame_pt) * instance);
                    minimum_time = get_starting(frame_pt);
                    minimum_time = minimum_time + (get_period(frame_pt) * instance);
                    if (set_offset_range(offset_pt, instance, replica, minimum_time, maximum_time, name, csolver) < 0) {
                        printf("Error setting the allowed range for the offset\n");
                        return ERROR_INIT_CONSTRAINTS;
                    }
                    // Set the fixed distances between the instance and replica 0 with the rest
                    if (instance != 0 || replica != 0) {
                        distance = get_period(frame_pt) * instance;
                        if (set_fixed_distance(offset_pt, 0, 0, offset_pt, instance, replica, distance, csolver) < 0) {
                            printf("Error setting the distance between instance and replicas of the same frame\n");
                            return ERROR_INIT_CONSTRAINTS;
                        }
                    }
                }
            }
            offset_pt = get_next_offset(offset_pt);
        }
    }
    return 0;
}

/**
 Add constraints for the solver to choose the paths of frames. For now, we allow only one possible path, chosen by the
 solver
 
 @return 0 if everything was ok, error code otherwise
 */
int choose_path_z3() {
    
    Frame *frame_pt;                            // Frame pointer
    Offset *offset_pt;                          // Pointer to the frame offset
    Path *path_pt;                              // Path pointer
    int num_paths;                              // Number of paths to arrive
    int num_receivers;                          // Number of receivers of the frame
    int receiver;                             //  reciever of the frame
    int sender;                                 // Sender of the frame
    Z3_sort z3_integer;
    Z3_ast *z3_add_args = NULL;
    Z3_ast *z3_or_args = NULL;
    Z3_ast z3_formula, z3_1, z3_0, z3_offset_eq0, z3_offset, z3_path_selectors_add;
    Z3_ast z3_path_selectros_larger_1, z3_offset_larger_1;
    int num_add_args, num_or_args;
    
    // For all given frames, check how many paths to we have, for each path allocate memory in the array and create
    // the constraints
    z3_integer = Z3_mk_int_sort(z3_context);
    z3_1 = Z3_mk_int(z3_context, 1, z3_integer);
    z3_0 = Z3_mk_int(z3_context, 0, z3_integer);
    for (int frame_it = 0; frame_it < get_num_frames(); frame_it++) {
        frame_pt = get_frame(frame_it);
        offset_pt = get_offset_root(frame_pt);
        sender = get_sender_id(frame_pt);
        num_receivers = get_num_receivers(frame_pt);
        while (!is_last_offset(offset_pt)) {        // For all the frame offsets
            // Now, for every path, check if the offset appears to add the initalization
            num_or_args = 0;
            z3_or_args = NULL;
            for (int receiver_it = 0; receiver_it < num_receivers; receiver_it++) {
                receiver = get_receiver_id(frame_pt, receiver_it);
                num_paths = get_num_paths(sender, receiver);
                num_add_args = 0;
                z3_add_args = NULL;
                for (int path_it = 0; path_it < num_paths; path_it++) {
                    path_pt = get_path(sender, receiver, path_it);
                    for (int link_it = 0; link_it < path_pt->length; link_it++) {
                        // If the offset is in the path, we have to add the constraint
                        if (path_pt->path[link_it] == get_offset_link(offset_pt)) {
                            num_add_args++;
                            z3_add_args = realloc(z3_add_args, sizeof(Z3_ast) * num_add_args);
                            z3_add_args[num_add_args - 1] = path_selector[frame_it][receiver_it][path_it];
                        }
                    }
                }
                if (num_add_args != 0) {    // Only if we have at least one offset (it can happen that there
                    // is not as frames can have many receivers, so we can have an offset not appearing in the
                    // current receiver
                    // When all the paths have been investigated, the summation should be multiplied by offset
                    z3_offset = get_z3_offset(offset_pt, 0, 0);
                    if (num_add_args > 1) {     // Only add if there is more than 1 possible path
                        z3_path_selectors_add = Z3_mk_add(z3_context, num_add_args, z3_add_args);
                    } else {
                        z3_path_selectors_add = z3_add_args[0];
                    }
                    z3_path_selectros_larger_1 = Z3_mk_ge(z3_context, z3_path_selectors_add, z3_1);
                    z3_offset_larger_1 = Z3_mk_ge(z3_context, z3_offset, z3_1);
                    z3_offset_eq0 = Z3_mk_eq(z3_context, z3_offset, z3_0);
                    z3_formula = Z3_mk_ite(z3_context, z3_path_selectros_larger_1, z3_offset_larger_1, z3_offset_eq0);
                    // Now we save everything to OR it with all possible receivers, as a link may be used in
                    // others receivers
                    num_or_args++;
                    z3_or_args = realloc(z3_or_args, sizeof(Z3_ast) * num_or_args);
                    z3_or_args[num_or_args - 1] = z3_formula;
                    
                }
                free(z3_add_args);
            }
            // If tere is more than 1 appearance of the link in the receivers, create or, if not ignore
            if (num_or_args != 0) {
                if (num_or_args > 1) {
                    z3_formula = Z3_mk_or(z3_context, num_or_args, z3_or_args);
                } else {
                    z3_formula = z3_or_args[0];
                }
                Z3_optimize_assert(z3_context, z3_optimize, z3_formula);
                free(z3_or_args);
            }
            offset_pt = get_next_offset(offset_pt);
        }
    }
    return 0;
}

/**
 Add constraints for the solver to choose the paths of frames. For now, we allow only one possible path, chosen by the
 solver
 
 @return 0 if everything was ok, error code otherwise
 */
int choose_path_gurobi() {
    
    Frame *frame_pt;                            // Frame pointer
    Offset *offset_pt;                          // Pointer to the frame offset
    Path *path_pt;                              // Path pointer
    int num_paths;                              // Number of paths to arrive
    int num_receivers;                          // Number of receivers of the frame
    int receiver;                             //reciever of the frame
    int sender;
    int num_add_args, num_or_args;
    int *add_path_selectors = NULL;
    int *or_path_selectors = NULL;
    int variables[1];
    double values[1] = {1.0};
    
    for (int frame_it = 0; frame_it < get_num_frames(); frame_it++) {
        frame_pt = get_frame(frame_it);
        offset_pt = get_offset_root(frame_pt);
        sender = get_sender_id(frame_pt);
        num_receivers = get_num_receivers(frame_pt);
        while (!is_last_offset(offset_pt)) {        // For all the frame offsets
            // Now, for every path, check if the offset appears to add the initalization
            num_or_args = 0;
            or_path_selectors = NULL;
            for (int receiver_it = 0; receiver_it < num_receivers; receiver_it++) {
                receiver = get_receiver_id(frame_pt, receiver_it);
                num_paths = get_num_paths(sender, receiver);
                num_add_args = 0;
                add_path_selectors = NULL;
                for (int path_it = 0; path_it < num_paths; path_it++) {
                    path_pt = get_path(sender, receiver, path_it);
                    for (int link_it = 0; link_it < path_pt->length; link_it++) {
                        // If the offset is in the path, we have to add the constraint
                        if (path_pt->path[link_it] == get_offset_link(offset_pt)) {
                            num_add_args++;
                            add_path_selectors = realloc(add_path_selectors, sizeof(int) * num_add_args);
                            add_path_selectors[num_add_args - 1] = gurobi_path_selector[frame_it][receiver_it][path_it];
                        }
                    }
                }
                if (num_add_args != 0) {    // Only if we have at least one offset (it can happen that there
                    // is not as frames can have many receivers, so we can have an offset not appearing in the
                    // current receiver
                    // When all the paths have been invetigated, we make the summation of all its path selectors
                    GRBaddvar(gurobi_model, 0, NULL, NULL, 0, 0, 1, GRB_BINARY, NULL);
                    gurobi_var_counter++;
                    GRBaddgenconstrOr(gurobi_model, NULL, gurobi_var_counter - 1, num_add_args, add_path_selectors);
                    
                    free(add_path_selectors);
                    num_or_args++;
                    
                    or_path_selectors = realloc(or_path_selectors, sizeof(int) * num_or_args);
                    or_path_selectors[num_or_args - 1] = gurobi_var_counter - 1;
                }
            }
            if (num_or_args != 0) {
                if (num_or_args > 1) {
                    GRBaddvar(gurobi_model, 0, NULL, NULL, 0, 0, 1, GRB_BINARY, NULL);
                    gurobi_var_counter++;
                    GRBaddgenconstrOr(gurobi_model, NULL, gurobi_var_counter - 1, num_or_args, or_path_selectors);
                }
                variables[0] = get_gurobi_offset(offset_pt, 0, 0);
                GRBaddgenconstrIndicator(gurobi_model, NULL, gurobi_var_counter - 1, 0, 1, variables, values,
                                         GRB_EQUAL, 0);
                GRBaddgenconstrIndicator(gurobi_model, NULL, gurobi_var_counter - 1, 1, 1, variables, values,
                                         GRB_GREATER_EQUAL, 1);
                free(or_path_selectors);
            }
            offset_pt = get_next_offset(offset_pt);
        }
    }
    return 0;
}

/**
 Add constraints for the solver to choose the paths of frames. For now, we allow only one possible path, chosen by the
 solver
 
 @param csolver indicates which solver are we using
 @return 0 if everything was ok, error code otherwise
 */
int choose_path(Solver csolver) {
    
    switch (csolver) {
        case z3:
            choose_path_z3();
            break;
        case gurobi:
            choose_path_gurobi();
            break;
        default:
            break;
    }
    return 0;
}

/**
 Assures that no frames are allowed to be transmitted at the same time in the same link
 
 @param csolver indicates which solver are we using
 @return 0 if everything was ok, error code otherwise
 */
int contention_free(Solver csolver) {
    
    Offset *offset_pt, *previous_offset_pt;                         // Pointer to an offset structure
    Frame *frame_pt, *previous_frame_pt;                            // Pointer to a frame structure
    int link;
    long long int distance1, distance2;                             // Distances of the intersection
    
    // For all the given frames, check if frames can be in the same link and time, if they do, avoid
    for (int frame_it = 0; frame_it < get_num_frames(); frame_it++) {
        frame_pt = get_frame(frame_it);
        offset_pt = get_offset_root(frame_pt);      // Get the root of the frames
        while (!is_last_offset(offset_pt)) {
            link = get_offset_link(offset_pt);
            // Iterate over all instances and replicas too
            for (int instance = 0; instance < get_num_instances(offset_pt); instance++) {
                for (int replica = 0; replica < get_num_replicas(offset_pt); replica++) {
                    
                    // For all previous frames that have been iterated already, check they do not share anything
                    for (int previous_frame_it = 0; previous_frame_it < frame_it; previous_frame_it++) {
                        previous_frame_pt = get_frame(previous_frame_it);
                        previous_offset_pt = get_frame_offset_by_link(previous_frame_pt, link);
                        if (previous_offset_pt != NULL) {       // If the previous frame has an offset with that link
                            for (int previous_instance = 0; previous_instance < get_num_instances(previous_offset_pt);
                                 previous_instance++) {
                                for (int previous_replica = 0; previous_replica < get_num_replicas(previous_offset_pt);
                                     previous_replica++) {
                                    // See if they both can collide
                                    if (offsets_share_interval(frame_pt, offset_pt, instance, replica,
                                                               previous_frame_pt, previous_offset_pt, previous_instance,
                                                               previous_replica) == 1) {
                                        // Add the constraint to avoid collision
                                        distance1 = get_timeslot_size(offset_pt);
                                        distance2 = get_timeslot_size(previous_offset_pt);
                                        if (avoid_intersection(offset_pt, instance, replica, previous_offset_pt,
                                                               previous_instance, previous_replica, distance1,
                                                               distance2, csolver) < 0) {
                                            printf("Error creating contention free constraints\n");
                                            return ERROR_CONTENTION_FREE_CONSTRAINTS;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            offset_pt = get_next_offset(offset_pt);
        }
    }
    
    return 0;
}

/**
 Assures that all frames follow their path in the correct order
 
 @param csolver indicates which solver are we using
 @return 0 if everything was ok, error code otherwise
 */
int frame_path_dependent(Solver csolver) {
    
    Frame *frame_pt;                            // Frame pointer
    Path *path_pt;                              // Path pointer
    Offset *offset_pt, *next_offset_pt;         // Offset pointers
    int num_paths;                              // Number of paths to arrive
    int num_receivers;                          // Number of receivers of the frame
    int receiver;                             // reciever of the frame
    int sender;                                 // Sender of the frame
    long long int distance;                     // Distance to wait to transmit the next one
    
    // For all given frames
    for (int frame_it = 0; frame_it < get_num_frames(); frame_it++) {
        frame_pt = get_frame(frame_it);
        sender = get_sender_id(frame_pt);
        num_receivers = get_num_receivers(frame_pt);
        for (int receiver_it = 0; receiver_it < num_receivers; receiver_it++) {         // For all the receivers
            receiver = get_receiver_id(frame_pt, receiver_it);
            num_paths = get_num_paths(sender, receiver);
            for (int path_it = 0; path_it < num_paths; path_it++) {                     // For all paths
                path_pt = get_path(sender, receiver, path_it);
                // For all link in the path but the last one, get the offset of the current and next link
                for (int link_it = 0; link_it < (path_pt->length - 1); link_it++) {
                    offset_pt = get_frame_offset_by_link(frame_pt, path_pt->path[link_it]);
                    distance = get_timeslot_size(offset_pt) + get_switch_minimum_time();
                    next_offset_pt = get_frame_offset_by_link(frame_pt, path_pt->path[link_it + 1]);
                    if (set_minimum_distance(offset_pt, 0, 0, next_offset_pt, 0, 0, distance, frame_it, receiver_it,
                                             path_it, csolver) < 0) {
                        printf("Error setting the minimum distance in the path dependent\n");
                        return ERROR_PATH_DEPENDENT_CONSTRAINS;
                    }
                }
            }
        }
    }
    
    return 0;
}

/**
 Assures that all frames follow their end to end delay in all paths from the first transmission to the last
 
 @param cssolver indicates which solver are we using
 @return 0 if everything was ok, error code otherwise
 */
int frame_end_to_end_delay(Solver cssolver) {
    
    Frame *frame_pt;                            // Frame pointer
    Path *path_pt;                              // Path pointer
    int num_paths;                              // Number of paths to arrive
    long long int delay;                        // End to end delay of the frame
    long long int distance;                     // Avaiable distance for the end to end delay
    int num_receivers;                          // Number of receivers of the frame
    int receiver;                             // reciever of the frame
    int sender;                                 // Sender of the frame
    int first_link, last_link;                  // First and last link of a path
    Offset *first_offset_pt, *last_offset_pt;   // Offset pointer to the first and last offsets of a possible path
    
    // For all the frames, add the end to end delay for the path from the first link to the last
    for (int frame_it = 0; frame_it < get_num_frames(); frame_it++) {
        frame_pt = get_frame(frame_it);
        delay = get_end_to_end_delay(frame_pt);
        sender = get_sender_id(frame_pt);
        num_receivers = get_num_receivers(frame_pt);
        for (int receiver_it = 0; receiver_it < num_receivers; receiver_it++) {         // For all the receivers
            receiver = get_receiver_id(frame_pt, receiver_it);
            num_paths = get_num_paths(sender, receiver);
            for (int path_it = 0; path_it < num_paths; path_it++) {                     // For all paths
                path_pt = get_path(sender, receiver, path_it);
                first_link = path_pt->path[0];
                last_link = path_pt->path[path_pt->length - 1];
                first_offset_pt = get_frame_offset_by_link(frame_pt, first_link);
                last_offset_pt = get_frame_offset_by_link(frame_pt, last_link);
                distance = delay - get_timeslot_size(last_offset_pt);
                if (set_maximum_distance(first_offset_pt, 0, 0, last_offset_pt, 0, 0, distance, frame_it, receiver_it,
                                         path_it, cssolver) < 0) {
                    printf("Error setting the max distance for the frame end to end delay\n");
                    return ERROR_END_TO_END_DELAY_CONSTRAINTS;
                }
            }
        }
    }
    
    return 0;
}

/**
 Check the constraint solver and returns the status of it, if everything went well, it creates the schedule model
 
 @param csolver indicates which solver are we using
 @param time limit time in seconds to solve the schedule
 @param tune if tune is active, instead of solving the schedule, it will tune and find good parameters
 @param tunetimelimit limit in seconds to tune
 @return 1 if the schedule was found, error code otherwise
 */
int check_solver(Solver csolver, int time, int tune, int tunetimelimit) {
    
    switch (csolver) {
        case z3:
            printf("%s", Z3_optimize_to_string(z3_context, z3_optimize));
            if (Z3_optimize_check(z3_context, z3_optimize) == Z3_L_TRUE) {
                z3_model = Z3_optimize_get_model(z3_context, z3_optimize);
                // To delete
                Z3_string model_string;
                model_string = Z3_model_to_string(z3_context, z3_model);
                printf("%s", model_string);
                
                Z3_del_context(z3_context);
            }
            break;
        case gurobi:
            GRBupdatemodel(gurobi_model);
            if (tune == 1) {
                GRBsetdblparam(GRBgetenv(gurobi_model), "TuneTimeLimit", tunetimelimit);
                GRBtunemodel(gurobi_model);
                int nresults;
                GRBgetintattr(gurobi_model, "TuneResultCount", &nresults);
                if (nresults > 0) {
                    GRBgettuneresult(gurobi_model, 0);
                    GRBwrite(gurobi_model, "Params.prm");
                    
                }
            } else {
                GRBreadparams(gurobi_env, "XML Files/Params.prm");
                GRBsetdblparam(GRBgetenv(gurobi_model), "TimeLimit", time);
                GRBwrite(gurobi_model, "Model.lp");
                GRBoptimize(gurobi_model);
                int sol_count = 0;
                GRBgetintattr(gurobi_model, "SolCount", &sol_count);
                if (sol_count > 0) {
                    GRBwrite(gurobi_model, "Debug.mps");
                    GRBwrite(gurobi_model, "Schedule.sol");
                }
            }
        default:
            break;
    }
    
    return 0;
}
