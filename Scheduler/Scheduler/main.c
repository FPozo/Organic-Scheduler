/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Main.c                                                                                                             *
 *  Organic Scheduler                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 18/02/18.                                                                             *
 *  Copyright © 2018 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include "Scheduler.h"

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    one_shot_scheduling("XML Files/Network.xml", "XML Files/Schedule.xml", "XML Files/ScheduleConfiguration.xml");
    printf("Maximum link utlization: %f\n", get_max_link_utilization());
    return 0;
}
