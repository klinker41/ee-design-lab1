/*
 * Copyright (C) 2015 Jacob Klinker
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package edu.uiowa.eedesign.lab1

import groovyx.net.http.HTTPBuilder
import static groovyx.net.http.Method.POST
import static groovyx.net.http.ContentType.TEXT

public class Client extends TimerTask {

    private float currentTime = 0

    public static void main(String[] args) {
        // execute the run method every 1 second
        Timer timer = new Timer()
        timer.schedule(new Client(), 0, 1000)
    }

    @Override
    void run() {
        currentTime++
        float temp = 20 * Math.sin(0.2094f * currentTime) + 30

        def http = new HTTPBuilder("http://default-environment-serpnbmp6z.elasticbeanstalk.com/temperature/add?temp=${temp}")

        http.request(POST, TEXT) { req ->
            response.success = { resp, reader ->
                println("Posted temp: ${temp} with response: ${reader.text}")
            }

            // called only for a 404 (not found) status code:
            response.'404' = { resp ->
                println 'Not found'
            }
        }
    }
}