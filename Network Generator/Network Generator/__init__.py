# -*- coding: utf-8 -*-

"""* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Network Generator                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 08/02/18.                                                                             *
 *  Copyright © 2018 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Package to create Deterministic Ethernet Networks                                                                  *
 *  This package is able to create networks using NetworkX package and fill it with time-triggered traffic.            *
 *  There are different ways to create different networks, but a method will exist to create any network.              *
 *  The traffic will be send from a sender to one or multiple receivers, and it can be specified a path in the network *
 *  or leave it open for others to implement. The traffic will contain a period, a deadline, an end to end delay and a *
 *  size.                                                                                                              *
 *  There is also the possibility to create dependencies between frames in terms of minimum and maximum time a frame   *
 *  has to wait another one to be delivered.                                                                           *
 *  The package will have an input and output interface with an specific XML file to feed with parameters and output  *
 *  another XML file with the constructed network information.                                                         *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * """