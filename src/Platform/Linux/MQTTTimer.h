/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#if !defined(__MQTT_TIMER_h)
#define __MQTT_TIMER_h

#include <sys/time.h>
#include <stdio.h>

class MQTTTimer
{
public:
	MQTTTimer()
    { 
	
    }

	MQTTTimer(int ms)
    { 
		countdown_ms(ms);
    }
    

    bool expired()
    {
		struct timeval now, res;
		gettimeofday(&now, NULL);
		timersub(&end_time, &now, &res);		
        return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
    }
    

    void countdown_ms(int ms)  
    {
		struct timeval now;
		gettimeofday(&now, NULL);
		struct timeval interval = {ms / 1000, (ms % 1000) * 1000};
		timeradd(&now, &interval, &end_time);
    }

    
    void countdown(int seconds)
    {
		struct timeval now;
		gettimeofday(&now, NULL);
		struct timeval interval = {seconds, 0};
		timeradd(&now, &interval, &end_time);
    }

    
    int left_ms()
    {
		struct timeval now, res;
		gettimeofday(&now, NULL);
		timersub(&end_time, &now, &res);
        return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
    }
    
private:

	struct timeval end_time;
};


#endif